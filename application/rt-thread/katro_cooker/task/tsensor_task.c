#include "aic_core.h"
#include "aic_log.h"
#include "sensor.h"
#include "hal_tsen.h"
#include <unistd.h>
#include <time.h>

static rt_device_t g_tsen_dev = NULL;
static struct rt_thread  g_tsen_thread_polling = {0};
static uint8_t tsen_statck[1024] = {0};
static rt_mutex_t tsen_mutex;
static struct rt_sensor_data data;

static void tsen_polling_thread_entry(void *parameter)
{
    while (1) 
    {
        rt_mutex_take(tsen_mutex, RT_WAITING_FOREVER);
        rt_device_read(g_tsen_dev, 0, &data, 1);
        rt_mutex_release(tsen_mutex);
        rt_thread_mdelay(1000);
    }
}


static int tsen_start_polling(void)
{
    int ret = 0;
    
    tsen_mutex = rt_mutex_create("tsen_mtx", RT_IPC_FLAG_PRIO);

    g_tsen_dev = rt_device_find("temp_tsen_cpu");
    if (g_tsen_dev == RT_NULL) {
        rt_kprintf("Failed to find %s device\n", "temp_tsen_cpu");
        return -1;
    }

    ret = rt_device_open(g_tsen_dev, RT_DEVICE_FLAG_RDONLY);
    if (ret != RT_EOK) {
        rt_kprintf("Failed to open %s device\n", "temp_tsen_cpu");
        return -1;
    }

    // rt_device_control(g_tsen_dev, RT_SENSOR_CTRL_GET_CH_INFO, &tsen_info);

    rt_thread_init(&g_tsen_thread_polling,
                    "tsen_polling",
                    tsen_polling_thread_entry, 
                    RT_NULL,
                    &tsen_statck[0],
                    sizeof(tsen_statck),
                    27, 10);
    rt_thread_startup(&g_tsen_thread_polling);
    return 0;
}
INIT_ENV_EXPORT(tsen_start_polling);


int cpu_temp_get(int * temp)
{
    if (rt_mutex_take(tsen_mutex, 50) == RT_EOK)
    {
        // data.data.temp / 10, (rt_uint32_t)data.data.temp % 10;
        *temp = data.data.temp;

        rt_mutex_release(tsen_mutex);
        return 0;
    }
    return -1;
}

