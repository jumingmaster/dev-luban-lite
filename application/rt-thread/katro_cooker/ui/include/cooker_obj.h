#include "app_ui_common.h"


typedef struct
{
    int                 channel;
    ui_cooker_state_t   state;
    ui_cooker_ctx_t     ctx;
    rt_mutex_t          mtx;
} cooker_obj_t;



void cooker_set_power(cooker_obj_t * obj);

void cooker_set_timing(cooker_obj_t * obj, int on_timing, int hour, int minute);

void cooker_set_thermos(cooker_obj_t * obj, int thermos);

void cooker_set_max_gear(cooker_obj_t * obj, int max_gear);

