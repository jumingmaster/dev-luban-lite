#include "ui_objects.h"
#include "aic_ui.h"
#include "ui_util.h"
#include "rtthread.h"

static int val = 0;

extern ui_manager_t ui_manager;

void screen_button_1_custom_clicked(void)
{
    char str[32] = {0};

    screen_t *scr = screen_get(&ui_manager);

    if (!ui_manager.auto_del && scr->obj)
    return;

    snprintf(str, 32, "%d", ++val);

    lv_label_set_text(scr->label_1, str);
}