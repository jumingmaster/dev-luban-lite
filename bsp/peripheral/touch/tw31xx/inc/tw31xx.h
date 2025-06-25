/*
 * Copyright (c) 2024, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date              Notes
 * 2024-06-24        the first version
 */

#ifndef __TW31XX_H__
#define __TW31XX_H__

#include "drivers/touch.h"
#include <aic_hal_gpio.h>
#include <aic_drv_gpio.h>

// #define TW31XX_ADDR                     0x0C

// #define TW31XX_REG_EVENT                0x0000
// #define TW31XX_REG_DATA                 0x0004
// #define TW31XX_REG_POINT                0x001C
// #define TW31XX_REG_DBG_HEADER           0x0100
// #define TW31XX_REG_DBG_DATA             0x0108
// #define TW31XX_REG_CHIP_ID              0x10FC
// #define TW31XX_REG_COMMAND              0x10FF

// #define DEVICE_TW3118                   0x3118
// #define DEVICE_TW3106                   0x3106
// #define DEVICE_TW3110                   0x3110

// #define TW31XX_CTRL_DONE                0x01
// #define TW31XX_CTRL_RESET               0x02
// #define TW31XX_EVENT_NO                 0x00
// #define TW31XX_EVENT_ABS                0x05
// #define TW31XX_EVENT_TPPWON             0x04
// #define TW31XX_EVENT_KEY                0x02
// #define TW31XX_EVENT_GES                0x04
// #define TW31XX_EVENT_REL                0x08
// #define TW31XX_EVENT_PRES               0x10
// #define TW31XX_EVENT_DBG_STR            0x40
// #define TW31XX_EVENT_DBG_CAP            0x80
// #define TW31XX_X2Y_Enable               0x08
// #define TW31XX_REGITER_LEN              2
// #define TWSC_MAX_FINGER                 10
// #define TW31XX_RESET_MDELAY             100
// #define TW31XX_CONFIG_MAX_LENGTH        186


#define TW31XX_ADDR                     0x0C

#define TW31XX_REG_EVENT                0x0000
#define TW31XX_REG_DATA                 0x0004
#define TW31XX_REG_POINT                0x001C
#define TW31XX_REG_DBG_HEADER           0x0100
#define TW31XX_REG_DBG_DATA             0x0108
#define TW31XX_REG_CHIP_ID              0x10FC
#define TW31XX_REG_COMMAND              0x10FF

#define DEVICE_TW3118                   0x3118
#define DEVICE_TW3106                   0x3106
#define DEVICE_TW3110                   0x3110

#define TW31XX_CTRL_DONE                0x01
#define TW31XX_CTRL_RESET               0x02
#define TW31XX_EVENT_NO                 0x00
#define TW31XX_EVENT_ABS                0x05
#define TW31XX_EVENT_TPPWON             0x04
#define TW31XX_EVENT_KEY                0x02
#define TW31XX_EVENT_GES                0x04
#define TW31XX_EVENT_REL                0x08
#define TW31XX_EVENT_PRES               0x10
#define TW31XX_EVENT_DBG_STR            0x40
#define TW31XX_EVENT_DBG_CAP            0x80
#define TW31XX_X2Y_Enable               0x08
#define TW31XX_REGITER_LEN              2
#define TWSC_MAX_FINGER                 10
#define TW31XX_RESET_MDELAY             100
#define TW31XX_CONFIG_MAX_LENGTH        186

#define I2C_STATUS_REG_ADDR             0x1052

#define TECHWIN_MAX_FINGER              1

struct tp_touch_head{
	u16	Event;
	u16 Address;
	u16 Length;
	u16	LoopTick;
	u32 Rsv1;
	u16 Rsv2;
	u8 Rsv3;
	u8 Checksum;
}__attribute__ ((packed));

 struct tp_ScrInfo{
  u32 ScrId   : 25;
  u32 ScrNum  : 5;
  u32 Fix     : 2; // 0x3
}__attribute__ ((packed));

 struct tp_KeyInfo{
  u32 KeyId   : 25;
  u32 KeyNum  : 5;
  u32 Fix     : 2; // 0x3
}__attribute__ ((packed));

 struct tp_SubInfo{
  u32 GesId    : 8;
  u32 Rsr0Id   : 8;
  u32 Rsr1Id   : 8;
  u32 Checksum : 8;
}__attribute__ ((packed));

 struct tp_point{
  u32 Y    : 13;
  u32 X    : 13;
  u32 Id   : 5;
  u32 Stat : 1;
}__attribute__ ((packed));

struct tp_touch_data{
	struct tp_ScrInfo ScrInfo;
	struct tp_KeyInfo KeyInfo;
	struct tp_SubInfo SubInfo;
	struct tp_point   point[TECHWIN_MAX_FINGER];
}__attribute__ ((packed));

struct fdata{  
	struct tp_touch_head  tp_head;
	struct tp_touch_data  tp_data;
};

#endif  // TW31XX_H
