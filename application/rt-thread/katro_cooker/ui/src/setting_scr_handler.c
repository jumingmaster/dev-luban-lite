#include "app_ui_common.h"
#include "fps_test.h"

static ui_cooker_state_t cur_state = {0};
static volatile int cur_channel = 0;


void setting_cook_custom_load_start(void) 
{
    char str[4] = {0};

    setting_cook_t *scr = setting_cook_get(&ui_manager);

    cooker_ui_state_get(&cur_state, cur_channel);

    // rt_kprintf("Start setting cooker%d.\r\n", cur_channel);

    if (cur_state.on_max_gear)
    {

    }
    else if (cur_state.thermos)
    {

    }
    else
    {
        snprintf(str, sizeof(str), "%d", cur_state.gear);
        lv_label_set_text(scr->gear_label, str);
        lv_slider_set_value(scr->gear_slider, cur_state.gear, LV_ANIM_OFF);
    }
    if (cur_state.on_timing)
    {
        lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
        lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

void setting_cook_custom_unload_start(void) 
{
    memset(&cur_state, 0x00, sizeof(ui_cooker_state_t));
    cur_channel = 0;
}

void setting_cooker_channel_set(int ch)
{
    cur_channel = ch;
}



void setting_cook_gear_slider_custom_value_changed(void) 
{
    char str[3] = {0};
    
    setting_cook_t *scr = setting_cook_get(&ui_manager);

    if (!ui_manager.auto_del || !scr->gear_slider || !scr->gear_label )
    {
        return;
    }

    int val = lv_slider_get_value(scr->gear_slider);

    if (val > 9)
    {
        val = 9;
    }
    else if (val < 0)
    {
        val = 0;
    }

    snprintf(str, 3, "%d", val);

    lv_label_set_text(scr->gear_label, str);

    cur_state.gear = val;
}

void setting_cook_min_label_custom_clicked(void) 
{
    setting_cook_t *scr = setting_cook_get(&ui_manager);

    if (!ui_manager.auto_del || !scr->gear_slider || 
        !scr->min_label || !scr->gear_label )
    {
        return;
    }
    lv_slider_set_value(scr->gear_slider, 0, LV_ANIM_OFF);
    lv_label_set_text(scr->gear_label, "0");
    cur_state.gear = 0;
    cooker_ui_state_set(&cur_state, cur_channel);
}

void setting_cook_max_label_custom_clicked(void) 
{
    setting_cook_t *scr = setting_cook_get(&ui_manager);

    if (!ui_manager.auto_del || !scr->gear_slider || 
        !scr->max_label || !scr->gear_label )
    {
        return;
    }

    lv_slider_set_value(scr->gear_slider, 9, LV_ANIM_OFF);
    lv_label_set_text(scr->gear_label, "9");
    cur_state.gear = 9;
    cooker_ui_state_set(&cur_state, cur_channel);
}



void setting_cook_back_cont_custom_clicked(void) 
{
    cooker_ui_state_set(&cur_state, cur_channel);
    setting_timing_channel_set(cur_channel);
    lv_scr_load_anim(main_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 100, 0, false);   
}

void setting_cook_thermos_cont_custom_clicked(void) 
{
    // cur_state.thermos = cur_state.thermos ? 0 : 10;
    // lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->thermos_cont, cur_state.thermos, LV_PART_MAIN | LV_STATE_DEFAULT);
    // cooker_ui_state_set(&cur_state, cur_channel);
}

void setting_cook_timing_cont_custom_clicked(void) 
{
    if (cur_state.gear > 0)
    {
        cooker_ui_state_set(&cur_state, cur_channel);
        setting_timing_channel_set(cur_channel);
        lv_scr_load_anim(setting_timing_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, false);
    }
}

void setting_cook_max_gear_cont_custom_clicked(void) 
{
    // cur_state.on_max_gear = cur_state.on_max_gear ? 0 : 10;
    // lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->max_gear_cont, cur_state.on_max_gear, LV_PART_MAIN | LV_STATE_DEFAULT);
    // cooker_ui_state_set(&cur_state, cur_channel);
}