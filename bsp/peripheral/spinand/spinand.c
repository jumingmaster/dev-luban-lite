/*
 * Copyright (c) 2024-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: xuan.wen <xuan.wen@artinchip.com>
 */

#include <rtconfig.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include "../../../bsp/artinchip/include/hal/spienc.h"
#include <spinand.h>
#include <manufacturer.h>
#include <bbt.h>

static const struct spinand_manufacturer *spinand_manufacturers[] = {
#ifdef SPI_NAND_WINBOND
    &winbond_spinand_manufacturer,
#endif
#ifdef SPI_NAND_XTX
    &xtx_spinand_manufacturer,
#endif
#ifdef SPI_NAND_GIGADEVICE
    &gigadevice_spinand_manufacturer,
#endif
#ifdef SPI_NAND_FORESEE
    &foresee_spinand_manufacturer,
#endif
#ifdef SPI_NAND_TOSHIBA
    &toshiba_spinand_manufacturer,
#endif
#ifdef SPI_NAND_MACRONIX
    &macronix_spinand_manufacturer,
#endif
#ifdef SPI_NAND_ZETTA
    &zetta_spinand_manufacturer,
#endif
#ifdef SPI_NAND_DOSILICON
    &dosilicon_spinand_manufacturer,
#endif
#ifdef SPI_NAND_ETRON
    &etron_spinand_manufacturer,
#endif
#ifdef SPI_NAND_MICRON
    &micron_spinand_manufacturer,
#endif
#ifdef SPI_NAND_ZBIT
    &zbit_spinand_manufacturer,
#endif
#ifdef SPI_NAND_ESMT
    &esmt_spinand_manufacturer,
#endif
#ifdef SPI_NAND_UMTEK
    &umtek_spinand_manufacturer,
#endif
#ifdef SPI_NAND_QUANXING
    &quanxing_spinand_manufacturer,
#endif
#ifdef SPI_NAND_XINCUN
    &xincun_spinand_manufacturer,
#endif
#ifdef SPI_NAND_FUDANMICRO
    &fudanmicro_spinand_manufacturer,
#endif
};

#define SPINAND_LIST_NUM \
    (sizeof(spinand_manufacturers) / sizeof(struct spinand_manufacturer *))

/**
 * fls - find last (most-significant) bit set
 * @x: the word to search
 *
 * This is defined the same way as ffs.
 * Note fls(0) = 0, fls(1) = 1, fls(0x80000000) = 32.
 */
static inline int generic_fls(int x)
{
    int r = 32;

    if (!x)
        return 0;
    if (!(x & 0xffff0000u)) {
        x <<= 16;
        r -= 16;
    }
    if (!(x & 0xff000000u)) {
        x <<= 8;
        r -= 8;
    }
    if (!(x & 0xf0000000u)) {
        x <<= 4;
        r -= 4;
    }
    if (!(x & 0xc0000000u)) {
        x <<= 2;
        r -= 2;
    }
    if (!(x & 0x80000000u)) {
        x <<= 1;
        r -= 1;
    }
    return r;
}

static void spinand_cache_op_adjust_colum(struct aic_spinand *flash, u16 blk,
                                          u16 *column)
{
    u32 shift = 0;
    u16 plane = 0;

    if (flash->info->planes_per_lun < 2)
        return;

    /* The plane number is passed in MSB just above the column address */
    shift = generic_fls(flash->info->page_size);
    plane = blk % flash->info->planes_per_lun;
    *column |= plane << shift;
}

struct spi_nand_cmd_cfg cmd_cfg_table[] = {
    /*opcode    opcode_bits addr_bytes	addr_bits	dummy_bytes	data_nbits*/
    { SPINAND_CMD_READ_FROM_CACHE, 1, 2, 1, 1, 1 },
    { SPINAND_CMD_READ_FROM_CACHE_X2, 1, 2, 1, 1, 2 },
    { SPINAND_CMD_READ_FROM_CACHE_X4, 1, 2, 1, 1, 4 },
    { SPINAND_CMD_PROG_LOAD, 1, 2, 1, 0, 1 },
    { SPINAND_CMD_PROG_LOAD_X4, 1, 2, 1, 0, 4 },
    { SPINAND_CMD_END },
};

static struct spi_nand_cmd_cfg *
spi_nand_lookup_cmd_cfg_table(u8 opcode, struct spi_nand_cmd_cfg *table)
{
    struct spi_nand_cmd_cfg *index = table;

    for (; index->opcode != SPINAND_CMD_END; index++) {
        if (index->opcode == opcode)
            return index;
    }

    pr_err("Invalid spi nand opcode %x\n", opcode);
    return NULL;
}

int get_command_cpos_wr(struct aic_spinand *flash)
{
    struct spi_nand_cmd_cfg *cfg;

    if (flash->qspi_dl_width == 4)
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_PROG_LOAD_X4,
                                            flash->info->cmd);
    else
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_PROG_LOAD,
                                            flash->info->cmd);

    if (!cfg)
        return -SPINAND_ERR;

    return 1 + cfg->addr_bytes + cfg->dummy_bytes;
}

int get_command_cpos_rd(struct aic_spinand *flash)
{
    struct spi_nand_cmd_cfg *cfg;

    if (flash->qspi_dl_width == 4)
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_READ_FROM_CACHE_X4,
                                            flash->info->cmd);
    else if (flash->qspi_dl_width == 2)
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_READ_FROM_CACHE_X2,
                                            flash->info->cmd);
    else
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_READ_FROM_CACHE,
                                            flash->info->cmd);

    if (!cfg)
        return -SPINAND_ERR;

    return 1 + cfg->addr_bytes + cfg->dummy_bytes;
}

static int spinand_read_reg_op(struct aic_spinand *flash, u8 reg, u8 *val)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_GET_FEATURE_CFG;

    return aic_spinand_transfer_message(flash, &cfg, (u32)reg, NULL, val, 1);
}

int spinand_die_select(struct aic_spinand *flash, u8 select_die)
{
    return 0;
}

static int spinand_read_status(struct aic_spinand *flash, u8 *status)
{
    return spinand_read_reg_op(flash, REG_STATUS, status);
}

static int spinand_read_cfg(struct aic_spinand *flash, u8 *cfg)
{
    return spinand_read_reg_op(flash, REG_CFG, cfg);
}

int spinand_check_ecc_status(struct aic_spinand *flash, u8 status)
{
    if (flash->info->get_status) {
        return flash->info->get_status(flash, status);
    }

    switch (status & STATUS_ECC_MASK) {
        case STATUS_ECC_NO_BITFLIPS:
            return 0;
        case STATUS_ECC_HAS_1_4_BITFLIPS:
            return 4;
        case STATUS_ECC_UNCOR_ERROR:
            return -SPINAND_ERR_ECC;

        default:
            break;
    }

    return -SPINAND_ERR;
}

int spinand_isbusy(struct aic_spinand *flash, u8 *status)
{
    u32 i = 0, cnt = 30000;
    u8 SR = 0xFF;
    int result;

    do {
        aic_udelay(1);
        i++;
        result = spinand_read_status(flash, &SR);
        if (result != SPINAND_SUCCESS)
            return result;

        if (i > cnt) {
            pr_warn("spinand timeout.\n");
            goto spinand_isbusy_out;
        }

    } while ((SR & 0x1) != 0x00);

    result = spinand_read_status(flash, &SR);
    if (result != SPINAND_SUCCESS)
        return result;

spinand_isbusy_out:
    if (status)
        *status = SR;

    return SR & 0x1 ? -SPINAND_ERR_TIMEOUT : 0;
}

int spinand_write_to_cache_op(struct aic_spinand *flash, u16 column, u8 *buf,
                              u32 count)
{
    struct spi_nand_cmd_cfg *cfg;

    if (flash->qspi_dl_width == 4)
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_PROG_LOAD_X4,
                                            flash->info->cmd);
    else
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_PROG_LOAD,
                                            flash->info->cmd);

    return aic_spinand_transfer_message(flash, cfg, column, buf, NULL, count);
}

static int spinand_write_reg_op(struct aic_spinand *flash, u8 reg, u8 val)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_SET_FEATURE_CFG;

    return aic_spinand_transfer_message(flash, &cfg, (u32)reg, &val, NULL, 1);
}

static int spinand_set_cfg(struct aic_spinand *flash, u8 cfg)
{
    return spinand_write_reg_op(flash, REG_CFG, cfg);
}

int spinand_write_enable_op(struct aic_spinand *flash)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_WR_ENABLE_CFG;

    return aic_spinand_transfer_message(flash, &cfg, 0, NULL, NULL, 0);
}

u8 spinand_write_disable_op(struct aic_spinand *flash)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_WR_DISABLE_CFG;

    return aic_spinand_transfer_message(flash, &cfg, 0, NULL, NULL, 0);
}

int spinand_program_op(struct aic_spinand *flash, u32 page)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_PROG_EXC_CFG;

    return aic_spinand_transfer_message(flash, &cfg, page, NULL, NULL, 0);
}

static int spinand_lock_block(struct aic_spinand *flash, u8 lock)
{
    /* write status register 1 */
    return spinand_write_reg_op(flash, REG_BLOCK_LOCK, lock);
}

static int spinand_hwecc_set(struct aic_spinand *flash, u8 enable)
{
    u8 status = 0;
    int result;

    result = spinand_read_cfg(flash, &status);
    if (result != SPINAND_SUCCESS)
        return result;

    if (enable)
        status |= 0x10; // Enable ECC-E bit
    else
        status &= 0xEF; // Disable ECC-E bit

    return spinand_set_cfg(flash, status);
}

int spinand_load_page_op(struct aic_spinand *flash, u32 page)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_PAGE_READ_CFG;

    return aic_spinand_transfer_message(flash, &cfg, page, NULL, NULL, 0);
}

int spinand_config_set(struct aic_spinand *flash, u8 mask, u8 val)
{
    u8 status = 0;
    int result;

    result = spinand_read_cfg(flash, &status);
    if (result != SPINAND_SUCCESS)
        return result;

    status &= ~mask;
    status |= val;

    return spinand_set_cfg(flash, status);
}

static bool spinand_support_regs(u8 reg) {
    if (reg == REG_BLOCK_LOCK || reg == REG_CFG || reg == REG_STATUS)
        return true;

    return false;
}

int spinand_get_feature(struct aic_spinand *flash, u8 reg_addr)
{
    u8 status = 0;
    int result;

    if (!spinand_support_regs(reg_addr))
        return -SPINAND_ERR;

    result = spinand_read_reg_op(flash, reg_addr, &status);
    if (result != SPINAND_SUCCESS)
        return result;

    return status;
}

int spinand_erase_op(struct aic_spinand *flash, u32 page)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_BLK_ERASE_CFG;

    return aic_spinand_transfer_message(flash, &cfg, page, NULL, NULL, 0);
}

int spinand_block_erase(struct aic_spinand *flash, u16 blk)
{
    int result;
    u8 status;
    u32 page = blk * flash->info->pages_per_eraseblock;

    result = spinand_write_enable_op(flash);
    if (result != SPINAND_SUCCESS)
        return result;

    result = spinand_erase_op(flash, page);
    if (result != SPINAND_SUCCESS)
        return result;

    if (spinand_isbusy(flash, &status))
        return -SPINAND_ERR;

    if (status & STATUS_ERASE_FAILED) {
        /* Fail to erase */
        result = -SPINAND_ERR;
    }

    return result;
}

/*
 * Reading data from SPINAND buf to local
 * column: in scope 0~2048+64
 */
int spinand_read_from_cache_op(struct aic_spinand *flash, u16 column, u8 *buf,
                               u32 count)
{
    struct spi_nand_cmd_cfg *cfg;

    if (flash->qspi_dl_width == 4)
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_READ_FROM_CACHE_X4,
                                            flash->info->cmd);
    else if (flash->qspi_dl_width == 2)
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_READ_FROM_CACHE_X2,
                                            flash->info->cmd);
    else
        cfg = spi_nand_lookup_cmd_cfg_table(SPINAND_CMD_READ_FROM_CACHE,
                                            flash->info->cmd);

    return aic_spinand_transfer_message(flash, cfg, column, NULL, buf, count);
}

#ifdef AIC_SPINAND_CONT_READ
int spinand_read_from_cache_cont_op(struct aic_spinand *flash, u8 *buf,
                                    u32 count)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_READ_FROM_CACHE_X4_CONT_CFG;

    return aic_spinand_transfer_message(flash, &cfg, 0, NULL, buf, count);
}
#endif

int spinand_read_id_op(struct aic_spinand *flash, u8 *id)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_READ_ID_CFG;
    int result = -1;

    result = aic_spinand_transfer_message(flash, &cfg, 0, NULL, id,
                                          SPINAND_MAX_ID_LEN);
    return result;
}

int spinand_reset_op(struct aic_spinand *flash)
{
    struct spi_nand_cmd_cfg cfg = SPINAND_CMD_RESET_CFG;

    return aic_spinand_transfer_message(flash, &cfg, 0, NULL, NULL, 0);
}

static int spinand_reset(struct aic_spinand *flash)
{
    int result;

    result = spinand_reset_op(flash);
    if (result != SPINAND_SUCCESS)
        return result;

    return spinand_isbusy(flash, NULL);
}

const struct aic_spinand_info *
spinand_match_and_init(u8 *devid, const struct aic_spinand_info *table,
                       u32 table_size)
{
    u32 i;

    for (i = 0; i < table_size; i++) {
        const struct aic_spinand_info *info = &table[i];
        if (memcmp(devid, info->devid.id, info->devid.len))
            continue;
        return &table[i];
    }
    return NULL;
}

static int spinand_info_read(struct aic_spinand *flash)
{
    int i;

    spinand_read_id_op(flash, flash->id.data);

    for (i = 0; i < SPINAND_LIST_NUM; i++) {
        if ((flash->id.data[0] & 0xFF) == spinand_manufacturers[i]->id) {
            flash->info = spinand_manufacturers[i]->ops->detect(flash);
            if (!flash->info)
                goto exit_spinand_info_read;

            if (spinand_manufacturers[i]->ops->init)
                spinand_manufacturers[i]->ops->init(flash);

            pr_info("find raw ID %02x%02x%02x%02x\n", flash->id.data[0],
                    flash->id.data[1], flash->id.data[2], flash->id.data[3]);
            return SPINAND_SUCCESS;
        }
    }

exit_spinand_info_read:
    pr_err("unknown raw ID %02x%02x%02x%02x\n", flash->id.data[0],
           flash->id.data[1], flash->id.data[2], flash->id.data[3]);
    return -SPINAND_ERR;
}

int spinand_flash_init(struct aic_spinand *flash)
{
    int result;

    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    if ((result = spinand_reset(flash)) != SPINAND_SUCCESS)
        goto exit_spinand_init;

    if ((result = spinand_info_read(flash)) != SPINAND_SUCCESS)
        goto exit_spinand_init;

    /* Enable quad mode */
    if ((result = spinand_config_set(flash, CFG_QUAD_ENABLE,
                                     CFG_QUAD_ENABLE)) != SPINAND_SUCCESS)
        goto exit_spinand_init;

    /* Un-protect */
    if ((result = spinand_lock_block(flash, 0)) != SPINAND_SUCCESS)
        goto exit_spinand_init;

    /* Enable HWECC */
    if ((result = spinand_hwecc_set(flash, 1)) != SPINAND_SUCCESS)
        goto exit_spinand_init;

    if (flash->info->is_die_select == 1) {
        /* Select die. */
        if ((result = spinand_die_select(flash, SPINAND_DIE_ID1)) !=
            SPINAND_SUCCESS)
            goto exit_spinand_init;

        /* Unprotect */
        if ((result = spinand_lock_block(flash, 0)) != SPINAND_SUCCESS)
            goto exit_spinand_init;
    }

    pr_info("Enabled BUF, HWECC. Unprotected.\n");
#if defined(AIC_SPIENC_DRV)
    /* Enable SPIENC */
    printf("init %d spienc...\n", flash->bus);
    if ((result = spienc_init()) != 0) {
        pr_err("spienc init failed.\n");
        goto exit_spinand_init;
    }
#endif

    flash->databuf = aicos_malloc_align(
        0, flash->info->page_size + flash->info->oob_size, CACHE_LINE_SIZE);
    if (!flash->databuf) {
        pr_err("malloc buf failed\n");
        return -1;
    }

    flash->oobbuf = flash->databuf + flash->info->page_size;

    if ((result = nand_bbt_init(flash)) != SPINAND_SUCCESS)
        pr_err("nand_bbt_init failed\n");

    return result;

exit_spinand_init:
    pr_warn("SPINAND flash init fail.\n");
    return result;
}

int spinand_check_if_do_memcpy(struct aic_spinand *flash, u8 *data,
                               u32 data_len, u8 *spare, u32 spare_len)
{
    int flag = SPINAND_TRUE;

    if (data == NULL) {
        return flag;
    }
    /*
     * When the user address is aligned to CACHE_LINE_SIZE,
     * if the user address and oob address are consecutive or the oob address is empty,
     * then directly write into user cache to reduce copying.
     */
    if (((unsigned long) data & (CACHE_LINE_SIZE - 1)) == 0) {
        if ((spare == NULL) || (spare_len == 0)) {
            flag = SPINAND_FALSE;
        } else if (spare == (data + flash->info->page_size)) {
            flag = SPINAND_FALSE;
        }
    }

    return flag;
}

int spinand_read_page(struct aic_spinand *flash, u32 page, u8 *data,
                      u32 data_len, u8 *spare, u32 spare_len)
{
    int result = SPINAND_SUCCESS;
    u32 cpos __attribute__((unused));
    u8 *buf = NULL;
    u32 nbytes = 0;
    u16 blk = 0;
    u16 column = 0;
    u8 status;
    u8 spare_only = SPINAND_FALSE;
    u8 data_copy_mode = SPINAND_TRUE;

    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    pr_debug("[R-%d]data len: %d, spare len: %d\n", page, data_len, spare_len);

    /* Data load, Read data from flash to cache */
    result = spinand_load_page_op(flash, page);
    if (result != SPINAND_SUCCESS)
        goto exit_spinand_read_page;

    if (spinand_isbusy(flash, &status)) {
        result = -SPINAND_ERR;
        goto exit_spinand_read_page;
    }

    result = spinand_check_ecc_status(flash, status);
    if (result < 0) {
        pr_err("Error ECC status error[0x%x], page: %u.\n", status, page);
        goto exit_spinand_read_page;
    } else if (result > 0) {
        pr_debug("with %d bit/page ECC corrections, status : [0x%x].\n", result,
                 status);
    }

    if (data && data_len) {
        buf = flash->databuf;
        nbytes = flash->info->page_size;

        if (!spinand_check_if_do_memcpy(flash, data, data_len,
                                        spare, spare_len)) {
            buf = data;
            data_copy_mode = SPINAND_FALSE;
        }
    }

    if (spare && spare_len) {
        /* if use extra buf, read oob data size used spare_len */
        if (data_copy_mode == SPINAND_FALSE)
            nbytes += spare_len;
        else
            nbytes += flash->info->oob_size;

        if (!buf) {
            buf = flash->oobbuf;
            spare_only = SPINAND_TRUE;
            column = flash->info->page_size;
        }
    }

    /* cpos = cmd.nbyte + addr.nbyte + dummy.nbyte */
    cpos = get_command_cpos_rd(flash);
    if (cpos < 0) {
        cpos = 1 + 2 + 1;
    }

#if defined(AIC_SPIENC_DRV)
    if (data && data_len) {
        spienc_set_cfg(flash->bus, page * flash->info->page_size, cpos, data_len);
    } else {
        spienc_set_cfg(flash->bus, page * flash->info->page_size, cpos, 0);
    }
    spienc_start();
#endif
    blk = page / flash->info->pages_per_eraseblock;
    spinand_cache_op_adjust_colum(flash, blk, &column);

    result = spinand_read_from_cache_op(flash, column, buf, nbytes);
#if defined(AIC_SPIENC_DRV)
    spienc_stop();
    if (spienc_check_empty())
        memset(buf, 0xFF, data_len);
#endif
    if (result != SPINAND_SUCCESS)
        goto exit_spinand_read_page;

    if (data && data_len) {
        /* Read data: 0~data_len, Read cache to data */
        if (data_copy_mode) {
            memcpy(data, flash->databuf, data_len);
        }
    }

    if (spare && spare_len) {
        if (data_copy_mode) {
            memcpy(spare, flash->oobbuf, spare_len);
        }

        if (spare_only) {
            memcpy(spare, flash->oobbuf, spare_len);
        }
    }

exit_spinand_read_page:
    return result;
}

bool spinand_isbad(struct aic_spinand *flash, u16 blk)
{
    int result;
    u32 page;
    u8 marker = 0;

    page = blk * flash->info->pages_per_eraseblock;

    result = spinand_read_page(flash, page, NULL, 0, &marker, 1);
    if (result != SPINAND_SUCCESS)
        return result;

    if (marker != 0xFF) {
        pr_info("bad block page = %d, marker = 0x%x.\n", page, marker);
        return true;
    }

    return false;
}

int spinand_block_isbad(struct aic_spinand *flash, u16 blk)
{
    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    return spinand_isbad(flash, blk);
}

int spinand_get_status(struct aic_spinand *flash, u16 blk)
{
    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    return nand_bbt_get_block_status(flash, blk);
}

int spinand_set_status(struct aic_spinand *flash, u16 blk, u16 status)
{
    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    nand_bbt_set_block_status(flash, blk, status);
    return 0;
}

#ifdef AIC_SPINAND_CONT_READ
int spinand_continuous_read(struct aic_spinand *flash, u32 page, u8 *data,
                            u32 size)
{
    int result = 0;
    u8 status = 0;
    u32 cpage = page, pagealign;
    u16 blk;

    if (size <= flash->info->page_size) {
        pr_err("[Error] continuous read size:%d less then page size:%d\n", size,
               flash->info->page_size);
        return -SPINAND_ERR;
    }

    while ((cpage - page) * flash->info->page_size < size) {
        /*cpage not align with flash->info->pages_per_eraseblock*/
        if (cpage % flash->info->pages_per_eraseblock) {
            pagealign = cpage / flash->info->pages_per_eraseblock *
                        flash->info->pages_per_eraseblock;
        } else {
            pagealign = cpage;
        }

        blk = pagealign / flash->info->pages_per_eraseblock;
        if (spinand_block_isbad(flash, blk) != 0) {
            pr_warn("Read block %d is bad. exit continuous mode\n",
                    page / flash->info->pages_per_eraseblock);
            return -SPINAND_ERR;
        }

        if (cpage % flash->info->pages_per_eraseblock) {
            cpage = pagealign + flash->info->pages_per_eraseblock;
        } else {
            cpage += flash->info->pages_per_eraseblock;
        }
    }

    if (((unsigned long)data) & (AIC_DMA_ALIGN_SIZE - 1)) {
        pr_err("ddr data adde: 0x%lx is not align with AIC_DMA_ALIGN_SIZE\n",
               (unsigned long)data);
        return -SPINAND_ERR;
    }

    size = (((size) + (32)) & ~(32));

    result = spinand_config_set(flash, CFG_BUF_ENABLE, 0);
    if (!result) {
        flash->use_continuous_read = true;
    }

    /* Data load, Read data from flash to cache */
    result = spinand_load_page_op(flash, page);
    if (result != 0)
        goto exit_spinand_continuous_read;

    if (spinand_isbusy(flash, &status)) {
        result = -SPINAND_ERR;
        pr_err("spinand_isbusy timeout.\n");
        goto exit_spinand_continuous_read;
    }

    result = spinand_check_ecc_status(flash, status);
    if (result < 0) {
        result = -SPINAND_ERR_ECC;
        pr_err("Error ECC status error[0x%x].\n", status);
        goto exit_spinand_continuous_read;
    } else if (result > 0) {
        pr_debug("with %d bit/page ECC corrections, status : [0x%x].\n", result,
                 status);
    }

    result = spinand_read_from_cache_cont_op(flash, data, size);
    if (result != 0)
        goto exit_spinand_continuous_read;

exit_spinand_continuous_read:

    result = spinand_config_set(flash, CFG_BUF_ENABLE, CFG_BUF_ENABLE);
    if (!result) {
        flash->use_continuous_read = false;
    }

    return result;
}
#endif

int spinand_write_page(struct aic_spinand *flash, u32 page, const u8 *data,
                       u32 data_len, const u8 *spare, u32 spare_len)
{
    int result = SPINAND_SUCCESS;
    u32 cpos __attribute__((unused));
    u8 *buf = NULL;
    u32 nbytes = 0;
    u16 column = 0;
    u16 blk = 0;
    u8 status;

    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    pr_debug("[W-%d]data len: %d, spare len: %d\n", page, data_len, spare_len);

    if (flash->info->is_die_select == 1) {
        /* Select die. */
        if ((result = spinand_die_select(flash, SPINAND_DIE_ID0)) !=
            SPINAND_SUCCESS)
            goto exit_spinand_write_page;
    }

    if (data && data_len) {
        if (!spinand_check_if_do_memcpy(flash, (u8 *)data, data_len,
                                        (u8 *)spare, spare_len)) {
            buf = (u8 *)data;
        } else {
            memset(flash->databuf, 0xFF,
                   flash->info->page_size + flash->info->oob_size);
            memcpy(flash->databuf, data, data_len);
            buf = flash->databuf;
        }

        nbytes = flash->info->page_size;
    }

    if (spare && spare_len) {
        /* if use extra buf, write oob data size used spare_len */
        if (!spinand_check_if_do_memcpy(flash, (u8 *)data, data_len,
                                        (u8 *)spare, spare_len)) {
            nbytes += spare_len;
        } else {
            nbytes += flash->info->oob_size;
            memcpy(flash->oobbuf, spare, spare_len);
        }

        if (!buf) {
            memcpy(flash->oobbuf, spare, spare_len);
            buf = flash->oobbuf;
            column = flash->info->page_size;
        }
    }

    result = spinand_write_enable_op(flash);
    if (result != SPINAND_SUCCESS)
        goto exit_spinand_write_page;

    /* cpos = cmd.nbyte + addr.nbyte + dummy.nbyte */
    cpos = get_command_cpos_wr(flash);
    if (cpos < 0) {
        cpos = 1 + 2;
    }

#if defined(AIC_SPIENC_DRV)
    if (data && data_len) {
        spienc_set_cfg(flash->bus, page * flash->info->page_size, cpos, data_len);
    } else {
        spienc_set_cfg(flash->bus, page * flash->info->page_size, cpos, 0);
    }
    spienc_start();
#endif
    blk = page / flash->info->pages_per_eraseblock;
    spinand_cache_op_adjust_colum(flash, blk, &column);

    result = spinand_write_to_cache_op(flash, column, (u8 *)buf, nbytes);
    if (result != SPINAND_SUCCESS)
        goto exit_spinand_write_page;
#if defined(AIC_SPIENC_DRV)
    spienc_stop();
#endif

    /* Flush data in cache to flash */
    result = spinand_program_op(flash, page);
    if (result != SPINAND_SUCCESS)
        goto exit_spinand_write_page;

    result = spinand_isbusy(flash, &status);
    if (result != SPINAND_SUCCESS) {
        result = -SPINAND_ERR;
        goto exit_spinand_write_page;
    }

    if ((status & STATUS_PROG_FAILED)) {
        result = -SPINAND_ERR;
        pr_err("Error write status!\n");
        goto exit_spinand_write_page;
    }

    result = spinand_check_ecc_status(flash, status);
    if (result < 0) {
        pr_err("Error ECC status error[0x%x].\n", status);
        goto exit_spinand_write_page;
    } else if (result > 0) {
        pr_debug("with %d bit/page ECC corrections, status : [0x%x].\n", result,
                 status);
    }

    result = SPINAND_SUCCESS;

exit_spinand_write_page:

    return result;
}

int spinand_block_markbad(struct aic_spinand *flash, u16 blk)
{
    u8 badblockmarker = 0xF0;
    int result = SPINAND_SUCCESS;
    u32 page;

    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    result = spinand_block_isbad(flash, blk);
    if (result)
        return 0;

    page = blk * flash->info->pages_per_eraseblock;

    spinand_block_erase(flash, blk);

    return spinand_write_page(flash, page, NULL, 0, &badblockmarker, 1);
}

#define READ_DATE_OFFSET_ALIGN   1
#define READ_DATE_OFFSET_UNALIGN 2

int spinand_read(struct aic_spinand *flash, u8 *addr, u32 offset, u32 size)
{
    int err = 0;
    u32 page, cplen;
    u32 off = offset;
    u8 *buf = NULL, *p, copy_flag = 0;
    u32 remaining = size;
    u16 blk;
    u32 blk_size;
    u32 off_in_page = 0;

    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    buf = malloc(flash->info->page_size + flash->info->oob_size);
    if (!buf) {
        pr_err("Failed to malloc spinand buf.\n");
        goto exit_spinand_read;
    }

    blk_size = flash->info->page_size * flash->info->pages_per_eraseblock;

    blk = off / blk_size;

    /* Search for the first good block after the given offset */
    while (spinand_block_isbad(flash, blk)) {
        pr_warn("find a bad block %d, off adjust to the next block\n", blk);
        off += blk_size;
        blk = off / blk_size;
    }

    while (remaining > 0) {
        if (off % flash->info->page_size == 0) {
            if (remaining >= flash->info->page_size) {
                p = addr;
                copy_flag = 0;
            } else {
                p = buf;
                copy_flag = READ_DATE_OFFSET_UNALIGN;
                memset(buf, 0xFF, flash->info->page_size);
            }
        } else {
            p = buf;
            copy_flag = READ_DATE_OFFSET_ALIGN;
            memset(buf, 0xFF, flash->info->page_size);
        }
        page = off / flash->info->page_size;

        err =
            spinand_read_page(flash, page, p, flash->info->page_size, NULL, 0);
        if (err != 0)
            goto exit_spinand_read;

        cplen = flash->info->page_size;

        if (copy_flag == READ_DATE_OFFSET_ALIGN) {
            /*Misaligned offset with page address*/
            off_in_page = off % flash->info->page_size;
            if (remaining <= flash->info->page_size - off_in_page) {
                cplen = remaining;
            } else {
                cplen = flash->info->page_size - off_in_page;
            }
            memcpy(addr, buf + off_in_page, cplen);
        } else if (copy_flag == READ_DATE_OFFSET_UNALIGN) {
            /*Align offset with page address*/
            cplen = remaining;
            memcpy(addr, buf, cplen);
        }

        remaining -= cplen;
        off += cplen;
        addr += cplen;

        if ((off % blk_size == 0) && remaining) {
            blk = off / blk_size;
            /* Search for the first good block after the given offset */
            while (spinand_block_isbad(flash, blk)) {
                pr_warn("find a bad block, off adjust to the next block\n");
                off += blk_size;
                blk = off / blk_size;
            }
        }
    }

exit_spinand_read:
    if (buf)
        free(buf);
    return err;
}

int spinand_erase(struct aic_spinand *flash, u32 offset, u32 size)
{
    int err = 0;
    u16 blk;
    u32 off = offset;
    u32 cnt = size;
    u32 blk_size;

    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    blk_size = flash->info->pages_per_eraseblock * flash->info->page_size;

    if (offset % blk_size) {
        pr_err("Offset not aligned with a block (0x%x)\r\n", blk_size);
        return -SPINAND_ERR;
    }

    if (!cnt)
        return -SPINAND_ERR;

    if (cnt % blk_size)
        cnt = (cnt + blk_size) / blk_size * blk_size;

    while (cnt > 0) {
        blk = off / blk_size;

        err = spinand_block_erase(flash, blk);
        if (err != 0)
            pr_err("Erase block %u failed, err: %d.\r\n", blk, err);

        cnt -= blk_size;
        off += blk_size;
    }

    return err;
}

int spinand_write(struct aic_spinand *flash, u8 *addr, u32 offset, u32 size)
{
    int err = 0;
    u32 page;
    u32 off = offset;
    u16 blk;
    u32 remaining = size;
    u32 blk_size;

    if (!flash) {
        pr_err("flash is NULL\r\n");
        return -SPINAND_ERR;
    }

    if (offset % flash->info->page_size) {
        pr_err("Offset not aligned with a page (0x%x)\r\n",
               flash->info->page_size);
        return -SPINAND_ERR;
    }

    blk_size = flash->info->page_size * flash->info->pages_per_eraseblock;

    blk = off / blk_size;

    /* Search for the first good block after the given offset */
    while (spinand_block_isbad(flash, blk)) {
        pr_warn("find a bad block %u, off adjust to the next block\n", blk);
        off += blk_size;
        blk = off / blk_size;
    }
    if (remaining % flash->info->page_size)
        remaining =
            (flash->info->page_size + remaining) / flash->info->page_size;
    else
        remaining = remaining / flash->info->page_size;

    while (remaining > 0) {
        page = off / flash->info->page_size;

        err = spinand_write_page(flash, page, (u8 *)addr,
                                 flash->info->page_size, NULL, 0);
        if (err != 0)
            return err;

        remaining--;
        off += flash->info->page_size;
        addr += flash->info->page_size;

        if ((off % blk_size == 0) && remaining) {
            blk = off / blk_size;
            /* Search for the first good block after the given offset */
            while (spinand_block_isbad(flash, blk)) {
                pr_warn("find a bad block, off adjust to the next block\n");
                off += blk_size;
                blk = off / blk_size;
            }
        }
    }

    return err;
}
