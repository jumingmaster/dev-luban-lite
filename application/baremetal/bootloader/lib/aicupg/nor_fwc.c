/*
 * Copyright (c) 2023-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors:  Xiong Hao <hao.xiong@artinchip.com>
 */

#include <stdlib.h>
#include <string.h>
#include <aic_core.h>
#include <aic_utils.h>
#include <aic_crc32.h>
#include <aicupg.h>
#include <mtd.h>
#include <sfud.h>
#include "upg_internal.h"
#include <spienc.h>
#include <firmware_security.h>

#define MAX_DUPLICATED_PART 6
#define MAX_NOR_NAME        32

struct aicupg_nor_priv {
    struct mtd_dev *mtd[MAX_DUPLICATED_PART];
    unsigned long start_offset[MAX_DUPLICATED_PART];
    unsigned long erase_offset[MAX_DUPLICATED_PART];
};

static s32 nor_fwc_get_mtd_partitions(struct fwc_info *fwc,
                                      struct aicupg_nor_priv *priv)
{
    char name[MAX_NOR_NAME], *p;
    int cnt = 0, idx = 0;

    p = fwc->meta.partition;
    while (*p) {
        if (cnt >= MAX_NOR_NAME) {
            pr_err("Partition name is too long.\n");
            return -1;
        }

        name[cnt] = *p;
        p++;
        cnt++;
        if (*p == ';' || *p == '\0') {
            name[cnt] = '\0';
            priv->mtd[idx] = mtd_get_device(name);
            if (!priv->mtd[idx]) {
                pr_err("MTD %s part not found.\n", name);
                return -1;
            }
            idx++;
            cnt = 0;
        }
        if (*p == ';')
            p++;
        if (*p == '\0')
            break;
    }
    return 0;
}

int nor_is_exist()
{
    return 0;
}

/*
 * storage device prepare,should probe spi norflash
 *  - probe spi device
 *  - set env for MTD partitions
 *  - probe MTD device
 */
extern sfud_flash *sfud_probe(u32 spi_bus);
s32 nor_fwc_prepare(struct fwc_info *fwc, u32 id)
{
    sfud_flash *flash;

    flash = sfud_probe(id);
    if (flash == NULL) {
        printf("Failed to probe spinor flash.\n");
        return -1;
    }

#ifdef AIC_SPIENC_BYPASS_IN_UPGMODE
        spienc_set_bypass(1);
#endif
    return 0;
}

/*
 * New FirmWare Component start, should prepare to burn FWC to NOR
 *  - Get FWC attributes
 *  - Parse MTD partitions  FWC going to upgrade
 */
void nor_fwc_start(struct fwc_info *fwc)
{
    struct aicupg_nor_priv *priv;
    int ret = 0;

    priv = malloc(sizeof(struct aicupg_nor_priv));
    if (!priv) {
        pr_err("Out of memory, malloc failed.\n");
        goto err;
    }
    memset(priv, 0, sizeof(struct aicupg_nor_priv));
    fwc->priv = priv;

    ret = nor_fwc_get_mtd_partitions(fwc, priv);
    if (ret) {
        pr_err("Get MTD partitions failed.\n");
        goto err;
    }

#ifdef AIC_USING_SPIENC
    if (strstr(fwc->meta.name, "target.spl"))
        spienc_select_tweak(AIC_SPIENC_HW_TWEAK);
#endif
    fwc->block_size = 2048;

#ifdef AICUPG_FIRMWARE_SECURITY
    firmware_security_init();
#endif

    return;
err:
    if (priv)
        free(priv);
}

/*
 * New FirmWare Component write, should write data to NOR
 *  - Erase MTD partitions for prepare write
 *  - Write data to MTD partions
 */
s32 nor_fwc_data_write(struct fwc_info *fwc, u8 *buf, s32 len)
{
    struct aicupg_nor_priv *priv;
    struct mtd_dev *mtd;
    unsigned long offset, erase_offset;
    int i, calc_len = 0, ret = 0;
    u8 __attribute__((unused)) *rdbuf = NULL;

    if ((fwc->meta.size - fwc->trans_size) < len)
        calc_len = fwc->meta.size - fwc->trans_size;
    else
        calc_len = len;

    fwc->calc_partition_crc = crc32(fwc->calc_partition_crc, buf, calc_len);

#ifdef AICUPG_FIRMWARE_SECURITY
    firmware_security_decrypt(buf, len);
#endif

#ifdef AICUPG_SINGLE_TRANS_BURN_CRC32_VERIFY
    rdbuf = aicupg_malloc_align(len, CACHE_LINE_SIZE);
    if (!rdbuf) {
        pr_err("Error: malloc buffer failed.\n");
        return 0;
    }
#endif

    priv = (struct aicupg_nor_priv *)fwc->priv;
    for (i = 0; i < MAX_DUPLICATED_PART; i++) {
        mtd = priv->mtd[i];
        if (!mtd)
            continue;

        offset = priv->start_offset[i];
        if ((offset + len) > (mtd->size)) {
            pr_err("Not enough space to write mtd %s\n", mtd->name);
            goto out;
        }

        /* erase 1 sector when offset+len more than erased address */
        erase_offset = priv->erase_offset[i];
        while ((offset + len) > erase_offset) {
            ret = mtd_erase(mtd, erase_offset, ROUNDUP(len, mtd->erasesize));
            if (ret) {
                pr_err("MTD erase sector failed!\n");
                goto out;
            }
            priv->erase_offset[i] = erase_offset + ROUNDUP(len, mtd->erasesize);
            erase_offset = priv->erase_offset[i];
        }

        ret = mtd_write(mtd, offset, buf, len);
        if (ret) {
            pr_err("Write mtd %s error.\n", mtd->name);
            goto out;
        }

#ifdef AICUPG_SINGLE_TRANS_BURN_CRC32_VERIFY
        // Read data to calc crc
        ret = mtd_read(mtd, offset, rdbuf, len);
        if (ret) {
            pr_err("Read mtd %s error.\n", mtd->name);
            goto out;
        }
#endif
        priv->start_offset[i] = offset + len;
    }

#ifdef AICUPG_SINGLE_TRANS_BURN_CRC32_VERIFY
    if (crc32(0, buf, calc_len) != crc32(0, rdbuf, calc_len)) {
        pr_err("calc_len:%d\n", calc_len);
        pr_err("crc err at trans len %u\n", fwc->trans_size);
    }
#endif

    fwc->trans_size += len;
    pr_debug("%s, data len %d, trans len %d\n", __func__, len, fwc->trans_size);

    if (rdbuf)
        aicupg_free_align(rdbuf);
    return len;

out:
    if (rdbuf)
        aicupg_free_align(rdbuf);
    return ret;
}

s32 nor_fwc_data_read(struct fwc_info *fwc, u8 *buf, s32 len)
{
    struct aicupg_nor_priv *priv;
    struct mtd_dev *mtd;
    unsigned long offset;
    int ret;

    priv = (struct aicupg_nor_priv *)fwc->priv;
    mtd = priv->mtd[0];
    if (!mtd)
        return 0;

    offset = priv->start_offset[0];
    ret = mtd_read(mtd, offset, buf, len);
    if (ret) {
        pr_err("read mtd %s error.\n", mtd->name);
        return 0;
    }
    priv->start_offset[0] = offset + len;

    fwc->trans_size += len;
    fwc->calc_partition_crc = fwc->meta.crc;
    pr_debug("%s, data len %d, trans len %d\n", __func__, len, fwc->trans_size);

    return len;
}

/*
 * New FirmWare Component end, should free memory
 *  - free the memory what to  deposit device information
 */
void nor_fwc_data_end(struct fwc_info *fwc)
{
    struct aicupg_nor_priv *priv;

    priv = (struct aicupg_nor_priv *)fwc->priv;
    if (!priv)
        return;

#ifdef AIC_USING_SPIENC
    if (strstr(fwc->meta.name, "target.spl"))
        spienc_select_tweak(AIC_SPIENC_USER_TWEAK);
#endif
    pr_debug("trans len all %d\n", fwc->trans_size);
    free(priv);
}
