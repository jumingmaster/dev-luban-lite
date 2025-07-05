#ifndef __APP_COMMON_H__
#define __APP_COMMON_H__



#include <board.h>
#include <rtthread.h>
#include <rtdevice.h>
#include "aic_core.h"
#include "aic_drv.h"

typedef struct
{
    int onoff;
    int resume;
    int timing;
    int gear;
} cooker_t;

typedef struct
{
    int fault;
    int pot_state;
    int pot_timeout;
    int spec_pot;
    int afterheat;
    int pan_temp;
    int igbt_temp;
    int voltage;
    int current;
} cooker_hw_t;



int cooker_ui_read_hw_data(int channel, cooker_hw_t * data);

int cpu_temp_get(int * temp);

#endif
