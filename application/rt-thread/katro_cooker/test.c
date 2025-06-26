#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"
#include "rtthread.h"
#include "lv_anim.h"

#define LV_ANIM_REF(obj)    (&((lv_animimg_t *)scr->logo_amni)->anim)

extern ui_manager_t ui_manager;

static int val = 0;

static void logo_anim_ready_cb(lv_anim_t * a)
{
    lv_obj_t *act_scr = lv_scr_act();

    if (!screen_is_loading(act_scr))
    {
        // delete child obj of act_scr
        lv_obj_clean(act_scr);
        // create main_screen
        main_screen_create(&ui_manager);
        // load main_screen
        lv_scr_load_anim(main_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_OUT_BOTTOM, 200, 0, true);
    }
}

void logo_scr_custom_created(void) 
{
    logo_scr_t *scr = logo_scr_get(&ui_manager);

    if (!ui_manager.auto_del && scr->logo_amni)
    {
        return;
    }
    
    lv_anim_set_completed_cb(LV_ANIM_REF(scr->logo_amni), logo_anim_ready_cb);
}






void main_screen_dropline_custom_scroll(void)
{
    rt_kprintf("Drop line request\r\n");
}


void main_screen_dropline_custom_gesture(void)
{
    rt_kprintf("Drop line gesture event\r\n");
}



void main_screen_button_1_custom_clicked(void)
{
    char str[32] = {0};

    main_screen_t *scr = main_screen_get(&ui_manager);

    if (!ui_manager.auto_del || !scr->label_1)
    {
        return;
    }

    snprintf(str, 32, "%d", ++val);

    lv_label_set_text(scr->label_1, str);
}






void main_screen_dropline_custom_pressing(void) 
{
    lv_point_t point = {0};

    lv_indev_get_point(lv_indev_get_act(), &point);

    rt_kprintf("indev[%d:%d]\r\n", point.x, point.y);
}
void main_screen_dropline_custom_released(void) 
{
    lv_point_t point = {0};

    lv_indev_get_point(lv_indev_get_act(), &point);

    rt_kprintf("indev[%d:%d]\r\n", point.x, point.y);
}