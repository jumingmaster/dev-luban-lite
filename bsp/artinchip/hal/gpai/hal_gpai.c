/*
 * Copyright (c) 2022-2025, ArtInChip Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Authors: matteo <duanmt@artinchip.com>
 */

#include "aic_core.h"
#include "hal_gpai.h"
#include "aic_hal_clk.h"
#include "hal_dma.h"

/* Register definition of GPAI Controller */
#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
#define GPAI_MCR            0x000
#define GPAI_INTR           0x004
#endif
#ifdef AIC_GPAI_DRV_V20
#define GPAI_MCHS           0x000
#define GPAI_MCHC           0x004
#endif

#ifdef AIC_GPAI_DRV_V20
#define GPAI_INTS           0x008
#define GPAI_INTC           0x00C
#define GPAI_INTSAT         0x010
#define GPAI_DRQS           0x014
#define GPAI_DRQC           0x018
#endif

#define GPAI_CHnCR(n)       (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x00)
#define GPAI_CHnINT(n)      (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x04)
#define GPAI_CHnPSI(n)      (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x08)
#define GPAI_CHnHLAT(n)     (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x10)
#define GPAI_CHnLLAT(n)     (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x14)
#define GPAI_CHnACR(n)      (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x18)
#define GPAI_CHnFCR(n)      (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x20)
#define GPAI_CHnDATA(n)     (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x24)
#if defined(AIC_GPAI_DRV_V20) || defined(AIC_GPAI_DRV_V21)
#define GPAI_CHnREGDATA(n)  (0x100 + (((n) & AIC_GPAI_CH_NUM_MASK) << 6) + 0x28)
#endif
#define GPAI_VERSION        0xFFC

#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
#define GPAI_MCR_CH0_EN                 BIT(8)
#define GPAI_MCR_CH_EN(n)               (GPAI_MCR_CH0_EN << (n))
#define GPAI_MCR_EN                     BIT(0)

#define GPAI_INTR_CH0_INT_FLAG          BIT(16)
#define GPAI_INTR_CH_INT_FLAG(n)        (GPAI_INTR_CH0_INT_FLAG << (n))
#define GPAI_INTR_CH0_INT_EN            BIT(0)
#define GPAI_INTR_CH_INT_EN(n)          (GPAI_INTR_CH0_INT_EN << (n))
#endif

#if defined(AIC_GPAI_DRV_V20) || defined(AIC_GPAI_DRV_V21)
#define GPAI_MCHS_CH0_SET               BIT(8)
#define GPAI_MCHS_CH_SET(n)             (GPAI_MCHS_CH0_SET << (n))
#define GPAI_MCHS_MDSET                 BIT(0)
#define GPAI_MCHC_CH0_CLS               BIT(8)
#define GPAI_MCHC_CH_CLS(n)             (GPAI_MCHC_CH0_CLS << (n))
#define GPAI_MCHC_MDCLS                 BIT(0)

#define GPAI_INTS_CH0_INT_SET           BIT(0)
#define GPAI_INTS_CH_INT_SET(n)         (GPAI_INTS_CH0_INT_SET << (n))
#define GPAI_INTC_CH0_INT_CLS           BIT(0)
#define GPAI_INTC_CH_INT_CLS(n)         (GPAI_INTC_CH0_INT_CLS << (n))
#define GPAI_INTSAT_CH0_INT_FLG         BIT(0)
#define GPAI_INTSAT_CH_INT_FLG(n)       (GPAI_INTSAT_CH0_INT_FLG << (n))

#define GPAI_DRQS_CH0_DRQ_SAT           BIT(16)
#define GPAI_DRQS_CH_DRQ_SAT(n)         (GPAI_DRQS_CH0_DRQ_SAT << (n))
#define GPAI_DRQS_CH0_DRQ_SET           BIT(0)
#define GPAI_DRQS_CH_DRQ_SET(n)         (GPAI_DRQS_CH0_DRQ_SET << (n))
#define GPAI_DRQC_CH0_DRQ_CLS           BIT(0)
#define GPAI_DRQC_CH_DRQ_CLS(n)         (GPAI_DRQC_CH0_DRQ_CLS << (n))
#endif

#ifdef AIC_GPAI_DRV_V21
#define GPAI_MCHS_ADC_SEL_SET           BIT(1)
#define GPAI_MCHS_ADC_SEL_CLS           BIT(1)
#endif


#define GPAI_CHnCR_SBC_SHIFT        24
#define GPAI_CHnCR_SBC_2_POINTS     1
#define GPAI_CHnCR_SBC_4_POINTS     2
#define GPAI_CHnCR_SBC_8_POINTS     3

#define GPAI_CHnCR_SBC_SHIFT            24
#define GPAI_CHnCR_SBC_MASK             GENMASK(25, 24)
#define GPAI_CHnCR_ADC_ACQ_SHIFT        8
#define GPAI_CHnCR_ADC_ACQ_MASK         GENMASK(15, 8)
#define GPAI_CHnCR_HIGH_ADC_PRIORITY    BIT(4)
#define GPAI_CHnCR_PERIOD_SAMPLE_EN     BIT(1)
#define GPAI_CHnCR_SINGLE_SAMPLE_EN     BIT(0)

#define GPAI_CHnINT_LLA_RM_FLAG     BIT(23)
#define GPAI_CHnINT_LLA_VALID_FLAG  BIT(22)
#define GPAI_CHnINT_HLA_RM_FLAG     BIT(21)
#define GPAI_CHnINT_HLA_VALID_FLAG  BIT(20)
#define GPAI_CHnINT_FIFO_ERR_FLAG   BIT(17)
#define GPAI_CHnINT_DRDY_FLG        BIT(16)
#define GPAI_CHnINT_LLA_RM_IE       BIT(7)
#define GPAI_CHnINT_LLA_VALID_IE    BIT(6)
#define GPAI_CHnINT_HLA_RM_IE       BIT(5)
#define GPAI_CHnINT_HLA_VALID_IE    BIT(4)
#define GPAI_CHnINT_FIFO_ERR_IE     BIT(1)
#define GPAI_CHnINT_DAT_RDY_IE      BIT(0)

#define GPAI_CHnLAT_HLLA_RM_THD_SHIFT   16
#define GPAI_CHnLAT_HLLA_RM_THD_MASK    GENMASK(27, 16)
#define GPAI_CHnLAT_HLLA_THD_MASK       GENMASK(11, 0)
#define GPAI_CHnLAT_HLA_RM_THD(n)       ((n) - 30)
#define GPAI_CHnLAT_LLA_RM_THD(n)       ((n) + 30)

#define GPAI_CHnACR_DISCARD_NOR_DAT     BIT(6)
#define GPAI_CHnACR_DISCARD_LL_DAT      BIT(5)
#define GPAI_CHnACR_DISCARD_HL_DAT      BIT(4)
#define GPAI_CHnACR_LLA_EN              BIT(1)
#define GPAI_CHnACR_HLA_EN              BIT(0)

#define GPAI_CHnFCR_DAT_CNT_MAX(ch)     ((ch) > 1 ? 0x8 : 0x40)
#define GPAI_CHnFCR_DAT_CNT_SHIFT       24
#define GPAI_CHnFCR_DAT_CNT_MASK        GENMASK(30, 24)
#define GPAI_CHnFCR_UF_STS              BIT(18)
#define GPAI_CHnFCR_OF_STS              BIT(17)
#define GPAI_CHnFCR_DAT_RDY_THD_SHIFT   8
#define GPAI_CHnFCR_DAT_RDY_THD_MASK    GENMASK(15, 8)
#define GPAI_CHnFCR_FLUSH               BIT(0)

#define GPAI_SRC_RX_MAXBURST            1
#define GPAI_DST_RX_MAXBURST            16

extern struct aic_gpai_ch aic_gpai_chs[];
static u32 aic_gpai_ch_num = 0; // the number of available channel

static inline void gpai_writel(u32 val, int reg)
{
    writel(val, GPAI_BASE + reg);
}

static inline u32 gpai_readl(int reg)
{
    return readl(GPAI_BASE + reg);
}

static u16 gpai_vol2data(s32 vol)
{
    return vol;
}

static u32 gpai_ms2itv(u32 pclk_rate, u32 us)
{
    u32 tmp = 0;

    tmp = pclk_rate / 1000000;
    tmp *= us;
    return tmp;
}

#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
static void gpai_reg_enable(int offset, int bit, int enable)
{
    int tmp = gpai_readl(offset);

    if (enable)
        tmp |= bit;
    else
        tmp &= ~bit;

    gpai_writel(tmp, offset);
}

void aich_gpai_enable(int enable)
{
    gpai_reg_enable(GPAI_MCR, GPAI_MCR_EN, enable);
}

void aich_gpai_ch_enable(u32 ch, int enable)
{
    gpai_reg_enable(GPAI_MCR, GPAI_MCR_CH_EN(ch), enable);
}
#endif

#if defined(AIC_GPAI_DRV_V20) || defined(AIC_GPAI_DRV_V21)
static void gpai_reg_enable(int offset, int bit)
{
    int tmp = gpai_readl(offset);
    tmp |= bit;
    gpai_writel(tmp, offset);
}

void aich_gpai_enable(int enable)
{
    if (enable)
        gpai_reg_enable(GPAI_MCHS, GPAI_MCHS_MDSET);
    else
        gpai_reg_enable(GPAI_MCHC, GPAI_MCHC_MDCLS);
}

void aich_gpai_ch_enable(u32 ch, int enable)
{
    if (enable)
        gpai_reg_enable(GPAI_MCHS, GPAI_MCHS_CH_SET(ch));
    else
        gpai_reg_enable(GPAI_MCHC, GPAI_MCHC_CH_CLS(ch));
}

void hal_gpai_drq_enable(u32 ch, u32 enable)
{
    if (enable)
        gpai_reg_enable(GPAI_DRQS, GPAI_DRQS_CH_DRQ_SET(ch));
    else
        gpai_reg_enable(GPAI_DRQC, GPAI_DRQC_CH_DRQ_CLS(ch));
}
#endif

#if defined(AIC_GPAI_DRV_V21)
void aich_gpai_adc_sel_enable(int adc_type)
{
    switch (adc_type) {
    case AIC_GPAI_ADC_12BIT:
        pr_info("GPAI selects the 12 bit ADC\n");
        gpai_reg_enable(GPAI_MCHS, GPAI_MCHS_ADC_SEL_SET);
        break;
    case AIC_GPAI_ADC_14BIT:
        pr_info("GPAI selects the 14 bit ADC\n");
        gpai_reg_enable(GPAI_MCHC, GPAI_MCHS_ADC_SEL_CLS);
        break;
    default:
        pr_err("The ADC type is error\n");
        break;
    }
}
#endif

static void gpai_int_enable(u32 ch, u32 enable, u32 detail)
{
#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
    u32 val = 0;
    val = gpai_readl(GPAI_INTR);
    if (enable) {
        val |= GPAI_INTR_CH_INT_EN(ch);
        gpai_writel(detail, GPAI_CHnINT(ch));
    } else {
        val &= ~GPAI_INTR_CH_INT_EN(ch);
        gpai_writel(0, GPAI_CHnINT(ch));
    }
    gpai_writel(val, GPAI_INTR);
#endif

#if defined(AIC_GPAI_DRV_V20) || defined(AIC_GPAI_DRV_V21)
    if (enable) {
        gpai_reg_enable(GPAI_INTS, GPAI_INTS_CH_INT_SET(ch));
        gpai_writel(detail, GPAI_CHnINT(ch));
    } else {
        gpai_reg_enable(GPAI_INTC, GPAI_INTC_CH_INT_CLS(ch));
        gpai_writel(0, GPAI_CHnINT(ch));
    }
#endif
}

static void gpai_fifo_init(u32 ch)
{
    u32 val = 0;

    val = 1 << GPAI_CHnFCR_DAT_RDY_THD_SHIFT;
    gpai_writel(val, GPAI_CHnFCR(ch));
}

static void gpai_fifo_flush(u32 ch)
{
    u32 val = gpai_readl(GPAI_CHnFCR(ch));

    if (val & GPAI_CHnFCR_UF_STS)
        pr_err("ch%d FIFO is Underflow!%#x\n", ch, val);
    if (val & GPAI_CHnFCR_OF_STS)
        pr_err("ch%d FIFO is Overflow!%#x\n", ch, val);

    gpai_writel(val | GPAI_CHnFCR_FLUSH, GPAI_CHnFCR(ch));
}

static void gpai_single_mode(u32 ch)
{
    u32 val = 0;
    struct aic_gpai_ch *chan;
    chan = hal_gpai_ch_is_valid(ch);

    val = gpai_readl(GPAI_CHnCR(ch));
    val |= GPAI_CHnCR_SBC_8_POINTS << GPAI_CHnCR_SBC_SHIFT;
    val &= ~GPAI_CHnCR_ADC_ACQ_MASK;
    val |= chan->adc_acq << GPAI_CHnCR_ADC_ACQ_SHIFT;
    gpai_writel(val, GPAI_CHnCR(ch));

    val = gpai_readl(GPAI_CHnCR(ch));
    val |= GPAI_CHnCR_SINGLE_SAMPLE_EN;
    gpai_writel(val, GPAI_CHnCR(ch));
    gpai_int_enable(ch, 1,
            GPAI_CHnINT_DAT_RDY_IE | GPAI_CHnINT_FIFO_ERR_IE);
}

/* Only in period mode, HLA and LLA are available */
static void gpai_period_mode(struct aic_gpai_ch *chan, u32 pclk)
{
    u32 val, acr = 0, ch = chan->id;
    u32 detail = GPAI_CHnINT_DAT_RDY_IE | GPAI_CHnINT_FIFO_ERR_IE;

    if (chan->hla_enable) {
        detail |= GPAI_CHnINT_HLA_RM_IE | GPAI_CHnINT_HLA_VALID_IE;
        val = ((gpai_vol2data(chan->hla_rm_thd) << GPAI_CHnLAT_HLLA_RM_THD_SHIFT)
            & GPAI_CHnLAT_HLLA_RM_THD_MASK)
            | (gpai_vol2data(chan->hla_thd) & GPAI_CHnLAT_HLLA_THD_MASK);
        gpai_writel(val, GPAI_CHnHLAT(ch));
        acr |= GPAI_CHnACR_HLA_EN;
    }

    if (chan->lla_enable) {
        detail |= GPAI_CHnINT_LLA_VALID_IE | GPAI_CHnINT_LLA_RM_IE;
        val = ((gpai_vol2data(chan->lla_rm_thd) << GPAI_CHnLAT_HLLA_RM_THD_SHIFT)
            & GPAI_CHnLAT_HLLA_RM_THD_MASK)
            | (gpai_vol2data(chan->lla_thd) & GPAI_CHnLAT_HLLA_THD_MASK);
        gpai_writel(val, GPAI_CHnLLAT(ch));
        acr |= GPAI_CHnACR_LLA_EN;
    }

    if (chan->obtain_data_mode == AIC_GPAI_OBTAIN_DATA_BY_CPU)
            gpai_int_enable(ch, 1, detail);

    gpai_writel(acr, GPAI_CHnACR(ch));

    val = gpai_ms2itv(pclk, chan->smp_period);
    gpai_writel(val, GPAI_CHnPSI(ch));

    val = gpai_readl(GPAI_CHnCR(ch));
    val |= GPAI_CHnCR_SBC_8_POINTS << GPAI_CHnCR_SBC_SHIFT;
    val &= ~GPAI_CHnCR_ADC_ACQ_MASK;
    val |= chan->adc_acq << GPAI_CHnCR_ADC_ACQ_SHIFT;
    gpai_writel(val, GPAI_CHnCR(ch));

    val = gpai_readl(GPAI_CHnCR(ch));
    val |= GPAI_CHnCR_PERIOD_SAMPLE_EN;
    gpai_writel(val, GPAI_CHnCR(ch));
}

void hal_gpai_set_high_priority(u32 ch)
{
    u32 val = 0;

    val = gpai_readl(GPAI_CHnCR(ch));
    val |= GPAI_CHnCR_HIGH_ADC_PRIORITY;
    gpai_writel(val, GPAI_CHnCR(ch));
}

int aich_gpai_ch_init(struct aic_gpai_ch *chan, u32 pclk)
{
    aich_gpai_ch_enable(chan->id, 1);
    gpai_fifo_init(chan->id);
    if (chan->mode == AIC_GPAI_MODE_PERIOD)
        gpai_period_mode(chan, pclk);

    /* For single mode, should init the channel in .read() */
    return 0;
}

void aich_gpai_status_show(struct aic_gpai_ch *chan)
{
    int version = gpai_readl(GPAI_VERSION);

#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
    int mcr = gpai_readl(GPAI_MCR);

    printf("In GPAI V%d.%02d:\n"
               "Ch Mode Enable Value  LTA  HTA\n"
               "%2d %4s %6d %5d %4d %4d\n",
               version >> 8, version & 0xff,
               chan->id, chan->mode ? "P" : "S",
               mcr & GPAI_MCR_CH_EN(chan->id) ? 1 : 0,
               chan->avg_data, chan->lla_thd, chan->hla_thd);
#endif

#if defined(AIC_GPAI_DRV_V20) || defined(AIC_GPAI_DRV_V21)
    int mcr = gpai_readl(GPAI_MCHS);

    printf("In GPAI V%d.%02d:\n"
               "Ch Mode Enable Value  LTA  HTA\n"
               "%2d %4s %6d %5d %4d %4d\n",
               version >> 8, version & 0xff,
               chan->id, chan->mode ? "P" : "S",
               mcr & GPAI_MCHS_CH_SET(chan->id) ? 1 : 0,
               chan->avg_data, chan->lla_thd, chan->hla_thd);
#endif
}

static int hal_gpai_irq_read_fifo(struct aic_gpai_ch *chan)
{
    u32 i, ch = chan->id, tmp = 0;
    u32 cnt = (gpai_readl(GPAI_CHnFCR(ch)) & GPAI_CHnFCR_DAT_CNT_MASK)
            >> GPAI_CHnFCR_DAT_CNT_SHIFT;
    chan->avg_data = 0;

    if (unlikely(cnt == 0 || cnt > GPAI_CHnFCR_DAT_CNT_MAX(ch))) {
        pr_err("ch%d invalid data count %d\n", ch, cnt);
        return -1;
    }

    for (i = 0; i < cnt; i++) {
        chan->fifo_data[i] = gpai_readl(GPAI_CHnDATA(ch));
        if (chan->mode == AIC_GPAI_MODE_SINGLE)
            tmp += chan->fifo_data[i];
    }

    chan->fifo_valid_cnt = cnt;
    if (chan->mode == AIC_GPAI_MODE_SINGLE) {
        chan->avg_data = tmp / cnt;
        pr_debug("There are %d data ready in ch%d, last %d\n", cnt, ch, chan->avg_data);
    }

    return 0;
}

int hal_gpai_read_poll(u32 ch, u16 *val, u32 timeout)
{
    u32 ch_flag = 0, ch_int = 0;
    struct aic_gpai_ch *chan = NULL;

    while (1) {
#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
        ch_flag = gpai_readl(GPAI_INTR);
        if (!(ch_flag & GPAI_INTR_CH_INT_FLAG(ch)))
            continue;
#endif
#if defined(AIC_GPAI_DRV_V20) || defined(AIC_GPAI_DRV_V21)
        ch_flag = gpai_readl(GPAI_INTSAT);
        if (!(ch_flag & GPAI_INTSAT_CH_INT_FLG(ch)))
            continue;
#endif

        chan = hal_gpai_ch_is_valid(ch);
        if (!chan)
            return -1;
        ch_int = gpai_readl(GPAI_CHnINT(ch));
        gpai_writel(ch_int, GPAI_CHnINT(ch));

#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
        if (ch_int & GPAI_CHnINT_DRDY_FLG) {
            hal_gpai_irq_read_fifo(chan);
            if (val)
                val[0] = chan->avg_data;
            break;
        }
#endif
#if defined(AIC_GPAI_DRV_V20) || defined(AIC_GPAI_DRV_V21)
        val[0] = gpai_readl(GPAI_CHnREGDATA(ch));
        break;
#endif
    }

    return 0;
}

int hal_gpai_get_data(struct aic_gpai_ch *chan, u16 *val, u32 timeout)
{
    int ret = 0;
    u32 ch = chan->id;

    if (!chan->available) {
        hal_log_err("Ch%d is unavailable!\n", chan->id);
        return -ENODATA;
    }

    if (chan->mode == AIC_GPAI_MODE_PERIOD) {
        memcpy(val, chan->fifo_data, chan->fifo_valid_cnt * sizeof(chan->fifo_data[0]));
        return 0;
    }

    gpai_single_mode(ch);

    if (chan->obtain_data_mode == AIC_GPAI_OBTAIN_DATA_BY_POLL) {
        hal_gpai_read_poll(ch, val, timeout);
        return 0;
    }

    ret = aicos_sem_take(chan->complete, timeout);
    if (ret < 0) {
        hal_log_err("Ch%d read timeout!\n", ch);
        aich_gpai_ch_enable(ch, 0);
        return -ETIMEDOUT;
    }

    if (val)
        val[0] = chan->avg_data;

    return 0;
}

struct aic_gpai_ch *hal_gpai_ch_is_valid(u32 ch)
{
    s32 i;

    if (ch >= AIC_GPAI_CH_NUM) {
        pr_err("Invalid channel %d\n", ch);
        return NULL;
    }

    for (i = 0; i < aic_gpai_ch_num; i++) {
        if (aic_gpai_chs[i].id != ch)
            continue;

        if (aic_gpai_chs[i].available)
            return &aic_gpai_chs[i];
        else
            break;
    }
    pr_warn("Ch%d is unavailable!\n", ch);
    return NULL;
}

irqreturn_t aich_gpai_isr(int irq, void *arg)
{
    u32 ch_flag = 0, ch_int = 0;
    int i;
    struct aic_gpai_ch *chan = NULL;

#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
    ch_flag = gpai_readl(GPAI_INTR);
#endif
#if defined(AIC_GPAI_DRV_V20) || defined(AIC_GPAI_DRV_V21)
    ch_flag = gpai_readl(GPAI_INTSAT);
#endif


    for (i = 0; i < AIC_GPAI_CH_NUM; i++) {
#if defined(AIC_GPAI_DRV_V10) || defined(AIC_GPAI_DRV_V11)
        if (!(ch_flag & GPAI_INTR_CH_INT_FLAG(i)))
            continue;
#endif
#if defined(AIC_GPAI_DRV_V21)
        if (!(ch_flag & GPAI_INTSAT_CH_INT_FLG(i)))
            continue;
#endif
#ifdef AIC_GPAI_DRV_V20
        if (!(ch_flag & GPAI_INTSAT_CH_INT_FLG(i)))
            continue;
#endif

        chan = hal_gpai_ch_is_valid(i);
        if (!chan)
            return IRQ_NONE;

        ch_int = gpai_readl(GPAI_CHnINT(i));
        gpai_writel(ch_int, GPAI_CHnINT(i));
        if (ch_int & GPAI_CHnINT_DRDY_FLG) {
            hal_gpai_irq_read_fifo(chan);
            chan->irq_count++;
            if (chan->mode == AIC_GPAI_MODE_SINGLE)
                aicos_sem_give(chan->complete);
            if (chan->irq_info.callback)
                chan->irq_info.callback(chan->irq_info.callback_param);
        }

        if (ch_int & GPAI_CHnINT_LLA_VALID_FLAG)
            pr_warn("LLA: ch%d %d!\n", i, chan->avg_data);
        if (ch_int & GPAI_CHnINT_LLA_RM_FLAG)
            pr_warn("LLA removed: ch%d %d\n", i,
                 chan->avg_data);
        if (ch_int & GPAI_CHnINT_HLA_VALID_FLAG)
            pr_warn("HLA: ch%d %d!\n", i, chan->avg_data);
        if (ch_int & GPAI_CHnINT_HLA_RM_FLAG)
            pr_warn("HLA removed: ch%d %d\n", i,
                 chan->avg_data);
        if (ch_int & GPAI_CHnINT_FIFO_ERR_FLAG)
            gpai_fifo_flush(i);
    }
    pr_debug("IRQ flag %#x, detail %#x\n", ch_flag, ch_int);

    return IRQ_HANDLED;
}

s32 hal_gpai_init(void)
{
    s32 ret = 0;

    ret = hal_clk_enable_deassertrst(CLK_GPAI);
    if (ret < 0) {
        pr_err("GPAI clock/reset enable failed!");
        return -1;
    }

    return ret;
}

s32 hal_gpai_deinit(void)
{
    s32 ret = 0;

    ret = hal_clk_disable_assertrst(CLK_GPAI);
    if (ret < 0) {
        pr_err("GPAI clock/reset disable failed!");
        return -1;
    }

    return ret;
}

void hal_gpai_clk_get(struct aic_gpai_ch *chan)
{
    chan->pclk_rate = hal_clk_get_freq(hal_clk_get_parent(CLK_GPAI));
}

void hal_gpai_set_ch_num(u32 num)
{
    aic_gpai_ch_num = num;
}

#ifdef AIC_GPAI_DRV_DMA
void hal_gpai_stop_dma(struct aic_gpai_ch *chan)
{
    int val;

    val = gpai_readl(GPAI_CHnCR(chan->id));
    val &= ~GPAI_CHnCR_PERIOD_SAMPLE_EN;
    gpai_writel(val, GPAI_CHnCR(chan->id));
    aich_gpai_enable(0);

    hal_dma_chan_stop(chan->dma_rx_info.dma_chan);
    hal_release_dma_chan(chan->dma_rx_info.dma_chan);
}

static void hal_dma_transfer_callback(void *arg)
{
    struct aic_gpai_ch *chan;

    void *dma_cb_data = NULL;
    dma_callback dma_cb = NULL;
    chan = (struct aic_gpai_ch *)arg;

    dma_cb = chan->dma_rx_info.callback;
    dma_cb_data = chan->dma_rx_info.callback_param;
    if (dma_cb)
        dma_cb(dma_cb_data);
}

void hal_gpai_config_dma(struct aic_gpai_ch *chan)
{
    struct dma_slave_config config;
    struct aic_dma_transfer_info *info;

    hal_gpai_drq_enable(chan->id, 1);

    config.direction = DMA_DEV_TO_MEM;
    config.src_addr = GPAI_BASE + GPAI_CHnDATA(chan->id);
    config.slave_id = chan->dma_port_id;
    config.src_maxburst = GPAI_SRC_RX_MAXBURST;
    config.dst_maxburst = GPAI_DST_RX_MAXBURST;
    config.src_addr_width = DMA_SLAVE_BUSWIDTH_4_BYTES;
    config.dst_addr_width = DMA_SLAVE_BUSWIDTH_UNDEFINED;

    info = &chan->dma_rx_info;

    info->dma_chan = hal_request_dma_chan();
    if (!info->dma_chan) {
        hal_log_err("GPAI request dma channel error\n");
        return;
    }
    hal_dma_chan_register_cb(info->dma_chan, hal_dma_transfer_callback,
                             (void *)chan);
    hal_dma_chan_config(info->dma_chan, &config);
    hal_dma_chan_prep_device(info->dma_chan, (ulong)info->buf,
                             config.src_addr, info->buf_size,
                             DMA_DEV_TO_MEM);
    return;
}

void hal_gpai_start_dma(struct aic_gpai_ch *chan)
{
    hal_dma_chan_start(chan->dma_rx_info.dma_chan);
}
#endif

