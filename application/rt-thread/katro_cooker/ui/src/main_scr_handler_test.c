// #include "app_ui_common.h"
// #include "app_common.h"
// #include "cooker_protocol.h"

// static const char *cooker_anim_imgs[16] = {
//     LVGL_IMAGE_PATH(1.png),
//     LVGL_IMAGE_PATH(2.png),
//     LVGL_IMAGE_PATH(3.png),
//     LVGL_IMAGE_PATH(4_410x322.png),
//     LVGL_IMAGE_PATH(5.png),
//     LVGL_IMAGE_PATH(6.png),
//     LVGL_IMAGE_PATH(7.png),
//     LVGL_IMAGE_PATH(8.png),
//     LVGL_IMAGE_PATH(9.png),
//     LVGL_IMAGE_PATH(10.png),
//     LVGL_IMAGE_PATH(11.png),
//     LVGL_IMAGE_PATH(12_410x322.png),
//     LVGL_IMAGE_PATH(13.png),
//     LVGL_IMAGE_PATH(14.png),
//     LVGL_IMAGE_PATH(15.png),
//     LVGL_IMAGE_PATH(16.png),
// };

// static const char *merge_anim_imgs[16] = {
//     LVGL_IMAGE_PATH(merge1_410x660.png),
//     LVGL_IMAGE_PATH(merge2_410x660.png),
//     LVGL_IMAGE_PATH(merge3_410x660.png),
//     LVGL_IMAGE_PATH(merge4_410x660.png),
//     LVGL_IMAGE_PATH(merge5_410x660.png),
//     LVGL_IMAGE_PATH(merge6_410x660.png),
//     LVGL_IMAGE_PATH(merge7_410x660.png),
//     LVGL_IMAGE_PATH(merge8_410x660.png),
//     LVGL_IMAGE_PATH(merge9_410x660.png),
//     LVGL_IMAGE_PATH(merge10_410x660.png),
//     LVGL_IMAGE_PATH(merge11_410x660.png),
//     LVGL_IMAGE_PATH(merge12_410x660.png),
//     LVGL_IMAGE_PATH(merge13_410x660.png),
//     LVGL_IMAGE_PATH(merge14_410x660.png),
//     LVGL_IMAGE_PATH(merge15_410x660.png),
//     LVGL_IMAGE_PATH(merge16_410x660.png),
// };

// static lv_obj_t * cpu_temp_obj = NULL;
// static lv_timer_t * cooker_data_tmr = NULL;
// static lv_timer_t * cpu_temp_tmr = NULL;
// static ui_cooker_state_t cur_cooker_state[UI_COOKER_NUM] = {0};
// static ui_cooker_ctx_t cooker_ui[UI_COOKER_NUM] = {0};
// static ui_cooker_ctx_t merge_cooker_ui[2] = {0};
// static cooker_hw_t cur_cooker_hw_state[UI_COOKER_NUM] = {0};
// static volatile int main_scr_glb_pause = 0;
// static rt_mutex_t cur_state_mtx[UI_COOKER_NUM] = {0};

// static const char main_ui_mtx_name[UI_COOKER_NUM][16] = {
//     {"cur_state_mtx0"},
//     {"cur_state_mtx1"},
//     {"cur_state_mtx2"},
//     {"cur_state_mtx3"}
// };

// static const int gear_timing_table[] = {
//     0, 8, 4, 4, 4, 2, 2, 2, 2, 2
// };



// static void cpu_temperature_sample(lv_timer_t * tmr)
// {
//     int temp = 0;

//     if (cpu_temp_get(&temp) == 0)
//     {
//         lv_label_set_text_fmt(cpu_temp_obj, "cpu temp:%3d.%d", 
//                                 temp / 10, temp % 10);
//     }
// }

// static void cooker_data_timer_handler(lv_timer_t * tmr)
// {
//     if (cur_cooker_state[0].on_merge)
//     {
//         cur_cooker_state[1].gear = cur_cooker_state[0].gear;
//         cur_cooker_state[1].on_max_gear = cur_cooker_state[0].on_max_gear;
//         cur_cooker_state[1].thermos = cur_cooker_state[0].thermos;
//         cur_cooker_state[1].pause = cur_cooker_state[0].pause;
//     }
//     if (cur_cooker_state[2].on_merge)
//     {
//         cur_cooker_state[3].gear = cur_cooker_state[3].gear;
//         cur_cooker_state[3].on_max_gear = cur_cooker_state[3].on_max_gear;
//         cur_cooker_state[3].thermos = cur_cooker_state[3].thermos;
//         cur_cooker_state[3].pause = cur_cooker_state[3].pause;
//     }
    
//     for (int i = 0; i < UI_COOKER_NUM; i++)
//     {
//         if (rt_mutex_take(cur_state_mtx[i], RT_WAITING_NO) == RT_EOK)
//         {
//             cooker_ui_state_set(&cur_cooker_state[i], i);
//             rt_mutex_release(cur_state_mtx[i]);
//         }

//         cooker_ui_read_hw_data(i, &cur_cooker_hw_state[i]);
//     }

//     cooker_ui_data_notify();
// }






// static void set_global_run(void)
// {
//     lv_img_set_src(main_screen_get(&ui_manager)->global_pause, LVGL_IMAGE_PATH(suspend_300x300.png));
//     lv_obj_clear_flag(main_screen_get(&ui_manager)->global_pause, LV_OBJ_FLAG_HIDDEN);
// }

// static void set_global_pause(void)
// {
//     lv_img_set_src(main_screen_get(&ui_manager)->global_pause, LVGL_IMAGE_PATH(start_300x300.png));
//     lv_obj_clear_flag(main_screen_get(&ui_manager)->global_pause, LV_OBJ_FLAG_HIDDEN);
// }

// static void set_global_idle(void)
// {
//     lv_obj_add_flag(main_screen_get(&ui_manager)->global_pause, LV_OBJ_FLAG_HIDDEN);
//     main_scr_glb_pause = 0;
// }

// static int global_idle_check(void)
// {
//     if (cur_cooker_state[0].on_merge)
//     {
//         if (cur_cooker_state[0].on_max_gear || cur_cooker_state[0].gear ||
//             cur_cooker_state[0].on_timing || cur_cooker_state[0].thermos)
//         {
//             return 0;
//         }
//     }
//     if (cur_cooker_state[2].on_merge)
//     {
//         if (cur_cooker_state[2].on_max_gear || cur_cooker_state[2].gear ||
//             cur_cooker_state[2].on_timing || cur_cooker_state[2].thermos)
//         {
//             return 0;
//         }
//     }

//     for (int i = 0; i < UI_COOKER_NUM; i++)
//     {
//         if (cur_cooker_state[i].on_max_gear || cur_cooker_state[i].gear ||
//             cur_cooker_state[i].on_timing || cur_cooker_state[i].thermos)
//         {
//             return 0;
//         }
//     }

//     return 1;
// }

//  static void cooker_set_idle(int ch, int merge)
// {
//     if (merge)
//     {
//         cur_cooker_state[ch].hour = 0;
//         cur_cooker_state[ch].minute = 0;
//         cur_cooker_state[ch].on_timing = 0;
//         cur_cooker_state[ch].total_seconds = 0;
//         cur_cooker_state[ch].gear = 0;
//         lv_obj_clear_flag(cooker_ui[ch].line, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(cooker_ui[ch].anim, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(cooker_ui[ch].timing, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
//     }
//     else
//     {
//         cur_cooker_state[ch].hour = 0;
//         cur_cooker_state[ch].minute = 0;
//         cur_cooker_state[ch].on_timing = 0;
//         cur_cooker_state[ch].total_seconds = 0;
//         cur_cooker_state[ch].gear = 0;
//         lv_obj_clear_flag(cooker_ui[ch].line, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(cooker_ui[ch].anim, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(cooker_ui[ch].timing, LV_OBJ_FLAG_HIDDEN);
//         lv_obj_add_flag(cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
//     }

//     if (global_pause_check())
//     {
//         set_global_idle();
//     }
// }


// static void cooker_timing_handler(int ch, ui_cooker_ctx_t * ctx)
// {
//     ui_cooker_state_t cooker_state = {0};

//     cooker_ui_state_get(&cooker_state, ch);
//     if (cooker_state.on_timing)
//     {
//         if (++ctx->cur_second >= cooker_state.total_seconds)
//         {
//             if (cooker_state.on_max_gear)
//             {
//                 cooker_state.hour = 2;
//                 cooker_state.minute = 0;
//                 cooker_state.gear = cooker_state.gear ? cooker_state.gear : 9;
//                 cooker_state.on_max_gear = 0;
//                 ctx->remain_hour = gear_timing_table[2];
//                 ctx->remain_minute = 0;
//                 ctx->cur_second = 0;
//                 cooker_state.total_seconds = cooker_timing_cal_total_sec(ctx->remain_hour, ctx->remain_minute);
//                 cooker_ui_state_set(&cooker_state, ch);
//                 lv_label_set_text_fmt(ctx->gear, "%d", cooker_state.gear);
//                 lv_obj_clear_flag(ctx->gear, LV_OBJ_FLAG_HIDDEN);
//                 lv_obj_add_flag(ctx->state, LV_OBJ_FLAG_HIDDEN);
//                 lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
//             }
//             else
//             {
//                 cooker_set_idle(ch, cooker_state.on_merge);
//             }
//         }
//         else
//         {
//             if (ctx->cur_second % 60 == 0)
//             {
//                 ctx->remain_minute--;
//             }
//             if (ctx->remain_minute < 0)
//             {
//                 ctx->remain_minute = 59;
//                 ctx->remain_hour--;
//             }

//             lv_label_set_text_fmt(ctx->timing, "%02d:%02d", ctx->remain_hour, ctx->remain_minute);
//         }
//     }
// }


// static void cooker_timer_handler(lv_timer_t * tmr)
// {
//     for (int i = 0; i < UI_COOKER_NUM; i++)
//     {
//         if (tmr == cooker_ui[i].timer && cooker_ui[i].timing)
//         {
//             cooker_timing_handler(i, &cooker_ui[i]);
//         }
//     }
// }

// static void merge_cooker_timer_handler(lv_timer_t * tmr)
// {
//     for (int i = 0; i < 2; i++)
//     {
//         if (tmr == merge_cooker_ui[i].timer && merge_cooker_ui[i].timing)
//         {
//             cooker_timing_handler(i, &merge_cooker_ui[i]);
//         }
//     }
// }


// static void stop_cooker_anim(int ch)
// {
//     if (cur_cooker_state[ch].on_merge)
//     {
//         lv_animimg_set_src(merge_cooker_ui[ch].anim, (const void **)merge_anim_imgs, 1);
//         lv_animimg_set_repeat_count(merge_cooker_ui[ch].anim, 0);
//     }
//     else
//     {
//         lv_animimg_set_src(cooker_ui[ch].anim, (const void **)cooker_anim_imgs, 1);
//         lv_animimg_set_repeat_count(cooker_ui[ch].anim, 0);
//     }
// }
