/*
 * Copyright (c) 2023-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <aic_core.h>
#include <aic_hal.h>
#include <aic_hal_dbi.h>

#include "drv_fb.h"

/*
 * DOC: overview
 *
 * This library provides helpers for MIPI Display Bus Interface (DBI)
 * compatible display controllers.
 *
 * There are 3 MIPI DBI implementation types:
 *
 * A. Motorola 6800 type parallel bus
 *
 * B. Intel 8080 type parallel bus
 *
 * C. SPI type parallel bus
 *
 * Currently aic mipi dbi only supports Type B and Type C.
 */

struct aic_dbi_comp
{
    void *regs;

    u32 pixclk;
    u64 sclk_rate;
    u64 pll_disp_rate;

    struct aic_panel *panel;
    struct panel_dbi *dbi;
};
static struct aic_dbi_comp *g_aic_dbi_comp;

static int i8080_clk[] = {20, 30, 20, 15, 20, 10, 10, 10};
static int spi_clk[] = {72, 108, 108, 64, 96, 96, 16, 24, 24};

struct aic_dbi_info {
    u32 type;
    u32 format;
    u32 bpp;
};

static const struct aic_dbi_info dbi_info[] = {
    { .type = I8080, .format = I8080_RGB565_8BIT,         .bpp = 16, },
    { .type = I8080, .format = I8080_RGB666_8BIT,         .bpp = 18, },
    { .type = I8080, .format = I8080_RGB666_9BIT,         .bpp = 18, },
    { .type = I8080, .format = I8080_RGB666_16BIT_3CYCLE, .bpp = 16, },
    { .type = I8080, .format = I8080_RGB666_16BIT_2CYCLE, .bpp = 16, },
    { .type = I8080, .format = I8080_RGB565_16BIT,        .bpp = 16, },
    { .type = I8080, .format = I8080_RGB666_18BIT,        .bpp = 18, },
    { .type = I8080, .format = I8080_RGB888_24BIT,        .bpp = 24, },
    { .type = SPI,   .format = SPI_3LINE_RGB565,          .bpp = 16, },
    { .type = SPI,   .format = SPI_3LINE_RGB666,          .bpp = 18, },
    { .type = SPI,   .format = SPI_3LINE_RGB888,          .bpp = 24, },
    { .type = SPI,   .format = SPI_4LINE_RGB565,          .bpp = 16, },
    { .type = SPI,   .format = SPI_4LINE_RGB666,          .bpp = 18, },
    { .type = SPI,   .format = SPI_4LINE_RGB888,          .bpp = 24, },
    { .type = SPI,   .format = SPI_4SDA_RGB565,           .bpp = 16, },
    { .type = SPI,   .format = SPI_4SDA_RGB666,           .bpp = 18, },
    { .type = SPI,   .format = SPI_4SDA_RGB888,           .bpp = 24, },
};

static struct aic_dbi_comp *aic_dbi_request_drvdata(void)
{
    return g_aic_dbi_comp;
}

static void aic_dbi_release_drvdata(void)
{

}

static int aic_dbi_clk_enable(void)
{
    struct aic_dbi_comp *comp = aic_dbi_request_drvdata();
    u32 pixclk = comp->pixclk;

    hal_clk_set_freq(CLK_PLL_FRA2, comp->pll_disp_rate);
    hal_clk_set_rate(CLK_SCLK, comp->sclk_rate, comp->pll_disp_rate);
    hal_clk_set_rate(CLK_PIX, pixclk, comp->sclk_rate);

    hal_clk_enable(CLK_PLL_FRA2);
    hal_clk_enable(CLK_SCLK);
    hal_clk_enable(CLK_DBI);

    hal_reset_deassert(RESET_DBI);

    aic_dbi_release_drvdata();
    return 0;
}

static int aic_dbi_clk_disable(void)
{
    hal_reset_assert(RESET_DBI);

    hal_clk_disable(CLK_DBI);
    hal_clk_disable(CLK_SCLK);

    return 0;
}

static void aic_dbi_i8080_cfg(void)
{
    struct aic_dbi_comp *comp = aic_dbi_request_drvdata();
    struct panel_dbi *dbi = comp->dbi;

    if (dbi->first_line || dbi->other_line)
        i8080_cmd_ctl(comp->regs, dbi->first_line, dbi->other_line);
}

static void aic_dbi_spi_cfg(void)
{
    struct aic_dbi_comp *comp = aic_dbi_request_drvdata();
    struct panel_dbi *dbi = comp->dbi;
    struct spi_cfg *spi = dbi->spi;

    if (dbi->first_line || dbi->other_line)
        spi_cmd_ctl(comp->regs, dbi->first_line,
                dbi->other_line);

    if (spi) {
        qspi_code_cfg(comp->regs, spi->code[0],
                spi->code[1], spi->code[2]);
        qspi_mode_cfg(comp->regs, spi->code1_cfg,
                spi->vbp_num, spi->qspi_mode);
    }
}

static int aic_dbi_enable(void)
{
    struct aic_dbi_comp *comp = aic_dbi_request_drvdata();
    struct panel_dbi *dbi = comp->dbi;

    reg_set_bits(comp->regs + DBI_CTL,
            DBI_CTL_TYPE_MASK,
            DBI_CTL_TYPE(dbi->type));

    switch (dbi->type) {
    case I8080:
        reg_set_bits(comp->regs + DBI_CTL,
                DBI_CTL_I8080_TYPE_MASK,
                DBI_CTL_I8080_TYPE(dbi->format));
        aic_dbi_i8080_cfg();
        break;
    case SPI:
        reg_set_bits(comp->regs + DBI_CTL,
                DBI_CTL_SPI_TYPE_MASK,
                DBI_CTL_SPI_TYPE(dbi->format / SPI_MODE_NUM));
        reg_set_bits(comp->regs + DBI_CTL,
                DBI_CTL_SPI_FORMAT_MASK,
                DBI_CTL_SPI_FORMAT(dbi->format % SPI_MODE_NUM));
        aic_dbi_spi_cfg();
        break;
    default:
        pr_err("Invalid type %d\n", dbi->type);
        break;
    }

    reg_set_bit(comp->regs + DBI_CTL, DBI_CTL_EN);
    aic_dbi_release_drvdata();

    return 0;
}

static int aic_dbi_disable(void)
{
    struct aic_dbi_comp *comp = aic_dbi_request_drvdata();

    reg_clr_bit(comp->regs + DBI_CTL, DBI_CTL_EN);
    aic_dbi_release_drvdata();
    return 0;
}

static int aic_dbi_attach_panel(struct aic_panel *panel, struct panel_desc *desc)
{
    struct aic_dbi_comp *comp = aic_dbi_request_drvdata();
    u64 pll_disp_rate = 0;
    int i = 0;
    struct panel_dbi *dbi;
    u32 pixclk;

    if (desc) {
        pixclk = desc->timings->pixelclock;
        dbi = desc->dbi;
    } else {
        pixclk = panel->timings->pixelclock;
        dbi = panel->dbi;
    }
    comp->dbi = dbi;
    comp->panel = panel;
    comp->pixclk = pixclk;

    if (dbi->type == I8080)
        comp->sclk_rate = pixclk * i8080_clk[dbi->format];
    else if (dbi->type == SPI)
        comp->sclk_rate = pixclk * spi_clk[dbi->format];

    pll_disp_rate = comp->sclk_rate;
    while (pll_disp_rate < PLL_DISP_FREQ_MIN)
    {
        pll_disp_rate = comp->sclk_rate * (2 << i);
        i++;
    }
    comp->pll_disp_rate = pll_disp_rate;

    aic_dbi_release_drvdata();
    return 0;
}

int aic_dbi_send_cmd(u32 dt, u32 vc, const u8 *data, u32 len)
{
    struct aic_dbi_comp *comp = aic_dbi_request_drvdata();
    struct panel_dbi *dbi = comp->dbi;

    if (dbi->type == I8080)
        i8080_cmd_wr(comp->regs, dt, len, data);

    if (dbi->type == SPI)
        spi_cmd_wr(comp->regs, dt, len, data);

    return 0;
}

static int aic_dbi_get_output_bpp(void)
{
    struct aic_dbi_comp *comp = aic_dbi_request_drvdata();
    struct panel_dbi *dbi = comp->dbi;
    int i, bpp = 24;

    for (i = 0; i < ARRAY_SIZE(dbi_info); i++) {
        if (dbi->type == dbi_info[i].type && dbi->format == dbi_info[i].format) {
            bpp = dbi_info[i].bpp;
            break;
        }
    }

    aic_dbi_release_drvdata();
    return bpp;
}

struct di_funcs aic_dbi_func = {
    .clk_enable = aic_dbi_clk_enable,
    .clk_disable = aic_dbi_clk_disable,
    .enable = aic_dbi_enable,
    .disable = aic_dbi_disable,
    .send_cmd = aic_dbi_send_cmd,
    .attach_panel = aic_dbi_attach_panel,
    .get_output_bpp = aic_dbi_get_output_bpp,
};

static int aic_dbi_probe(void)
{
    struct aic_dbi_comp *comp;

    comp = aicos_malloc(0, sizeof(*comp));
    if (!comp)
    {
        pr_err("alloc dbi comp failed\n");
        return -ENOMEM;
    }

    memset(comp, 0, sizeof(*comp));

    comp->regs = (void *)DBI_BASE;
    g_aic_dbi_comp = comp;

    return 0;
}

static void aic_dbi_remove(void)
{

}

struct platform_driver artinchip_dbi_driver = {
    .name = "artinchip-dbi",
    .component_type = AIC_DBI_COM,
    .probe = aic_dbi_probe,
    .remove = aic_dbi_remove,
    .di_funcs = &aic_dbi_func,
};
