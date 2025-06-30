#ifndef __APP_UI_COMMON_H__
#define __APP_UI_COMMON_H__

#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"
#include "rtthread.h"
#include "lv_anim.h"
#include "drop_menu.h"

#define LV_ANIM_REF(obj)    (&((lv_animimg_t *)scr->logo_amni)->anim)


#define UI_COOKER_NUM   4

typedef struct
{
    int         on_timing;
    int         hour;
    int         minute;
    int         on_max_gear;
    int         gear;
    int         thermos;
    int         fault;
} ui_cooker_state_t;

typedef struct
{
    // lv_obj_t    * cont;
    lv_obj_t    * line;
    lv_obj_t    * anim;
    lv_obj_t    * state;
    lv_obj_t    * timing;
    lv_obj_t    * gear;
} ui_cooker_ctx_t;


extern ui_manager_t ui_manager;

// int cooker_ui_manager_init(void);

void cooker_ui_state_get(ui_cooker_state_t * state, int num);

void cooker_ui_state_set(const ui_cooker_state_t * state, int num);

void setting_cooker_channel_set(int ch);

void setting_timing_channel_set(int ch);

void cooker_ui_data_notify(void);

void update_main_scr_ui_data(const ui_cooker_state_t * state, int num);

#endif
