/*
 * Copyright (c) 2024-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: matteo <duanmt@artinchip.com>
 */

#define LOG_TAG     "sc035"

#include <drivers/i2c.h>
#include <drivers/pin.h>

#include "aic_core.h"
#include "mpp_types.h"
#include "mpp_img_size.h"
#include "mpp_vin.h"

#include "drv_camera.h"
#include "camera_inner.h"

/* Default format configuration of SC035 */
#define SC035_DFT_WIDTH        VGA_WIDTH
#define SC035_DFT_HEIGHT       VGA_HEIGHT
#define SC035_DFT_BUS_TYPE     MEDIA_BUS_RAW8_MONO
#define SC035_DFT_CODE         MEDIA_BUS_FMT_Y8_1X8

#define SC035_I2C_SLAVE_ID     0x30
#define SC035_CHIP_ID          0x310b

#define ENABLE                                  1
#define DISABLE                                 0

#define MAX_SET_REG_NUMS                        10
#define FRAME_MAX_WIDTH                         640
#define FRAME_MAX_HEIGHT                        480

#define SC035_DELAY_ADDR                        0x0000
#define SC035_SOFTWARE_RESET_ADDR               0x0103
#define SC035_SLEEP_MODE_CTRL_ADDR              0x0100
#define SC035_CHIP_ID_HIGH_ADDR                 0x3108
#define SC035_CHIP_ID_LOW_ADDR                  0x3109
/* Output Window */
#define SC035_FRAME_WIDTH_HIGH_ADDR             0x3208
#define SC035_FRAME_WIDTH_LOW_ADDR              0x3209
#define SC035_FRAME_HEIGHT_HIGH_ADDR            0x320a
#define SC035_FRAME_HEIGHT_LOW_ADDR             0x320b
#define SC035_WINDOW_WIDTH_HIGH_ADDR            0x320c
#define SC035_WINDOW_WIDTH_LOW_ADDR             0x320d
#define SC035_WINDOW_HEIGHT_HIGH_ADDR           0x320e
#define SC035_WINDOW_HEIGHT_LOW_ADDR            0x320f
#define SC035_FRAME_COL_START_HIGH_ADDR         0x3210
#define SC035_FRAME_COL_START_LOW_ADDR          0x3211
#define SC035_FRAME_ROW_START_HIGH_ADDR         0x3212
#define SC035_FRAME_ROW_START_LOW_ADDR          0x3213
#define SC035_EXT_EXPOSURE_BLANK_ROWS_ADDR      0x3218
#define SC035_EXT_EXPOSURE_TRIG_DELAY_ADDR      0x3226
#define SC035_EXT_EXPOSURE_EXP_ROWS_HIGH_ADDR   0x3e01
#define SC035_EXT_EXPOSURE_EXP_ROWS_LOW_ADDR    0x3e02
/* HDR */
#define SC035_HDR_ENABLE_CTRL_ADDR              0x3220
/* BLC */
#define SC035_BLC_ENABLE_ADDR                   0x3900
#define SC035_BLC_MODE_CTRL_ADDR                0x3902
#define SC035_BLC_CHANNEL_HIGH_ADDR             0x3928
#define SC035_BLC_CHANNEL_LOW_ADDR              0x3905
#define SC035_BLC_TARGET_VALUE_HIGH_ADDR        0x3907
#define SC035_BLC_TARGET_VALUE_LOW_ADDR         0x3908
/* DVP */
#define SC035_DVP_POL_CTRL_ADDR                 0x3d08
/* AGC */
#define SC035_AGC_MODE_CTRL_ADDR                0x3e03
#define SC035_AGC_GAIN_MAPPING_HIGH_ADDR        0x3e08
#define SC035_AGC_GAIN_MAPPING_LOW_ADDR         0x3e09
#define SC035_DIGITAL_COARSE_GAIN_ADDR          0x3e06
#define SC035_DIGITAL_FINE_GAIN_ADDR            0x3e07
#define SC035_ANALOG_COARSE_GAIN_ADDR           0x3e08
#define SC035_ANALOG_FINE_GAIN_ADDR             0x3e09
/* AEC */
#define SC035_AEC_TOTAL_EXPOSURE_TIME_HIGH_ADDR 0x3e01
#define SC035_AEC_TOTAL_EXPOSURE_TIME_LOW_ADDR  0x3e02
#define SC035_HDR_EXPOSURE_TIME_HIGH_ADDR       0x3e31
#define SC035_HDR_EXPOSURE_TIME_LOW_ADDR        0x3e32
/* ISP */
#define SC035_MIRROR_ENABLE_ADDR                0x3221      // Bit[2:1]
#define SC035_FLIP_ENABLE_ADDR                  0x3221      // Bit[6:5]
/* Test mode */
#define SC035_INCREMENT_PATTERN_ENABLE_ADDR     0x4501

#define SC035_SLEEP_ENABLE          0
#define SC035_SLEEP_DISABLE         1

/* AGC */

typedef enum {
    ANALOG_COARSE_GAIN_X1           = 0x0,
    ANALOG_COARSE_GAIN_X2           = 0x1,
    ANALOG_COARSE_GAIN_X4           = 0x3,
    ANALOG_COARSE_GAIN_X8           = 0x7,
} analog_coarse_gain;

typedef enum {
    ANALOG_FINE_GAIN_X1             = 0x10,
    ANALOG_FINE_GAIN_1X0625         = 0x11,
    ANALOG_FINE_GAIN_1X125          = 0x12,
    ANALOG_FINE_GAIN_1X1875         = 0x13,
    ANALOG_FINE_GAIN_1X25           = 0x14,
    ANALOG_FINE_GAIN_1X3125         = 0x15,
    ANALOG_FINE_GAIN_1X375          = 0x16,
    ANALOG_FINE_GAIN_1X4375         = 0x17,
    ANALOG_FINE_GAIN_1X5            = 0x18,
    ANALOG_FINE_GAIN_1X5625         = 0x19,
    ANALOG_FINE_GAIN_1X625          = 0x1a,
    ANALOG_FINE_GAIN_1X6875         = 0x1b,
    ANALOG_FINE_GAIN_1X75           = 0x1c,
    ANALOG_FINE_GAIN_1X8125         = 0x1d,
    ANALOG_FINE_GAIN_1X875          = 0x1e,
    ANALOG_FINE_GAIN_1X9375         = 0x1f
} analog_fine_gain;

typedef enum {
    DIGITAL_COARSE_GAIN_X1          = 0x0,
    DIGITAL_COARSE_GAIN_X2          = 0x1,
    DIGITAL_COARSE_GAIN_X4          = 0x3
} digital_coarse_gain;

typedef enum {
    DIGITAL_FINE_GAIN_X1            = 0x80,
    DIGITAL_FINE_GAIN_1X0625        = 0x88,
    DIGITAL_FINE_GAIN_1X125         = 0x90,
    DIGITAL_FINE_GAIN_1X1875        = 0x98,
    DIGITAL_FINE_GAIN_1X25          = 0xa0,
    DIGITAL_FINE_GAIN_1X3125        = 0xa8,
    DIGITAL_FINE_GAIN_1X375         = 0xb0,
    DIGITAL_FINE_GAIN_1X4375        = 0xb8,
    DIGITAL_FINE_GAIN_1X5           = 0xc0,
    DIGITAL_FINE_GAIN_1X5625        = 0xc8,
    DIGITAL_FINE_GAIN_1X625         = 0xd0,
    DIGITAL_FINE_GAIN_1X6875        = 0xd8,
    DIGITAL_FINE_GAIN_1X75          = 0xe0,
    DIGITAL_FINE_GAIN_1X8125        = 0xe8,
    DIGITAL_FINE_GAIN_1X875         = 0xf0,
    DIGITAL_FINE_GAIN_1X9375        = 0xf8
} digital_fine_gain;

/* BLC */
typedef enum {
    BLC_CHANNEL_OFFSET_8            = 0,
    BLC_CHANNEL_OFFSET_4,
    BLC_CHANNEL_ENABLE_1_OFFSET_8_OR_4,
    BLC_CHANNEL_ENABLE_1
} BLC_Channel;

typedef enum {
    BLC_MODE_MANUAL =   0x0,
    BLC_MODE_AUTO   =   0x1
} BLC_Mode;

typedef struct {
    u8 Enable;
    BLC_Mode Mode;
    BLC_Channel Channel;
    u16 TargetValue;
} BLC_Args;

typedef enum {
    EXPOSURE_NORMAL_MODE            = 0,
    EXPOSURE_HDR_MODE
} Exposure_Mode;

/* 640*480, xclk=24M, pclk=85MHz, fps=60 */
static const struct reg16_info sc035_640x480_60fps_regs[] = {
    {SC035_SOFTWARE_RESET_ADDR, 1},
    {SC035_SLEEP_MODE_CTRL_ADDR, SC035_SLEEP_ENABLE},
    {0x36e9, 0x80},
    {0x36f9, 0x80},
    {SC035_DELAY_ADDR, 0x80},
    {SC035_DELAY_ADDR, 0x80},
    {0x300f, 0x0f},
    {0x3018, 0x1f},
    {0x3019, 0xff},
    {0x301c, 0xb4},
    {0x301f, 0x89},
    {0x3028, 0x82},
    {SC035_FRAME_WIDTH_HIGH_ADDR, 0x02},                  // width: 0x0280 = 640
    {SC035_FRAME_WIDTH_LOW_ADDR, 0x80},
    {SC035_FRAME_HEIGHT_HIGH_ADDR, 0x01},                 // height: 0x01e0 = 480
    {SC035_FRAME_HEIGHT_LOW_ADDR, 0xe0},
    {SC035_WINDOW_WIDTH_HIGH_ADDR, 0x05},                 // stride: 0x0554 = 1364
    {SC035_WINDOW_WIDTH_LOW_ADDR, 0x54},
    {SC035_WINDOW_HEIGHT_HIGH_ADDR, 0x04},                // frame: 0x040e = 1038, 0x0210=528
    {SC035_WINDOW_HEIGHT_LOW_ADDR, 0x0e},
    {SC035_FRAME_COL_START_HIGH_ADDR, 0x00},              // column start position
    {SC035_FRAME_COL_START_LOW_ADDR, 0x08},
    {SC035_FRAME_ROW_START_HIGH_ADDR, 0x00},              // row start position
    {SC035_FRAME_ROW_START_LOW_ADDR, 0x08},
    {0x3217, 0x00},
    {SC035_EXT_EXPOSURE_BLANK_ROWS_ADDR, 0x00},
    {SC035_HDR_ENABLE_CTRL_ADDR, 0x10},                   // Disable HDR
    {0x3223, 0x48},
    {SC035_EXT_EXPOSURE_TRIG_DELAY_ADDR, 0x74},
    {0x3227, 0x07},
    {0x323b, 0x00},
    {0x3250, 0xf0},
    {0x3251, 0x02},
    {0x3252, 0x02},
    {0x3253, 0x08},
    {0x3254, 0x02},
    {0x3255, 0x07},
    {0x3304, 0x48},
    {0x3305, 0x00},
    {0x3306, 0x98},
    {0x3309, 0x50},
    {0x330a, 0x01},
    {0x330b, 0x18},
    {0x330c, 0x18},
    {0x330f, 0x40},
    {0x3310, 0x10},
    {0x3314, 0x1e},
    {0x3315, 0x30},
    {0x3316, 0x68},
    {0x3317, 0x1b},
    {0x3329, 0x5c},
    {0x332d, 0x5c},
    {0x332f, 0x60},
    {0x3335, 0x64},
    {0x3344, 0x64},
    {0x335b, 0x80},
    {0x335f, 0x80},
    {0x3366, 0x06},
    {0x3385, 0x31},
    {0x3387, 0x39},
    {0x3389, 0x01},
    {0x33b1, 0x03},
    {0x33b2, 0x06},
    {0x33bd, 0xe0},
    {0x33bf, 0x10},
    {0x3621, 0xa4},
    {0x3622, 0x05},
    {0x3624, 0x47},
    {0x3630, 0x4a},
    {0x3631, 0x58},
    {0x3633, 0x52},
    {0x3635, 0x03},
    {0x3636, 0x25},
    {0x3637, 0x8a},
    {0x3638, 0x0f},
    {0x3639, 0x08},
    {0x363a, 0x00},
    {0x363b, 0x48},
    {0x363c, 0x86},
    {0x363d, 0x10},
    {0x363e, 0xf8},
    {0x3640, 0x00},
    {0x3641, 0x01},
    {0x36ea, 0x2f},
    {0x36eb, 0x0d},
    {0x36ec, 0x0d},
    {0x36ed, 0x20},
    {0x36fa, 0x2f},
    {0x36fb, 0x10},
    {0x36fc, 0x02},
    {0x36fd, 0x00},
    {SC035_BLC_TARGET_VALUE_LOW_ADDR, 0x91},
    {0x391b, 0x81},
    {SC035_DVP_POL_CTRL_ADDR, 0x01},
    {SC035_EXT_EXPOSURE_EXP_ROWS_HIGH_ADDR, 0x18},
    {SC035_EXT_EXPOSURE_EXP_ROWS_LOW_ADDR, 0xf0},
    {SC035_AGC_MODE_CTRL_ADDR, 0x2b},
    {0x3e06, 0x0c},
    {0x3f04, 0x03},
    {0x3f05, 0x80},
    {0x4500, 0x59},
    {SC035_INCREMENT_PATTERN_ENABLE_ADDR, 0xc4},
    {0x4800, 0x64},
    {0x4809, 0x01},
    {0x4810, 0x00},
    {0x4811, 0x01},
    {0x4837, 0x18},
    {0x5011, 0x00},
    {0x5988, 0x02},
    {0x598e, 0x05},
    {0x598f, 0x17},
    {0x36e9, 0x40},
    {0x36f9, 0x62},
    {SC035_SLEEP_MODE_CTRL_ADDR, SC035_SLEEP_DISABLE},
    {SC035_DELAY_ADDR, 0x0a}, // Must delay (2^17/mclk + 1) ms before 0x4418/0x4419
    {0x4418, 0x0a},
    {0x4419, 0x80},
    {REG16_ADDR_INVALID, 0x00}
};

struct sc03_dev {
    struct rt_device dev;
    struct rt_i2c_bus_device *i2c;
    u32 pwdn_pin;
    struct clk *clk;

    struct mpp_video_fmt fmt;

    bool on;
    bool streaming;
};

static struct sc03_dev g_sc03_dev = {0};

static int sc035_read_reg(struct rt_i2c_bus_device *i2c, u16 reg, u8 *val)
{
    if (rt_i2c_read_reg16(i2c, SC035_I2C_SLAVE_ID, reg, val, 1) != 1) {
        LOG_E("%s: error: reg = 0x%x, val = 0x%x", __func__, reg, *val);
        return -1;
    }

    return 0;
}

static u16 sc035_read_u16(struct rt_i2c_bus_device *i2c, u16 reg_h, u16 reg_l)
{
    u8 val_h = 0, val_l = 0;

    if (sc035_read_reg(i2c, reg_h, &val_h) ||
        sc035_read_reg(i2c, reg_l, &val_l))
        return 0;

    return val_h << 8 | val_l;
}

static int sc035_write_reg(struct rt_i2c_bus_device *i2c, u16 reg, u8 val)
{
    if (rt_i2c_write_reg16(i2c, SC035_I2C_SLAVE_ID, reg, &val, 1) != 1) {
        LOG_E("%s: error: reg = 0x%x, val = 0x%x", __func__, reg, val);
        return -1;
    }

    return 0;
}

static int sc035_write_array(struct rt_i2c_bus_device *i2c,
                               const struct reg16_info *regs, u32 size)
{
    int i, ret = 0;

    for (i = 0; i < size; i++) {

        if (regs[i].reg == REG16_ADDR_INVALID)
            break;

        if (regs[i].reg == SC035_DELAY_ADDR) {
            aicos_udelay(regs[i].val * 1000);
            continue;
        }

        ret = sc035_write_reg(i2c, regs[i].reg, regs[i].val);
        if (ret < 0)
            return ret;
    }

    return ret;
}

static int sc03_set_fps(struct sc03_dev *sensor, u32 fps)
{
    u16 cur = 0, base = 0x207, val = 0;

    if (fps < 10)
        fps = 10;
    if (fps > 120)
        fps = 120;

    cur = sc035_read_u16(sensor->i2c, 0x320e, 0x320f);
    if (!cur) {
        LOG_E("Failed to read FPS\n");
        return -1;
    }

    val = base * 120 / fps;

    LOG_I("Set FPS %d[0x%03x] -> %d[0x%03x]\n",
          120 * base / cur, cur, 120 * base / val, val);

    if (cur == val)
        return 0;

    if (sc035_write_reg(sensor->i2c, 0x320e, val >> 8) ||
        sc035_write_reg(sensor->i2c, 0x320f, val & 0xFF))
        return -1;
    else
        return 0;
}

/* Adjust AEC preferentially, and then adjust AGC */
static int sc03_set_agc(struct sc03_dev *sensor, u32 percent)
{
    u16 cur = 0, val = 0, gain = PERCENT_TO_INT(0x10, 0xFF, percent);
    u8 val_h = 0, val_l = 0;

    if (sc035_read_reg(sensor->i2c, 0x3e08, &val_h)
        || sc035_read_reg(sensor->i2c, 0x3e09, &val_l)) {
        LOG_E("Failed to get current AGC\n");
        return -1;
    }
    cur = ((val_h & 0xC) << 6) | val_l; /* {16'h3e08[4:2],16h3e09} */

    sc035_write_reg(sensor->i2c, 0x3303, 0x0b);

    if (gain < 0x20) { /* 1x ~ 2x */
        val_h = 0x03;
        val_l = 0x10 | ((gain - 0x10) & 0xF);
    } else if (gain < 0x40) { /* 2x ~ 4x */
        val_h = 0x7;
        val_l = 0x10 | (((gain - 0x20) >> 1) & 0xF);
    } else if (gain < 0x80) { /* 4x ~ 8x */
        val_h = 0xf;
        val_l = 0x10 | (((gain - 0x40) >> 2) & 0xF);
    } else { /* 8x ~ 16x */
        val_h = 0x1f;
        val_l = 0x10 | (((gain - 0x80) >> 3) & 0xF);
    }

    val = (val_h << 8) | val_l;
    LOG_I("Set AGC %d[0x%03x] -> %d[0x%03x]\n", cur, cur, val, val);

    if (cur == val)
        return 0;

    if (sc035_write_reg(sensor->i2c, 0x3e08, val_h) ||
        sc035_write_reg(sensor->i2c, 0x3e09, val_l))
        return -1;
    else
        return 0;
}

/* SC031GS doesn't support AEC. Adjust the exposure time to control brightness. */
static int sc03_set_aec(struct sc03_dev *sensor, u32 percent)
{
    u16 frame_len = 0, cur = 0, val = 0;
    u8 val_h = 0, val_l = 0;

    frame_len = sc035_read_u16(sensor->i2c, 0x320e, 0x320f);
    if (!frame_len) {
        LOG_E("Failed to get frame length\n");
        return -1;
    }
    if (frame_len - 6 > 0xFFF) {
        LOG_W("Frame length %d is too large\n", frame_len);
        frame_len = 0xFFF + 6;
    }

    if (sc035_read_reg(sensor->i2c, 0x3e01, &val_h)
        || sc035_read_reg(sensor->i2c, 0x3e02, &val_l)) {
        LOG_E("Failed to get current AEC\n");
        return -1;
    }
    cur = (val_h << 4) | (val_l >> 4); // {16'h3e01,16'h3e02[7:4]}

    val = PERCENT_TO_INT(1, frame_len - 6, percent);
    LOG_I("Set AEC %d[0x%03x] -> %d[0x%03x] (Frame length %d[0x%03x])\n",
          cur, cur, val, val, frame_len, frame_len);

    if (cur == val)
        return 0;

    if (sc035_write_reg(sensor->i2c, 0x3e01, (val >> 4) & 0xFF) ||
        sc035_write_reg(sensor->i2c, 0x3e02, (val & 0xF) << 4))
        return -1;
    else
        return 0;
}

static int sc03_enable_h_flip(struct sc03_dev *sensor, bool enable)
{
    u8 cur = 0, mask = 0x6;

    if (sc035_read_reg(sensor->i2c, 0x3221, &cur)) {
        LOG_E("Failed to get the H flip status\n");
        return -1;
    }

    LOG_I("Set H flip 0x%x -> 0x%x\n", (cur & mask) >> 1, enable ? 3 : 0);
    if (enable)
        return sc035_write_reg(sensor->i2c, 0x3221, cur | mask);
    else
        return sc035_write_reg(sensor->i2c, 0x3221, cur & ~mask);
}

static int sc03_enable_v_flip(struct sc03_dev *sensor, bool enable)
{
    u8 cur = 0, mask = 0x60;

    if (sc035_read_reg(sensor->i2c, 0x3221, &cur)) {
        LOG_E("Failed to get the V flip status\n");
        return -1;
    }

    LOG_I("Set V flip 0x%x -> 0x%x\n", (cur & mask) >> 5, enable ? 3 : 0);
    if (enable)
        return sc035_write_reg(sensor->i2c, 0x3221, cur | mask);
    else
        return sc035_write_reg(sensor->i2c, 0x3221, cur & ~mask);
}

static int sc035_init(struct sc03_dev *sensor)
{
    if (sc035_write_array(sensor->i2c, sc035_640x480_60fps_regs, ARRAY_SIZE(sc035_640x480_60fps_regs)))
        return -1;

    if (sc03_set_aec(sensor, 100))
        return -1;

    if (sc03_set_agc(sensor, 6))
        return -1;
    return 0;
}

static int sc035_probe(struct sc03_dev *sensor)
{
    u8 id_h = 0, id_l = 0;

    if (sc035_read_reg(sensor->i2c, SC035_CHIP_ID_LOW_ADDR, &id_l) ||
            sc035_read_reg(sensor->i2c, SC035_CHIP_ID_HIGH_ADDR, &id_h))
        return -1;

    if ((id_h << 8 | id_l) != SC035_CHIP_ID) {
        LOG_E("Invalid chip ID: %02x %02x\n", id_h, id_l);
        return -1;
    }
    return sc035_init(sensor);
}

static bool sc035_is_open(struct sc03_dev *sensor)
{
    return sensor->on;
}

static int sc035_power_on(struct sc03_dev *sensor)
{
    if (sensor->on)
        return -1;

    camera_pin_set_high(sensor->pwdn_pin);
    aicos_udelay(2);

    LOG_I("Power on");
    sensor->on = true;
    return 0;
}

static int sc035_power_off(struct sc03_dev *sensor)
{
    if (!sensor->on)
        return -1;

    camera_pin_set_low(sensor->pwdn_pin);

    LOG_I("Power off");
    sensor->on = false;
    return 0;
}

static rt_err_t sc03_init(rt_device_t dev)
{
    struct sc03_dev *sensor = (struct sc03_dev *)dev;

    sensor->i2c = camera_i2c_get();
    if (!sensor->i2c)
        return -RT_EINVAL;

    sensor->fmt.code     = SC035_DFT_CODE;
    sensor->fmt.width    = SC035_DFT_WIDTH;
    sensor->fmt.height   = SC035_DFT_HEIGHT;
    sensor->fmt.bus_type = SC035_DFT_BUS_TYPE;
    sensor->fmt.flags = MEDIA_SIGNAL_HSYNC_ACTIVE_HIGH |
                        MEDIA_SIGNAL_VSYNC_ACTIVE_LOW |
                        MEDIA_SIGNAL_PCLK_SAMPLE_FALLING;

    sensor->pwdn_pin = camera_pwdn_pin_get();
    if (!sensor->pwdn_pin)
        return -RT_EINVAL;

    return RT_EOK;
}

static rt_err_t sc03_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct sc03_dev *sensor = (struct sc03_dev *)dev;

    if (sc035_is_open(sensor))
        return RT_EOK;

    sc035_power_on(sensor);

    if (sc035_probe(sensor)) {
        sc035_power_off(sensor);
        return -RT_ERROR;
    }

    LOG_I("SC035HGS inited");
    return RT_EOK;
}

static rt_err_t sc03_close(rt_device_t dev)
{
    struct sc03_dev *sensor = (struct sc03_dev *)dev;

    if (!sc035_is_open(sensor))
        return -RT_ERROR;

    sc035_power_off(sensor);
    return RT_EOK;
}

static int sc03_get_fmt(struct sc03_dev *sensor, struct mpp_video_fmt *cfg)
{
    cfg->code   = sensor->fmt.code;
    cfg->width  = sensor->fmt.width;
    cfg->height = sensor->fmt.height;
    cfg->flags  = sensor->fmt.flags;
    cfg->bus_type = sensor->fmt.bus_type;
    return RT_EOK;
}

static int sc03_start(struct sc03_dev *sensor)
{
    return 0;
}

static int sc03_stop(struct sc03_dev *sensor)
{
    return 0;
}

static int sc03_pause(rt_device_t dev)
{
    struct sc03_dev *sensor = (struct sc03_dev *)dev;

    return sc035_write_reg(sensor->i2c, SC035_SLEEP_MODE_CTRL_ADDR,
                           SC035_SLEEP_ENABLE);
}

static int sc03_resume(rt_device_t dev)
{
    struct sc03_dev *sensor = (struct sc03_dev *)dev;

    return sc035_write_reg(sensor->i2c, SC035_SLEEP_MODE_CTRL_ADDR,
                           SC035_SLEEP_DISABLE);
}

static rt_err_t sc03_control(rt_device_t dev, int cmd, void *args)
{
    struct sc03_dev *sensor = (struct sc03_dev *)dev;

    switch (cmd) {
    case CAMERA_CMD_START:
        return sc03_start(sensor);
    case CAMERA_CMD_STOP:
        return sc03_stop(sensor);
    case CAMERA_CMD_PAUSE:
        return sc03_pause(dev);
    case CAMERA_CMD_RESUME:
        return sc03_resume(dev);
    case CAMERA_CMD_GET_FMT:
        return sc03_get_fmt(sensor, (struct mpp_video_fmt *)args);
    case CAMERA_CMD_SET_FPS:
        return sc03_set_fps(sensor, *(u32 *)args);
    case CAMERA_CMD_SET_AUTOGAIN:
        return sc03_set_agc(sensor, *(u32 *)args);
    case CAMERA_CMD_SET_AEC_VAL:
        return sc03_set_aec(sensor, *(u32 *)args);
    case CAMERA_CMD_SET_H_FLIP:
        return sc03_enable_h_flip(sensor, *(bool *)args);
    case CAMERA_CMD_SET_V_FLIP:
        return sc03_enable_v_flip(sensor, *(bool *)args);
    default:
        LOG_I("Unsupported cmd: 0x%x", cmd);
        return -RT_EINVAL;
    }
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops sc03_ops =
{
    .init = sc03_init,
    .open = sc03_open,
    .close = sc03_close,
    .control = sc03_control,
};
#endif

int rt_hw_sc03_init(void)
{
#ifdef RT_USING_DEVICE_OPS
    g_sc03_dev.dev.ops = &sc03_ops;
#else
    g_sc03_dev.dev.init = sc03_init;
    g_sc03_dev.dev.open = sc03_open;
    g_sc03_dev.dev.close = sc03_close;
    g_sc03_dev.dev.control = sc03_control;
#endif
    g_sc03_dev.dev.type = RT_Device_Class_CAMERA;

    rt_device_register(&g_sc03_dev.dev, CAMERA_DEV_NAME, 0);

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_sc03_init);
