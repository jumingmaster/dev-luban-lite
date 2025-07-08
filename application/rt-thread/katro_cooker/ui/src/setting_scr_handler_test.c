// #include "app_ui_common.h"
// #include "cooker_obj.h"

// static cooker_obj_t * cooker = NULL;

// static const char max_gear_str[24] = "Maximum Firepower";
// static const char thermos_str[32] = "Heating Temperature\nStill ";


// static const int gear_timing_table[] = {
//     0, 8, 4, 4, 4, 2, 2, 2, 2, 2
// };


// void cooker_setting(cooker_obj_t * obj)
// {
//     cooker = obj;
//     lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
// }

// void setting_cook_custom_load_start(void)
// {
//     setting_cook_t *scr = setting_cook_get(&ui_manager);

//     if (cooker->state.on_max_gear)
//     {
//         lv_obj_add_flag(scr->gear_slider, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(scr->max_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(scr->min_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(scr->gear_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_set_style_border_width(scr->max_gear_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_obj_set_style_border_width(scr->thermos_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_label_set_text(scr->func_label, max_gear_str);
//         lv_obj_clear_flag(scr->func_label, LV_OBJ_FLAG_HIDDEN);
//     }
//     else if (cooker->state.thermos)
//     {
//         lv_obj_add_flag(scr->gear_slider, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(scr->max_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(scr->min_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(scr->gear_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_set_style_border_width(scr->thermos_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_obj_set_style_border_width(scr->max_gear_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_label_set_text_fmt(scr->func_label, "%s%02d:%02d", thermos_str, cooker->state.hour, cooker->state.minute);
//         lv_obj_clear_flag(scr->func_label, LV_OBJ_FLAG_HIDDEN);
//     }
//     else
//     {
//         lv_obj_add_flag(scr->func_label, LV_OBJ_FLAG_HIDDEN);

//         lv_label_set_text_fmt(scr->gear_label, "%d", cooker->state.gear);
//         lv_slider_set_value(scr->gear_slider, cooker->state.gear, LV_ANIM_OFF);
//         lv_obj_set_style_border_width(scr->max_gear_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_obj_set_style_border_width(scr->thermos_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

//         lv_obj_clear_flag(scr->gear_slider, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(scr->max_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(scr->min_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(scr->gear_label, LV_OBJ_FLAG_HIDDEN);
//     }
//     if (cooker->state.on_timing)
//     {
//         lv_obj_set_style_border_width(scr->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
//     }
//     else
//     {
//         lv_obj_set_style_border_width(scr->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//     }
// }

// void setting_cook_custom_unload_start(void) 
// {
//     cooker = NULL;
// }

// void setting_cook_gear_slider_custom_value_changed(void) 
// {
//     setting_cook_t *scr = setting_cook_get(&ui_manager);

//     int val = lv_slider_get_value(scr->gear_slider);

//     if (val > 9)
//     {
//         val = 9;
//     }
//     else if (val < 0)
//     {
//         val = 0;
//     }

//     // snprintf(str, 3, "%d", val);

//     lv_label_set_text_fmt(scr->gear_label, "%d", val);
// }

// void setting_cook_gear_slider_custom_released(void) 
// {
//     setting_cook_t *scr = setting_cook_get(&ui_manager);

//     int val = lv_slider_get_value(scr->gear_slider);

//     cooker_set_gear(cooker, val);

//     if (val == 0)
//     {
//         cooker_set_timing(cooker, 0, 0, 0);
//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//     }
//     else
//     {
//         cooker_set_timing(cooker, 1, gear_timing_table[val], 0);
//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
//     }

//     // update_main_scr_ui_data(&cur_state, cur_channel);
//     // reset_cooker_timer(cur_channel);
// }

// void setting_cook_min_label_custom_clicked(void) 
// {
//     setting_cook_t *scr = setting_cook_get(&ui_manager);
//     lv_slider_set_value(scr->gear_slider, 0, LV_ANIM_OFF);
//     lv_label_set_text(scr->gear_label, "0");

//     cooker_set_gear(cooker, 0);
//     cooker_set_timing(cooker, 0, 0, 0);
//     lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
// }

// void setting_cook_max_label_custom_clicked(void) 
// {
//     setting_cook_t *scr = setting_cook_get(&ui_manager);
//     lv_slider_set_value(scr->gear_slider, 9, LV_ANIM_OFF);
//     lv_label_set_text(scr->gear_label, "9");
//     cooker_set_gear(cooker, 9);
//     cooker_set_timing(cooker, 1, gear_timing_table[9], 0);
//     lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
// }

// void setting_cook_thermos_cont_custom_clicked(void) 
// {
//     setting_cook_t *scr = setting_cook_get(&ui_manager);

//     if (cooker->state.on_max_gear)
//     {
//         return;
//     }

//     if (cooker->state.thermos)
//     {
//         cooker_set_gear(cooker, 0);
//         cooker_set_thermos(cooker, 0);
//         cooker_set_timing(cooker, 0, 0, 0);

//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->thermos_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_obj_add_flag(setting_cook_get(&ui_manager)->func_label, LV_OBJ_FLAG_HIDDEN);

//         lv_slider_set_value(setting_cook_get(&ui_manager)->gear_slider, 1, LV_ANIM_OFF);
//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->gear_slider, LV_OBJ_FLAG_HIDDEN);

//         lv_label_set_text_fmt(setting_cook_get(&ui_manager)->gear_label, "%d", 1);
//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->gear_label, LV_OBJ_FLAG_HIDDEN);

//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->max_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->min_label, LV_OBJ_FLAG_HIDDEN);
//     }
//     else
//     {
//         cooker_set_gear(cooker, 1);
//         cooker_set_thermos(cooker, 1);
//         cooker_set_timing(cooker, 1, gear_timing_table[1], 0);

//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->thermos_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

//         lv_obj_add_flag(setting_cook_get(&ui_manager)->gear_slider, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(setting_cook_get(&ui_manager)->max_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(setting_cook_get(&ui_manager)->min_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(setting_cook_get(&ui_manager)->gear_label, LV_OBJ_FLAG_HIDDEN);

//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_label_set_text_fmt(setting_cook_get(&ui_manager)->func_label, "%s%02d:%02d", thermos_str, cooker->state.hour, cooker->state.minute);
//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->func_label, LV_OBJ_FLAG_HIDDEN);
//     }
// }

// void setting_cook_max_gear_cont_custom_clicked(void)
// {
//     setting_cook_t *scr = setting_cook_get(&ui_manager);

//     if (cooker->state.thermos)
//     {
//         return;
//     }

//     if (cooker->state.on_max_gear)
//     {
//         if (cooker->state.gear == 0)
//         {

//         }
//         cooker_set_gear(cooker, 0);
//         cooker_set_thermos(cooker, 0);
//         cooker_set_timing(cooker, 0, 0, 0);

//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->thermos_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_obj_add_flag(setting_cook_get(&ui_manager)->func_label, LV_OBJ_FLAG_HIDDEN);

//         lv_slider_set_value(setting_cook_get(&ui_manager)->gear_slider, 1, LV_ANIM_OFF);
//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->gear_slider, LV_OBJ_FLAG_HIDDEN);

//         lv_label_set_text_fmt(setting_cook_get(&ui_manager)->gear_label, "%d", 1);
//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->gear_label, LV_OBJ_FLAG_HIDDEN);

//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->max_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->min_label, LV_OBJ_FLAG_HIDDEN);
//     }
//     else
//     {
//         cooker_set_gear(cooker, 1);
//         cooker_set_thermos(cooker, 1);
//         cooker_set_timing(cooker, 1, gear_timing_table[1], 0);

//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->thermos_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);

//         lv_obj_add_flag(setting_cook_get(&ui_manager)->gear_slider, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(setting_cook_get(&ui_manager)->max_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(setting_cook_get(&ui_manager)->min_label, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(setting_cook_get(&ui_manager)->gear_label, LV_OBJ_FLAG_HIDDEN);

//         lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 10, LV_PART_MAIN | LV_STATE_DEFAULT);
//         lv_label_set_text_fmt(setting_cook_get(&ui_manager)->func_label, "%s%02d:%02d", thermos_str, cooker->state.hour, cooker->state.minute);
//         lv_obj_clear_flag(setting_cook_get(&ui_manager)->func_label, LV_OBJ_FLAG_HIDDEN);
//     }
// }

// void setting_cook_timing_cont_custom_clicked(void) 
// {
//     if (cooker->state.gear > 0)
//     {
//         lv_scr_load_anim(setting_timing_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
//     }
// }

// void setting_cook_back_cont_custom_clicked(void)
// {

//     lv_scr_load_anim(main_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
// }