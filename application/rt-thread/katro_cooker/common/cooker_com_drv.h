#ifndef __COOKER_COM_DRV_H__
#define __COOKER_COM_DRV_H__

#include "app_common.h"

void cooker_drv_init(void);

int cooker_send_bytes(uint8_t * data, uint8_t length);

int cooker_wait_idle(void);

int cooker_wait_data(uint8_t * data);

















#endif
