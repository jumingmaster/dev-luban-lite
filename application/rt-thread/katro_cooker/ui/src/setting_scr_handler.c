#include "app_ui_common.h"
#include "fps_test.h"

TCM_DATA_DEFINE static ui_cooker_state_t cur_state = {0};
TCM_DATA_DEFINE static volatile int cur_channel = 0;

static const char max_gear_str[24] = "Maximum Firepower";
static const char thermos_str[32] = "Heating Temperature\nStill ";

// static const char max_gear_str_cn[24] = "最大火力";
// static const char thermos_str_cn[32] = "保温  ";




static const int gear_timing_table[] = {
    0, 8, 4, 4, 4, 2, 2, 2, 2, 2
};


TCM_CODE_DEFINE void setting_cook_custom_load_start(void) 
{
    char str[4] = {0};

    setting_cook_t *scr = setting_cook_get(&ui_manager);

    cooker_ui_state_get(&cur_state, cur_channel);

    // rt_kprintf("Start setting cooker%d.\r\n", cur_channel);

    if (cur_state.on_max_gear)
    {
        lv_obj_add_flag(scr->gear_slider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->max_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->min_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->gear_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_border_width(scr->max_gear_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(scr->thermos_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(scr->func_label, max_gear_str);
        lv_obj_clear_flag(scr->func_label, LV_OBJ_FLAG_HIDDEN);
    }
    else if (cur_state.thermos)
    {
        lv_obj_add_flag(scr->gear_slider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->max_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->min_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->gear_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_border_width(scr->thermos_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(scr->max_gear_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text_fmt(scr->func_label, "%s%02d:%02d", thermos_str, cur_state.hour, cur_state.minute);
        lv_obj_clear_flag(scr->func_label, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_add_flag(scr->func_label, LV_OBJ_FLAG_HIDDEN);

        snprintf(str, sizeof(str), "%d", cur_state.gear);
        lv_label_set_text(scr->gear_label, str);
        lv_slider_set_value(scr->gear_slider, cur_state.gear, LV_ANIM_OFF);
        lv_obj_set_style_border_width(scr->max_gear_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_set_style_border_width(scr->thermos_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

        lv_obj_clear_flag(scr->gear_slider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(scr->max_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(scr->min_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(scr->gear_label, LV_OBJ_FLAG_HIDDEN);
    }
    if (cur_state.on_timing)
    {
        // rt_kprintf("Current timing %02d:%02d.\r\n", cur_state.hour, cur_state.minute);
        lv_obj_set_style_border_width(scr->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
        lv_obj_set_style_border_width(scr->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
}

TCM_CODE_DEFINE void setting_cook_custom_unload_start(void) 
{
    memset(&cur_state, 0x00, sizeof(ui_cooker_state_t));
    cur_channel = 0;
}

TCM_CODE_DEFINE void setting_cooker_channel_set(int ch)
{
    cur_channel = ch;
}



TCM_CODE_DEFINE void setting_cook_gear_slider_custom_value_changed(void) 
{
    // char str[3] = {0};
    
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

    // snprintf(str, 3, "%d", val);

    lv_label_set_text_fmt(scr->gear_label, "%d", val);

    cur_state.gear = val;
}

TCM_CODE_DEFINE void setting_cook_gear_slider_custom_released(void) 
{
    if (cur_state.gear == 0)
    {
        cur_state.on_timing = 0;
        cur_state.hour = 0;
        cur_state.minute = 0;
        cur_state.total_seconds = 0;
        lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    }
    else
    {
        cur_state.on_timing = 1;
        cur_state.hour = gear_timing_table[cur_state.gear];
        cur_state.minute = 0;
        cur_state.total_seconds = cooker_timing_cal_total_sec(cur_state.hour, cur_state.minute);
        lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        // rt_kprintf("setting_cook_gear_slider_custom_released");
    }

    update_main_scr_ui_data(&cur_state, cur_channel);
    reset_cooker_timer(cur_channel);
}

TCM_CODE_DEFINE void setting_cook_min_label_custom_clicked(void) 
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

    cur_state.on_timing = 0;
    cur_state.hour = 0;
    cur_state.minute = 0;
    cur_state.total_seconds = 0;
    lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

    update_main_scr_ui_data(&cur_state, cur_channel);
    reset_cooker_timer(cur_channel);
}

TCM_CODE_DEFINE void setting_cook_max_label_custom_clicked(void) 
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
    cur_state.on_timing = 1;
    cur_state.hour = gear_timing_table[9];
    cur_state.minute = 0;
    cur_state.total_seconds = cooker_timing_cal_total_sec(cur_state.hour, cur_state.minute);
    lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
    update_main_scr_ui_data(&cur_state, cur_channel);
    reset_cooker_timer(cur_channel);
}



TCM_CODE_DEFINE void setting_cook_back_cont_custom_clicked(void) 
{
    update_main_scr_ui_data(&cur_state, cur_channel);
    cooker_ui_state_set(&cur_state, cur_channel);
    // setting_timing_channel_set(cur_channel);
    
    lv_scr_load_anim(main_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 150, 0, false);   
}

TCM_CODE_DEFINE void setting_cook_thermos_cont_custom_clicked(void) 
{
    if (cur_state.on_max_gear)
    {
        return;
    }
    cur_state.thermos = cur_state.thermos ? 0 : 10;
    lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->thermos_cont, cur_state.thermos, LV_PART_MAIN | LV_STATE_DEFAULT);
    

    if (cur_state.thermos)
    {
        cur_state.gear = 1;
        cur_state.hour = gear_timing_table[cur_state.gear];
        cur_state.minute = 0;
        cur_state.on_timing = 1;
        cur_state.total_seconds = cooker_timing_cal_total_sec(cur_state.hour, cur_state.minute);

        lv_obj_add_flag(setting_cook_get(&ui_manager)->gear_slider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(setting_cook_get(&ui_manager)->max_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(setting_cook_get(&ui_manager)->min_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(setting_cook_get(&ui_manager)->gear_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text_fmt(setting_cook_get(&ui_manager)->func_label, "%s%02d:%02d", thermos_str, cur_state.hour, cur_state.minute);
        lv_obj_clear_flag(setting_cook_get(&ui_manager)->func_label, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        cur_state.gear = 0;
        cur_state.hour = 0;
        cur_state.minute = 0;
        cur_state.on_timing = 0;
        cur_state.total_seconds = 0;
        
        lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_obj_add_flag(setting_cook_get(&ui_manager)->func_label, LV_OBJ_FLAG_HIDDEN);

        lv_slider_set_value(setting_cook_get(&ui_manager)->gear_slider, cur_state.gear, LV_ANIM_OFF);
        lv_obj_clear_flag(setting_cook_get(&ui_manager)->gear_slider, LV_OBJ_FLAG_HIDDEN);

        lv_label_set_text_fmt(setting_cook_get(&ui_manager)->gear_label, "%d", cur_state.gear);
        lv_obj_clear_flag(setting_cook_get(&ui_manager)->gear_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(setting_cook_get(&ui_manager)->max_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(setting_cook_get(&ui_manager)->min_label, LV_OBJ_FLAG_HIDDEN);

        
    }

    update_main_scr_ui_data(&cur_state, cur_channel);
    cooker_ui_state_set(&cur_state, cur_channel);
    reset_cooker_timer(cur_channel);
}

TCM_CODE_DEFINE void setting_cook_timing_cont_custom_clicked(void) 
{
    if (cur_state.gear > 0)
    {
        update_main_scr_ui_data(&cur_state, cur_channel);
        cooker_ui_state_set(&cur_state, cur_channel);
        setting_timing_channel_set(cur_channel);
        lv_scr_load_anim(setting_timing_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 100, 0, false);
    }
}

TCM_CODE_DEFINE void setting_cook_max_gear_cont_custom_clicked(void) 
{
    if (cur_state.thermos)
    {
        return;
    }

    cur_state.on_max_gear = cur_state.on_max_gear ? 0 : 10;
    lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->max_gear_cont, cur_state.on_max_gear, LV_PART_MAIN | LV_STATE_DEFAULT);

    if (cur_state.on_max_gear)
    {
        cur_state.hour = 0;
        cur_state.minute = 5;
        cur_state.on_timing = 1;
        cur_state.total_seconds = cooker_timing_cal_total_sec(cur_state.hour, cur_state.minute);
        lv_obj_add_flag(setting_cook_get(&ui_manager)->gear_slider, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(setting_cook_get(&ui_manager)->max_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(setting_cook_get(&ui_manager)->min_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(setting_cook_get(&ui_manager)->gear_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        lv_label_set_text(setting_cook_get(&ui_manager)->func_label, max_gear_str);
        lv_obj_clear_flag(setting_cook_get(&ui_manager)->func_label, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        if (cur_state.gear == 0)
        {
            cur_state.gear = 9;
        }

        cur_state.hour = gear_timing_table[cur_state.gear];
        cur_state.minute = 0;
        lv_slider_set_value(setting_cook_get(&ui_manager)->gear_slider, cur_state.gear, LV_ANIM_OFF);
        lv_obj_clear_flag(setting_cook_get(&ui_manager)->gear_slider, LV_OBJ_FLAG_HIDDEN);

        lv_label_set_text_fmt(setting_cook_get(&ui_manager)->gear_label, "%d", cur_state.gear);
        lv_obj_clear_flag(setting_cook_get(&ui_manager)->gear_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_clear_flag(setting_cook_get(&ui_manager)->max_label, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(setting_cook_get(&ui_manager)->min_label, LV_OBJ_FLAG_HIDDEN);

        lv_obj_add_flag(setting_cook_get(&ui_manager)->func_label, LV_OBJ_FLAG_HIDDEN);
    }

    update_main_scr_ui_data(&cur_state, cur_channel);
    cooker_ui_state_set(&cur_state, cur_channel);
    reset_cooker_timer(cur_channel);
}