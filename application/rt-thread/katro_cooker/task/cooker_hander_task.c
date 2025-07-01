#include "cooker_drv.h"
#include "cooker_protocol.h"
#include "app_ui_common.h"
#include "cooker_task.h"

#define HAND_THREAD_PRIORITY         16 
#define HAND_THREAD_TIMESLICE        5

static char cook_handler_stack[1024];

static struct rt_thread cook_handler_thread;

static rt_sem_t ui_sem = RT_NULL;

TCM_DATA_DEFINE  static ui_cooker_state_t cooker_ui_state[UI_COOKER_NUM] = {0};

static const char mtx_name[UI_COOKER_NUM][16] = {
    {"cooker_mtx1"},
    {"cooker_mtx2"},
    {"cooker_mtx3"},
    {"cooker_mtx4"},
};

typedef struct
{
    int onoff;
    int resume;
    int pot_state;
    int pot_timeout;
    int timing;
    int afterheat;
    int fault;
    int gear;
} cooker_t;

struct
{
    rt_mutex_t  mutex;
    rt_mutex_t  cooker_mutex[4];
    int         onoff;
    int         language;
    cooker_t    cooker[4];
} cooker_inst;

static uint8_t tx_buffer[4] = {0};


void cooker_recv_handler(uint8_t * data)
{
    
}

static void cooker_get_ui_msg(void)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        // rt_kprintf("%s cooker_ui_state_get[%d]\r\n", rt_thread_self()->name, i);
        cooker_ui_state_get(&cooker_ui_state[i], i);
    }
}

TCM_CODE_DEFINE static void cook_ui_msg_send(void)
{
    cooker_resp_t resp[UI_COOKER_NUM] = {0};

    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        resp[i].serial = i;
        resp[i].gear = cooker_ui_state[i].on_max_gear ? 0x0A : cooker_ui_state[i].gear;
        resp[i].onoff = (cooker_ui_state[i].gear || cooker_ui_state[i].on_max_gear) ? 1 : 0;
        memset(tx_buffer, 0x00, sizeof(tx_buffer));
        memcpy(tx_buffer, &resp[i], 1);
        // cooker_send_bytes(tx_buffer, sizeof(tx_buffer));
        // if (resp[i].gear)
        // {
        //     rt_kprintf("Cooker%d: gear = %d, onoff = %d\r\n", resp[i].serial, resp[i].gear, resp[i].onoff);
        //     rt_kprintf("Cooker%d send: %02X %02X %02X %02X\r\n", i, tx_buffer[0], tx_buffer[1], tx_buffer[2], tx_buffer[3]);
        // }
        rt_thread_mdelay(5);
    }
}



static void cook_handler_thread_entry(void *param)
{
    while (1)
    {
        if (rt_sem_take(ui_sem, RT_WAITING_FOREVER) == RT_EOK)
        {
            cooker_get_ui_msg();
            cook_ui_msg_send();
        }
    }
}


static int cook_handler_thread_init(void)
{
    cooker_inst.mutex = rt_mutex_create("cooker_inst_mtx", RT_IPC_FLAG_PRIO);
    ui_sem = rt_sem_create("ui_sem", 0, RT_IPC_FLAG_PRIO);

    for (int i = 0; i < 4; i++)
    {
        cooker_inst.cooker_mutex[i] = rt_mutex_create(mtx_name[i], RT_IPC_FLAG_PRIO);
    }


    rt_thread_init(&cook_handler_thread,
                    "cooker_com_ui",
                    cook_handler_thread_entry,
                    RT_NULL,
                    &cook_handler_stack[0],
                    sizeof(cook_handler_stack),
                    HAND_THREAD_PRIORITY, HAND_THREAD_TIMESLICE);
    rt_thread_startup(&cook_handler_thread);
    
    return 0;
}
INIT_ENV_EXPORT(cook_handler_thread_init);



int cooker_hw_onoff_state_get(void)
{
    int state = 0;
    rt_mutex_take(cooker_inst.mutex, RT_WAITING_FOREVER);
    state = cooker_inst.onoff;
    rt_mutex_release(cooker_inst.mutex);
    return state;
}

int cooker_language_get(void)
{
    int lang = 0;
    rt_mutex_take(cooker_inst.mutex, RT_WAITING_FOREVER);
    lang = cooker_inst.language;
    rt_mutex_release(cooker_inst.mutex);
    return lang;
}

int cooker_state_get(uint8_t chan, cooker_t * cooker)
{
    if (chan >= 4 || !cooker)
    {
        return -1;
    }

    rt_mutex_take(cooker_inst.cooker_mutex[chan], RT_WAITING_FOREVER);

    *cooker = cooker_inst.cooker[chan];

    rt_mutex_release(cooker_inst.cooker_mutex[chan]);

    return 0;
}

void cooker_ui_data_notify(void)
{
    rt_sem_release(ui_sem);
}















