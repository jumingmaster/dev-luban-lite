#include "app_ui_common.h"

static ui_cooker_state_t cooker_ui[UI_COOKER_NUM] = {0};

static rt_mutex_t ui_cooker_mutex[UI_COOKER_NUM] = {0};

static const char mtx_name[UI_COOKER_NUM][16] = {
    {"cookerui_mtx1"},
    {"cookerui_mtx2"},
    {"cookerui_mtx3"},
    {"cookerui_mtx4"},
};

static int cooker_ui_manager_init(void)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        
        ui_cooker_mutex[i] = rt_mutex_create(mtx_name[i], RT_IPC_FLAG_PRIO);
        // if (ui_cooker_mutex[i])
        // {
        //     rt_kprintf("CookerUI mutex %d created.\r\n", i);
        // }
    }

    return 0;
}
INIT_COMPONENT_EXPORT(cooker_ui_manager_init);




void cooker_ui_state_get(ui_cooker_state_t * state, int num)
{
    if (num > UI_COOKER_NUM)
    {
        return;
    }

    rt_mutex_take(ui_cooker_mutex[num], RT_WAITING_FOREVER);

    *state = cooker_ui[num];

    rt_mutex_release(ui_cooker_mutex[num]);
}

void cooker_ui_state_set(const ui_cooker_state_t * state, int num)
{
    if (num > UI_COOKER_NUM)
    {
        return;
    }
    // rt_kprintf("Test 3\r\n");
    rt_mutex_take(ui_cooker_mutex[num], RT_WAITING_FOREVER);

    cooker_ui[num] = *state;

    rt_mutex_release(ui_cooker_mutex[num]);
}






