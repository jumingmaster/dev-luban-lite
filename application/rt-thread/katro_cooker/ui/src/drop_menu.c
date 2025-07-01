#include "drop_menu.h"
#include "aic_core.h"


TCM_DATA_DEFINE static volatile int dropped = 0;

TCM_DATA_DEFINE static lv_obj_t * parent = NULL;


TCM_CODE_DEFINE static void anim_y_cb(void * var, int32_t v)
{
    lv_obj_set_y(var, v);
}

TCM_CODE_DEFINE static void anim_line_completed_cb(lv_anim_t * a)
{
    // rt_kprintf("Dropline animate completed, dropped=%d\r\n", dropped);
    if (parent)
    {
        lv_obj_set_parent(a->var, parent);
        // lv_obj_invalidate(lv_scr_act());
    }
}

TCM_CODE_DEFINE static void anim_menu_completed_cb(lv_anim_t * a)
{
    lv_obj_add_flag(a->var, LV_OBJ_FLAG_HIDDEN);
}


TCM_CODE_DEFINE void drop_menu_start(lv_obj_t * line, lv_obj_t * menu)
{
    dropped = (lv_obj_get_y2(line) >= LV_VER_RES) ? 1 : 0; 

    lv_obj_clear_flag(menu, LV_OBJ_FLAG_HIDDEN);
}

TCM_CODE_DEFINE void drop_menu_dropping_handler(lv_obj_t * line, lv_obj_t * menu)
{
    lv_point_t point = {0};
    lv_indev_get_point(lv_indev_get_act(), &point);

    // 如果菜单已经下拉，那么下拉线的父级对象就是下拉菜单，无须设置子对象的坐标
    if (!dropped)
    {
        lv_obj_set_y(line, point.y);
    }
    
    lv_obj_set_y(menu, point.y - LV_VER_RES);

    // rt_kprintf("line y = %d, menu y = %d\r\n", point.y, point.y - LV_VER_RES);
}


TCM_CODE_DEFINE void drop_menu_released_handler(lv_obj_t * main_scr, lv_obj_t * line, lv_obj_t * menu)
{
    lv_point_t point = {0};
    lv_anim_t roll_back_menu = {0};
    lv_anim_t roll_back_line = {0};
    int32_t line_end = 0;
    int32_t menu_end = 0;

    lv_indev_get_point(lv_indev_get_act(), &point);

    lv_anim_init(&roll_back_menu);
    lv_anim_init(&roll_back_line);

    lv_anim_set_var(&roll_back_menu, menu);
    lv_anim_set_var(&roll_back_line, line);

    lv_anim_set_duration(&roll_back_menu, DROP_MENU_ANIM_TIME);
    lv_anim_set_duration(&roll_back_line, DROP_MENU_ANIM_TIME);

    lv_anim_set_repeat_count(&roll_back_menu, 0);
    lv_anim_set_repeat_count(&roll_back_line, 0);

    lv_anim_set_exec_cb(&roll_back_menu, anim_y_cb);
    lv_anim_set_exec_cb(&roll_back_line, anim_y_cb);

    if (dropped)
    {
        if (point.y <= ROLLBACK_MAX)
        {
            line_end = 0;
            menu_end = -LV_VER_RES;
            lv_anim_set_path_cb(&roll_back_line, lv_anim_path_linear);
            lv_anim_set_path_cb(&roll_back_menu, lv_anim_path_linear);
            parent = main_scr;
            lv_anim_set_completed_cb(&roll_back_menu, anim_menu_completed_cb);
        }
        else
        {
            line_end = LV_VER_RES - lv_obj_get_height(line);
            menu_end = 0;
            lv_anim_set_path_cb(&roll_back_line, lv_anim_path_linear);
            lv_anim_set_path_cb(&roll_back_menu, lv_anim_path_linear);
            parent = menu;
        }
    }
    else
    {
        if (point.y < DROPPED_MAX)
        {
            line_end = 0;
            menu_end = -LV_VER_RES;
            lv_anim_set_path_cb(&roll_back_line, lv_anim_path_linear);
            lv_anim_set_path_cb(&roll_back_menu, lv_anim_path_linear);
            parent = main_scr;
        }
        else
        {
            line_end = LV_VER_RES - lv_obj_get_height(line);
            menu_end = 0;
            
            lv_anim_set_path_cb(&roll_back_line, lv_anim_path_linear);
            lv_anim_set_path_cb(&roll_back_menu, lv_anim_path_linear);
            parent = menu;
        }
    }
    lv_anim_set_completed_cb(&roll_back_line, anim_line_completed_cb);
    lv_anim_set_values(&roll_back_menu, lv_obj_get_y(menu), menu_end);
    lv_anim_set_values(&roll_back_line, lv_obj_get_y(line), line_end);
    lv_anim_start(&roll_back_menu);
    lv_anim_start(&roll_back_line);
}



