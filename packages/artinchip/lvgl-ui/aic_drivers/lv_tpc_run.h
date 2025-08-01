/*
 * Copyright (c) 2022-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 */

#ifndef LV_TPC_RUN
#define LV_TPC_RUN

#ifdef __cplusplus
extern "C" {
#endif

#include <rtconfig.h>

#ifdef KERNEL_RTTHREAD
#include <rtthread.h>
#include <rtdevice.h>

int tpc_run(const char *name, rt_uint16_t x, rt_uint16_t y);
void lvgl_get_tp(void);
void lvgl_put_tp(void);
void rtp_check_event_type(int event_type, int pressure);

#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_TPC_RUN */
