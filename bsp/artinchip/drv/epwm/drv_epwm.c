/*
 * Copyright (c) 2022-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: zrq <ruiqi.zheng@artinchip.com>
 */

#include <drivers/rt_drv_pwm.h>
#include <drivers/pm.h>

#define LOG_TAG         "EPWM"
#include "aic_core.h"
#include "aic_hal_clk.h"

#include "hal_epwm.h"

static struct rt_device_pwm g_aic_epwm = {0};
static struct aic_epwm_pulse_para g_pulse_para[AIC_EPWM_CH_NUM] = {0};
#ifdef AIC_EPWM_DRV_V11
static u8 g_epwm_pul_flag[AIC_EPWM_CH_NUM] = {0};
#endif
static void aic_epwm_default_action(void)
{
    struct aic_epwm_action action0 = {
        /*       CBD,          CBU,          CAD, */
        EPWM_ACT_NONE, EPWM_ACT_NONE, EPWM_ACT_NONE,
        /*      CAU,           PRD,         ZRO  */
        EPWM_ACT_LOW, EPWM_ACT_NONE, EPWM_ACT_HIGH};
    struct aic_epwm_action action1 = {
        /*       CBD,          CBU,          CAD, */
        EPWM_ACT_NONE, EPWM_ACT_LOW, EPWM_ACT_NONE,
        /*      CAU,           PRD,         ZRO  */
        EPWM_ACT_NONE, EPWM_ACT_NONE,  EPWM_ACT_HIGH};

#ifdef AIC_USING_EPWM0
    hal_epwm_ch_init(0, AIC_EPWM0_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM1
    hal_epwm_ch_init(1, AIC_EPWM1_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM2
    hal_epwm_ch_init(2, AIC_EPWM2_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM3
    hal_epwm_ch_init(3, AIC_EPWM3_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM4
    hal_epwm_ch_init(4, AIC_EPWM4_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM5
    hal_epwm_ch_init(5, AIC_EPWM5_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM6
    hal_epwm_ch_init(6, AIC_EPWM6_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM7
    hal_epwm_ch_init(7, AIC_EPWM7_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM8
    hal_epwm_ch_init(8, AIC_EPWM8_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM9
    hal_epwm_ch_init(9, AIC_EPWM9_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM10
    hal_epwm_ch_init(10, AIC_EPWM10_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
#ifdef AIC_USING_EPWM11
    hal_epwm_ch_init(11, AIC_EPWM11_SYNC, EPWM_MODE_UP_COUNT, 0, &action0, &action1);
#endif
}

static rt_bool_t drv_epwm_ch_valid(struct rt_pwm_configuration *cfg)
{
    if (cfg->channel > (AIC_EPWM_CH_NUM - 1)) {
        LOG_E("Invalid channel No.%d", cfg->channel);
        return RT_TRUE;
    }

    return RT_FALSE;
}

static rt_err_t drv_epwm_enable(struct rt_device_pwm *device,
                                struct rt_pwm_configuration *cfg,
                                rt_bool_t enable)
{
    if (drv_epwm_ch_valid(cfg))
        return -RT_EINVAL;

    if (enable)
        return !hal_epwm_enable(cfg->channel) ? RT_EOK : -RT_ERROR;
    else
        return !hal_epwm_disable(cfg->channel) ? RT_EOK : -RT_ERROR;
}

static rt_err_t drv_epwm_set(struct rt_device_pwm *device,
                             struct rt_pwm_configuration *cfg)
{
    if (drv_epwm_ch_valid(cfg))
        return -RT_EINVAL;

    if (hal_epwm_set(cfg->channel, cfg->pulse, cfg->period, (rt_uint32_t)EPWM_SET_CMPA_CMPB))
        return -RT_ERROR;

    return RT_EOK;
}

static rt_err_t drv_epwm_set_output(struct rt_device_pwm *device,
                             struct rt_pwm_configuration *cfg)
{
    if (drv_epwm_ch_valid(cfg))
        return -RT_EINVAL;

    if (hal_epwm_set(cfg->channel, cfg->pulse, cfg->period, cfg->output))
        return -RT_ERROR;

    return RT_EOK;
}

static rt_err_t drv_epwm_set_pul(struct rt_device_pwm *device,
                             struct rt_pwm_configuration *cfg)
{
    if (drv_epwm_ch_valid(cfg))
        return -RT_EINVAL;

    g_pulse_para[cfg->channel].pulse_cnt = cfg->pul_cnt;
    g_pulse_para[cfg->channel].duty_ns = cfg->pulse;
    g_pulse_para[cfg->channel].prd_ns = cfg->period;

#ifdef AIC_EPWM_DRV_V11
    hal_epwm_pul_config(cfg->channel);

    g_epwm_pul_flag[cfg->channel] = 1;

    /* temporarily control two outputs simultaneously */
    if (hal_epwm_pul_set(cfg->channel, cfg->pulse, cfg->period, EPWM_SET_CMPA_CMPB, cfg->pul_cnt))
        return -RT_ERROR;

    hal_epwm_act_sw_ct(cfg->channel, EPWM_SET_CMPA_CMPB, EPWM_ACT_SW_NONE);

    hal_epwm_count_ct(cfg->channel, EPWM_MODE_UP_COUNT);

    hal_epwm_pul_sw_updt(cfg->channel);
#else
    if (hal_epwm_set(cfg->channel, cfg->pulse, cfg->period, (rt_uint32_t)EPWM_SET_CMPA_CMPB))
        return -RT_ERROR;

    hal_epwm_int_config(cfg->channel, cfg->irq_mode, 1);

    hal_epwm_enable(cfg->channel);
#endif

    return RT_EOK;
}

static rt_err_t drv_epwm_get(struct rt_device_pwm *device,
                             struct rt_pwm_configuration *cfg)
{
    if (drv_epwm_ch_valid(cfg))
        return -RT_EINVAL;

    if (hal_epwm_get(cfg->channel, (u32 *)&cfg->pulse, (u32 *)&cfg->period))
        return -RT_ERROR;

    return RT_EOK;
}

static rt_err_t drv_epwm_control(struct rt_device_pwm *device,
                                int cmd, void *arg)
{
    struct rt_pwm_configuration *cfg = (struct rt_pwm_configuration *)arg;

    switch (cmd) {
    case PWM_CMD_ENABLE:
        return drv_epwm_enable(device, cfg, RT_TRUE);
    case PWM_CMD_DISABLE:
        return drv_epwm_enable(device, cfg, RT_FALSE);
    case PWM_CMD_SET:
        return drv_epwm_set(device, cfg);
    case PWM_CMD_SET_OUTPUT:
        return drv_epwm_set_output(device, cfg);
    case PWM_CMD_GET:
        return drv_epwm_get(device, cfg);
    case PWM_CMD_SET_PUL:
        return drv_epwm_set_pul(device, cfg);
    default:
        LOG_I("Unsupported cmd: 0x%x", cmd);
        return -RT_EINVAL;
    }

    return RT_EOK;
}

static struct rt_pwm_ops aic_epwm_ops = {
    .control = drv_epwm_control
};

#ifdef RT_USING_PM
static int drv_epwm_suspend(const struct rt_device *device, rt_uint8_t mode)
{
    switch (mode)
    {
    case PM_SLEEP_MODE_IDLE:
        break;
    case PM_SLEEP_MODE_LIGHT:
    case PM_SLEEP_MODE_DEEP:
    case PM_SLEEP_MODE_STANDBY:
        hal_clk_disable(CLK_PWMCS);
        break;
    default:
        break;
    }

    return 0;
}

static void drv_epwm_resume(const struct rt_device *device, rt_uint8_t mode)
{
    switch (mode)
    {
    case PM_SLEEP_MODE_IDLE:
        break;
    case PM_SLEEP_MODE_LIGHT:
    case PM_SLEEP_MODE_DEEP:
    case PM_SLEEP_MODE_STANDBY:
#ifdef AIC_EPWM_DRV_V11
        hal_clk_set_freq(CLK_PWMCS_SDFM, EPWM_CLK_RATE);
#else
        hal_clk_set_freq(CLK_PWMCS, EPWM_CLK_RATE);
#endif
        hal_clk_enable(CLK_PWMCS);
        break;
    default:
        break;
    }
}

static struct rt_device_pm_ops drv_epwm_pm_ops =
{
    SET_LATE_DEVICE_PM_OPS(drv_epwm_suspend, drv_epwm_resume)
    NULL,
};
#endif

irqreturn_t aic_epwm_irq(int irq, void *arg)
{
    u32 stat;
    int i;
#ifndef AIC_EPWM_DRV_V11
    static u32 isr_cnt[AIC_EPWM_CH_NUM] = {0};
#endif

    for (i = 0; i < AIC_EPWM_CH_NUM; i++) {
        stat = hal_epwm_int_sts(i);
        if (stat & EPWM_INT_FLG) {
#ifdef AIC_EPWM_DRV_V11
            int level = hal_epwm_get_default_level(i);
            hal_epwm_act_sw_ct(i, EPWM_SET_CMPA_CMPB, level + EPWM_ACT_SW_LOW);
            if (g_epwm_pul_flag[i] == 0) {
                /* enable the EPWM_PUL_OUT_EN will enter the interrupt */
                hal_epwm_count_ct(i, (u32)EPWM_MODE_STOP_COUNT);
            }
            if (g_epwm_pul_flag[i] == 1) {
                g_epwm_pul_flag[i] = 0;
            }
#else
            isr_cnt[i]++;
            if (isr_cnt[i] == g_pulse_para[i].pulse_cnt) {
                hal_epwm_set(i, g_pulse_para[i].prd_ns, g_pulse_para[i].prd_ns, (rt_uint32_t)EPWM_SET_CMPA_CMPB);
                hal_epwm_int_config(i, 0, 0);
                pr_info("\nisr cnt:%d,disabled the epwm%d interrupt now.\n", isr_cnt[i], i);
                isr_cnt[i] = 0;
            }
#endif
            hal_epwm_clr_int(stat, i);
        }
    }

    return IRQ_HANDLED;
}

int drv_epwm_init(void)
{
    hal_epwm_init();
    aic_epwm_default_action();

    aicos_request_irq(PWMCS_PWM_IRQn, aic_epwm_irq, 0, NULL, NULL);

    if (rt_device_pwm_register(&g_aic_epwm, "epwm", &aic_epwm_ops, NULL))
        return -RT_ERROR;

#ifdef RT_USING_PM
    rt_pm_device_register(&g_aic_epwm.parent, &drv_epwm_pm_ops);
#endif

    LOG_I("ArtInChip EPWM loaded");
    return RT_EOK;
}
INIT_PREV_EXPORT(drv_epwm_init);


#if defined(RT_USING_FINSH)
#include <finsh.h>

static void cmd_epwm_status(int argc, char **argv)
{
     hal_epwm_status_show();
}

MSH_CMD_EXPORT_ALIAS(cmd_epwm_status, epwm_status, Show the status of EPWM);

#endif
