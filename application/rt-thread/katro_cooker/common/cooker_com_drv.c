#include "aic_core.h"
#include <rtthread.h>
#include "app_common.h"
#include <rtdevice.h>


#define TX_HWTIMER_DEV_NAME        "hrtimer0"
#define RX_HWTIMER_DEV_NAME        "hrtimer1"



static rt_device_t  tx_hw_tmr_dev;
static rt_device_t  rx_hw_tmr_dev;

static rt_sem_t     tx_sio_sem;
static rt_sem_t     rx_sio_sem;

static volatile int tx_sample_inx = 0;
static uint8_t      tx_sample_array[137] = {0};
static volatile int rx_sample_inx = 0;
static uint8_t      rx_sample_array[1600] = {0};
static volatile int idle_time = 0;
static volatile int test_flag = 0;


static int rx_read_level() { return rt_pin_read(drv_pin_get("PA.5")); }

static void rx_done_init(void)
{
    rt_device_control(rx_hw_tmr_dev, HWTIMER_CTRL_STOP, NULL);
    rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_DISABLE);
}


static rt_err_t tx_timeout_cb(rt_device_t dev, rt_size_t size)
{
    if (++tx_sample_inx >= sizeof(tx_sample_array))
    {
        rt_pin_write(drv_pin_get("PA.4"), 1);
        rt_device_control(tx_hw_tmr_dev, HWTIMER_CTRL_STOP, NULL);
        rt_sem_release(tx_sio_sem);
    }
    else
    {
        rt_pin_write(drv_pin_get("PA.4"), tx_sample_array[tx_sample_inx]);
    }
    return 0;
}

static rt_err_t rx_timeout_cb(rt_device_t dev, rt_size_t size)
{
    rx_sample_array[rx_sample_inx++] = rx_read_level();
    // test_flag = test_flag ? 0 : 1;
    // rt_pin_write(drv_pin_get("PB.6"), test_flag);

    if (rx_sample_inx == sizeof(rx_sample_array))
    {
        rx_done_init();
        rt_sem_release(rx_sio_sem);
        // test_flag = 0;
        // rt_pin_write(drv_pin_get("PB.6"), 1);
    }
    return 0;
}


static rt_err_t rx_wait_idle_cb(rt_device_t dev, rt_size_t size)
{
    idle_time += 100;

    int level = rx_read_level();

    if (level && idle_time >= 10000)
    {
        rt_device_control(rx_hw_tmr_dev, HWTIMER_CTRL_STOP, NULL);
        rt_sem_release(rx_sio_sem);
    }
    else if (level == 0)
    {
        idle_time = 0;
    }
    return 0;
}


static void rx_sio_irq(void * arg)
{
    rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_DISABLE);


    rt_hwtimerval_t timeout_s = {.usec = 100};
    rx_sample_inx = 0;
    idle_time = 0;

    memset(rx_sample_array, 0x00, sizeof(rx_sample_array));

    rt_device_write(rx_hw_tmr_dev, 0, &timeout_s, sizeof(timeout_s));

    // rt_pin_write(drv_pin_get("PB.6"), 0);
}



void cooker_drv_init(void)
{
    rt_hwtimer_mode_t mode = HWTIMER_MODE_PERIOD;

    tx_sio_sem = rt_sem_create("tx_sio_sem", 0, RT_IPC_FLAG_PRIO);
    rx_sio_sem = rt_sem_create("tx_sio_sem", 0, RT_IPC_FLAG_PRIO);

    tx_hw_tmr_dev = rt_device_find(TX_HWTIMER_DEV_NAME);
    rt_device_open(tx_hw_tmr_dev, RT_DEVICE_OFLAG_RDWR);
    rt_device_set_rx_indicate(tx_hw_tmr_dev, tx_timeout_cb);
    rt_device_control(tx_hw_tmr_dev, HWTIMER_CTRL_MODE_SET, &mode);

    rx_hw_tmr_dev = rt_device_find(RX_HWTIMER_DEV_NAME);
    rt_device_open(rx_hw_tmr_dev, RT_DEVICE_OFLAG_RDWR);
    rt_device_set_rx_indicate(rx_hw_tmr_dev, rx_timeout_cb);
    rt_device_control(rx_hw_tmr_dev, HWTIMER_CTRL_MODE_SET, &mode);

    rt_pin_mode(drv_pin_get("PA.4"), PIN_MODE_OUTPUT);
    rt_pin_mode(drv_pin_get("PB.6"), PIN_MODE_OUTPUT);
    rt_pin_mode(drv_pin_get("PA.5"), PIN_MODE_INPUT);
    rt_pin_attach_irq(drv_pin_get("PA.5"), PIN_IRQ_MODE_FALLING, rx_sio_irq, RT_NULL);


    // rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_ENABLE);

    rt_pin_write(drv_pin_get("PA.4"), 1);
}



int cooker_send_bytes(uint8_t * data, uint8_t length)
{
    uint32_t offset = 0;
    uint8_t sum = 0;
    rt_hwtimerval_t timeout_s = {0};
    
    memset(tx_sample_array, 0x00, sizeof(tx_sample_array));


    for (int i = 0; i < length - 1; i++)
    {
        sum += data[i];
    }
    sum = ~sum;

    data[length - 1] = sum;

    tx_sample_array[7] = 1;
    tx_sample_array[8] = 1;
    
    for (int i = 0; i < length; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            offset = (i * 32) + (j * 4) + 9;

            if (data[i] & (1 << j))
            {
                tx_sample_array[offset] = 0;
                tx_sample_array[offset + 1] = 0;
                tx_sample_array[offset + 2] = 0;
                tx_sample_array[offset + 3] = 1;
            }
            else
            {
                tx_sample_array[offset] = 0;
                tx_sample_array[offset + 1] = 1;
                tx_sample_array[offset + 2] = 1;
                tx_sample_array[offset + 3] = 1;
            }
        }
    }

    tx_sample_inx = 0;
    timeout_s.usec = 520;

    rt_device_write(tx_hw_tmr_dev, 0, &timeout_s, sizeof(timeout_s));

    rt_sem_take(tx_sio_sem, RT_WAITING_FOREVER);

    rt_device_control(tx_hw_tmr_dev, HWTIMER_CTRL_STOP, NULL);

    return length;
}


int cooker_wait_idle(void)
{
    rt_hwtimerval_t timeout_s = {.usec = 100};
    rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_DISABLE);
    rt_device_set_rx_indicate(rx_hw_tmr_dev, rx_wait_idle_cb);
    rt_device_write(rx_hw_tmr_dev, 0, &timeout_s, sizeof(timeout_s));
    rt_sem_take(rx_sio_sem, RT_WAITING_FOREVER);

    rt_device_set_rx_indicate(rx_hw_tmr_dev, rx_timeout_cb);
    rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_ENABLE);

    return 0;
}


int cooker_wait_data(uint8_t * data)
{
    uint8_t sum = 0;
    uint32_t ret = 0;
    uint32_t start_time = 0;
    uint32_t low_time = 0;
    // uint32_t high_time = 0;
    uint32_t offset = 0;
    volatile uint32_t bit_off = 0;
    uint8_t buf[4] = {0};
    uint32_t * p_data = (uint32_t * )buf;

    rt_sem_take(rx_sio_sem, RT_WAITING_FOREVER);

    rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_DISABLE);
    rt_device_control(rx_hw_tmr_dev, HWTIMER_CTRL_STOP, NULL);

    for (int i = 0; i < 35; i++, offset++)
    {
        if (rx_sample_array[i] == 0)
        {
            start_time += 100;
        }
        else
        {
            break;
        }
    }
    if (start_time < 3000)
    {
        // rt_kprintf("接收失败，起始帧过短 %d us.\r\n", start_time);
        ret = -1;
        goto __exit;
    }
    

    for (int i = offset; i < 50; i++, offset++)
    {
        if (rx_sample_array[i] == 1)
        {
            start_time += 100;
        }
        else
        {
            break;
        }
    }

    if (start_time < 4000)
    {
        // rt_kprintf("接收失败，起始帧格式错误, start_time = %d\r\n", start_time);
        ret = -1;
        goto __exit;
    }

    start_time = 0;

    for (int i = offset; i < sizeof(rx_sample_array); i++)
    {
        if (rx_sample_array[i] == 0)
        {
            low_time += 100;
        }
        start_time += 100;
        if (start_time >= 1500)
        {
            *p_data |= (low_time > 1000 ? 1 : 0) << (bit_off);

            if (++bit_off == 32)
            {
                break;
            }
            while (rx_sample_array[++i] == 1);
            start_time = 0;
            low_time = 0;
            continue;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        sum += buf[i];
    }
    sum = ~sum;

    if (sum != buf[3])
    {
        ret = -1;
        // rt_kprintf("Received failed, check sum error(%02X): %02X %02X %02X %02X \r\n",
        //                                                     sum, buf[0], buf[1], buf[2], buf[3]);
        goto __exit;
    }

    memcpy(data, buf, 4);

__exit:
    // rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_ENABLE);
    return ret;
}

