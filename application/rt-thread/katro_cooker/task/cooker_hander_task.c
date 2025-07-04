#include "cooker_protocol.h"
#include "app_ui_common.h"
#include "app_common.h"
#include "cooker_com_drv.h"

#define HAND_THREAD_PRIORITY         26
#define HAND_THREAD_TIMESLICE        5

#define LOG_THREAD_PRIORITY          30
#define LOG_THREAD_TIMESLICE         5



struct
{
    rt_mutex_t  mutex;
    rt_mutex_t  cooker_mutex[UI_COOKER_NUM];
    rt_mutex_t  cooker_hw_mutex[UI_COOKER_NUM];
    int         onoff;
    int         language;
    cooker_t    cooker[UI_COOKER_NUM];
    cooker_hw_t cooker_hw[UI_COOKER_NUM];
} cooker_inst;



static uint8_t tx_buffer[4] = {0};
static uint8_t rx_buffer[4] = {0};

static char cook_com_tx_stack[1024];
static char cook_com_rx_stack[1024];

static struct rt_thread cook_com_tx_thread;
static struct rt_thread cook_com_rx_thread;


static rt_sem_t ui_sem = RT_NULL;
static ui_cooker_state_t cooker_ui_state[UI_COOKER_NUM] = {0};

static const char mtx_name[UI_COOKER_NUM][16] = {
    {"cooker_mtx1"},
    {"cooker_mtx2"},
    {"cooker_mtx3"},
    {"cooker_mtx4"},
};
static const char hw_mtx_name[UI_COOKER_NUM][16] = {
    {"cooker_hw_mtx1"},
    {"cooker_hw_mtx2"},
    {"cooker_hw_mtx3"},
    {"cooker_hw_mtx4"},
};





static int list_cook_hw(int argc, char **argv)
{

    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        rt_mutex_take(cooker_inst.cooker_hw_mutex[i], RT_WAITING_FOREVER);
        rt_kprintf("\r\n=====================\tCooker %d State\t=====================\r\n", i);
        rt_kprintf("========\t故障码:%d 有无锅:%d 无锅超时%d 特殊锅面:%d\t========\r\n", cooker_inst.cooker_hw[i].fault,
                                                                            cooker_inst.cooker_hw[i].pot_state,
                                                                            cooker_inst.cooker_hw[i].pot_timeout,
                                                                            cooker_inst.cooker_hw[i].spec_pot);
        rt_kprintf("========\t线盘温度:%d IGBT温度:%d 电压:%d 电流:%d\t========\r\n", cooker_inst.cooker_hw[i].pan_temp,
                                                                            cooker_inst.cooker_hw[i].igbt_temp,
                                                                            cooker_inst.cooker_hw[i].voltage,
                                                                            cooker_inst.cooker_hw[i].current);
        rt_kprintf("=====================================================================\r\n", i);
        rt_mutex_release(cooker_inst.cooker_hw_mutex[i]);
    }
    return 0;
}
MSH_CMD_EXPORT(list_cook_hw, List all cooker hardware state);




static void cooker_msg_handler(uint8_t * data)
{
    cooker_fault_t * fault = (cooker_fault_t *)&data[0];
    cooker_state_t * state = (cooker_state_t *)&data[1];
    if (fault->serial > 3)
    {
        return;
    }
    rt_mutex_take(cooker_inst.cooker_hw_mutex[fault->serial], RT_WAITING_FOREVER);
    cooker_inst.cooker_hw[fault->serial].fault = fault->fault;
    cooker_inst.cooker_hw[fault->serial].pot_state = state->pot_state;
    cooker_inst.cooker_hw[fault->serial].pot_timeout = state->pot_timeout;
    cooker_inst.cooker_hw[fault->serial].spec_pot = state->spec_pot;
    cooker_inst.cooker_hw[fault->serial].afterheat = state->afterheat;
    switch(state->pot_chan)
    {
        case STATE_CH_PAN_TEMP:
            cooker_inst.cooker_hw[fault->serial].pan_temp = data[2];
        break;

        case STATE_CH_IGBT_TEMP:
            cooker_inst.cooker_hw[fault->serial].igbt_temp = data[2];
        break;

        case STATE_CH_VOLTAGE:
            cooker_inst.cooker_hw[fault->serial].voltage = data[2];
        break;
        
        case STATE_CH_CURRENT:
            cooker_inst.cooker_hw[fault->serial].current = data[2];
        break;
    }
    rt_mutex_release(cooker_inst.cooker_hw_mutex[fault->serial]);
}


static void cooker_get_ui_msg(void)
{
    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        cooker_ui_state_get(&cooker_ui_state[i], i);
    }
}



TCM_CODE_DEFINE static void cook_ui_msg_send(void)
{
    cooker_resp_t resp[UI_COOKER_NUM] = {0};

    for (int i = 0; i < UI_COOKER_NUM; i++)
    {
        resp[i].serial = i;

        if (cooker_ui_state[i].pause)
        {
            resp[i].gear = 0;
        }
        else if (cooker_ui_state[i].on_max_gear)
        {
            resp[i].gear = 0x0A;
        }
        else if (cooker_ui_state[i].thermos)
        {
            resp[i].gear = 0x01;
        }
        else
        {
            resp[i].gear = cooker_ui_state[i].gear;
        }
        resp[i].onoff = (cooker_ui_state[i].gear || cooker_ui_state[i].on_max_gear) ? 1 : 0;
        memset(tx_buffer, 0x00, sizeof(tx_buffer));
        memcpy(tx_buffer, &resp[i], 1);
        cooker_send_bytes(tx_buffer, sizeof(tx_buffer));
        rt_thread_mdelay(100);
    }
}



static void cook_com_tx_thread_entry(void *param)
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


static void cook_com_rx_thread_entry(void *param)
{ 
    int ret = 0;

    while (1)
    {
        ret = cooker_wait_data(rx_buffer);
        if (ret == 0)
        {
            cooker_msg_handler(rx_buffer);
        }
        rt_thread_mdelay(100);
    }
}




static int cook_handler_thread_init(void)
{
    cooker_drv_init();

    cooker_inst.mutex = rt_mutex_create("cooker_inst_mtx", RT_IPC_FLAG_PRIO);
    ui_sem = rt_sem_create("ui_sem", 0, RT_IPC_FLAG_PRIO);

    for (int i = 0; i < 4; i++)
    {
        cooker_inst.cooker_mutex[i] = rt_mutex_create(mtx_name[i], RT_IPC_FLAG_PRIO);
        cooker_inst.cooker_hw_mutex[i] = rt_mutex_create(hw_mtx_name[i], RT_IPC_FLAG_PRIO);
    }


    rt_thread_init(&cook_com_tx_thread,
                    "cooker_com_tx",
                    cook_com_tx_thread_entry,
                    RT_NULL,
                    &cook_com_tx_stack[0],
                    sizeof(cook_com_tx_stack),
                    HAND_THREAD_PRIORITY, HAND_THREAD_TIMESLICE);
    rt_thread_startup(&cook_com_tx_thread);

    rt_thread_init(&cook_com_rx_thread,
                    "cooker_com_rx",
                    cook_com_rx_thread_entry,
                    RT_NULL,
                    &cook_com_rx_stack[0],
                    sizeof(cook_com_rx_stack),
                    HAND_THREAD_PRIORITY, HAND_THREAD_TIMESLICE);
    rt_thread_startup(&cook_com_rx_thread);
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

int cooker_ui_read_hw_data(int channel, cooker_hw_t * data)
{
    if (channel >= 4 || !data)
    {
        return -1;
    }

    rt_mutex_take(cooker_inst.cooker_hw_mutex[channel], RT_WAITING_FOREVER);

    *data = cooker_inst.cooker_hw[channel];

    rt_mutex_release(cooker_inst.cooker_hw_mutex[channel]);

    return 0;
}













