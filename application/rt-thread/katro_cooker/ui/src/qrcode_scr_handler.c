#include "app_ui_common.h"
#include "app_common.h"
#include "cooker_protocol.h"


void qrcode_screen_qr_back_custom_clicked(void) 
{
    lv_scr_load_anim(main_screen_get(&ui_manager)->obj, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
}
