#include "app_ui_common.h"

TCM_DATA_DEFINE static ui_cooker_state_t cur_state = {0};
TCM_DATA_DEFINE static volatile int cur_channel = 0;


// TCM_CODE_DEFINE static void get_setting_timing(int * hour, int * min)
// {
//     char str[4] = {0};
//     // int sel_hour = 0;
//     // int sel_min = 0;

//     setting_timing_t *scr = setting_timing_get(&ui_manager);
    
//     lv_roller_get_selected_str(scr->roller_hour, str, sizeof(str));
//     *hour = atoi(str);
//     lv_roller_get_selected_str(scr->roller_minute, str, sizeof(str));
//     *min = atoi(str);

//     // rt_kprintf("Selected Hour:%d, Minute:%d\r\n", *hour, *min);
// }


TCM_CODE_DEFINE void setting_timing_custom_load_start(void) 
{
    setting_timing_t * scr = setting_timing_get(&ui_manager);

    cooker_ui_state_get(&cur_state, cur_channel);

    // rt_kprintf("Start setting cooker%d timing.\r\n", cur_channel);

    lv_roller_set_selected(scr->roller_hour, cur_state.hour, LV_ANIM_OFF);
    lv_roller_set_selected(scr->roller_minute, cur_state.minute, LV_ANIM_OFF);

}
TCM_CODE_DEFINE void setting_timing_custom_unload_start(void) 
{
    memset(&cur_state, 0x00, sizeof(ui_cooker_state_t));
    cur_channel = 0;
}


TCM_CODE_DEFINE void setting_timing_channel_set(int ch)
{
    cur_channel = ch;
}


TCM_CODE_DEFINE void setting_timing_cancel_cont_custom_clicked(void) 
{
    // lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    cur_state.on_timing = 0;
    cur_state.hour = 0;
    cur_state.minute = 0;
    
    setting_cooker_channel_set(cur_channel);

    update_main_scr_ui_data(&cur_state, cur_channel);
    
    cooker_ui_state_set(&cur_state, cur_channel);

    reset_cooker_timer(cur_channel);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_OVER_LEFT, 100, 0, false);
}

TCM_CODE_DEFINE void setting_timing_confirm_cont_custom_clicked(void) 
{
    // int hour = 0, min = 0;

    // get_setting_timing(&hour, &min);
    char str[4] = {0};
    // int sel_hour = 0;
    // int sel_min = 0;

    setting_timing_t *scr = setting_timing_get(&ui_manager);
    
    lv_roller_get_selected_str(scr->roller_hour, str, sizeof(str));
    cur_state.hour = atoi(str);
    lv_roller_get_selected_str(scr->roller_minute, str, sizeof(str));
    cur_state.minute = atoi(str);

    // cur_state.hour = hour;
    // cur_state.minute = min;

    if (cur_state.hour + cur_state.minute > 0)
    {
        cur_state.on_timing = 1;
        // lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
        // rt_kprintf("Selected Hour:%d, Minute:%d\r\n", cur_state.hour, cur_state.minute);
        cur_state.total_seconds = cooker_timing_cal_total_sec(cur_state.hour, cur_state.minute);
    }
    // else
    // {
    //     cur_state.hour = 0;
    //     cur_state.minute = 0;
    //     cur_state.on_timing = 0;
        // lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    // }

    update_main_scr_ui_data(&cur_state, cur_channel);

    cooker_ui_state_set(&cur_state, cur_channel);

    setting_cooker_channel_set(cur_channel);

    reset_cooker_timer(cur_channel);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_OVER_RIGHT, 100, 0, false);
}
