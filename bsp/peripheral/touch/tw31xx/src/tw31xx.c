/*
 * Copyright (c) 2024, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date              Notes
 * 2024-06-24        the first version
 */
// #include <rtthread.h>
// #include <rtdevice.h>
// #include <string.h>
// #include "tw31xx.h"
// #define DBG_TAG "tw31xx"
// #define DBG_LVL DBG_INFO
// #include <rtdbg.h>

// static struct rt_i2c_client tw31xx_client;

// static rt_err_t tw31xx_write_reg(struct rt_i2c_client *dev, rt_uint8_t *data,
//                                 rt_uint8_t len)
// {
//     struct rt_i2c_msg msgs;

//     msgs.addr = dev->client_addr;
//     msgs.flags = RT_I2C_WR;
//     msgs.buf = data;
//     msgs.len = len;

//     if (rt_i2c_transfer(dev->bus, &msgs, 1) == 1) {
//         return RT_EOK;
//     } else {
//         rt_kprintf("i2c write tw31xx fail!\n");
//         return -RT_ERROR;
//     }
// }

// static rt_err_t tw31xx_read_regs(struct rt_i2c_client *dev, rt_uint8_t *reg,
//                                 rt_uint8_t *data, rt_uint8_t len)
// {
//     struct rt_i2c_msg msgs[2];

//     msgs[0].addr = dev->client_addr;
//     msgs[0].flags = RT_I2C_WR;
//     msgs[0].buf = reg;
//     msgs[0].len = TW31XX_REGITER_LEN;

//     msgs[1].addr = dev->client_addr;
//     msgs[1].flags = RT_I2C_RD;
//     msgs[1].buf = data;
//     msgs[1].len = len;

//     if (rt_i2c_transfer(dev->bus, msgs, 2) == 2) {
//         return RT_EOK;
//     } else {
//         rt_kprintf("i2c read tw31xx fail!\n");
//         return -RT_ERROR;
//     }
// }

// static rt_err_t tw31xx_get_product_id(struct rt_i2c_client *dev,
//                                      rt_uint8_t *data, rt_uint8_t len)
// {
//     rt_uint8_t reg[2];

//     reg[0] = (rt_uint8_t)(TW31XX_REG_CHIP_ID >> 8);
//     reg[1] = (rt_uint8_t)(TW31XX_REG_CHIP_ID & 0xff);

//     tw31xx_read_regs(dev, reg, data, len);

//     return RT_EOK;
// }

// static void tw31xx_i2c_done(void)
// {
//     rt_uint8_t buf[3];

//     buf[0] = (rt_uint8_t)(TW31XX_REG_COMMAND >> 8);
//     buf[1] = (rt_uint8_t)(TW31XX_REG_COMMAND & 0xff);
//     buf[2] = 0x01;

//     tw31xx_write_reg(&tw31xx_client, buf, 3);
// }

// static rt_size_t tw31xx_read_point(struct rt_touch_device *touch, void *buf,
//                                   rt_size_t read_num)
// {
//     rt_uint8_t reg[2];
//     rt_uint32_t tp_buf;
//     rt_uint8_t read_buf[40] = {0};
//     struct rt_touch_data *temp_data;
//     rt_uint8_t event = RT_TOUCH_EVENT_NONE;
//     static rt_uint16_t judge = 0;

//     rt_memset(read_buf, 0, sizeof(read_buf));

//     temp_data = (struct rt_touch_data *)buf;
//     temp_data->event = RT_TOUCH_EVENT_NONE;

//     reg[0] = (rt_uint8_t)(TW31XX_REG_EVENT >> 8);
//     reg[1] = (rt_uint8_t)(TW31XX_REG_EVENT & 0xff);

//     tw31xx_read_regs(&tw31xx_client, reg, read_buf, sizeof(read_buf));

//     if (read_buf[0] == TW31XX_EVENT_ABS) {
//         reg[0] = (rt_uint8_t)(TW31XX_REG_POINT >> 8);
//         reg[1] = (rt_uint8_t)(TW31XX_REG_POINT & 0xff);
//         tw31xx_read_regs(&tw31xx_client, reg, read_buf, 4);

//         tp_buf = read_buf[0] | read_buf[1] << 8 | read_buf[2] << 16 | read_buf[3] << 24;
//         event = (read_buf[3] & 0x80) ? 1 : 0;    // 1: press, 0 : release
//         if (event)
//             judge++;

//         if (event && judge == 1) {
//             temp_data->event = RT_TOUCH_EVENT_DOWN;
//         } else if (event && judge > 1) {
//             temp_data->event = RT_TOUCH_EVENT_MOVE;
//         } else if (!event) {
//             temp_data->event = RT_TOUCH_EVENT_UP;
//             judge = 0;
//         } else {
//             temp_data->event = RT_TOUCH_EVENT_NONE;
//         }

//         temp_data->y_coordinate = tp_buf & 0x1fff;
//         temp_data->x_coordinate = (tp_buf >> 13) & 0x1fff;
//     } else {
//         goto __exit;
//     }

// __exit:
//     tw31xx_i2c_done();
//     return read_num;
// }

// static rt_err_t tw31xx_control(struct rt_touch_device *touch, int cmd, void *arg)
// {
//     struct rt_touch_info *info;

//     switch(cmd)
//     {
//     case RT_TOUCH_CTRL_GET_ID:
//         tw31xx_get_product_id(&tw31xx_client, arg, 2);
//         break;
//     case RT_TOUCH_CTRL_GET_INFO:
//         info = (struct rt_touch_info *)arg;
//         info->point_num = 1;
//         info->type = touch->info.type;
//         info->vendor = touch->info.vendor;

//         break;
//     case RT_TOUCH_CTRL_SET_MODE:
//         break;
//     case RT_TOUCH_CTRL_SET_X_RANGE:
//         break;
//     case RT_TOUCH_CTRL_SET_Y_RANGE:
//         break;
//     case RT_TOUCH_CTRL_SET_X_TO_Y:
//         break;
//     case RT_TOUCH_CTRL_DISABLE_INT:
//         break;
//     case RT_TOUCH_CTRL_ENABLE_INT:
//         break;
//     default:
//         break;
//     }

//     return RT_EOK;
// }

// static struct rt_touch_ops tw31xx_touch_ops = {
//     .touch_readpoint = tw31xx_read_point,
//     .touch_control = tw31xx_control,
// };

// static int tw31xx_hw_init(const char *name, struct rt_touch_config *cfg)
// {
//     struct rt_touch_device *touch_device = RT_NULL;

//     touch_device =
//         (struct rt_touch_device *)rt_malloc(sizeof(struct rt_touch_device));
//     if (touch_device == RT_NULL) {
//         LOG_E("touch device malloc fail");
//         return -RT_ERROR;
//     }
//     rt_memset((void *)touch_device, 0, sizeof(struct rt_touch_device));
//     //rst output 1
//     rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
//     rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_LOW);
//     //irq output 1
//     rt_pin_mode(cfg->irq_pin.pin, PIN_MODE_OUTPUT);
//     rt_pin_write(cfg->irq_pin.pin, PIN_HIGH);
//     rt_thread_mdelay(100);
//     //rst output 1
//     rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
//     rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_HIGH);
//     rt_thread_mdelay(100);
//     //rst output 0
//     rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
//     rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_LOW);
//     rt_thread_mdelay(100);
//     //rst output 1
//     rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
//     rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_HIGH);
//     rt_thread_mdelay(100);

//     tw31xx_client.bus =
//         (struct rt_i2c_bus_device *)rt_device_find(cfg->dev_name);

//     if (tw31xx_client.bus == RT_NULL) {
//         LOG_E("Can't find %s device", cfg->dev_name);
//         return -RT_ERROR;
//     }

//     if (rt_device_open((rt_device_t)tw31xx_client.bus, RT_DEVICE_FLAG_RDWR) !=
//         RT_EOK) {
//         LOG_E("open %s device failed", cfg->dev_name);
//         return -RT_ERROR;
//     }

//     tw31xx_client.client_addr = TW31XX_ADDR;

//     /* get init even */
//     rt_uint8_t reg[2];
//     rt_uint8_t read_buf[2] = {0};
//     reg[0] = (rt_uint8_t)(TW31XX_REG_EVENT >> 8);
//     reg[1] = (rt_uint8_t)(TW31XX_REG_EVENT & 0xff);

//     tw31xx_read_regs(&tw31xx_client, reg, read_buf, 1);
//     if (read_buf[0] == TW31XX_EVENT_GES) {
//         tw31xx_i2c_done();
//     }

//     //rst output 1
//     rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
//     rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_HIGH);
//     rt_thread_mdelay(100);
//     //rst output 0
//     rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
//     rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_LOW);
//     rt_thread_mdelay(100);
//     //rst output 1
//     rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
//     rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_HIGH);
//     rt_thread_mdelay(100);
//     //irq input
//     rt_pin_mode(cfg->irq_pin.pin, PIN_MODE_INPUT_PULLUP);
//     // rst input
//     rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_INPUT);

//     /* register touch device */
//     touch_device->info.type = RT_TOUCH_TYPE_CAPACITANCE;
//     touch_device->info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
//     rt_memcpy(&touch_device->config, cfg, sizeof(struct rt_touch_config));
//     touch_device->ops = &tw31xx_touch_ops;

//     if (RT_EOK != rt_hw_touch_register(touch_device, name, RT_DEVICE_FLAG_INT_RX, RT_NULL)) {
//         LOG_E("touch device tw31xx init failed !!!");
//         return -RT_ERROR;
//     }

//     LOG_I("touch device tw31xx init success");
//     return RT_EOK;
// }

// static int tw31xx_gpio_cfg()
// {
//     unsigned int g, p;
//     long pin;

//     // RST
//     pin = drv_pin_get(AIC_TOUCH_PANEL_RST_PIN);
//     g = GPIO_GROUP(pin);
//     p = GPIO_GROUP_PIN(pin);
//     hal_gpio_direction_input(g, p);

//     // INT
//     pin = drv_pin_get(AIC_TOUCH_PANEL_INT_PIN);
//     g = GPIO_GROUP(pin);
//     p = GPIO_GROUP_PIN(pin);
//     hal_gpio_direction_input(g, p);
//     hal_gpio_set_irq_mode(g, p, 0);

//     return 0;
// }

// static int rt_hw_tw31xx_init(void)
// {
//     struct rt_touch_config cfg;
//     rt_uint8_t rst_pin;

//     tw31xx_gpio_cfg();

//     rst_pin = drv_pin_get(AIC_TOUCH_PANEL_RST_PIN);
//     cfg.dev_name = AIC_TOUCH_PANEL_I2C_CHAN;
//     cfg.irq_pin.pin = drv_pin_get(AIC_TOUCH_PANEL_INT_PIN);
//     cfg.irq_pin.mode = PIN_MODE_INPUT_PULLUP;
//     cfg.user_data = &rst_pin;

//     tw31xx_hw_init("tw31xx", &cfg);

//     return 0;
// }

// INIT_DEVICE_EXPORT(rt_hw_tw31xx_init);



#include <rtthread.h>
#include <rtdevice.h>
#include <string.h>
#include "tw31xx.h"
#define DBG_TAG "tw31xx"
#define DBG_LVL DBG_INFO
#include <rtdbg.h>
#include "touch_common.h"

static struct rt_i2c_client tw31xx_client;
struct fdata gfdata_tp;
rt_uint8_t gPwFlag = 0;

unsigned int reset_g;
unsigned int reset_p;

rt_thread_t thread_esd;

static rt_err_t tw31xx_write_reg(struct rt_i2c_client *dev, rt_uint8_t *data,
                                rt_uint8_t len)
{
    struct rt_i2c_msg msgs;

    msgs.addr = dev->client_addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf = data;
    msgs.len = len;

    if (rt_i2c_transfer(dev->bus, &msgs, 1) == 1) {
        return RT_EOK;
    } else {
        // rt_kprintf("i2c write tw31xx fail!\n");
        return -RT_ERROR;
    }
}

static rt_err_t tw31xx_read_regs(struct rt_i2c_client *dev, rt_uint8_t *reg,
                                rt_uint8_t *data, rt_uint8_t len)
{
    struct rt_i2c_msg msgs[2];

    msgs[0].addr = dev->client_addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = reg;
    msgs[0].len = TW31XX_REGITER_LEN;

    msgs[1].addr = dev->client_addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = data;
    msgs[1].len = len;

    if (rt_i2c_transfer(dev->bus, msgs, 2) == 2) {
        return RT_EOK;
    } else {
        // rt_kprintf("i2c read tw31xx fail!\n");
        return -RT_ERROR;
    }
}

static rt_err_t tw31xx_get_product_id(struct rt_i2c_client *dev,
                                     rt_uint8_t *data, rt_uint8_t len)
{
    rt_uint8_t reg[2];

    reg[0] = (rt_uint8_t)(TW31XX_REG_CHIP_ID >> 8);
    reg[1] = (rt_uint8_t)(TW31XX_REG_CHIP_ID & 0xff);

    tw31xx_read_regs(dev, reg, data, len);

    return RT_EOK;
}

static void tw31xx_i2c_done(void)
{
    rt_uint8_t buf[3];

    buf[0] = (rt_uint8_t)(TW31XX_REG_COMMAND >> 8);
    buf[1] = (rt_uint8_t)(TW31XX_REG_COMMAND & 0xff);
    buf[2] = 0x01;

    tw31xx_write_reg(&tw31xx_client, buf, 3);
}

static rt_uint8_t techwin_Checksum_u8(rt_uint8_t* pbuff, int length, rt_uint8_t resum)
{
	rt_uint8_t sum = 0;
	
	while(length--)
		sum += *pbuff++;

	return (sum-resum);
}

static rt_size_t tw31xx_read_point(struct rt_touch_device *touch, void *buf, rt_size_t read_num)
{
    const int retry_config = 10;
	int ret = 0;
	int retry = 0;
    rt_uint8_t reg[2];
    // rt_uint32_t tp_buf;
    rt_uint8_t read_buf[40] = {0};
    struct rt_touch_data *temp_data;
    rt_uint8_t event = RT_TOUCH_EVENT_NONE;
    static rt_uint16_t judge = 0;
	rt_uint8_t clear_buf[3] = {0x00, 0x00, 0xFF};

    rt_memset(read_buf, 0, sizeof(read_buf));

    temp_data = (struct rt_touch_data *)buf;
    temp_data->event = RT_TOUCH_EVENT_NONE;

    reg[0] = (rt_uint8_t)(TW31XX_REG_EVENT >> 8);
    reg[1] = (rt_uint8_t)(TW31XX_REG_EVENT & 0xff);

    retry = retry_config;

	do{
		ret = tw31xx_read_regs(&tw31xx_client, reg, (rt_uint8_t*)(&gfdata_tp.tp_head), 16);
		if(ret)
		{	
			// LOG_E("ERROR, i2c read head error\n");
			continue;
		}
		
		if(techwin_Checksum_u8((rt_uint8_t*)(&gfdata_tp.tp_head), 16, gfdata_tp.tp_head.Checksum) != gfdata_tp.tp_head.Checksum)//checksum error
		{
			if(retry == 1){
				goto __exit;
			}
			//LOG_E("touch head checksum error\n");//wabil
		}
		else
		{ //checksum ok
			retry = retry_config;
			break;
		}
	}while(retry--);

	/* clear reg */
	tw31xx_write_reg(&tw31xx_client, clear_buf, 3);

	if(TW31XX_EVENT_ABS == gfdata_tp.tp_head.Event)
	{
		do{
			reg[0] = (rt_uint8_t)(gfdata_tp.tp_head.Address >> 8);
	        reg[1] = (rt_uint8_t)(gfdata_tp.tp_head.Address & 0xff);
	        ret = tw31xx_read_regs(&tw31xx_client, reg, (u8*)(&gfdata_tp.tp_data), gfdata_tp.tp_head.Length);
			if(ret)
			{
				// LOG_E("ERROR, i2c read data error\n");
				continue;
			}
	
			if(techwin_Checksum_u8((u8*)(&gfdata_tp.tp_data), gfdata_tp.tp_head.Length, gfdata_tp.tp_data.SubInfo.Checksum) != gfdata_tp.tp_data.SubInfo.Checksum)
			{ 
				if(retry == 1){
					goto __exit;
				}
				//LOG_E("touch data checksum error\n"); //wabil
			}
			else //checksum ok
			{
				break;
			}
	
		}while(retry--);

		event = gfdata_tp.tp_data.point[0].Stat;
			
		if (event == 1)
            judge++;

        if (event && judge == 1) {
            temp_data->event = RT_TOUCH_EVENT_DOWN;
        } else if (event && judge > 1) {
            temp_data->event = RT_TOUCH_EVENT_MOVE;
        } else if (!event) {
            temp_data->event = RT_TOUCH_EVENT_UP;
            judge = 0;
        } else {
            temp_data->event = RT_TOUCH_EVENT_NONE;
        }

        temp_data->y_coordinate = gfdata_tp.tp_data.point[0].X;
        temp_data->x_coordinate = gfdata_tp.tp_data.point[0].Y;
            aic_touch_flip(&temp_data->x_coordinate, &temp_data->y_coordinate);
            aic_touch_rotate(&temp_data->x_coordinate, &temp_data->y_coordinate);
            aic_touch_scale(&temp_data->x_coordinate, &temp_data->y_coordinate);
            aic_touch_crop(&temp_data->x_coordinate, &temp_data->y_coordinate);
             

        // rt_kprintf("tp:(%d, %d)\n", temp_data->x_coordinate, temp_data->y_coordinate);
	}

	if(TW31XX_EVENT_TPPWON == gfdata_tp.tp_head.Event)
	{
		gPwFlag = 1;
	}


__exit:
    tw31xx_i2c_done();
    return read_num;
}

void techwin_esd_work(void *parameter)
{
    int ret = 0, i = 0;
	uint16_t TimeTick;
	static uint8_t FlagReset = 0;
	static uint16_t WdtTimeTick = 0;
    rt_uint8_t reg[2];

    reset_g = GPIO_GROUP(drv_pin_get(AIC_TOUCH_PANEL_RST_PIN));
    reset_p = GPIO_GROUP_PIN(drv_pin_get(AIC_TOUCH_PANEL_RST_PIN));

    rt_kprintf("%s(%d)\n", __func__, __LINE__);

	while(1)
	{
		rt_thread_delay(2000);
		
		/* Read Time Tick Register */
		for(i=0; i<30; i++)
		{
			reg[0] = (rt_uint8_t)(I2C_STATUS_REG_ADDR >> 8);
			reg[1] = (rt_uint8_t)(I2C_STATUS_REG_ADDR & 0xff);
			ret = tw31xx_read_regs(&tw31xx_client, reg, (u8*)&TimeTick, 2);
			if (ret == RT_EOK)
			{
				if(FlagReset == 1)//复位后处理
				{
					LOG_E("after reset WDT = (%d, %d)\n", TimeTick, WdtTimeTick);
					WdtTimeTick = TimeTick;
					FlagReset = 0;
					goto esd_start;
				}
				else
				{
					if(TimeTick != WdtTimeTick){ //正常
						LOG_E("WDT = (%d, %d)\n", TimeTick, WdtTimeTick);
						WdtTimeTick = TimeTick;
						goto esd_start;
					}
					else //不正常
					{
						goto error_reset_device;
					}
				}
			}
			else
			{
				LOG_E("error, esd i2c read error, retry = %d\n", i);	
			}
	
			//msleep(1);
	    }
error_reset_device:
		/* device reset */
		// techwin_reset();
        hal_gpio_clr_output(reset_g, reset_p);
        rt_kprintf("%s(%d)\n", __func__, __LINE__);
        rt_thread_mdelay(10);
        hal_gpio_set_output(reset_g, reset_p);
        
		FlagReset = 1;
		LOG_E("esd reset IC!!!\n");
	
esd_start:
        LOG_E("esd start\n");
	}
}

static rt_err_t tw31xx_control(struct rt_touch_device *touch, int cmd, void *arg)
{
    struct rt_touch_info *info;

    switch(cmd)
    {
    case RT_TOUCH_CTRL_GET_ID:
        tw31xx_get_product_id(&tw31xx_client, arg, 2);
        break;
    case RT_TOUCH_CTRL_GET_INFO:
        info = (struct rt_touch_info *)arg;
        info->point_num = 1;
        info->type = touch->info.type;
        info->vendor = touch->info.vendor;

        break;
    case RT_TOUCH_CTRL_SET_MODE:
        break;
    case RT_TOUCH_CTRL_SET_X_RANGE:
        break;
    case RT_TOUCH_CTRL_SET_Y_RANGE:
        break;
    case RT_TOUCH_CTRL_SET_X_TO_Y:
        break;
    case RT_TOUCH_CTRL_DISABLE_INT:
        break;
    case RT_TOUCH_CTRL_ENABLE_INT:
        break;
    default:
        break;
    }

    return RT_EOK;
}

static struct rt_touch_ops tw31xx_touch_ops = {
    .touch_readpoint = tw31xx_read_point,
    .touch_control = tw31xx_control,
};

static int tw31xx_hw_init(const char *name, struct rt_touch_config *cfg)
{
    struct rt_touch_device *touch_device = RT_NULL;

    touch_device =
        (struct rt_touch_device *)rt_malloc(sizeof(struct rt_touch_device));
    if (touch_device == RT_NULL) {
        LOG_E("touch device malloc fail");
        return -RT_ERROR;
    }
    rt_memset((void *)touch_device, 0, sizeof(struct rt_touch_device));
    //rst output 1
    rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
    rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_LOW);
    //irq output 1
    rt_pin_mode(cfg->irq_pin.pin, PIN_MODE_OUTPUT);
    rt_pin_write(cfg->irq_pin.pin, PIN_HIGH);
    rt_thread_mdelay(100);
    //rst output 1
    rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
    rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_HIGH);
    rt_thread_mdelay(100);
    //rst output 0
    rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
    rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_LOW);
    rt_thread_mdelay(100);
    //rst output 1
    rt_pin_mode(*(rt_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
    rt_pin_write(*(rt_uint8_t *)cfg->user_data, PIN_HIGH);
    rt_thread_mdelay(100);

    tw31xx_client.bus =
        (struct rt_i2c_bus_device *)rt_device_find(cfg->dev_name);

    if (tw31xx_client.bus == RT_NULL) {
        LOG_E("Can't find %s device", cfg->dev_name);
        return -RT_ERROR;
    }

    if (rt_device_open((rt_device_t)tw31xx_client.bus, RT_DEVICE_FLAG_RDWR) !=
        RT_EOK) {
        LOG_E("open %s device failed", cfg->dev_name);
        return -RT_ERROR;
    }

    tw31xx_client.client_addr = TW31XX_ADDR;

    /* get init even */
    rt_uint8_t reg[2];
    rt_uint8_t read_buf[2] = {0};
    reg[0] = (rt_uint8_t)(TW31XX_REG_EVENT >> 8);
    reg[1] = (rt_uint8_t)(TW31XX_REG_EVENT & 0xff);

    tw31xx_read_regs(&tw31xx_client, reg, read_buf, 1);
    if (read_buf[0] == TW31XX_EVENT_GES) {
        tw31xx_i2c_done();
    }
    /* register touch device */
    touch_device->info.type = RT_TOUCH_TYPE_CAPACITANCE;
    touch_device->info.vendor = RT_TOUCH_VENDOR_UNKNOWN;
    rt_memcpy(&touch_device->config, cfg, sizeof(struct rt_touch_config));
    touch_device->ops = &tw31xx_touch_ops;

    if (RT_EOK != rt_hw_touch_register(touch_device, name, RT_DEVICE_FLAG_INT_RX, RT_NULL)) {
        LOG_E("touch device tw31xx init failed !!!");
        return -RT_ERROR;
    }

    LOG_I("touch device tw31xx init success");

    // add thread

    // thread_esd = rt_thread_create("esd", techwin_esd_work, RT_NULL,
    //                        RT_MAIN_THREAD_STACK_SIZE, RT_MAIN_THREAD_PRIORITY, 20);
    // RT_ASSERT(thread_esd != RT_NULL);
    // rt_thread_startup(thread_esd);

    return RT_EOK;
}

static int tw31xx_gpio_cfg()
{
    unsigned int g, p;
    long pin;

    // RST
    pin = drv_pin_get(AIC_TOUCH_PANEL_RST_PIN);
    g = GPIO_GROUP(pin);
    p = GPIO_GROUP_PIN(pin);
    hal_gpio_direction_input(g, p);

    // INT
    pin = drv_pin_get(AIC_TOUCH_PANEL_INT_PIN);
    g = GPIO_GROUP(pin);
    p = GPIO_GROUP_PIN(pin);
    hal_gpio_direction_input(g, p);
    hal_gpio_set_irq_mode(g, p, 0);

    return 0;
}

static int rt_hw_tw31xx_init(void)
{
    struct rt_touch_config cfg;
    rt_uint8_t rst_pin;

    tw31xx_gpio_cfg();

    rst_pin = drv_pin_get(AIC_TOUCH_PANEL_RST_PIN);
    cfg.dev_name = AIC_TOUCH_PANEL_I2C_CHAN;
    cfg.irq_pin.pin = drv_pin_get(AIC_TOUCH_PANEL_INT_PIN);
    cfg.irq_pin.mode = PIN_MODE_INPUT_PULLUP;
    cfg.user_data = &rst_pin;

    tw31xx_hw_init("tw31xx", &cfg);

    return 0;
}

INIT_DEVICE_EXPORT(rt_hw_tw31xx_init);

