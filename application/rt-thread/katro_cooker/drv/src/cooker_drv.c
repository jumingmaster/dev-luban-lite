#include "cooker_drv.h"

static void tx_set_high() { rt_pin_write(drv_pin_get("PA.4"), PIN_HIGH); }
static void tx_set_low() { rt_pin_write(drv_pin_get("PA.4"), PIN_LOW); }
static int rx_read_level() { return rt_pin_read(drv_pin_get("PA.5")); }



static void send_bit(uint8_t bit) 
{
    tx_set_high();
    aic_udelay(bit ? BIT1_HIGH : BIT0_HIGH);
    tx_set_low();
    aic_udelay(LOW_MIN);

    
}

static void send_byte(uint8_t data) 
{
    for(int i = 0; i < 8; i++) 
    {
        send_bit(data & (1 << (7 - i)));
    }
}

static uint8_t receive_bit(void)
{
    u64  start = 0, end = 0;
    u64  duration = 0;

    while(rx_read_level() == 0) __NOP();
    start = aic_get_time_us();
    while(rx_read_level() == 1) __NOP();
    end = aic_get_time_us();
    duration = end - start;

    return duration > (BIT1_HIGH + BIT0_HIGH)/2;
}

static uint8_t receive_byte(void)
{
    uint8_t byte = 0;

    for(int i = 0; i < 8; i++)
    {
        byte |= receive_bit() << (7 - i);
    }
}


void cooker_drv_init(void (*rx_irq)(void *args))
{
    rt_pin_mode(drv_pin_get("PA.4"), PIN_MODE_OUTPUT);
    rt_pin_mode(drv_pin_get("PA.5"), PIN_MODE_INPUT_PULLDOWN);
    rt_pin_attach_irq(drv_pin_get("PA.5"), PIN_IRQ_MODE_RISING, rx_irq, RT_NULL);
    rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_ENABLE);

    tx_set_low();
}

int cooker_read_bytes(uint8_t * data, uint8_t length)
{
    int ret = 0;
    uint8_t chksum = 0;

    rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_DISABLE);

    aic_udelay(START_MIN/2);

    if(rx_read_level() == 0) 
    {
        ret = -1;
        goto __exit;
    } 

    for(int i = 0; i < length; i++) 
    {
        data[i] = receive_byte();
    }

    for (int i = 0; i < length - 1; i++)
    {
        chksum += data[i];
    }

    chksum = ~chksum;

    if (chksum != data[3])
    {
        ret = -1;
    }

__exit:
    rt_pin_irq_enable(drv_pin_get("PA.5"), PIN_IRQ_ENABLE);

    return ret;
}


int cooker_send_bytes(uint8_t * data, uint8_t length)
{
    // Calculate check sum;
    for (int i = 0; i < length; i++)
    {
        data[length - 1] = data[i]++;
    }
    data[length - 1] = ~data[length - 1];

    for (int i = 0; i < length; i++)
    {
        send_byte(data[i]);
    }
}




