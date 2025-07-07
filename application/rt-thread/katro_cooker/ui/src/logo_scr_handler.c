#include "app_ui_common.h"

static void logo_anim_ready_cb(lv_anim_t * a)
{
    lv_img_cache_invalidate_src(NULL);
    
    lv_scr_load_anim(main_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_NONE, 0, 0, true);
}

void logo_scr_custom_created(void) 
{
    logo_scr_t *scr = logo_scr_get(&ui_manager);
    
    lv_anim_set_completed_cb(LV_ANIM_REF(scr->logo_amni), logo_anim_ready_cb);

    main_screen_create(&ui_manager);
    setting_cook_create(&ui_manager);
    setting_timing_create(&ui_manager);
    lang_screen_create(&ui_manager);
    cook_screen_create(&ui_manager);
    about_screen_create(&ui_manager);
    qrcode_screen_create(&ui_manager);
}


