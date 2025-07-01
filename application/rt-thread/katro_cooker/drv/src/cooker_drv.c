#include "cooker_drv.h"


static void tx_set_high() { rt_pin_write(drv_pin_get("PA.4"), PIN_HIGH); }
static void tx_set_low() { rt_pin_write(drv_pin_get("PA.4"), PIN_LOW); }
static int rx_read_level() { return rt_pin_read(drv_pin_get("PA.5")); }





static void send_bit(uint8_t bit) 
{
    tx_set_low();
    aic_udelay(bit ? BIT1_HIGH : BIT0_HIGH);
    tx_set_high();
    aic_udelay(bit ? BIT0_HIGH : BIT1_HIGH);
}

static void send_byte(uint8_t data) 
{
    for(int i = 0; i < 8; i++) 
    {
        send_bit(data & (1 << i));
    }
}

static uint8_t receive_bit(void)
{
    u64 timemout = 0;
    u64  start = 0, end = 0;
    u64  duration = 0;

    timemout = aic_get_time_us();

    while(rx_read_level() == 0)
    {
        if (aic_get_time_us() - timemout > 3500)
        {
            return 0;
        }
    }
    
    start = aic_get_time_us();
    timemout = aic_get_time_us();

    while(rx_read_level() == 1)
    {
        if (aic_get_time_us() - timemout > 3500)
        {
            return 0;
        }
    }

    end = aic_get_time_us();
    duration = end - start;

    return duration > (BIT1_HIGH + BIT0_HIGH)/2;
}



// static uint8_t receive_bit(void)
// {
//     if(rx_read_level() == 0)
//     {

//     }

    

//     return 0;
// }


static uint8_t receive_byte(void)
{
    uint8_t byte = 0;

    for(int i = 0; i < 8; i++)
    {
        byte |= receive_bit() << i;
    }
    return byte;
}


void cooker_drv_init(void (*rx_irq)(void *args))
{
    rt_pin_mode(drv_pin_get("PA.4"), PIN_MODE_OUTPUT);
    rt_pin_mode(drv_pin_get("PA.5"), PIN_MODE_INPUT);
    // rt_pin_attach_irq(drv_pin_get("PA.5"), PIN_IRQ_MODE_FALLING, rx_irq, RT_NULL);
    // rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_ENABLE);

    tx_set_low();
}

int cooker_read_bytes(uint8_t * data, uint8_t length)
{
    // int ret = 0;
    uint8_t chksum = 0;
    u64 timeout = 0;

    rt_enter_critical();

    while (rx_read_level() == 0) 
    {
        aic_udelay(100);
        timeout += 100;
    }

    if (timeout < 2500)
    {
        rt_exit_critical();
        return -1;
    }

    timeout = 0;

    while (rx_read_level() == 1)
    {
        aic_udelay(100);
        timeout += 100;
    }

    if (timeout < 1000)
    {
        rt_exit_critical();
        return -1;
    }
    
    for(int i = length - 1; i > 0; i--) 
    {
        data[i] = receive_byte();
    }
    rt_exit_critical();
    for (int i = 0; i < length - 1; i++)
    {
        chksum += data[i];
    }

    chksum = ~chksum;

    if (chksum != data[3])
    {
        // ret = -1;
        rt_kprintf("%02X %02X %02X %02X chksum error\r\n", data[0], data[1], data[2], data[3]);
        return -1;
    }

// __exit:
    return 0;
}


int cooker_send_bytes(uint8_t * data, uint8_t length)
{
    uint8_t sum = 0;

    // Calculate check sum;
    for (int i = 0; i < length; i++)
    {
        sum += data[i];
    }
   sum = ~sum;

   data[length - 1] = sum;
    
    rt_enter_critical();
    tx_set_low();
    aic_udelay(START_MIN);
    tx_set_high();
    aic_udelay(1050);
    
    for (int i = 0; i < length; i++)
    {
        send_byte(data[i]);
    }
    

    tx_set_high();
    rt_exit_critical();

    // rt_kprintf("send_byte :\r\n");
    // for (int i = 0; i < length; i++)
    // {
    //     rt_kprintf("%02X ", data[i]);
    // }
    // rt_kprintf("\r\n");
    

    return length;
}




// uint8_t cooker_drv_gen_sum(const uint8_t * data, uint8_t length)
// {
//     uint8_t sum = 0;
//     for (int i = 0; i < length; i++)
//     {
//         sum = data[i]++;
//     }
//    sum = ~sum;

//    data[length - 1] = sum;
// }





