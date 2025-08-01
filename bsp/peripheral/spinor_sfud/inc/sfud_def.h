/*
 * This file is part of the Serial Flash Universal Driver Library.
 *
 * Copyright (c) 2016-2018, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: It is the macro definition head file for this library.
 * Created on: 2016-04-23
 */

#ifndef _SFUD_DEF_H_
#define _SFUD_DEF_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sfud_cfg.h>
#include "sfud_flash_def.h"

#ifdef __cplusplus
extern "C" {
#endif

/* debug print function. Must be implement by user. */
#ifdef SFUD_DEBUG_MODE
#ifndef SFUD_DEBUG
#define SFUD_DEBUG(...) sfud_log_debug(__FILE__, __LINE__, __VA_ARGS__)
#endif /* SFUD_DEBUG */
#else
#ifndef SFUD_DEBUG
#define SFUD_DEBUG(...)
#endif /* SFUD_DEBUG */
#endif /* SFUD_DEBUG_MODE */

#ifndef SFUD_INFO
#define SFUD_INFO(...)  sfud_log_info(__VA_ARGS__)
#endif

/* assert for developer. */
#ifdef SFUD_DEBUG_MODE
#define SFUD_ASSERT(EXPR)                                                       \
if (!(EXPR))                                                                    \
{                                                                               \
    SFUD_DEBUG("(%s) has assert failed at %s.", #EXPR, __FUNCTION__);           \
    return 0;                                                                   \
}
#else
#define SFUD_ASSERT(EXPR)
#endif

extern void sfud_log_debug(const char *file, const long line, const char *format, ...);
extern void sfud_log_info(const char *format, ...);
/**
 * retry process
 *
 * @param delay delay function for every retry. NULL will not delay for every retry.
 * @param retry retry counts
 * @param result SFUD_ERR_TIMEOUT: retry timeout
 */
#define SFUD_RETRY_PROCESS(delay, retry, result)                                \
    void (*__delay_temp)(void) = (void (*)(void))delay;                         \
    if (retry == 0) {                                                           \
        result = SFUD_ERR_TIMEOUT;                                              \
        break;                                                                  \
    } else {                                                                    \
        if (__delay_temp) {                                                     \
            __delay_temp();                                                     \
        }                                                                       \
        retry --;                                                               \
    }

/* software version number */
#define SFUD_SW_VERSION                             "1.1.0"
/*
 * all defined supported command
 */
#ifndef SFUD_CMD_WRITE_ENABLE
#define SFUD_CMD_WRITE_ENABLE                          0x06
#endif

#ifndef SFUD_CMD_WRITE_DISABLE
#define SFUD_CMD_WRITE_DISABLE                         0x04
#endif

#ifndef SFUD_CMD_READ_STATUS_REGISTER
#define SFUD_CMD_READ_STATUS_REGISTER                  0x05
#endif

#ifndef SFUD_CMD_READ_CONFIG_REGISTER
#define SFUD_CMD_READ_CONFIG_REGISTER                  0x35
#endif

#ifndef SFUD_VOLATILE_SR_WRITE_ENABLE
#define SFUD_VOLATILE_SR_WRITE_ENABLE                  0x50
#endif

#ifndef SFUD_CMD_WRITE_STATUS_REGISTER
#define SFUD_CMD_WRITE_STATUS_REGISTER                 0x01
#endif

#ifndef SFUD_CMD_WRITE_STATUS2_REGISTER
#define SFUD_CMD_WRITE_STATUS2_REGISTER                0x31
#endif

#ifndef SFUD_CMD_WRITE_STATUS3_REGISTER
#define SFUD_CMD_WRITE_STATUS3_REGISTER                0x11
#endif

#ifndef SFUD_CMD_PAGE_PROGRAM
#define SFUD_CMD_PAGE_PROGRAM                          0x02
#endif

#ifndef SFUD_CMD_AAI_WORD_PROGRAM
#define SFUD_CMD_AAI_WORD_PROGRAM                      0xAD
#endif

#ifndef SFUD_CMD_ERASE_CHIP
#define SFUD_CMD_ERASE_CHIP                            0xC7
#endif

#ifndef SFUD_CMD_READ_DATA
#define SFUD_CMD_READ_DATA                             0x03
#endif

#ifndef SFUD_CMD_DUAL_OUTPUT_READ_DATA
#define SFUD_CMD_DUAL_OUTPUT_READ_DATA                 0x3B
#endif

#ifndef SFUD_CMD_DUAL_IO_READ_DATA
#define SFUD_CMD_DUAL_IO_READ_DATA                     0xBB
#endif

#ifndef SFUD_CMD_QUAD_IO_READ_DATA
#define SFUD_CMD_QUAD_IO_READ_DATA                     0xEB
#endif

#ifndef SFUD_CMD_QUAD_OUTPUT_READ_DATA
#define SFUD_CMD_QUAD_OUTPUT_READ_DATA                 0x6B
#endif

#ifndef SFUD_CMD_MANUFACTURER_DEVICE_ID
#define SFUD_CMD_MANUFACTURER_DEVICE_ID                0x90
#endif

#ifndef SFUD_CMD_JEDEC_ID
#define SFUD_CMD_JEDEC_ID                              0x9F
#endif

#ifndef SFUD_CMD_READ_UNIQUE_ID
#define SFUD_CMD_READ_UNIQUE_ID                        0x4B
#endif

#ifndef SFUD_CMD_READ_SFDP_REGISTER
#define SFUD_CMD_READ_SFDP_REGISTER                    0x5A
#endif

#ifndef SFUD_CMD_ENABLE_RESET
#define SFUD_CMD_ENABLE_RESET                          0x66
#endif

#ifndef SFUD_CMD_RESET
#define SFUD_CMD_RESET                                 0x99
#endif

#ifndef SFUD_CMD_ENTER_4B_ADDRESS_MODE
#define SFUD_CMD_ENTER_4B_ADDRESS_MODE                 0xB7
#endif

#ifndef SFUD_CMD_EXIT_4B_ADDRESS_MODE
#define SFUD_CMD_EXIT_4B_ADDRESS_MODE                  0xE9
#endif

#ifndef SFUD_CMD_READ_SECURITY_REGISTER
#define SFUD_CMD_READ_SECURITY_REGISTER                0x48
#endif

#ifndef SFUD_CMD_SECURITY_REGISTER_PROGRAM
#define SFUD_CMD_SECURITY_REGISTER_PROGRAM             0x42
#endif

#ifndef SFUD_CMD_ERASE_SECURITY_REGISTER
#define SFUD_CMD_ERASE_SECURITY_REGISTER               0x44
#endif

#ifndef SFUD_WRITE_MAX_PAGE_SIZE
#define SFUD_WRITE_MAX_PAGE_SIZE                        256
#endif

/* send dummy data for read data */
#ifndef SFUD_DUMMY_DATA
#define SFUD_DUMMY_DATA                                0xFF
#endif

/* maximum number of erase type support on JESD216 (V1.0) */
#define SFUD_SFDP_ERASE_TYPE_MAX_NUM                      4

/* write protection state mask */
#ifndef SFUD_WRITE_PROTECTION_MASK
#define SFUD_WRITE_PROTECTION_MASK                      0xFC
#endif

/* time limit in wait_busy                           100us   ms     s    margin */
#define SFUD_CHIP_ERASE_TIMEOUT                        (10 * 1000 * 40 * 10)
#define SFUD_SECTOR_ERASE_TIMEOUT                      (10 * 400       * 10)
#define SFUD_PAGE_PROGRAM_TIMEOUT                      (10 * 5         * 10)
#define SFUD_WRITE_STATUS_TIMEOUT                      (10 * 30        * 10)
#define SFUD_CHECK_BUSY_TIMEOUT                        (10 * 10        * 10)

/**
 * status register bits
 */
enum {
    SFUD_STATUS_REGISTER_BUSY = (1 << 0),                  /**< busing */
    SFUD_STATUS_REGISTER_WEL = (1 << 1),                   /**< write enable latch */
    SFUD_STATUS_REGISTER_SRP = (1 << 7),                   /**< status register protect */
};

/**
 * error code
 */
typedef enum {
    SFUD_SUCCESS = 0,                                      /**< success */
    SFUD_ERR_NOT_FOUND = 1,                                /**< not found or not supported */
    SFUD_ERR_WRITE = 2,                                    /**< write error */
    SFUD_ERR_READ = 3,                                     /**< read error */
    SFUD_ERR_TIMEOUT = 4,                                  /**< timeout error */
    SFUD_ERR_ADDR_OUT_OF_BOUND = 5,                        /**< address is out of flash bound */
} sfud_err;

enum spi_nor_option_flags {
    SNOR_F_USE_FSR = (0),
    SNOR_F_HAS_SR_TB = (1 << 1),
    SNOR_F_NO_OP_CHIP_ERASE = (1 << 2),
    SNOR_F_READY_XSR_RDY = (1 << 3),
    SNOR_F_USE_CLSR = (1 << 4),
    SNOR_F_BROKEN_RESET = (1 << 5),
    SNOR_F_4B_OPCODES = (1 << 6),
    SNOR_F_HAS_4BAIT = (1 << 7),
    SNOR_F_HAS_LOCK = (1 << 8),
    SNOR_F_HAS_16BIT_SR = (1 << 9),
    SNOR_F_NO_READ_CR = (1 << 10),
    SNOR_F_HAS_SR_TB_BIT6 = (1 << 11),
    SNOR_F_HAS_4BIT_BP = (1 << 12),
    SNOR_F_HAS_SR_BP3_BIT6 = (1 << 13),
};

#ifdef SFUD_USING_QSPI
/**
 * QSPI flash read cmd format
 */
typedef struct {
    uint8_t instruction;
    uint8_t instruction_lines;
    uint8_t address_size;
    uint8_t address_lines;
    uint8_t alternate_bytes_lines;
    uint8_t dummy_cycles;
    uint8_t data_lines;
} sfud_qspi_read_cmd_format;
#endif /* SFUD_USING_QSPI */

/* SPI bus write read data function type */
typedef sfud_err (*spi_write_read_func)(const uint8_t *write_buf, size_t write_size, uint8_t *read_buf, size_t read_size);

#ifdef SFUD_USING_SFDP

/* Keep the same with enum sfud_qspi_read_mode */
#define HWCAPS_NO_FAST_READ     (1 << 0)
#define HWCAPS_FAST_READ_1_1_2  (1 << 1)
#define HWCAPS_FAST_READ_1_2_2  (1 << 2)
#define HWCAPS_FAST_READ_1_1_4  (1 << 3)
#define HWCAPS_FAST_READ_1_4_4  (1 << 4)

/**
 * the SFDP (Serial Flash Discoverable Parameters) parameter info which used on this library
 */
typedef struct {
    bool available;                              /**< available when read SFDP OK */
    uint8_t major_rev;                           /**< SFDP Major Revision */
    uint8_t minor_rev;                           /**< SFDP Minor Revision */
    uint16_t write_gran;                         /**< write granularity (bytes) */
    uint8_t erase_4k;                            /**< 4 kilobyte erase is supported throughout the device */
    uint8_t erase_4k_cmd;                        /**< 4 Kilobyte erase command */
    bool sr_is_non_vola;                         /**< status register is supports non-volatile */
    uint8_t vola_sr_we_cmd;                      /**< volatile status register write enable command */
    bool addr_3_byte;                            /**< supports 3-Byte addressing */
    bool addr_4_byte;                            /**< supports 4-Byte addressing */
    uint32_t capacity;                           /**< flash capacity (bytes) */
    struct {
        uint32_t size;                           /**< erase sector size (bytes). 0x00: not available */
        uint8_t cmd;                             /**< erase command */
    } eraser[SFUD_SFDP_ERASE_TYPE_MAX_NUM];      /**< supported eraser types table */
    //TODO lots of fast read-related stuff (like modes supported and number of wait states/dummy cycles needed in each)
    uint8_t hwcaps_fast_read;
} sfud_sfdp, *sfud_sfdp_t;
#endif

/**
 * SPI device
 */
typedef struct __sfud_spi {
    /* SPI device name */
    char *name;
    /* SPI bus work frequency */
    sfud_err (*set_speed)(const struct __sfud_spi *spi, uint32_t bus_hz);
    sfud_err (*get_bus_id)(const struct __sfud_spi *spi, uint32_t *bus_id);
    void (*set_rx_delay)(const struct __sfud_spi *spi, uint32_t mode);
    /* SPI bus write read data function */
    sfud_err (*wr)(const struct __sfud_spi *spi, const uint8_t *write_buf, size_t write_size, uint8_t *read_buf,
                   size_t read_size);
#ifdef SFUD_USING_QSPI
    /* QSPI fast read function */
    sfud_err (*qspi_read)(const struct __sfud_spi *spi, uint32_t addr, sfud_qspi_read_cmd_format *qspi_read_cmd_format,
                          uint8_t *read_buf, size_t read_size);
#endif
    /* lock SPI bus */
    void (*lock)(const struct __sfud_spi *spi);
    /* unlock SPI bus */
    void (*unlock)(const struct __sfud_spi *spi);
    /* some user data */
    void *user_data;
} sfud_spi, *sfud_spi_t;

/**
 * serial flash device
 */
typedef struct {
    char *name;                                  /**< serial flash name */
    size_t index;                                /**< index of flash device information table  @see flash_table */
    sfud_flash_chip chip;                        /**< flash chip information */
    sfud_spi spi;                                /**< SPI device */
    bool init_ok;                                /**< initialize OK flag */
    bool addr_in_4_byte;                         /**< flash is in 4-Byte addressing */
    struct {
        void (*delay)(void);                     /**< every retry's delay */
        size_t times;                            /**< default times for error retry */
    } retry;
    void *user_data;                             /**< some user data */

#ifdef SFUD_USING_QSPI
    sfud_qspi_read_cmd_format read_cmd_format;   /**< fast read cmd format */
    uint32_t flags;
    void *quad_enable;
#endif

#ifdef SFUD_USING_SFDP
    sfud_sfdp sfdp;                              /**< serial flash discoverable parameters by JEDEC standard */
    uint32_t init_hz;                            /** Reading SFDP, bus frequency should be <=50MHz, this is requirement from SPEC */
    uint32_t bus_hz;                             /** Working bus frequency */
#endif

} sfud_flash, *sfud_flash_t;

typedef void (*quad_enable_func)(sfud_flash *flash);
#ifdef __cplusplus
}
#endif

#endif /* _SFUD_DEF_H_ */
