/*
 * Copyright (c) 2023-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "panel_dbi.h"

static inline int panel_dbi_command(struct aic_panel *panel, u8 code)
{
    return panel->callbacks.di_send_cmd((u32)code, 0, NULL, 0);
}

static inline int panel_dbi_command_stackbuf(struct aic_panel *panel, u32 code,
                    const u8 *data, size_t size)
{
    return panel->callbacks.di_send_cmd((u32)code, 0, data, size);
}

int panel_dbi_commands_execute(struct aic_panel *panel,
                    struct panel_dbi_commands *commands)
{
    unsigned int i = 0;

    if (!commands)
        return -1;

    while (i < commands->len) {
        u8 command = commands->buf[i++];
        u8 num_parameters = commands->buf[i++];
        const u8 *parameters = &commands->buf[i];

        if (command == 0x00 && num_parameters == 1)
            aic_delay_ms(parameters[0]);
        else if (num_parameters)
            panel_dbi_command_stackbuf(panel, command, parameters, num_parameters);
        else
            panel_dbi_command(panel, command);

        i += num_parameters;
    }
    return 0;
}

int panel_dbi_default_enable(struct aic_panel *panel)
{
    struct panel_dbi_commands *commands;
    struct panel_dbi *dbi;
    struct panel_desc *desc;

    if (panel->desc && panel->match_num) {
        desc = &panel->desc[panel->match_id];
        dbi = desc->dbi;
    } else {
        dbi = panel->dbi;
    }

    commands = &dbi->commands;

    panel_di_enable(panel, 0);

    panel_dbi_commands_execute(panel, commands);

    panel_de_timing_enable(panel, 0);
    panel_backlight_enable(panel, 0);
    return 0;
}
