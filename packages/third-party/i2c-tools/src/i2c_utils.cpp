/*
  SoftwareI2C.cpp
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.
  Author:Loovee
*/

#include "i2c_utils.h"

#ifdef I2C_TOOLS_USE_SW_I2C
    #include "SoftwareI2C.h"
    SoftwareI2C softwarei2c;
#else
    struct rt_i2c_bus_device *i2c_bus;
#endif

long strtonum(const char* str)
{
    long data;
    if (strcmp(str, "0x"))
    {
        data = (rt_uint16_t)strtol(str, NULL, 0);
    }
    else
    {
        data = atoi(str);
    }
    return data;
}

void i2c_help()
{
    rt_kprintf("---------------\n");
    rt_kprintf("i2c tools help:\n");
    rt_kprintf("---------------\n");
#ifdef I2C_TOOLS_USE_SW_I2C
    rt_kprintf("i2c scan  pin_sda pin_scl [start addr] [stop addr]\n");
    rt_kprintf("i2c read  pin_sda pin_scl address  register  [len=1]\n");
    rt_kprintf("i2c write pin_sda pin_scl address [register] data_0 data_1 ...\n\n");
#else
    rt_kprintf("i2c scan bus_name [start addr] [stop addr]\n");
    rt_kprintf("i2c read   bus_name address  register  [len=1]\n");
    rt_kprintf("i2c read16 bus_name address  register  [len=1]\n");
    rt_kprintf("i2c write   bus_name address [register] data_0 data_1 ...\n");
    rt_kprintf("i2c write16 bus_name address [register] data_0 data_1 ...\n\n");
#endif
}

#ifdef I2C_TOOLS_USE_SW_I2C
rt_bool_t i2c_init(int sda, int scl)
{
    softwarei2c.begin(sda, scl);
    return RT_EOK;
}
#else
rt_bool_t i2c_init(const char* busname)
{
    i2c_bus = rt_i2c_bus_device_find(busname);
    if(i2c_bus == RT_NULL)
    {
        return RT_ERROR;
    }
    else
    {
        return RT_EOK;
    }
}
#endif

int i2c_probe(char addr)
{
#ifdef I2C_TOOLS_USE_SW_I2C
    int ret = softwarei2c.beginTransmission(addr);
    softwarei2c.endTransmission();
    return ret;
#else
    unsigned char cmd[1];
    cmd[0] = 0;

#ifdef AIC_I2C_INTERRUPT_MODE
    rt_uint8_t buffer[1];
    struct rt_i2c_msg msgs[2];

    msgs[0].addr = addr;
    msgs[0].flags = RT_I2C_WR;
    msgs[0].buf = cmd;
    msgs[0].len = 1;

    msgs[1].addr = addr;
    msgs[1].flags = RT_I2C_RD;
    msgs[1].buf = buffer;
    msgs[1].len = 1;

    if (rt_i2c_transfer(i2c_bus, msgs, 2) == 2)
    {
        return GET_ACK;
    }
    else
    {
        return GET_NACK;
    }
#else
    struct rt_i2c_msg msgs;
    msgs.addr = addr;
    msgs.flags = RT_I2C_WR;
    msgs.buf = cmd;
    msgs.len = 0;

    return rt_i2c_transfer(i2c_bus, &msgs, 1);
#endif
#endif
}

rt_bool_t i2c_write(char addr, rt_uint8_t* data, int len)
{
    #ifdef I2C_TOOLS_USE_SW_I2C
        softwarei2c.beginTransmission(addr);
        int ret = softwarei2c.write(data, len);
        softwarei2c.endTransmission();
        return ret;
    #else
        struct rt_i2c_msg msgs;
        msgs.addr = addr;
        msgs.flags = RT_I2C_WR;
        msgs.buf = data;
        msgs.len = len;
        return (rt_i2c_transfer(i2c_bus, &msgs, 1) == 1);
    #endif
}

rt_bool_t i2c_write16(char addr, rt_uint8_t* data, int len)
{
    return i2c_write(addr, data, len);
}

rt_uint8_t i2c_read(rt_uint8_t addr, rt_uint8_t reg, rt_uint8_t* buffer, rt_uint8_t len)
{
    #ifdef I2C_TOOLS_USE_SW_I2C
        rt_uint8_t ret = 0;
        softwarei2c.requestFrom(addr, len);
        while (softwarei2c.available()) {
            buffer[ret] = softwarei2c.read();
            ret++;
        }
        return ret;
    #else
        struct rt_i2c_msg msgs[2];

        msgs[0].addr = addr;
        msgs[0].flags = RT_I2C_WR;
        msgs[0].buf = &reg;
        msgs[0].len = 1;

        msgs[1].addr = addr;
        msgs[1].flags = RT_I2C_RD;
        msgs[1].buf = buffer;
        msgs[1].len = len;

        if (rt_i2c_transfer(i2c_bus, msgs, 2) == 2)
        {
            return len;
        }
        else
        {
            return 0;
        }
    #endif
}

rt_uint8_t i2c_read16(rt_uint8_t addr, rt_uint16_t reg, rt_uint8_t *buffer, rt_uint8_t len)
{
    if (rt_i2c_read_reg16(i2c_bus, addr, reg, buffer, len) != len) {
        printf("Failed to read I2C dev 0x%02x - reg 0x%04x\n", addr, reg);
        return 0;
    }
    return len;
}

void i2c_scan(rt_uint8_t start_addr, rt_uint8_t stop_addr)
{
    rt_kputs("    00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
    rt_uint8_t pos = start_addr <= 0xF? 0x00: start_addr & 0xF;
    for(; pos < stop_addr; pos++)
    {
        if((pos & 0xF) == 0)
        {
            rt_kprintf("\n%02X: ", pos);
        }
        if(pos < start_addr)
        {
            rt_kputs("   ");
            continue;
        }
        if(i2c_probe(pos) == 1)
        {
            rt_kprintf("%02X", pos);
        }
        else
        {
            rt_kputs("--");
        }
        rt_kputs(" ");
    }
    rt_kputs("\n");
}
