#include "app_ui_common.h"
#include "lvgl.h"
#include <stdint.h>
#include <rtthread.h>


// static uint32_t my_get_tick_us_cb(void)
// {
//     return rt_tick_get();
// }

// static int my_get_tid_cb(void)
// {
//     return (int)rt_thread_self();
// }

// static int my_get_cpu_cb(void)
// {
//     return 0;
// }

// static void my_log_print_cb(const char * buf)
// {
//     printf("%s", buf);
//     // usbd_ep_start_write(0, buf, strlen(buf));
// }

// void my_profiler_init(void)
// {
    
//     lv_profiler_builtin_config_t config;
//     lv_profiler_builtin_config_init(&config);
//     config.tick_per_sec = 1000; /* 一秒等于1000000微秒 */
//     config.tick_get_cb = my_get_tick_us_cb;
//     config.tid_get_cb = my_get_tid_cb;
//     config.cpu_get_cb = my_get_cpu_cb;
//     config.flush_cb = my_log_print_cb;
//     lv_profiler_builtin_init(&config);
// }
// INIT_LATE_APP_EXPORT(my_profiler_init);







