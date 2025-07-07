#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"
#include "rtthread.h"
#include "lv_anim.h"

#define DROPPED_MAX     (LV_VER_RES / 2 + 100)
#define ROLLBACK_MAX    (LV_VER_RES / 2 + 100)

#define DROP_MENU_ANIM_TIME     (150)

void drop_menu_start(lv_obj_t * line, lv_obj_t * menu);

void drop_menu_dropping_handler(lv_obj_t * line, lv_obj_t * menu);

void drop_menu_released_handler(lv_obj_t * main_scr, lv_obj_t * line, lv_obj_t * menu);

void drop_menu_return(lv_obj_t * main_scr, lv_obj_t * line, lv_obj_t * menu, void (*ext_cb)(void));

void drop_window_up(void);

void drop_window_down(void);
