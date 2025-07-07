#include "app_ui_common.h"
#include "app_common.h"
#include "cooker_protocol.h"
#include <rtconfig.h>
#include <aic_osal.h>
#include "boot_param.h"

extern struct boot_args boot_arg;


void about_screen_custom_created(void) 
{
    
    lv_label_set_text_fmt(about_screen_get(&ui_manager)->about_text, "ArtInChip Luban-Lite %d.%d.%d\n\n"
                                                                     "Image version: %s\n\n"
                                                                     "Built on %s %s",
                                                                        LL_VERSION, LL_SUBVERSION, LL_REVISION, 
                                                                        boot_arg.image_version, __DATE__, __TIME__);
}