// #include "app_ui_common.h"
// #include "cooker_obj.h"
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



// void cooker_setting(cooker_obj_t * obj)
// {
//     cooker_set_power(obj);
// }

// void cooker_pause(cooker_obj_t * obj)
// {

// }

// void cooker_resume(cooker_obj_t * obj)
// {

// }

// void cooker_stop(cooker_obj_t * obj)
// {

// }

// void cooker_set_gear(cooker_obj_t * obj, int gear)
// {
//     rt_mutex_take(obj->mtx, RT_WAITING_FOREVER);
//     obj->state.gear = gear;
//     rt_mutex_release(obj->mtx);
// }

// void cooker_set_timing(cooker_obj_t * obj, int on_timing, int hour, int minute)
// {
//     rt_mutex_take(obj->mtx, RT_WAITING_FOREVER);
//     obj->state.on_timing = on_timing;
//     obj->state.hour = hour;
//     obj->state.minute = minute;
//     obj->state.total_seconds = cooker_timing_cal_total_sec(obj->state.hour, obj->state.minute);
//     rt_mutex_release(obj->mtx);
// }

// void cooker_set_thermos(cooker_obj_t * obj, int thermos)
// {
//     rt_mutex_take(obj->mtx, RT_WAITING_FOREVER);
//     obj->state.thermos = thermos;
//     rt_mutex_release(obj->mtx);
// }

// void cooker_set_max_gear(cooker_obj_t * obj, int max_gear)
// {
//     rt_mutex_take(obj->mtx, RT_WAITING_FOREVER);
//     obj->state.on_max_gear = max_gear;
//     rt_mutex_release(obj->mtx);
// }







