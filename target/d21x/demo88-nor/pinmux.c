/*
 * Copyright (c) 2022-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: weilin.peng@artinchip.com
 */

#include <aic_core.h>
#include <aic_hal.h>
#include "board.h"
#include <aic_utils.h>

struct aic_pinmux aic_pinmux_config[] = {
#ifdef AIC_USING_UART0
    /* uart0 */
    {5, PIN_PULL_DIS, 3, "PA.0"},
    {5, PIN_PULL_UP, 3, "PA.1"},
#endif
#ifdef AIC_USING_UART1
    /* uart1 */
    {5, PIN_PULL_DIS, 3, "PA.4"},
    {5, PIN_PULL_UP, 3, "PA.5"},
#endif
#ifdef AIC_USING_UART2
    /* uart2 */
    {5, PIN_PULL_DIS, 3, "PA.8"},
    {5, PIN_PULL_UP, 3, "PA.9"},
#endif
#ifdef AIC_USING_SDMC0
    {2, PIN_PULL_UP, 3, "PB.0"},
    {2, PIN_PULL_UP, 3, "PB.1"},
    {2, PIN_PULL_UP, 3, "PB.2"},
    {2, PIN_PULL_UP, 3, "PB.3"},
    {2, PIN_PULL_UP, 3, "PB.4"},
    {2, PIN_PULL_UP, 3, "PB.5"},
    {2, PIN_PULL_UP, 3, "PB.6"},
    {2, PIN_PULL_UP, 3, "PB.7"},
    {2, PIN_PULL_UP, 3, "PB.8"},
    {2, PIN_PULL_UP, 3, "PB.9"},
    {2, PIN_PULL_UP, 3, "PB.10"},
    {2, PIN_PULL_UP, 3, "PB.11"},
#endif
#ifdef AIC_USING_SDMC1
    {2, PIN_PULL_UP, 3, "PC.0"},
    {2, PIN_PULL_UP, 3, "PC.1"},
    {2, PIN_PULL_UP, 3, "PC.2"},
    {2, PIN_PULL_UP, 3, "PC.3"},
    {2, PIN_PULL_UP, 3, "PC.4"},
    {2, PIN_PULL_UP, 3, "PC.5"},
    {2, PIN_PULL_UP, 3, "PC.6"},
#endif
#ifdef AIC_USING_SDMC2
    {2, PIN_PULL_UP, 3, "PF.0"},
    {2, PIN_PULL_UP, 3, "PF.1"},
    {2, PIN_PULL_UP, 3, "PF.2"},
    {2, PIN_PULL_UP, 3, "PF.3"},
    {2, PIN_PULL_UP, 3, "PF.4"},
    {2, PIN_PULL_UP, 3, "PF.5"},
#endif
#ifdef AIC_USING_I2C0
    {4, PIN_PULL_DIS, 3, "PD.6"}, // SCK
    {4, PIN_PULL_DIS, 3, "PD.7"}, // SDA
#endif
#ifdef AIC_USING_I2C1
    {5, PIN_PULL_DIS, 3, "PD.18"}, // SCK
    {5, PIN_PULL_DIS, 3, "PD.19"}, // SDA
#endif
#ifdef AIC_USING_I2C2
    {4, PIN_PULL_DIS, 3, "PA.8"}, // SCK
    {4, PIN_PULL_DIS, 3, "PA.9"}, // SDA
#endif
#ifdef AIC_USING_I2C3
    {4, PIN_PULL_DIS, 3, "PA.10"}, // SCK
    {4, PIN_PULL_DIS, 3, "PA.11"}, // SDA
#endif
#if defined(AIC_USING_QSPI0) && !defined(AIC_SYSCFG_SIP_FLASH_ENABLE)
    /* qspi0 */
    {3, PIN_PULL_DIS, 3, "PB.0"},
    {3, PIN_PULL_DIS, 3, "PB.1"},
    {3, PIN_PULL_DIS, 3, "PB.2"},
    {3, PIN_PULL_DIS, 3, "PB.3"},
    {3, PIN_PULL_DIS, 3, "PB.4"},
    {3, PIN_PULL_DIS, 3, "PB.5"},
#endif
#ifdef AIC_PRGB_24BIT
    {2, PIN_PULL_DIS, 3, "PD.0"},
    {2, PIN_PULL_DIS, 3, "PD.1"},
    {2, PIN_PULL_DIS, 3, "PD.2"},
    {2, PIN_PULL_DIS, 3, "PD.3"},
    {2, PIN_PULL_DIS, 3, "PD.4"},
    {2, PIN_PULL_DIS, 3, "PD.5"},
    {2, PIN_PULL_DIS, 3, "PD.6"},
    {2, PIN_PULL_DIS, 3, "PD.7"},
    {2, PIN_PULL_DIS, 3, "PD.8"},
    {2, PIN_PULL_DIS, 3, "PD.9"},
    {2, PIN_PULL_DIS, 3, "PD.10"},
    {2, PIN_PULL_DIS, 3, "PD.11"},
    {2, PIN_PULL_DIS, 3, "PD.12"},
    {2, PIN_PULL_DIS, 3, "PD.13"},
    {2, PIN_PULL_DIS, 3, "PD.14"},
    {2, PIN_PULL_DIS, 3, "PD.15"},
    {2, PIN_PULL_DIS, 3, "PD.16"},
    {2, PIN_PULL_DIS, 3, "PD.17"},
    {2, PIN_PULL_DIS, 3, "PD.18"},
    {2, PIN_PULL_DIS, 3, "PD.19"},
    {2, PIN_PULL_DIS, 3, "PD.20"},
    {2, PIN_PULL_DIS, 3, "PD.21"},
    {2, PIN_PULL_DIS, 3, "PD.22"},
    {2, PIN_PULL_DIS, 3, "PD.23"},
    {2, PIN_PULL_DIS, 3, "PD.24"},
    {2, PIN_PULL_DIS, 3, "PD.25"},
    {2, PIN_PULL_DIS, 3, "PD.26"},
    {2, PIN_PULL_DIS, 3, "PD.27"},
#endif
#ifdef AIC_PRGB_16BIT_LD
    {2, PIN_PULL_DIS, 3, "PD.8"},
    {2, PIN_PULL_DIS, 3, "PD.9"},
    {2, PIN_PULL_DIS, 3, "PD.10"},
    {2, PIN_PULL_DIS, 3, "PD.11"},
    {2, PIN_PULL_DIS, 3, "PD.12"},
    {2, PIN_PULL_DIS, 3, "PD.13"},
    {2, PIN_PULL_DIS, 3, "PD.14"},
    {2, PIN_PULL_DIS, 3, "PD.15"},
    {2, PIN_PULL_DIS, 3, "PD.16"},
    {2, PIN_PULL_DIS, 3, "PD.17"},
    {2, PIN_PULL_DIS, 3, "PD.18"},
    {2, PIN_PULL_DIS, 3, "PD.19"},
    {2, PIN_PULL_DIS, 3, "PD.20"},
    {2, PIN_PULL_DIS, 3, "PD.21"},
    {2, PIN_PULL_DIS, 3, "PD.22"},
    {2, PIN_PULL_DIS, 3, "PD.23"},
    {2, PIN_PULL_DIS, 3, "PD.24"},
    {2, PIN_PULL_DIS, 3, "PD.25"},
    {2, PIN_PULL_DIS, 3, "PD.26"},
    {2, PIN_PULL_DIS, 3, "PD.27"},
#endif
#ifdef AIC_PRGB_16BIT_HD
    {2, PIN_PULL_DIS, 3, "PD.0"},
    {2, PIN_PULL_DIS, 3, "PD.1"},
    {2, PIN_PULL_DIS, 3, "PD.2"},
    {2, PIN_PULL_DIS, 3, "PD.3"},
    {2, PIN_PULL_DIS, 3, "PD.4"},
    {2, PIN_PULL_DIS, 3, "PD.5"},
    {2, PIN_PULL_DIS, 3, "PD.6"},
    {2, PIN_PULL_DIS, 3, "PD.7"},
    {2, PIN_PULL_DIS, 3, "PD.8"},
    {2, PIN_PULL_DIS, 3, "PD.9"},
    {2, PIN_PULL_DIS, 3, "PD.10"},
    {2, PIN_PULL_DIS, 3, "PD.11"},
    {2, PIN_PULL_DIS, 3, "PD.12"},
    {2, PIN_PULL_DIS, 3, "PD.13"},
    {2, PIN_PULL_DIS, 3, "PD.14"},
    {2, PIN_PULL_DIS, 3, "PD.15"},
    {2, PIN_PULL_DIS, 3, "PD.24"},
    {2, PIN_PULL_DIS, 3, "PD.25"},
    {2, PIN_PULL_DIS, 3, "PD.26"},
    {2, PIN_PULL_DIS, 3, "PD.27"},
#endif
#ifdef AIC_LVDS_LINK_1
    {3, PIN_PULL_DIS, 3, "PD.8"},
    {3, PIN_PULL_DIS, 3, "PD.9"},
    {3, PIN_PULL_DIS, 3, "PD.10"},
    {3, PIN_PULL_DIS, 3, "PD.11"},
    {3, PIN_PULL_DIS, 3, "PD.12"},
    {3, PIN_PULL_DIS, 3, "PD.13"},
    {3, PIN_PULL_DIS, 3, "PD.14"},
    {3, PIN_PULL_DIS, 3, "PD.15"},
    {3, PIN_PULL_DIS, 3, "PD.16"},
    {3, PIN_PULL_DIS, 3, "PD.17"},
#endif
#ifdef AIC_LVDS_LINK_0
    {3, PIN_PULL_DIS, 3, "PD.18"},
    {3, PIN_PULL_DIS, 3, "PD.19"},
    {3, PIN_PULL_DIS, 3, "PD.20"},
    {3, PIN_PULL_DIS, 3, "PD.21"},
    {3, PIN_PULL_DIS, 3, "PD.22"},
    {3, PIN_PULL_DIS, 3, "PD.23"},
    {3, PIN_PULL_DIS, 3, "PD.24"},
    {3, PIN_PULL_DIS, 3, "PD.25"},
    {3, PIN_PULL_DIS, 3, "PD.26"},
    {3, PIN_PULL_DIS, 3, "PD.27"},
#endif
#if defined(AIC_LVDS_DOUBLE_SCREEN) || defined(AIC_LVDS_DUAL_LINK)
    {3, PIN_PULL_DIS, 3, "PD.8"},
    {3, PIN_PULL_DIS, 3, "PD.9"},
    {3, PIN_PULL_DIS, 3, "PD.10"},
    {3, PIN_PULL_DIS, 3, "PD.11"},
    {3, PIN_PULL_DIS, 3, "PD.12"},
    {3, PIN_PULL_DIS, 3, "PD.13"},
    {3, PIN_PULL_DIS, 3, "PD.14"},
    {3, PIN_PULL_DIS, 3, "PD.15"},
    {3, PIN_PULL_DIS, 3, "PD.16"},
    {3, PIN_PULL_DIS, 3, "PD.17"},
    {3, PIN_PULL_DIS, 3, "PD.18"},
    {3, PIN_PULL_DIS, 3, "PD.19"},
    {3, PIN_PULL_DIS, 3, "PD.20"},
    {3, PIN_PULL_DIS, 3, "PD.21"},
    {3, PIN_PULL_DIS, 3, "PD.22"},
    {3, PIN_PULL_DIS, 3, "PD.23"},
    {3, PIN_PULL_DIS, 3, "PD.24"},
    {3, PIN_PULL_DIS, 3, "PD.25"},
    {3, PIN_PULL_DIS, 3, "PD.26"},
    {3, PIN_PULL_DIS, 3, "PD.27"},
#endif
#ifdef AIC_PANEL_ENABLE_GPIO
    {1, PIN_PULL_DIS, 3, AIC_PANEL_ENABLE_GPIO},
#endif
#ifdef AIC_USING_CLK_OUT0
    {6, PIN_PULL_DIS, 3, "PD.21"},
#endif
#ifdef AIC_USING_CLK_OUT1
    {5, PIN_PULL_DIS, 3, "PE.11"},
#endif
#ifdef AIC_USING_CLK_OUT2
    {6, PIN_PULL_DIS, 3, "PE.10"},
#endif
#ifdef AIC_USING_CLK_OUT3
    {6, PIN_PULL_DIS, 3, "PF.10"},
#endif
#if (defined(AIC_USING_USB0_DEVICE) || defined(AIC_USING_USB0_HOST))
    /* usb0 */
    {2, PIN_PULL_DIS, 3, "PO.0"},
    {2, PIN_PULL_DIS, 3, "PO.1"},
#endif
#ifdef AIC_USING_PWM0
    {7, PIN_PULL_DIS, 3, "PC.0"},
    {7, PIN_PULL_DIS, 3, "PC.1"},
#endif
#ifdef AIC_USING_PWM1
    {7, PIN_PULL_DIS, 3, "PC.2"},
    {7, PIN_PULL_DIS, 3, "PC.3"},
#endif
#ifdef AIC_USING_PWM2
    {7, PIN_PULL_DIS, 3, "PC.4"},
    {7, PIN_PULL_DIS, 3, "PC.5"},
#endif
#ifdef AIC_USING_PWM3
    {5, PIN_PULL_DIS, 3, "PE.18"},
    {5, PIN_PULL_DIS, 3, "PE.19"},
#endif
#ifdef AIC_USING_CAP0
    {8, PIN_PULL_DIS, 3, "PE.6"},
#endif
#ifdef AIC_USING_CAP1
    {8, PIN_PULL_DIS, 3, "PE.7"},
#endif
#ifdef AIC_USING_CAP2
    {8, PIN_PULL_DIS, 3, "PE.8"},
#endif
#ifdef AIC_USING_GPAI0
    {2, PIN_PULL_DIS, 3, "PA.0"},
#endif
#ifdef AIC_USING_GPAI1
    {2, PIN_PULL_DIS, 3, "PA.1"},
#endif
#ifdef AIC_USING_GPAI2
    {2, PIN_PULL_DIS, 3, "PA.2"},
#endif
#ifdef AIC_USING_GPAI7
    {2, PIN_PULL_DIS, 3, "PA.7"},
#endif
#ifdef AIC_USING_AUDIO
#ifdef AIC_AUDIO_PLAYBACK
    {4, PIN_PULL_DIS, 3, "PE.11"},
    {1, PIN_PULL_DIS, 3, AIC_AUDIO_PA_ENABLE_GPIO},
#endif
#ifdef AIC_AUDIO_DMIC
    {4, PIN_PULL_DIS, 3, "PF.14"},
    {4, PIN_PULL_DIS, 3, "PF.15"},
#endif
#endif
#ifdef AIC_USING_DVP
    {3, PIN_PULL_DIS, 3, "PE.0"},
    {3, PIN_PULL_DIS, 3, "PE.1"},
    {3, PIN_PULL_DIS, 3, "PE.2"},
    {3, PIN_PULL_DIS, 3, "PE.3"},
    {3, PIN_PULL_DIS, 3, "PE.4"},
    {3, PIN_PULL_DIS, 3, "PE.5"},
    {3, PIN_PULL_DIS, 3, "PE.6"},
    {3, PIN_PULL_DIS, 3, "PE.7"},
    {3, PIN_PULL_DIS, 3, "PE.8"},
    {3, PIN_PULL_DIS, 3, "PE.9"},
    {3, PIN_PULL_DIS, 3, "PE.10"},
#endif
#ifdef AIC_USING_RTP
    {2, PIN_PULL_DIS, 3, "PA.8"},
    {2, PIN_PULL_DIS, 3, "PA.9"},
    {2, PIN_PULL_DIS, 3, "PA.10"},
    {2, PIN_PULL_DIS, 3, "PA.11"},
#endif
#ifdef AIC_USING_PSADC0
    {3, PIN_PULL_DIS, 3, "PA.0"},
#endif
#ifdef AIC_USING_PSADC1
    {3, PIN_PULL_DIS, 3, "PA.1"},
#endif
#ifdef AIC_USING_PSADC2
    {3, PIN_PULL_DIS, 3, "PA.2"},
#endif
#ifdef AIC_USING_PSADC3
    {3, PIN_PULL_DIS, 3, "PA.3"},
#endif
#ifdef AIC_USING_PSADC4
    {3, PIN_PULL_DIS, 3, "PA.4"},
#endif
#ifdef AIC_USING_PSADC5
    {3, PIN_PULL_DIS, 3, "PA.5"},
#endif
#ifdef AIC_USING_PSADC6
    {3, PIN_PULL_DIS, 3, "PA.6"},
#endif
#ifdef AIC_USING_PSADC7
    {3, PIN_PULL_DIS, 3, "PA.7"},
#endif
#ifdef AIC_USING_PSADC8
    {3, PIN_PULL_DIS, 3, "PA.8"},
#endif
#ifdef AIC_USING_PSADC9
    {3, PIN_PULL_DIS, 3, "PA.9"},
#endif
#ifdef AIC_USING_PSADC10
    {3, PIN_PULL_DIS, 3, "PA.10"},
#endif
#ifdef AIC_USING_PSADC11
    {3, PIN_PULL_DIS, 3, "PA.11"},
#endif
#ifdef AIC_USING_CTP
    {1, PIN_PULL_DIS, 3, AIC_TOUCH_PANEL_RST_PIN},
#ifdef AIC_PM_DEMO_TOUCH_WAKEUP
    {1, PIN_PULL_DIS, 3, AIC_TOUCH_PANEL_INT_PIN, FLAG_WAKEUP_SOURCE},
#else
    {1, PIN_PULL_DIS, 3, AIC_TOUCH_PANEL_INT_PIN},
#endif
#endif
#ifdef AIC_USING_PM
    {1, PIN_PULL_DIS, 3, AIC_BOARD_LEVEL_POWER_PIN, FLAG_POWER_PIN},
#ifdef AIC_PM_DEMO
    {1, PIN_PULL_UP, 3, AIC_PM_POWER_KEY_GPIO, FLAG_WAKEUP_SOURCE},
#endif
#endif
};

uint32_t aic_pinmux_config_size = ARRAY_SIZE(aic_pinmux_config);
