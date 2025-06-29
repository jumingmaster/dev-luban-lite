#ifndef __COOKER_DRV_H__
#define __COOKER_DRV_H__
#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "aic_core.h"
#include "aic_drv.h"

// 定义时间参数(微秒)
#define FRAME_INTERVAL 100000  // 100ms
#define START_MIN 3000         // 启动高电平最小3ms
#define BIT1_HIGH 1500         // 1的高电平1.5ms
#define BIT0_HIGH 500          // 0的高电平0.5ms
#define LOW_MIN 500            // 低电平最小0.5ms

#define COOKER_TX_PIN   GET_PIN(A, 4)
#define COOKER_RX_PIN   GET_PIN(A, 5)

void cooker_drv_init(void (*rx_irq)(void *args));

int cooker_read_bytes(uint8_t * data, uint8_t length);

int cooker_send_bytes(uint8_t * data, uint8_t length);

#endif
