/*
 * Copyright (c) 2022-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: dwj <weijie.ding@artinchip.com>
 */

#include <stdio.h>
#include <rtdevice.h>
#include <rtthread.h>
#include <aic_core.h>
#include <aic_drv.h>
#include <string.h>
#include <aic_osal.h>
#include <aic_utils.h>

uint64_t sleep_counter;
uint64_t resume_counter;

RT_WEAK void rt_pm_board_level_power_off(void)
{
    return;
}

RT_WEAK void rt_pm_board_level_power_on(void)
{
    return;
}

void aic_pm_enter_idle(void)
{
    __WFI();
}

void aic_pm_enter_light_sleep(void)
{
    /* change bus frequency to 24M */
    hal_clk_set_parent(CLK_AXI0, CLK_OSC24M);
    hal_clk_set_parent(CLK_AHB0, CLK_OSC24M);
    hal_clk_set_parent(CLK_APB0, CLK_OSC24M);
    /* change cpu frequency to 24M */
    hal_clk_set_parent(CLK_CPU, CLK_OSC24M);
    /* disable PLL_FRA2: display pll */
    hal_clk_disable(CLK_PLL_FRA2);
    /* disable PLL_INT0: cpu pll */
    hal_clk_disable(CLK_PLL_INT0);

    __WFI();

    /* wakeup flow */
    /* enable PLL_INT0: cpu pll */
    hal_clk_enable(CLK_PLL_INT0);
    /* change cpu frequency to pll */
    hal_clk_set_parent(CLK_CPU, CLK_CPU_SRC1);
    hal_clk_set_parent(CLK_AXI0, CLK_AXI_AHB_SRC1);
    hal_clk_set_parent(CLK_AHB0, CLK_AXI_AHB_SRC1);
    hal_clk_set_parent(CLK_APB0, CLK_APB0_SRC1);
    /* enable PLL_FRA2: display pll */
    hal_clk_enable(CLK_PLL_FRA2);
}

void aic_pm_enter_deep_sleep(void)
{
    uint32_t fra0_freq;

    /* change bus frequency to 24M */
    hal_clk_set_parent(CLK_AXI0, CLK_OSC24M);
    hal_clk_set_parent(CLK_AHB0, CLK_OSC24M);
    hal_clk_set_parent(CLK_APB0, CLK_OSC24M);
    /* change cpu frequency to 24M */
    hal_clk_set_parent(CLK_CPU, CLK_OSC24M);
    /* disable PLL_FRA2: display pll */
    hal_clk_disable(CLK_PLL_FRA2);
    /* disable PLL_INT1: bus pll */
    hal_clk_disable(CLK_PLL_INT1);
    /* disable PLL_INT0: cpu pll */
    hal_clk_disable(CLK_PLL_INT0);
    /* bypass PLL_FRA0: spi/sdmc/psram pll */
    fra0_freq = hal_clk_get_freq(CLK_PLL_FRA0);
    hal_clk_set_freq(CLK_PLL_FRA0, 24000000);
    /* disable PLL_FRA0 */
    hal_clk_disable(CLK_PLL_FRA0);
    /* Turn off board level power that can be controlled via GPIO */
    rt_pm_board_level_power_off();
    /* deinit all non-wakup pinmux configuration */
    aic_board_pinmux_deinit();

    /* reset all pins */
    //TO DO
    __WFI();

    /* wakeup flow */
    hal_clk_set_freq(CLK_PLL_FRA0, fra0_freq);
    /* enable PLL_INT0: cpu pll */
    hal_clk_enable(CLK_PLL_INT0);
    /* change cpu frequency to pll */
    hal_clk_set_parent(CLK_CPU, CLK_CPU_SRC1);
    /* restore all pinmux configuration */
    aic_board_pinmux_init();
    /* Turn on board level power that can be controlled via GPIO */
    rt_pm_board_level_power_on();
    /* enable PLL_INT1: bus pll */
    hal_clk_enable(CLK_PLL_INT1);
    hal_clk_set_parent(CLK_AXI0, CLK_AXI_AHB_SRC1);
    hal_clk_set_parent(CLK_AHB0, CLK_AXI_AHB_SRC1);
    hal_clk_set_parent(CLK_APB0, CLK_APB0_SRC1);
    /* enable PLL_FRA2: display pll */
    hal_clk_enable(CLK_PLL_FRA2);
}


static void aic_sleep(struct rt_pm *pm, uint8_t mode)
{
    switch (mode)
    {
    case PM_SLEEP_MODE_NONE:
        break;

    case PM_SLEEP_MODE_IDLE:
        aic_pm_enter_idle();
        break;
    case PM_SLEEP_MODE_LIGHT:
        aic_pm_enter_light_sleep();
        break;
    case PM_SLEEP_MODE_DEEP:
        aic_pm_enter_deep_sleep();
        break;
    case PM_SLEEP_MODE_STANDBY:
        //TO DO
        break;
    case PM_SLEEP_MODE_SHUTDOWN:
        break;
    default:
        RT_ASSERT(0);
        break;
    }
}

static void aic_run(struct rt_pm *pm, rt_uint8_t mode)
{
    static rt_uint8_t prev_mode = 0;

    RT_ASSERT(pm != RT_NULL);

    if(prev_mode == mode)
    {
        return;
    }

    switch(mode)
    {
    case PM_RUN_MODE_HIGH_SPEED: /* 480MHz */
    case PM_RUN_MODE_NORMAL_SPEED:
        hal_clk_set_parent(CLK_CPU, CLK_CPU_SRC1);
        hal_clk_set_freq(CLK_CPU_SRC1, 480000000);
        break;

    case PM_RUN_MODE_MEDIUM_SPEED: /* 240Mhz */
        hal_clk_set_parent(CLK_CPU, CLK_CPU_SRC1);
        hal_clk_set_freq(CLK_CPU_SRC1, 240000000);
        break;

    case PM_RUN_MODE_LOW_SPEED: /* 24Mhz */
        hal_clk_set_parent(CLK_CPU, CLK_OSC24M);
        break;

    default:
        return;
    }
}

/* timeout unit is rt_tick_t, but MTIMECMPH/L unit is HZ
 * one tick is 4000 counter
 */
static void aic_timer_start(struct rt_pm *pm, rt_uint32_t timeout)
{
    uint64_t tmp_counter;
    uint32_t tick_resolution = drv_get_sys_freq() / CONFIG_SYSTICK_HZ;

    sleep_counter = ((uint64_t)csi_coret_get_valueh() << 32) |
                    csi_coret_get_value();
    tmp_counter = (uint64_t)timeout * tick_resolution;

    csi_coret_set_load(tmp_counter + sleep_counter);
}

static void aic_timer_stop(struct rt_pm *pm)
{
    uint64_t tmp_counter = ((uint64_t)csi_coret_get_valueh() << 32) |
                           csi_coret_get_value();

    uint32_t tick_resolution = drv_get_sys_freq() / CONFIG_SYSTICK_HZ;

    csi_coret_set_load(tmp_counter + tick_resolution);
}

static rt_tick_t aic_timer_get_tick(struct rt_pm *pm)
{
    rt_tick_t delta_tick;
    uint32_t delta_counter;
    uint32_t tick_resolution = drv_get_sys_freq() / CONFIG_SYSTICK_HZ;

    resume_counter = ((uint64_t)csi_coret_get_valueh() << 32) |
                     csi_coret_get_value();
    delta_counter = resume_counter - sleep_counter;

    delta_tick = delta_counter / tick_resolution;

    return delta_tick;
}

static const struct rt_pm_ops aic_pm_ops =
{
    aic_sleep,
    aic_run,
    aic_timer_start,
    aic_timer_stop,
    aic_timer_get_tick,
};

/**
 * This function initialize the power manager
 */

int aic_pm_hw_init(void)
{
    rt_uint8_t timer_mask = 0;

#ifdef AIC_PM_POWER_DEFAULT_LIGHT_MODE
    rt_pm_default_set(PM_SLEEP_MODE_LIGHT);
#endif
    timer_mask = 3UL << PM_SLEEP_MODE_LIGHT;
    /* initialize system pm module */
    rt_system_pm_init(&aic_pm_ops, timer_mask, RT_NULL);

    return 0;
}
INIT_BOARD_EXPORT(aic_pm_hw_init);
