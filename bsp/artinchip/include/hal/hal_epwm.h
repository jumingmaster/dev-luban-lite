/*
 * Copyright (c) 2022-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: zrq <ruiqi.zheng@artinchip.com>
 */

#ifndef _ARTINCHIP_HAL_EPWM_H_
#define _ARTINCHIP_HAL_EPWM_H_

#include "aic_common.h"

#define AIC_EPWM_NAME        "aic-epwm"

#ifdef CONFIG_FPGA_BOARD_ARTINCHIP
#define EPWM_CLK_RATE        24000000 /* 24 MHz */
#else
#define EPWM_CLK_RATE        200000000 /* 200 MHz */
#endif

#define EPWM_INT_FLG         BIT(0)

enum aic_epwm_act_sw_ct {
    EPWM_ACT_SW_NONE = 0,
    EPWM_ACT_SW_LOW,
    EPWM_ACT_SW_HIGH,
};

enum aic_epwm_mode {
    EPWM_MODE_UP_COUNT = 0,
    EPWM_MODE_DOWN_COUNT,
    EPWM_MODE_UP_DOWN_COUNT,
    EPWM_MODE_STOP_COUNT,
    EPWM_MODE_NUM
};

enum aic_epwm_action_type {
    EPWM_ACT_NONE = 0,
    EPWM_ACT_LOW,
    EPWM_ACT_HIGH,
    EPWM_ACT_INVERSE,
    EPWM_ACT_NUM
};

enum aic_epwm_cmp_write_type {
    EPWM_SET_CMPA = 0,
    EPWM_SET_CMPB,
    EPWM_SET_CMPA_CMPB
};

struct aic_epwm_action {
    enum aic_epwm_action_type CBD;
    enum aic_epwm_action_type CBU;
    enum aic_epwm_action_type CAD;
    enum aic_epwm_action_type CAU;
    enum aic_epwm_action_type PRD;
    enum aic_epwm_action_type ZRO;
};

struct aic_epwm_arg {
    bool sync_mode;
    u16 available;
    u16 id;
    enum aic_epwm_mode mode;
    u32 clk_rate;
    u32 tb_clk_rate;
    float freq;
    struct aic_epwm_action action0;
    struct aic_epwm_action action1;
    u32 period;
    u32 duty;
    s32 def_level;
};

enum aic_epwm_int_event {
    EPWM_CMPA_UP = 0,
    EPWM_CMPA_DOWN,
    EPWM_CMPB_UP,
    EPWM_CMPB_DOWN
};

struct aic_epwm_pulse_para {
    u32 prd_ns;
    u32 duty_ns;
    u32 pulse_cnt;
};

void hal_epwm_ch_init(u32 ch, bool sync_mode, enum aic_epwm_mode mode, u32 default_level,
                     struct aic_epwm_action *a0, struct aic_epwm_action *a1);
void hal_epwm_act_sw_ct(u32 ch, u32 output, u32 action);
void hal_epwm_count_ct(u32 ch, enum aic_epwm_mode mode);
#ifdef AIC_EPWM_DRV_V11
void hal_epwm_pul_config(u32 ch);
int hal_epwm_pul_set(u32 ch, u32 duty_ns, u32 period_ns, u32 output, u32 num);
void hal_epwm_pul_out_ct(u32 ch, u32 enable);
void hal_epwm_pul_sw_updt(u32 ch);
#endif
int hal_epwm_get_default_level(u32 ch);
int hal_epwm_set(u32 ch, u32 duty_ns, u32 period_ns, u32 output);
int hal_epwm_get(u32 ch, u32 *duty_ns, u32 *period_ns);
int hal_epwm_enable(u32 ch);
int hal_epwm_disable(u32 ch);
u32 hal_epwm_int_sts(u32 ch);
void hal_epwm_clr_int(u32 stat, u32 ch);
void hal_epwm_int_config(u32 ch, u8 irq_mode, u8 enable);

int hal_epwm_init(void);
int hal_epwm_deinit(void);

void hal_epwm_status_show(void);


#endif // end of _ARTINCHIP_HAL_EPWM_H_
