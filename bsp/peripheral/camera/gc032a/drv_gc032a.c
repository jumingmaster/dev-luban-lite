/*
 * Copyright (c) 2024-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: matteo <duanmt@artinchip.com>
 */

#define LOG_TAG     "gc032a"

#include <drivers/i2c.h>
#include <drivers/pin.h>

#include "aic_core.h"
#include "mpp_types.h"
#include "mpp_img_size.h"
#include "mpp_vin.h"

#include "drv_camera.h"
#include "camera_inner.h"

/* Default format configuration of GC03xx */
#define GC03_DFT_WIDTH        VGA_WIDTH
#define GC03_DFT_HEIGHT       VGA_HEIGHT
#define GC03_DFT_BUS_TYPE     MEDIA_BUS_PARALLEL
#define GC03_DFT_CODE         MEDIA_BUS_FMT_YUYV8_2X8

#define GC03_I2C_SLAVE_ID     0x21
#define GC032A_CHIP_ID        0x232A

static const struct reg8_info sensor_init_data[] = {
    /* System */
    {0xf3, 0xff},
    {0xf5, 0x06},
    {0xf7, 0x01},
    {0xf8, 0x05}, // Framerate
    {0xf9, 0xce},
    {0xfa, 0x00},
    {0xfc, 0x02},
    {0xfe, 0x02},
    {0x81, 0x03},

    /* ANALOG & CISCTL */
    {0xfe, 0x00},
    {0x03, 0x01},
    {0x04, 0xce},
    {0x05, 0x01},
    {0x06, 0xad},
    {0x07, 0x00},
    {0x08, 0x10},
    {0x0a, 0x00}, //04
    {0x0c, 0x00}, //04
    {0x0d, 0x01},
    {0x0e, 0xe8}, // height 488
    {0x0f, 0x02},
    {0x10, 0x88}, // width 648

    {0x17, 0x54},
    {0x19, 0x08}, //04
    {0x1a, 0x0a},
    {0x1f, 0x40},
    {0x20, 0x30},
    {0x2e, 0x80},
    {0x2f, 0x2b},
    {0x30, 0x1a},

    {0xfe, 0x02},
    {0x03, 0x02},
    {0x05, 0xd7},
    {0x06, 0x60},
    {0x08, 0x80},
    {0x12, 0x89},

    /* BLK */
    {0xfe, 0x00},
    {0x18, 0x02},
    {0xfe, 0x02},
    {0x40, 0x22},
    {0x45, 0x00},
    {0x46, 0x00},
    {0x49, 0x20},
    {0x4b, 0x3c},
    {0x50, 0x20},
    {0x42, 0x10},

    /* ISP */
    {0xfe, 0x01},
    {0x0a, 0xc5},
    {0x45, 0x00},
    {0xfe, 0x00},
    {0x40, 0xff},
    {0x41, 0x25},
    {0x42, 0xcf},
    {0x43, 0x10},
    {0x44, 0x83},
    {0x46, 0x22},
    {0x49, 0x03},
    {0xfe, 0x02},
    {0x22, 0xf6},

    /* LSC */
    {0xfe, 0x01},
    {0x12, 0xa0},
    {0x13, 0x3a},
    {0xc1, 0x38}, //3c
    {0xc2, 0x4c}, //50
    {0xc3, 0x00},
    {0xc4, 0x32},
    {0xc5, 0x24},
    {0xc6, 0x16},
    {0xc7, 0x08},
    {0xc8, 0x08},
    {0xc9, 0x00},
    {0xca, 0x20},
    {0xdc, 0x8a},
    {0xdd, 0xa0},
    {0xde, 0xa6},
    {0xdf, 0x75},
    {0xfe, 0x01},
    {0x7c, 0x09},
    {0x65, 0x06},
    {0x7c, 0x08},
    {0x56, 0xf4},
    {0x66, 0x0f},
    {0x67, 0x84},
    {0x6b, 0x80},
    {0x6d, 0x12},
    {0x6e, 0xb0},
    {0x86, 0x00},
    {0x87, 0x00},
    {0x88, 0x00},
    {0x89, 0x00},
    {0x8a, 0x00},
    {0x8b, 0x00},
    {0x8c, 0x00},
    {0x8d, 0x00},
    {0x8e, 0x00},
    {0x8f, 0x00},
    {0x90, 0xef},
    {0x91, 0xe1},
    {0x92, 0x0c},
    {0x93, 0xef},
    {0x94, 0x65},
    {0x95, 0x1f},
    {0x96, 0x0c},
    {0x97, 0x2d},
    {0x98, 0x20},
    {0x99, 0xaa},
    {0x9a, 0x3f},
    {0x9b, 0x2c},
    {0x9c, 0x5f},
    {0x9d, 0x3e},
    {0x9e, 0xaa},
    {0x9f, 0x67},
    {0xa0, 0x60},
    {0xa1, 0x00},
    {0xa2, 0x00},
    {0xa3, 0x0a},
    {0xa4, 0xb6},
    {0xa5, 0xac},
    {0xa6, 0xc1},
    {0xa7, 0xac},
    {0xa8, 0x55},
    {0xa9, 0xc3},
    {0xaa, 0xa4},
    {0xab, 0xba},
    {0xac, 0xa8},
    {0xad, 0x55},
    {0xae, 0xc8},
    {0xaf, 0xb9},
    {0xb0, 0xd4},
    {0xb1, 0xc3},
    {0xb2, 0x55},
    {0xb3, 0xd8},
    {0xb4, 0xce},
    {0xb5, 0x00},
    {0xb6, 0x00},
    {0xb7, 0x05},
    {0xb8, 0xd6},
    {0xb9, 0x8c},
    {0xfe, 0x01},
    {0xd0, 0x40},
    {0xd1, 0xf8},
    {0xd2, 0x00},
    {0xd3, 0xfa},
    {0xd4, 0x45},
    {0xd5, 0x02},
    {0xd6, 0x30},
    {0xd7, 0xfa},
    {0xd8, 0x08},
    {0xd9, 0x08},
    {0xda, 0x58},
    {0xdb, 0x02},
    {0xfe, 0x00},

    /* Y Gamma */
    {0xfe, 0x00},
    {0xba, 0x00},
    {0xbb, 0x04},
    {0xbc, 0x0a},
    {0xbd, 0x0e},
    {0xbe, 0x22},
    {0xbf, 0x30},
    {0xc0, 0x3d},
    {0xc1, 0x4a},
    {0xc2, 0x5d},
    {0xc3, 0x6b},
    {0xc4, 0x7a},
    {0xc5, 0x85},
    {0xc6, 0x90},
    {0xc7, 0xa5},
    {0xc8, 0xb5},
    {0xc9, 0xc2},
    {0xca, 0xcc},
    {0xcb, 0xd5},
    {0xcc, 0xde},
    {0xcd, 0xea},
    {0xce, 0xf5},
    {0xcf, 0xff},

    /* Auto Gamma */
    {0xfe, 0x00},
    {0x5a, 0x08},
    {0x5b, 0x0f},
    {0x5c, 0x15},
    {0x5d, 0x1c},
    {0x5e, 0x28},
    {0x5f, 0x36},
    {0x60, 0x45},
    {0x61, 0x51},
    {0x62, 0x6a},
    {0x63, 0x7d},
    {0x64, 0x8d},
    {0x65, 0x98},
    {0x66, 0xa2},
    {0x67, 0xb5},
    {0x68, 0xc3},
    {0x69, 0xcd},
    {0x6a, 0xd4},
    {0x6b, 0xdc},
    {0x6c, 0xe3},
    {0x6d, 0xf0},
    {0x6e, 0xf9},
    {0x6f, 0xff},

    /* Gain */
    {0xfe, 0x00},
    {0x70, 0x50},

    /* AEC */
    {0xfe, 0x00},
    {0x4f, 0x01},
    {0xfe, 0x01},
    {0x44, 0x04},
    {0x1f, 0x30},
    {0x20, 0x40},
    {0x26, 0x9a},
    {0x27, 0x01},
    {0x28, 0xce},
    {0x29, 0x03},
    {0x2a, 0x02},
    {0x2b, 0x04},
    {0x2c, 0x36},
    {0x2d, 0x07},
    {0x2e, 0xd2},
    {0x2f, 0x0b},
    {0x30, 0x6e},
    {0x31, 0x0e},
    {0x32, 0x70},
    {0x33, 0x12},
    {0x34, 0x0c},
    {0x3c, 0x30},
    {0x3e, 0x20},
    {0x3f, 0x2d},
    {0x40, 0x40},
    {0x41, 0x5b},
    {0x42, 0x82},
    {0x43, 0xb7},
    {0x04, 0x0a},
    {0x02, 0x79},
    {0x03, 0xc0},
    {0xcc, 0x08},
    {0xcd, 0x08},
    {0xce, 0xa4},
    {0xcf, 0xec},

    /* DNDD */
    {0xfe, 0x00},
    {0x81, 0xb8},
    {0x82, 0x12},
    {0x83, 0x0a},
    {0x84, 0x01},
    {0x86, 0x50},
    {0x87, 0x18},
    {0x88, 0x10},
    {0x89, 0x70},
    {0x8a, 0x20},
    {0x8b, 0x10},
    {0x8c, 0x08},
    {0x8d, 0x0a},

    /* INTPEE */
    {0xfe, 0x00},
    {0x8f, 0xaa},
    {0x90, 0x9c},
    {0x91, 0x52},
    {0x92, 0x03},
    {0x93, 0x03},
    {0x94, 0x08},
    {0x95, 0x44},
    {0x97, 0x00},
    {0x98, 0x00},

    {0xfe, 0x00},
    {0xa1, 0x30},
    {0xa2, 0x41},
    {0xa4, 0x30},
    {0xa5, 0x20},
    {0xaa, 0x30},
    {0xac, 0x32},

    /* YCP */
    {0xfe, 0x00},
    {0xd1, 0x3c},
    {0xd2, 0x3c},
    {0xd3, 0x38},
    {0xd6, 0xf4},
    {0xd7, 0x1d},
    {0xdd, 0x73},
    {0xde, 0x84},

    {0xfe, 0x00},
    {0xd3, 0x40}, // Luma_contrast
    {0xd1, 0x00},
    {0xd2, 0x00},
    {0X40, 0xed}, // Close noise enable
    {0X40, 0xfd}, //
};

struct gc03_dev {
    struct rt_device dev;
    struct rt_i2c_bus_device *i2c;
    u32 pwdn_pin;
    struct clk *clk;

    struct mpp_video_fmt fmt;

    bool on;
    bool streaming;
};

static struct gc03_dev g_gc03_dev = {0};

static int gc03_write_reg(struct rt_i2c_bus_device *i2c, u8 reg, u8 val)
{
    if (rt_i2c_write_reg(i2c, GC03_I2C_SLAVE_ID, reg, &val, 1) != 1) {
        LOG_E("%s: error: reg = 0x%x, val = 0x%x", __func__, reg, val);
        return -1;
    }

    return 0;
}

static int gc03_read_reg(struct rt_i2c_bus_device *i2c, u8 reg, u8 *val)
{
    if (rt_i2c_read_reg(i2c, GC03_I2C_SLAVE_ID, reg, val, 1) != 1) {
        LOG_E("%s: error: reg = 0x%x, val = 0x%x", __func__, reg, *val);
        return -1;
    }

    return 0;
}

static void gc032a_reset(struct gc03_dev *sensor)
{
    // gc03_write_reg(sensor->i2c, 0xFE, 0xF0);
}

static int gc032a_init(struct gc03_dev *sensor)
{
    int i = 0;
    const struct reg8_info *info = sensor_init_data;

    gc032a_reset(sensor);
    aicos_udelay(1000);

    for (i = 0; i < ARRAY_SIZE(sensor_init_data); i++, info++) {
        if (gc03_write_reg(sensor->i2c, info->reg, info->val))
            return -1;
    }

    return 0;
}

static int gc032a_probe(struct gc03_dev *sensor)
{
    u8 id_h = 0, id_l = 0;

    if (gc03_read_reg(sensor->i2c, 0xf0, &id_h) ||
        gc03_read_reg(sensor->i2c, 0xf1, &id_l))
        return -1;

    if ((id_h << 8 | id_l) != GC032A_CHIP_ID) {
        LOG_E("Invalid chip ID: %02x %02x\n", id_h, id_l);
        return -1;
    }
    return gc032a_init(sensor);
}

static bool gc032a_is_open(struct gc03_dev *sensor)
{
    return sensor->on;
}

static void gc032a_power_on(struct gc03_dev *sensor)
{
    if (sensor->on)
        return;

    camera_pin_set_low(sensor->pwdn_pin);
    aicos_udelay(1);
    camera_pin_set_high(sensor->pwdn_pin);
    aicos_udelay(2);
    camera_pin_set_low(sensor->pwdn_pin);

    LOG_I("Power on");
    sensor->on = true;
}

static void gc032a_power_off(struct gc03_dev *sensor)
{
    if (!sensor->on)
        return;

    camera_pin_set_high(sensor->pwdn_pin);
    aicos_udelay(1);
    camera_pin_set_low(sensor->pwdn_pin);

    LOG_I("Power off");
    sensor->on = false;
}

static rt_err_t gc03_init(rt_device_t dev)
{
    struct gc03_dev *sensor = &g_gc03_dev;

    sensor->i2c = camera_i2c_get();
    if (!sensor->i2c)
        return -RT_EINVAL;

    sensor->fmt.code   = GC03_DFT_CODE;
    sensor->fmt.width  = GC03_DFT_WIDTH;
    sensor->fmt.height = GC03_DFT_HEIGHT;
    sensor->fmt.bus_type = GC03_DFT_BUS_TYPE;
    sensor->fmt.flags = MEDIA_SIGNAL_HSYNC_ACTIVE_HIGH |
                        MEDIA_SIGNAL_VSYNC_ACTIVE_LOW |
                        MEDIA_SIGNAL_PCLK_SAMPLE_FALLING;

    sensor->pwdn_pin = camera_pwdn_pin_get();
    if (!sensor->pwdn_pin)
        return -RT_EINVAL;


    return RT_EOK;
}

static rt_err_t gc03_open(rt_device_t dev, rt_uint16_t oflag)
{
    struct gc03_dev *sensor = (struct gc03_dev *)dev;

    if (gc032a_is_open(sensor))
        return RT_EOK;

    gc032a_power_on(sensor);

    if (gc032a_probe(sensor)) {
        gc032a_power_off(sensor);
        return -RT_ERROR;
    }

    LOG_I("GC032A inited");
    return RT_EOK;
}

static rt_err_t gc03_close(rt_device_t dev)
{
    struct gc03_dev *sensor = (struct gc03_dev *)dev;

    if (!gc032a_is_open(sensor))
        return -RT_ERROR;

    gc032a_power_off(sensor);
    return RT_EOK;
}

static int gc03_get_fmt(rt_device_t dev, struct mpp_video_fmt *cfg)
{
    struct gc03_dev *sensor = (struct gc03_dev *)dev;

    cfg->code   = sensor->fmt.code;
    cfg->width  = sensor->fmt.width;
    cfg->height = sensor->fmt.height;
    cfg->flags  = sensor->fmt.flags;
    cfg->bus_type = sensor->fmt.bus_type;
    return RT_EOK;
}

static int gc03_start(rt_device_t dev)
{
    return 0;
}

static int gc03_stop(rt_device_t dev)
{
    return 0;
}

static int gc03_pause(rt_device_t dev)
{
    return gc03_close(dev);
}

static int gc03_resume(rt_device_t dev)
{
    return gc03_open(dev, 0);
}

static rt_err_t gc03_control(rt_device_t dev, int cmd, void *args)
{
    switch (cmd) {
    case CAMERA_CMD_START:
        return gc03_start(dev);
    case CAMERA_CMD_STOP:
        return gc03_stop(dev);
    case CAMERA_CMD_PAUSE:
        return gc03_pause(dev);
    case CAMERA_CMD_RESUME:
        return gc03_resume(dev);
    case CAMERA_CMD_GET_FMT:
        return gc03_get_fmt(dev, (struct mpp_video_fmt *)args);
    default:
        LOG_I("Unsupported cmd: 0x%x", cmd);
        return -RT_EINVAL;
    }
    return RT_EOK;
}

#ifdef RT_USING_DEVICE_OPS
static const struct rt_device_ops gc03_ops =
{
    .init = gc03_init,
    .open = gc03_open,
    .close = gc03_close,
    .control = gc03_control,
};
#endif

int rt_hw_gc03_init(void)
{
#ifdef RT_USING_DEVICE_OPS
    g_gc03_dev.dev.ops = &gc03_ops;
#else
    g_gc03_dev.dev.init = gc03_init;
    g_gc03_dev.dev.open = gc03_open;
    g_gc03_dev.dev.close = gc03_close;
    g_gc03_dev.dev.control = gc03_control;
#endif
    g_gc03_dev.dev.type = RT_Device_Class_CAMERA;

    rt_device_register(&g_gc03_dev.dev, CAMERA_DEV_NAME, 0);

    return 0;
}
INIT_DEVICE_EXPORT(rt_hw_gc03_init);
