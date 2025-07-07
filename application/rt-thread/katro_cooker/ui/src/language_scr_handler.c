#include "app_ui_common.h"
#include "app_common.h"
#include "cooker_protocol.h"


void lang_screen_custom_created(void) 
{
    lang_screen_t * scr = lang_screen_get(&ui_manager);

}

void lang_screen_lang_back_custom_clicked(void) 
{
    lv_scr_load_anim(main_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
}


void lang_screen_cn_img_custom_clicked(void) 
{

}

void lang_screen_eng_img_custom_clicked(void) 
{

}



