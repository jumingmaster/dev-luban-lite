#include "app_ui_common.h"
#include "cooker_protocol.h"

static lv_timer_t * main_scr_tmr = NULL;

TCM_DATA_DEFINE static ui_cooker_state_t cur_cooker_state[UI_COOKER_NUM] = {0};

TCM_DATA_DEFINE static ui_cooker_ctx_t cooker_ui[UI_COOKER_NUM] = {0};

static rt_mutex_t cur_state_mtx[UI_COOKER_NUM] = {0};

static const char main_ui_mtx_name[UI_COOKER_NUM][16] = {
    {"cur_state_mtx0"},
    {"cur_state_mtx1"},
    {"cur_state_mtx2"},
    {"cur_state_mtx3"}
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


TCM_CODE_DEFINE void update_main_scr_ui_data(const ui_cooker_state_t * state, int num)
{
    if (rt_mutex_take(cur_state_mtx[num], 100) == RT_EOK)
    {
        memcpy(&cur_cooker_state[num], state, sizeof(ui_cooker_state_t));
        rt_mutex_release(cur_state_mtx[num]);
        // rt_kprintf("Selected Hour:%d, Minute:%d\r\n", cur_cooker_state[num].hour, cur_cooker_state[num].minute);
    }
}



TCM_CODE_DEFINE void main_screen_custom_unload_start(void) 
{
    // lv_timer_pause(main_scr_tmr);
}


TCM_CODE_DEFINE void main_screen_custom_created(void) 
{
    // cooker_ui_manager_init();

    main_screen_t *scr = main_screen_get(&ui_manager);

    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        cur_state_mtx[i] = rt_mutex_create(main_ui_mtx_name[i], RT_IPC_FLAG_PRIO);
    }

    main_scr_tmr = lv_timer_create(main_scr_timer_handler, 100, NULL);

    cooker_ui[0].anim = scr->cooker1_anim;
    cooker_ui[0].gear = scr->cooker1_gear;
    cooker_ui[0].line = scr->cooker1_line;
    cooker_ui[0].state = scr->cooker1_state;
    cooker_ui[0].timing = scr->cooker1_timing;

    cooker_ui[1].anim = scr->cooker2_anim;
    cooker_ui[1].gear = scr->cooker2_gear;
    cooker_ui[1].line = scr->cooker2_line;
    cooker_ui[1].state = scr->cooker2_state;
    cooker_ui[1].timing = scr->cooker2_timing;

    cooker_ui[2].anim = scr->cooker3_anim;
    cooker_ui[2].gear = scr->cooker3_gear;
    cooker_ui[2].line = scr->cooker3_line;
    cooker_ui[2].state = scr->cooker3_state;
    cooker_ui[2].timing = scr->cooker3_timing;

    cooker_ui[3].anim = scr->cooker4_anim;
    cooker_ui[3].gear = scr->cooker4_gear;
    cooker_ui[3].line = scr->cooker4_line;
    cooker_ui[3].state = scr->cooker4_state;
    cooker_ui[3].timing = scr->cooker4_timing;
}


TCM_CODE_DEFINE void main_screen_custom_load_start(void) 
{
    char str[16] = {0};

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
                lv_obj_clear_flag(cooker_ui[i].anim, LV_OBJ_FLAG_HIDDEN);

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
                    memset(str, 0x00, sizeof(str));
                    snprintf(str, sizeof(str), "%d", cur_cooker_state[i].gear);
                    lv_label_set_text(cooker_ui[i].gear, str);
                    lv_obj_clear_flag(cooker_ui[i].gear, LV_OBJ_FLAG_HIDDEN);
                    lv_obj_add_flag(cooker_ui[i].state, LV_OBJ_FLAG_HIDDEN);
                }
                if (cur_cooker_state[i].on_timing)
                {
                    
                    memset(str, 0x00, sizeof(str));
                    snprintf(str, sizeof(str), "%d:%02d", cur_cooker_state[i].hour, cur_cooker_state[i].minute);
                    // rt_kprintf("Cooker%d start timing %s\r\n", str);
                    lv_label_set_text(cooker_ui[i].timing, str);
                    lv_obj_clear_flag(cooker_ui[i].timing, LV_OBJ_FLAG_HIDDEN);
                }
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

    // lv_timer_resume(main_scr_tmr);
}


TCM_CODE_DEFINE void main_screen_global_pause_custom_clicked(void) 
{
    
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

