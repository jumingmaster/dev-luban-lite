#include "app_ui_common.h"
#include "app_common.h"
#include "cooker_protocol.h"

static const char *cooker_anim_imgs[16] = {
    LVGL_IMAGE_PATH(1.png),
    LVGL_IMAGE_PATH(2.png),
    LVGL_IMAGE_PATH(3.png),
    LVGL_IMAGE_PATH(4_410x322.png),
    LVGL_IMAGE_PATH(5.png),
    LVGL_IMAGE_PATH(6.png),
    LVGL_IMAGE_PATH(7.png),
    LVGL_IMAGE_PATH(8.png),
    LVGL_IMAGE_PATH(9.png),
    LVGL_IMAGE_PATH(10.png),
    LVGL_IMAGE_PATH(11.png),
    LVGL_IMAGE_PATH(12_410x322.png),
    LVGL_IMAGE_PATH(13.png),
    LVGL_IMAGE_PATH(14.png),
    LVGL_IMAGE_PATH(15.png),
    LVGL_IMAGE_PATH(16.png),
};

static const char *merge_anim_imgs[16] = {
    LVGL_IMAGE_PATH(merge1_410x660.png),
    LVGL_IMAGE_PATH(merge2_410x660.png),
    LVGL_IMAGE_PATH(merge3_410x660.png),
    LVGL_IMAGE_PATH(merge4_410x660.png),
    LVGL_IMAGE_PATH(merge5_410x660.png),
    LVGL_IMAGE_PATH(merge6_410x660.png),
    LVGL_IMAGE_PATH(merge7_410x660.png),
    LVGL_IMAGE_PATH(merge8_410x660.png),
    LVGL_IMAGE_PATH(merge9_410x660.png),
    LVGL_IMAGE_PATH(merge10_410x660.png),
    LVGL_IMAGE_PATH(merge11_410x660.png),
    LVGL_IMAGE_PATH(merge12_410x660.png),
    LVGL_IMAGE_PATH(merge13_410x660.png),
    LVGL_IMAGE_PATH(merge14_410x660.png),
    LVGL_IMAGE_PATH(merge15_410x660.png),
    LVGL_IMAGE_PATH(merge16_410x660.png),
};



static lv_timer_t * main_scr_tmr = NULL;

static lv_timer_t * cpu_temp_tmr = NULL;

static ui_cooker_state_t cur_cooker_state[UI_COOKER_NUM] = {0};

static ui_cooker_ctx_t cooker_ui[UI_COOKER_NUM] = {0};

static ui_cooker_ctx_t merge_cooker_ui[2] = {0};

static cooker_hw_t cur_cooker_hw_state[UI_COOKER_NUM] = {0};

static volatile int main_scr_glb_pause = 0;

static volatile int main_scr_running_cooker = 0;

static lv_obj_t * cpu_temp_obj = NULL;



static rt_mutex_t cur_state_mtx[UI_COOKER_NUM] = {0};

static const char main_ui_mtx_name[UI_COOKER_NUM][16] = {
    {"cur_state_mtx0"},
    {"cur_state_mtx1"},
    {"cur_state_mtx2"},
    {"cur_state_mtx3"}
};


static const int gear_timing_table[] = {
    0, 8, 4, 4, 4, 2, 2, 2, 2, 2
};



static void list_cooker_ui_state(int argc, char **argv)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        rt_kprintf("\r\ncooker%d state: \r\n", i);
        rt_kprintf("gear:%d\ton max gear:%d\tthermos:%d\r\n", cur_cooker_state[i].gear, cur_cooker_state[i].on_max_gear, cur_cooker_state[i].thermos);
        rt_kprintf("on_timing:%d\tpause:%d\tfault:%d\r\n", cur_cooker_state[i].on_timing, cur_cooker_state[i].pause, cur_cooker_state[i].fault);
    }
}
MSH_CMD_EXPORT(list_cooker_ui_state, List all cooker ui state);




static void main_scr_timer_handler(lv_timer_t * tmr)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        if (rt_mutex_take(cur_state_mtx[i], RT_WAITING_NO) == RT_EOK)
        {
            cooker_ui_state_set(&cur_cooker_state[i], i);
            rt_mutex_release(cur_state_mtx[i]);
        }

        cooker_ui_read_hw_data(i, &cur_cooker_hw_state[i]);
    }

    cooker_ui_data_notify();
}

static void cpu_temperature_sample(lv_timer_t * tmr)
{
    int temp = 0;

    if (cpu_temp_get(&temp) == 0)
    {
        lv_label_set_text_fmt(cpu_temp_obj, "cpu temp:%3d.%d", 
                                temp / 10, temp % 10);
    }
}


 static void cooker_set_idle(int ch)
{
    rt_kprintf("Thread[%s] set cooker%d idle\r\n", rt_thread_self()->name, ch);
    cur_cooker_state[ch].hour = 0;
    cur_cooker_state[ch].minute = 0;
    cur_cooker_state[ch].on_timing = 0;
    cur_cooker_state[ch].total_seconds = 0;
    cur_cooker_state[ch].gear = 0;
    lv_obj_clear_flag(cooker_ui[ch].line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cooker_ui[ch].anim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cooker_ui[ch].timing, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);

    if (--main_scr_running_cooker <= 0)
    {
        lv_obj_add_flag(main_screen_get(&ui_manager)->global_pause, LV_OBJ_FLAG_HIDDEN);
        main_scr_glb_pause = 0;
        main_scr_running_cooker = 0;
    }
}

 static void merge_cooker_set_idle(int ch)
{
    if (ch == 1)
    {
        ch = 2;
    }

    cur_cooker_state[ch].hour = 0;
    cur_cooker_state[ch].minute = 0;
    cur_cooker_state[ch].on_timing = 0;
    cur_cooker_state[ch].total_seconds = 0;
    cur_cooker_state[ch].gear = 0;
    lv_obj_clear_flag(merge_cooker_ui[ch].line, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(merge_cooker_ui[ch].anim, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(merge_cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(merge_cooker_ui[ch].timing, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(merge_cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);

    if (--main_scr_running_cooker <= 0)
    {
        lv_obj_add_flag(main_screen_get(&ui_manager)->global_pause, LV_OBJ_FLAG_HIDDEN);
        main_scr_glb_pause = 0;
        main_scr_running_cooker = 0;
    }
}



 static void cooker_timing_handler(int ch, ui_cooker_ctx_t * ctx)
{
    ui_cooker_state_t cooker_state = {0};

    cooker_ui_state_get(&cooker_state, ch);
    if (cooker_state.on_timing)
    {
        if (++ctx->cur_second >= cooker_state.total_seconds)
        {
            if (cooker_state.on_max_gear)
            {
                cooker_state.hour = 2;
                cooker_state.minute = 0;
                cooker_state.gear = cooker_state.gear ? cooker_state.gear : 9;
                cooker_state.on_max_gear = 0;
                ctx->remain_hour = 2;
                ctx->remain_minute = 0;
                ctx->cur_second = 0;
                cooker_state.total_seconds = cooker_timing_cal_total_sec(ctx->remain_hour, ctx->remain_minute);
                cooker_ui_state_set(&cooker_state, ch);
                lv_label_set_text_fmt(ctx->gear, "%d", cooker_state.gear);
                lv_obj_clear_flag(ctx->gear, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ctx->state, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
            else
            {
                cooker_set_idle(ch);
            }
        }
        else
        {
            if (ctx->cur_second % 60 == 0)
            {
                ctx->remain_minute--;
            }
            if (ctx->remain_minute < 0)
            {
                ctx->remain_minute = 59;
                ctx->remain_hour--;
            }

            lv_label_set_text_fmt(ctx->timing, "%02d:%02d", ctx->remain_hour, ctx->remain_minute);
        }
        // rt_kprintf("Cooker%d current second:%d, total second:%d\r\n", ch, ctx->cur_second, cur_cooker_state->total_seconds);
    }

}

 static void merge_cooker_timing_handler(int ch, ui_cooker_ctx_t * ctx)
{
    ui_cooker_state_t cooker_state = {0};

    if (ch == 1)
    {
        cooker_ui_state_get(&cooker_state, 2);
    }
    else
    {
        cooker_ui_state_get(&cooker_state, 0);
    }

    if (cooker_state.on_timing)
    {
        if (++ctx->cur_second >= cooker_state.total_seconds)
        {
            if (cooker_state.on_max_gear)
            {
                cooker_state.hour = 2;
                cooker_state.minute = 0;
                cooker_state.gear = cooker_state.gear ? cooker_state.gear : 9;
                cooker_state.on_max_gear = 0;
                ctx->remain_hour = 2;
                ctx->remain_minute = 0;
                ctx->cur_second = 0;
                cooker_state.total_seconds = cooker_timing_cal_total_sec(ctx->remain_hour, ctx->remain_minute);
                cooker_ui_state_set(&cooker_state, ch);
                lv_label_set_text_fmt(ctx->gear, "%d", cooker_state.gear);
                lv_obj_clear_flag(ctx->gear, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ctx->state, LV_OBJ_FLAG_HIDDEN);
                lv_obj_set_style_border_width(setting_cook_get(&ui_manager)->timing_cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
            }
            else
            {
                merge_cooker_set_idle(ch);
            }
        }
        else
        {
            if (ctx->cur_second % 60 == 0)
            {
                ctx->remain_minute--;
            }
            if (ctx->remain_minute < 0)
            {
                ctx->remain_minute = 59;
                ctx->remain_hour--;
            }

            lv_label_set_text_fmt(ctx->timing, "%02d:%02d", ctx->remain_hour, ctx->remain_minute);
        }
        // rt_kprintf("Cooker%d current second: %d\r\n", ch, ctx->cur_second);
    }

}


static void cooker_timer_handler(lv_timer_t * tmr)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        if (tmr == cooker_ui[i].timer && cooker_ui[i].timing)
        {
            cooker_timing_handler(i, &cooker_ui[i]);
        }
    }
}

static void merge_cooker_timer_handler(lv_timer_t * tmr)
{
    for (int i = 0; i < 2; i++)
    {
        if (tmr == merge_cooker_ui[i].timer && merge_cooker_ui[i].timing)
        {
            merge_cooker_timing_handler(i, &merge_cooker_ui[i]);
        }
    }
}


 void update_main_scr_ui_data(const ui_cooker_state_t * state, int num)
{
    if (rt_mutex_take(cur_state_mtx[num], RT_WAITING_FOREVER) == RT_EOK)
    {
        memcpy(&cur_cooker_state[num], state, sizeof(ui_cooker_state_t));
        rt_mutex_release(cur_state_mtx[num]);
    }
}



static void stop_cooker_anim(int all, int ch)
{
    if (all)
    {
        if (cur_cooker_state[0].on_merge)
        {
            lv_animimg_set_src(merge_cooker_ui[0].anim, (const void **)merge_anim_imgs, 1);
            lv_animimg_set_repeat_count(merge_cooker_ui[0].anim, 0);
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                if (cur_cooker_hw_state[i].fault || cur_cooker_state[i].gear || 
                    cur_cooker_state[i].on_timing || cur_cooker_state[i].thermos || 
                    cur_cooker_state[i].on_max_gear)
                {
                    lv_animimg_set_src(cooker_ui[i].anim, (const void **)cooker_anim_imgs, 1);
                    lv_animimg_set_repeat_count(cooker_ui[i].anim, 0);
                }
            }
        }

        if (cur_cooker_state[2].on_merge)
        {
            lv_animimg_set_src(merge_cooker_ui[1].anim, (const void **)merge_anim_imgs, 1);
            lv_animimg_set_repeat_count(merge_cooker_ui[1].anim, 0);
        }
        else
        {
            for (int i = 2; i < 4; i++)
            {
                if (cur_cooker_hw_state[i].fault || cur_cooker_state[i].gear || 
                    cur_cooker_state[i].on_timing || cur_cooker_state[i].thermos || 
                    cur_cooker_state[i].on_max_gear)
                {
                    lv_animimg_set_src(cooker_ui[i].anim, (const void **)cooker_anim_imgs, 1);
                    lv_animimg_set_repeat_count(cooker_ui[i].anim, 0);
                }
            }
        }

    }
    else
    {
        if (cur_cooker_state[ch].on_merge)
        {
            lv_animimg_set_src(merge_cooker_ui[ch].anim, (const void **)merge_anim_imgs, 1);
            lv_animimg_set_repeat_count(merge_cooker_ui[ch].anim, 0);
        }
        else
        {
            lv_animimg_set_src(cooker_ui[ch].anim, (const void **)cooker_anim_imgs, 1);
            lv_animimg_set_repeat_count(cooker_ui[ch].anim, 0);
        }
        
    }
}

 static void start_cooker_anim(int all, int ch)
{
    if (all)
    {
        if (cur_cooker_state[0].on_merge)
        {
            lv_animimg_set_src(merge_cooker_ui[0].anim, (const void **)merge_anim_imgs, 16);
            lv_animimg_set_repeat_count(merge_cooker_ui[0].anim, LV_ANIM_REPEAT_INFINITE);
        }
        else
        {
            for (int i = 0; i < 2; i++)
            {
                if (cur_cooker_state[i].gear || cur_cooker_state[i].on_timing ||
                    cur_cooker_state[i].thermos || cur_cooker_state[i].on_max_gear)
                {
                    lv_animimg_set_src(cooker_ui[i].anim, (const void **)cooker_anim_imgs, 16);
                    lv_animimg_set_repeat_count(cooker_ui[i].anim, LV_ANIM_REPEAT_INFINITE);
                }
            }
        }
        if (cur_cooker_state[2].on_merge)
        {
            lv_animimg_set_src(merge_cooker_ui[1].anim, (const void **)merge_anim_imgs, 16);
            lv_animimg_set_repeat_count(merge_cooker_ui[1].anim, LV_ANIM_REPEAT_INFINITE);
        }
        else
        {
            for (int i = 2; i < 4; i++)
            {
                if (cur_cooker_state[i].gear || cur_cooker_state[i].on_timing ||
                    cur_cooker_state[i].thermos || cur_cooker_state[i].on_max_gear)
                {
                    lv_animimg_set_src(cooker_ui[i].anim, (const void **)cooker_anim_imgs, 16);
                    lv_animimg_set_repeat_count(cooker_ui[i].anim, LV_ANIM_REPEAT_INFINITE);
                }
            }
        }
    }
    else
    {
        if (cur_cooker_state[ch].on_merge)
        {
            lv_animimg_set_src(merge_cooker_ui[ch].anim, (const void **)merge_anim_imgs, 16);
            lv_animimg_set_repeat_count(merge_cooker_ui[ch].anim, LV_ANIM_REPEAT_INFINITE);
        }
        else
        {
            lv_animimg_set_src(cooker_ui[ch].anim, (const void **)cooker_anim_imgs, 16);
            lv_animimg_set_repeat_count(cooker_ui[ch].anim, LV_ANIM_REPEAT_INFINITE);
        }
    }
}

static void pause_all_cooker_timer(int pause)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        pause ? lv_timer_pause(cooker_ui[i].timer) : lv_timer_resume(cooker_ui[i].timer);
        cur_cooker_state[i].pause = pause;
    }
}

static void pause_all_merge_cooker_timer(int pause)
{
    for (int i = 0; i < 2; i++)
    {
        pause ? lv_timer_pause(merge_cooker_ui[i].timer) : lv_timer_resume(merge_cooker_ui[i].timer);
    }
    cur_cooker_state[0].pause = pause;
    cur_cooker_state[2].pause = pause;
}

static void cooker_single_to_merge(int ch)
{
    main_screen_t * scr = main_screen_get(&ui_manager);

    if (ch == 0)
    {
        cooker_set_idle(0);
        cooker_set_idle(1);
        // pause_cooker_timer(0, 1);
        pause_cooker_timer(1, 1);
        lv_obj_add_flag(scr->cooker1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->cooker2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(scr->merge_cooker1, LV_OBJ_FLAG_HIDDEN);
        cur_cooker_state[0].on_merge = 1;
        main_scr_running_cooker -= 2;
        if (main_scr_running_cooker < 0)
        {
            main_scr_running_cooker = 0;
        }
    }
    else
    {
        cooker_set_idle(2);
        cooker_set_idle(3);
        // pause_cooker_timer(2, 1);
        pause_cooker_timer(3, 1);
        lv_obj_add_flag(scr->cooker3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->cooker4, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(scr->merge_cooker2, LV_OBJ_FLAG_HIDDEN);
        cur_cooker_state[2].on_merge = 1;
        main_scr_running_cooker -= 2;
        if (main_scr_running_cooker < 0)
        {
            main_scr_running_cooker = 0;
        }
    }
}

static void cooker_merge_to_single(int ch)
{
    main_screen_t * scr = main_screen_get(&ui_manager);

    if (ch == 0)
    {
        cooker_set_idle(0);
        pause_cooker_timer(1, 0);
        lv_obj_clear_flag(scr->cooker1, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(scr->cooker2, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->merge_cooker1, LV_OBJ_FLAG_HIDDEN);
        cur_cooker_state[0].on_merge = 0;

        main_scr_running_cooker --;
        if (main_scr_running_cooker < 0)
        {
            main_scr_running_cooker = 0;
        }
    }
    else
    {
        cooker_set_idle(2);
        pause_cooker_timer(2, 0);
        lv_obj_clear_flag(scr->cooker3, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(scr->cooker4, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(scr->merge_cooker2, LV_OBJ_FLAG_HIDDEN);
        cur_cooker_state[2].on_merge = 0;
        main_scr_running_cooker --;
        if (main_scr_running_cooker < 0)
        {
            main_scr_running_cooker = 0;
        }
    }
}


static void on_merge_ui_load(int ch)
{
    if (cur_cooker_hw_state[ch].fault || cur_cooker_state[ch].gear || 
        cur_cooker_state[ch].on_timing || cur_cooker_state[ch].thermos || 
        cur_cooker_state[ch].on_max_gear)
    {
        lv_obj_add_flag(merge_cooker_ui[ch].line, LV_OBJ_FLAG_HIDDEN);

        if (cur_cooker_hw_state[ch].fault || cur_cooker_hw_state[ch + 1].fault)
        {
            lv_img_set_src(merge_cooker_ui[ch].state, LVGL_IMAGE_PATH(Fault.png));
            lv_obj_clear_flag(merge_cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            if (cur_cooker_state[ch].on_max_gear)
            {
                lv_obj_add_flag(merge_cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
                lv_img_set_src(merge_cooker_ui[ch].state, LVGL_IMAGE_PATH(max_gear_156x156.png));
                lv_obj_clear_flag(merge_cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
            }
            else if (cur_cooker_state[ch].thermos)
            {
                lv_obj_add_flag(merge_cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
                lv_img_set_src(merge_cooker_ui[ch].state, LVGL_IMAGE_PATH(thermos_156x156.png));
                lv_obj_clear_flag(merge_cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
            }
            else if (cur_cooker_state[ch].gear > 0)
            {
                lv_label_set_text_fmt(merge_cooker_ui[ch].gear, "%d", cur_cooker_state[ch].gear);
                lv_obj_clear_flag(merge_cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(merge_cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
            }
            if (cur_cooker_state[ch].on_timing && !cur_cooker_state[ch].on_max_gear)
            {
                lv_label_set_text_fmt(merge_cooker_ui[ch].timing, "%d:%02d", cur_cooker_state[ch].hour, cur_cooker_state[ch].minute);
                lv_obj_clear_flag(merge_cooker_ui[ch].timing, LV_OBJ_FLAG_HIDDEN);
            }
            lv_obj_clear_flag(merge_cooker_ui[ch].anim, LV_OBJ_FLAG_HIDDEN);
            main_scr_running_cooker++;
        }
    }
    else
    {
        lv_obj_clear_flag(merge_cooker_ui[ch].line, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(merge_cooker_ui[ch].anim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(merge_cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(merge_cooker_ui[ch].timing, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(merge_cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
    }
}

static void normal_ui_load(int ch)
{
    if (cur_cooker_hw_state[ch].fault || cur_cooker_state[ch].gear || 
        cur_cooker_state[ch].on_timing || cur_cooker_state[ch].thermos || 
        cur_cooker_state[ch].on_max_gear)
    {
        
        lv_obj_add_flag(cooker_ui[ch].line, LV_OBJ_FLAG_HIDDEN);

        if (cur_cooker_hw_state[ch].fault)
        {
            lv_img_set_src(cooker_ui[ch].state, LVGL_IMAGE_PATH(Fault.png));
            lv_obj_clear_flag(cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
        }
        else
        {
            if (cur_cooker_state[ch].on_max_gear)
            {
                lv_obj_add_flag(cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
                lv_img_set_src(cooker_ui[ch].state, LVGL_IMAGE_PATH(max_gear_156x156.png));
                lv_obj_clear_flag(cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
            }
            else if (cur_cooker_state[ch].thermos)
            {
                lv_obj_add_flag(cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
                lv_img_set_src(cooker_ui[ch].state, LVGL_IMAGE_PATH(thermos_156x156.png));
                lv_obj_clear_flag(cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
            }
            else if (cur_cooker_state[ch].gear > 0)
            {
                lv_label_set_text_fmt(cooker_ui[ch].gear, "%d", cur_cooker_state[ch].gear);
                lv_obj_clear_flag(cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
            }
            if (cur_cooker_state[ch].on_timing && !cur_cooker_state[ch].on_max_gear)
            {
                lv_label_set_text_fmt(cooker_ui[ch].timing, "%d:%02d", cur_cooker_state[ch].hour, cur_cooker_state[ch].minute);
                lv_obj_clear_flag(cooker_ui[ch].timing, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_clear_flag(cooker_ui[ch].anim, LV_OBJ_FLAG_HIDDEN);
            main_scr_running_cooker++;
        }
    }
    else
    {
        lv_obj_clear_flag(cooker_ui[ch].line, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cooker_ui[ch].anim, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cooker_ui[ch].state, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cooker_ui[ch].timing, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(cooker_ui[ch].gear, LV_OBJ_FLAG_HIDDEN);
    }
}


void pause_cooker_timer(int ch, int pause)
{
    pause ? lv_timer_pause(cooker_ui[ch].timer) : lv_timer_resume(cooker_ui[ch].timer);
    cur_cooker_state[ch].pause = pause;
}

 void main_screen_custom_unload_start(void) 
{
    // lv_timer_pause(main_scr_tmr);
}


void reset_cooker_timer(int ch)
{
    lv_timer_pause(cooker_ui[ch].timer);
    cooker_ui[ch].remain_hour = cur_cooker_state[ch].hour;
    cooker_ui[ch].remain_minute = cur_cooker_state[ch].minute;
    cooker_ui[ch].cur_second = 0;
    lv_timer_resume(cooker_ui[ch].timer);
}

 void reset_merge_cooker_timer(int ch)
{
    lv_timer_pause(merge_cooker_ui[ch].timer);
    merge_cooker_ui[ch].remain_hour = cur_cooker_state[ch].hour;
    merge_cooker_ui[ch].remain_minute = cur_cooker_state[ch].minute;
    merge_cooker_ui[ch].cur_second = 0;
    lv_timer_resume(merge_cooker_ui[ch].timer);
}


void main_screen_custom_created(void) 
{
    main_screen_t *scr = main_screen_get(&ui_manager);

    cpu_temp_obj = lv_sysmon_create(lv_layer_sys());
    lv_obj_align(cpu_temp_obj, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        cur_state_mtx[i] = rt_mutex_create(main_ui_mtx_name[i], RT_IPC_FLAG_PRIO);
    }

    main_scr_tmr = lv_timer_create(main_scr_timer_handler, 200, NULL);
    cpu_temp_tmr = lv_timer_create(cpu_temperature_sample, 1000, NULL);

    merge_cooker_ui[0].cont = scr->merge_cooker1;
    merge_cooker_ui[0].anim = scr->merge1_anim;
    merge_cooker_ui[0].gear = scr->merge1_gear;
    merge_cooker_ui[0].line = scr->merge_cooker1_line;
    merge_cooker_ui[0].state = scr->merge1_state;
    merge_cooker_ui[0].timing = scr->merge1_timing;
    merge_cooker_ui[0].timer = lv_timer_create(merge_cooker_timer_handler, 1000, NULL);

    
    merge_cooker_ui[1].cont = scr->merge_cooker2;
    merge_cooker_ui[1].anim = scr->merge2_anim;
    merge_cooker_ui[1].gear = scr->merge2_gear;
    merge_cooker_ui[1].line = scr->merge_cooker2_line;
    merge_cooker_ui[1].state = scr->merge2_state;
    merge_cooker_ui[1].timing = scr->merge2_timing;
    merge_cooker_ui[1].timer = lv_timer_create(merge_cooker_timer_handler, 1000, NULL);

    cooker_ui[0].cont = scr->cooker1;
    cooker_ui[0].anim = scr->cooker1_anim;
    cooker_ui[0].gear = scr->cooker1_gear;
    cooker_ui[0].line = scr->cooker1_line;
    cooker_ui[0].state = scr->cooker1_state;
    cooker_ui[0].timing = scr->cooker1_timing;
    cooker_ui[0].timer = lv_timer_create(cooker_timer_handler, 1000, NULL);

    cooker_ui[1].cont = scr->cooker2;
    cooker_ui[1].anim = scr->cooker2_anim;
    cooker_ui[1].gear = scr->cooker2_gear;
    cooker_ui[1].line = scr->cooker2_line;
    cooker_ui[1].state = scr->cooker2_state;
    cooker_ui[1].timing = scr->cooker2_timing;
    cooker_ui[1].timer = lv_timer_create(cooker_timer_handler, 1000, NULL);

    cooker_ui[2].cont = scr->cooker3;
    cooker_ui[2].anim = scr->cooker3_anim;
    cooker_ui[2].gear = scr->cooker3_gear;
    cooker_ui[2].line = scr->cooker3_line;
    cooker_ui[2].state = scr->cooker3_state;
    cooker_ui[2].timing = scr->cooker3_timing;
    cooker_ui[2].timer = lv_timer_create(cooker_timer_handler, 1000, NULL);

    cooker_ui[3].cont = scr->cooker4;
    cooker_ui[3].anim = scr->cooker4_anim;
    cooker_ui[3].gear = scr->cooker4_gear;
    cooker_ui[3].line = scr->cooker4_line;
    cooker_ui[3].state = scr->cooker4_state;
    cooker_ui[3].timing = scr->cooker4_timing;
    cooker_ui[3].timer = lv_timer_create(cooker_timer_handler, 1000, NULL);
}


 void main_screen_custom_load_start(void) 
{    
    main_scr_running_cooker = 0;
    
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        cooker_ui_state_get(&cur_cooker_state[i], i);
    }

    if (cur_cooker_state[0].on_merge)
    {
        on_merge_ui_load(0);
    }
    else
    {
        normal_ui_load(0);
        normal_ui_load(1);
    }

    if (cur_cooker_state[2].on_merge)
    {
        on_merge_ui_load(1);
    }
    else
    {
        normal_ui_load(2);
        normal_ui_load(3);
    }

    if (main_scr_running_cooker && lv_obj_has_flag(main_screen_get(&ui_manager)->global_pause, LV_OBJ_FLAG_HIDDEN))
    {
        lv_img_set_src(main_screen_get(&ui_manager)->global_pause, LVGL_IMAGE_PATH(suspend_300x300.png));
        lv_obj_clear_flag(main_screen_get(&ui_manager)->global_pause, LV_OBJ_FLAG_HIDDEN);
        main_scr_glb_pause = 0;
    }
    else if (main_scr_running_cooker == 0)
    {
        lv_obj_add_flag(main_screen_get(&ui_manager)->global_pause, LV_OBJ_FLAG_HIDDEN);
        main_scr_glb_pause = 0;
    }
    else if (main_scr_running_cooker)
    {
        if (main_scr_glb_pause)
        {
            stop_cooker_anim(1, 0);
        }
    }
}


void main_screen_drop_about_custom_clicked(void) 
{
    lv_scr_load_anim(about_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

void main_screen_drop_lang_custom_clicked(void) 
{
    lv_scr_load_anim(lang_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}


void main_screen_drop_cook_custom_clicked(void) 
{
    lv_scr_load_anim(cook_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

void main_screen_drop_qrcode_custom_clicked(void) 
{
    lv_scr_load_anim(qrcode_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

void main_screen_drop_merge_custom_clicked(void) 
{
    if (cur_cooker_state[0].on_merge)
    {
        lv_obj_clear_flag(main_screen_get(&ui_manager)->merge_cooker1, LV_OBJ_FLAG_CLICKABLE);
    }
    else
    {
        lv_obj_clear_flag(main_screen_get(&ui_manager)->cooker1, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(main_screen_get(&ui_manager)->cooker2, LV_OBJ_FLAG_CLICKABLE);
    }
    if (cur_cooker_state[2].on_merge)
    {
        lv_obj_clear_flag(main_screen_get(&ui_manager)->merge_cooker2, LV_OBJ_FLAG_CLICKABLE);
    }
    else
    {
        lv_obj_clear_flag(main_screen_get(&ui_manager)->cooker3, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_clear_flag(main_screen_get(&ui_manager)->cooker4, LV_OBJ_FLAG_CLICKABLE);
    }


    lv_obj_clear_flag(main_screen_get(&ui_manager)->dropline, LV_OBJ_FLAG_CLICKABLE);

    drop_menu_return(main_screen_get(&ui_manager)->obj, main_screen_get(&ui_manager)->dropline, 
                        main_screen_get(&ui_manager)->drop_menu, drop_window_up);

}


 void main_screen_global_pause_custom_clicked(void) 
{
    main_scr_glb_pause = main_scr_glb_pause ? 0 : 1;

    if (main_scr_glb_pause)
    {
        pause_all_cooker_timer(1);
        pause_all_merge_cooker_timer(1);
        stop_cooker_anim(1, 0);
        lv_img_set_src(main_screen_get(&ui_manager)->global_pause, LVGL_IMAGE_PATH(start_300x300.png));
        lv_refr_now(lv_disp_get_default());
    }
    else
    {
        pause_all_cooker_timer(0);
        pause_all_merge_cooker_timer(0);
        start_cooker_anim(1, 0);
        lv_img_set_src(main_screen_get(&ui_manager)->global_pause, LVGL_IMAGE_PATH(suspend_300x300.png));
        lv_refr_now(lv_disp_get_default());
    }
}


 void main_screen_dropline_custom_pressed(void)
{
    main_screen_t *scr = main_screen_get(&ui_manager);

    if (!ui_manager.auto_del && scr->drop_menu)
    {
        return;
    }

    drop_menu_start(scr->dropline, scr->drop_menu);
}

 void main_screen_dropline_custom_pressing(void) 
{
    main_screen_t *scr = main_screen_get(&ui_manager);

    if (!ui_manager.auto_del && scr->drop_menu)
    {
        return;
    }
    
    drop_menu_dropping_handler(scr->dropline, scr->drop_menu);
}


 void main_screen_dropline_custom_released(void) 
{
    main_screen_t *scr = main_screen_get(&ui_manager);

    if (!ui_manager.auto_del && scr->drop_menu)
    {
        return;
    }

    drop_menu_released_handler(scr->obj, scr->dropline, scr->drop_menu);
}


 void main_screen_cooker1_custom_clicked(void) 
{
    setting_cooker_channel_set(0);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

 void main_screen_cooker2_custom_clicked(void) 
{
    setting_cooker_channel_set(1);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

 void main_screen_cooker3_custom_clicked(void) 
{
    setting_cooker_channel_set(2);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

 void main_screen_cooker4_custom_clicked(void) 
{
    setting_cooker_channel_set(3);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

void main_screen_wnd_back_custom_clicked(void) 
{
    drop_window_down();

    if (cur_cooker_state[0].on_merge)
    {
        lv_obj_add_flag(main_screen_get(&ui_manager)->merge_cooker1, LV_OBJ_FLAG_CLICKABLE);
    }
    else
    {
        lv_obj_add_flag(main_screen_get(&ui_manager)->cooker1, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(main_screen_get(&ui_manager)->cooker2, LV_OBJ_FLAG_CLICKABLE);
    }
    if (cur_cooker_state[2].on_merge)
    {
        lv_obj_add_flag(main_screen_get(&ui_manager)->merge_cooker2, LV_OBJ_FLAG_CLICKABLE);
    }
    else
    {
        lv_obj_add_flag(main_screen_get(&ui_manager)->cooker3, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_flag(main_screen_get(&ui_manager)->cooker4, LV_OBJ_FLAG_CLICKABLE);
    }

    lv_obj_add_flag(main_screen_get(&ui_manager)->dropline, LV_OBJ_FLAG_CLICKABLE);
}

void main_screen_group1_img_custom_clicked(void) 
{
    if (cur_cooker_state[0].on_merge)
    {
        cooker_merge_to_single(0);
    }
    else
    {
        cooker_single_to_merge(0);
    }
}

void main_screen_group2_img_custom_clicked(void) 
{
    if (cur_cooker_state[2].on_merge)
    {
        cooker_merge_to_single(1);
    }
    else
    {
        cooker_single_to_merge(1);
    }
}

void main_screen_merge_cooker1_custom_clicked(void) 
{
    setting_cooker_channel_set(0);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

void main_screen_merge_cooker2_custom_clicked(void) 
{
    setting_cooker_channel_set(2);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
}

