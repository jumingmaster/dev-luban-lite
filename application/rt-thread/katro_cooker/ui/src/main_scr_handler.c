#include "app_ui_common.h"
#include "cooker_protocol.h"

static const char *cooker_anim_imgs[16] = {
    LVGL_IMAGE_PATH(1_400x345.png),
    LVGL_IMAGE_PATH(2_400x345.png),
    LVGL_IMAGE_PATH(3_400x345.png),
    LVGL_IMAGE_PATH(4_400x345.png),
    LVGL_IMAGE_PATH(5_400x345.png),
    LVGL_IMAGE_PATH(6_400x345.png),
    LVGL_IMAGE_PATH(7_400x345.png),
    LVGL_IMAGE_PATH(8_400x345.png),
    LVGL_IMAGE_PATH(9_400x345.png),
    LVGL_IMAGE_PATH(10_400x345.png),
    LVGL_IMAGE_PATH(11_400x345.png),
    LVGL_IMAGE_PATH(12_400x345.png),
    LVGL_IMAGE_PATH(13_400x345.png),
    LVGL_IMAGE_PATH(14_400x345.png),
    LVGL_IMAGE_PATH(15_400x345.png),
    LVGL_IMAGE_PATH(16_400x345.png),
};



TCM_DATA_DEFINE static lv_timer_t * main_scr_tmr = NULL;

TCM_DATA_DEFINE static ui_cooker_state_t cur_cooker_state[UI_COOKER_NUM] = {0};

TCM_DATA_DEFINE static ui_cooker_ctx_t cooker_ui[UI_COOKER_NUM] = {0};

TCM_DATA_DEFINE static int main_scr_glb_pause = 0;

TCM_DATA_DEFINE static int main_scr_running_cooker = 0;





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


TCM_CODE_DEFINE static void main_scr_timer_handler(lv_timer_t * tmr)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        if (rt_mutex_take(cur_state_mtx[i], 10) == RT_EOK)
        {
            cooker_ui_state_set(&cur_cooker_state[i], i);
            rt_mutex_release(cur_state_mtx[i]);
        }
        // cook_ui_msg_send();
    }

    cooker_ui_data_notify();
}


TCM_CODE_DEFINE static void cooker_set_idle(int ch)
{
    if (cur_cooker_state[ch].on_merge)
    {

    }
    else
    {
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
        }
    }
}



TCM_CODE_DEFINE static void cooker_timing_handler(int ch, ui_cooker_ctx_t * ctx)
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
                ctx->remain_hour = gear_timing_table[2];
                ctx->remain_minute = 0;
                ctx->cur_second = 0;
                cooker_state.total_seconds = cooker_timing_cal_total_sec(ctx->remain_hour, ctx->remain_minute);
                cooker_ui_state_set(&cooker_state, ch);
                lv_label_set_text_fmt(ctx->gear, "%d", cooker_state.gear);
                lv_obj_clear_flag(ctx->gear, LV_OBJ_FLAG_HIDDEN);
                lv_obj_add_flag(ctx->state, LV_OBJ_FLAG_HIDDEN);
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
        rt_kprintf("Cooker%d current second: %d\r\n", ch, ctx->cur_second);
    }

}


TCM_CODE_DEFINE static void cooker_timer_handler(lv_timer_t * tmr)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        if (tmr == cooker_ui[i].timer && cooker_ui[i].timing)
        {
            cooker_timing_handler(i, &cooker_ui[i]);
        }
    }
}


TCM_CODE_DEFINE void update_main_scr_ui_data(const ui_cooker_state_t * state, int num)
{
    if (rt_mutex_take(cur_state_mtx[num], RT_WAITING_FOREVER) == RT_EOK)
    {
        memcpy(&cur_cooker_state[num], state, sizeof(ui_cooker_state_t));
        rt_mutex_release(cur_state_mtx[num]);
    }
}



TCM_CODE_DEFINE static void stop_cooker_anim(int all, int ch)
{
    if (all)
    {
        for (int i = 0; i < UI_COOKER_NUM; i++)
        {
            if (cur_cooker_state[i].fault || cur_cooker_state[i].gear || 
                cur_cooker_state[i].on_timing || cur_cooker_state[i].thermos || 
                cur_cooker_state[i].on_max_gear)
            {
                lv_animimg_set_src(cooker_ui[i].anim, (const void **)cooker_anim_imgs, 1);
                lv_animimg_set_repeat_count(cooker_ui[i].anim, 0);
            }
        }
    }
    else
    {
        lv_animimg_set_src(cooker_ui[ch].anim, (const void **)cooker_anim_imgs, 1);
        lv_animimg_set_repeat_count(cooker_ui[ch].anim, 0);
    }
}

TCM_CODE_DEFINE static void start_cooker_anim(int all, int ch)
{
    if (all)
    {
        for (int i = 0; i < UI_COOKER_NUM; i++)
        {
            if (cur_cooker_state[i].gear || cur_cooker_state[i].on_timing ||
                cur_cooker_state[i].thermos || cur_cooker_state[i].on_max_gear)
            {
                lv_animimg_set_src(cooker_ui[i].anim, (const void **)cooker_anim_imgs, 16);
                lv_animimg_set_repeat_count(cooker_ui[i].anim, LV_ANIM_REPEAT_INFINITE);
            }
        }
    }
    else
    {
        lv_animimg_set_src(cooker_ui[ch].anim, (const void **)cooker_anim_imgs, 16);
        lv_animimg_set_repeat_count(cooker_ui[ch].anim, LV_ANIM_REPEAT_INFINITE);
    }
}

static void pause_all_cooker_timer(int pause)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        pause ? lv_timer_pause(cooker_ui[i].timer) : lv_timer_resume(cooker_ui[i].timer);
    }
}


TCM_CODE_DEFINE void main_screen_custom_unload_start(void) 
{
    // lv_timer_pause(main_scr_tmr);
}


TCM_CODE_DEFINE void reset_cooker_timer(int ch)
{
    lv_timer_pause(cooker_ui[ch].timer);
    cooker_ui[ch].remain_hour = cur_cooker_state[ch].hour;
    cooker_ui[ch].remain_minute = cur_cooker_state[ch].minute;
    cooker_ui[ch].cur_second = 0;
    lv_timer_resume(cooker_ui[ch].timer);
}


void main_screen_custom_created(void) 
{
    // cooker_ui_manager_init();

    main_screen_t *scr = main_screen_get(&ui_manager);

    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        cur_state_mtx[i] = rt_mutex_create(main_ui_mtx_name[i], RT_IPC_FLAG_PRIO);
    }

    main_scr_tmr = lv_timer_create(main_scr_timer_handler, 100, NULL);

    cooker_ui[0].cont = scr->cooker1;
    cooker_ui[0].anim = scr->cooker1_anim;
    cooker_ui[0].gear = scr->cooker1_gear;
    cooker_ui[0].line = scr->cooker1_line;
    cooker_ui[0].state = scr->cooker1_state;
    cooker_ui[0].timing = scr->cooker1_timing;
    // cooker_ui[0].timer = lv_timer_create_basic();
    // lv_timer_set_cb(cooker_ui[0].timer, cooker_timer_handler);
    // lv_timer_set_period(cooker_ui[0].timer, 1000);
    // lv_timer_set_repeat_count(cooker_ui[0].timer, -1);
    cooker_ui[0].timer = lv_timer_create(cooker_timer_handler, 1000, NULL);

    cooker_ui[1].cont = scr->cooker2;
    cooker_ui[1].anim = scr->cooker2_anim;
    cooker_ui[1].gear = scr->cooker2_gear;
    cooker_ui[1].line = scr->cooker2_line;
    cooker_ui[1].state = scr->cooker2_state;
    cooker_ui[1].timing = scr->cooker2_timing;
    // cooker_ui[1].timer = lv_timer_create_basic();
    // lv_timer_set_cb(cooker_ui[1].timer, cooker_timer_handler);
    // lv_timer_set_period(cooker_ui[1].timer, 1000);
    // lv_timer_set_repeat_count(cooker_ui[1].timer, -1);
    cooker_ui[1].timer = lv_timer_create(cooker_timer_handler, 1000, NULL);

    cooker_ui[2].cont = scr->cooker3;
    cooker_ui[2].anim = scr->cooker3_anim;
    cooker_ui[2].gear = scr->cooker3_gear;
    cooker_ui[2].line = scr->cooker3_line;
    cooker_ui[2].state = scr->cooker3_state;
    cooker_ui[2].timing = scr->cooker3_timing;
    // cooker_ui[2].timer = lv_timer_create_basic();
    // lv_timer_set_cb(cooker_ui[2].timer, cooker_timer_handler);
    // lv_timer_set_period(cooker_ui[2].timer, 1000);
    cooker_ui[2].timer = lv_timer_create(cooker_timer_handler, 1000, NULL);

    cooker_ui[3].cont = scr->cooker4;
    cooker_ui[3].anim = scr->cooker4_anim;
    cooker_ui[3].gear = scr->cooker4_gear;
    cooker_ui[3].line = scr->cooker4_line;
    cooker_ui[3].state = scr->cooker4_state;
    cooker_ui[3].timing = scr->cooker4_timing;
    // cooker_ui[3].timer = lv_timer_create_basic();
    // lv_timer_set_cb(cooker_ui[3].timer, cooker_timer_handler);
    // lv_timer_set_period(cooker_ui[3].timer, 1000);
    cooker_ui[3].timer = lv_timer_create(cooker_timer_handler, 1000, NULL);
}


TCM_CODE_DEFINE void main_screen_custom_load_start(void) 
{    
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        cooker_ui_state_get(&cur_cooker_state[i], i);
    }

    for (int i = 0; i < UI_COOKER_NUM; i++)
    {

        if (cur_cooker_state[i].fault || cur_cooker_state[i].gear || 
            cur_cooker_state[i].on_timing || cur_cooker_state[i].thermos || 
            cur_cooker_state[i].on_max_gear)
        {
            
            lv_obj_add_flag(cooker_ui[i].line, LV_OBJ_FLAG_HIDDEN);

            if (cur_cooker_state[i].fault)
            {

            }
            else
            {
                if (cur_cooker_state[i].on_max_gear)
                {
                    lv_obj_add_flag(cooker_ui[i].gear, LV_OBJ_FLAG_HIDDEN);
                    lv_img_set_src(cooker_ui[i].state, LVGL_IMAGE_PATH(max_gear_150x150.png));
                    lv_obj_clear_flag(cooker_ui[i].state, LV_OBJ_FLAG_HIDDEN);
                }
                else if (cur_cooker_state[i].thermos)
                {
                    lv_obj_add_flag(cooker_ui[i].gear, LV_OBJ_FLAG_HIDDEN);
                    lv_img_set_src(cooker_ui[i].state, LVGL_IMAGE_PATH(thermos_150x150.png));
                    lv_obj_clear_flag(cooker_ui[i].state, LV_OBJ_FLAG_HIDDEN);
                }
                else if (cur_cooker_state[i].gear > 0)
                {
                    lv_label_set_text_fmt(cooker_ui[i].gear, "%d", cur_cooker_state[i].gear);
                    lv_obj_clear_flag(cooker_ui[i].gear, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(cooker_ui[i].state, LV_OBJ_FLAG_HIDDEN);
                }
                if (cur_cooker_state[i].on_timing)
                {
                    lv_label_set_text_fmt(cooker_ui[i].timing, "%d:%02d", cur_cooker_state[i].hour, cur_cooker_state[i].minute);
                    lv_obj_clear_flag(cooker_ui[i].timing, LV_OBJ_FLAG_HIDDEN);
                }

                lv_obj_clear_flag(cooker_ui[i].anim, LV_OBJ_FLAG_HIDDEN);

                main_scr_running_cooker++;
            }
        }
        else
        {
            lv_obj_clear_flag(cooker_ui[i].line, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(cooker_ui[i].anim, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(cooker_ui[i].state, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(cooker_ui[i].timing, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(cooker_ui[i].gear, LV_OBJ_FLAG_HIDDEN);
        }
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


TCM_CODE_DEFINE void main_screen_global_pause_custom_clicked(void) 
{
    main_scr_glb_pause = main_scr_glb_pause ? 0 : 1;

    if (main_scr_glb_pause)
    {
        pause_all_cooker_timer(1);
        stop_cooker_anim(1, 0);
        lv_img_set_src(main_screen_get(&ui_manager)->global_pause, LVGL_IMAGE_PATH(start_300x300.png));
        lv_refr_now(lv_disp_get_default());
    }
    else
    {
        pause_all_cooker_timer(0);
        start_cooker_anim(1, 0);
        lv_img_set_src(main_screen_get(&ui_manager)->global_pause, LVGL_IMAGE_PATH(suspend_300x300.png));
        lv_refr_now(lv_disp_get_default());
    }
}


TCM_CODE_DEFINE void main_screen_dropline_custom_pressed(void)
{
    main_screen_t *scr = main_screen_get(&ui_manager);

    if (!ui_manager.auto_del && scr->drop_menu)
    {
        return;
    }

    drop_menu_start(scr->dropline, scr->drop_menu);
}

TCM_CODE_DEFINE void main_screen_dropline_custom_pressing(void) 
{
    main_screen_t *scr = main_screen_get(&ui_manager);

    if (!ui_manager.auto_del && scr->drop_menu)
    {
        return;
    }
    
    drop_menu_dropping_handler(scr->dropline, scr->drop_menu);
}


TCM_CODE_DEFINE void main_screen_dropline_custom_released(void) 
{
    main_screen_t *scr = main_screen_get(&ui_manager);

    if (!ui_manager.auto_del && scr->drop_menu)
    {
        return;
    }

    drop_menu_released_handler(scr->obj, scr->dropline, scr->drop_menu);
}


TCM_CODE_DEFINE void main_screen_cooker1_custom_clicked(void) 
{
    // lv_obj_t * cur_scr = main_screen_get(&ui_manager)->obj;

    // setting_cook_create(&ui_manager);
    
    setting_cooker_channel_set(0);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_TOP, 100, 0, false);

    // if (cur_scr && lv_obj_is_valid(cur_scr))
    // {
    //     // lv_obj_clean(cur_scr);
    //     lv_obj_del_async(cur_scr);
    // }
}

TCM_CODE_DEFINE void main_screen_cooker2_custom_clicked(void) 
{
    // lv_obj_t * cur_scr = main_screen_get(&ui_manager)->obj;

    // setting_cook_create(&ui_manager);

    setting_cooker_channel_set(1);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_TOP, 100, 0, false);

    // if (cur_scr && lv_obj_is_valid(cur_scr))
    // {
    //     // lv_obj_clean(cur_scr);
    //     lv_obj_del_async(cur_scr);
    // }
}

TCM_CODE_DEFINE void main_screen_cooker3_custom_clicked(void) 
{
    // lv_obj_t * cur_scr = main_screen_get(&ui_manager)->obj;

    // setting_cook_create(&ui_manager);
    setting_cooker_channel_set(2);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_TOP, 100, 0, false);

    // if (cur_scr && lv_obj_is_valid(cur_scr))
    // {
    //     // lv_obj_clean(cur_scr);
    //     lv_obj_del_async(cur_scr);
    // }
}

TCM_CODE_DEFINE void main_screen_cooker4_custom_clicked(void) 
{
    // lv_obj_t * cur_scr = main_screen_get(&ui_manager)->obj;

    // setting_cook_create(&ui_manager);
    setting_cooker_channel_set(3);

    lv_scr_load_anim(setting_cook_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_TOP, 100, 0, false);

    // if (cur_scr && lv_obj_is_valid(cur_scr))
    // {
    //     // lv_obj_clean(cur_scr);
    //     lv_obj_del_async(cur_scr);
    // }
}

