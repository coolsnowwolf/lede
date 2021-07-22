/*
 ***************************************************************************
 * MediaTek Inc.
 *
 * All rights reserved. source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek, Inc. is obtained.
 ***************************************************************************

	Module Name: wifi_offload
	whnat_hw.c
*/


#include "woe.h"
#include "woe_hw.h"
#include "wed_def.h"


/*whnat cr interesting list*/
static struct whnat_wifi_cr_map cr_map_list[] = {
	{WIFI_INT_STA, WED_INT_STA, WHNAT_CR_WED},
	{WIFI_INT_MSK, WED_INT_MSK, WHNAT_CR_WED},
	/* {WIFI_WPDMA_GLO_CFG,WED_GLO_CFG,WHNAT_CR_WED}, */
#ifdef WED_RX_SUPPORT
	{WIFI_RX_RING1_BASE, WED_RX1_CTRL0, WHNAT_CR_WED},
	{WIFI_RX_RING1_CNT, WED_RX1_CTRL1, WHNAT_CR_WED},
	{WIFI_RX_RING1_CIDX, WED_RX1_CTRL2, WHNAT_CR_WED},
	{WIFI_RX_RING1_DIDX, WED_RX1_CTRL3, WHNAT_CR_WED},
#endif /*RX*/
#ifdef WED_TX_SUPPORT
	{WIFI_TX_RING0_BASE, WED_TX0_CTRL0, WHNAT_CR_WED},
	{WIFI_TX_RING0_CNT, WED_TX0_CTRL1, WHNAT_CR_WED},
	{WIFI_TX_RING0_CIDX, WED_TX0_CTRL2, WHNAT_CR_WED},
	{WIFI_TX_RING0_DIDX, WED_TX0_CTRL3, WHNAT_CR_WED},
	{WIFI_TX_RING1_BASE, WED_TX1_CTRL0, WHNAT_CR_WED},
	{WIFI_TX_RING1_CNT, WED_TX1_CTRL1, WHNAT_CR_WED},
	{WIFI_TX_RING1_CIDX, WED_TX1_CTRL2, WHNAT_CR_WED},
	{WIFI_TX_RING1_DIDX, WED_TX1_CTRL3, WHNAT_CR_WED},
#endif
	{0, 0}
};


#define WHNAT_RESET(_entry, _addr, _value) {\
		unsigned int cnt = 0;\
		WHNAT_IO_WRITE32(_entry, _addr, _value);\
		while (_value != 0 && cnt < WED_POLL_MAX) {\
			WHNAT_IO_READ32(_entry, _addr, &_value);\
			cnt++;\
		} \
		if (cnt >= WED_POLL_MAX) {\
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): Reset addr=%x,value=%x,cnt=%d fail!\n",\
					__func__, _addr, _value, cnt);\
		} \
	}

/*Local function*/

/*
*
*/
static inline int wed_agt_dis_ck(struct wed_entry *wed, unsigned int addr, unsigned int busy_bit)
{
	unsigned int cnt = 0;
	unsigned int value;

	WHNAT_IO_READ32(wed, addr, &value);

	while ((value & (1 << busy_bit)) &&
		   cnt < WED_POLL_MAX) {
		usleep_range(10000, 15000);
		WHNAT_IO_READ32(wed, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): %x disable bit %d fail!!\n", __func__, addr, busy_bit);
		return -1;
	}

	return 0;
}

/*
*
*/
static inline int wdma_agt_dis_ck(struct wdma_entry *wdma, unsigned int addr, unsigned busy_bit)
{
	unsigned int cnt = 0;
	unsigned int value;

	WHNAT_IO_READ32(wdma, addr, &value);

	while ((value & (1 << busy_bit)) &&
		   cnt < WED_POLL_MAX) {
		usleep_range(10000, 15000);
		WHNAT_IO_READ32(wdma, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): %x disable bit %d fail!!\n", __func__, addr, busy_bit);
		return -1;
	}

	return 0;
}

/*
*
*/
static inline int wifi_agt_dis_ck(struct wifi_entry *wifi, unsigned int addr, unsigned int busy_bit)
{
	unsigned int cnt = 0;
	unsigned int value;

	WHNAT_IO_READ32(wifi, addr, &value);

	while ((value & (1 << busy_bit)) &&
		   cnt < WED_POLL_MAX) {
		usleep_range(10000, 15000);
		WHNAT_IO_READ32(wifi, addr, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): %x disable bit %d fail!!\n", __func__, addr, busy_bit);
		return -1;
	}

	return 0;
}

/*
*
*/
static void wed_dma_ctrl(struct wed_entry *wed, unsigned char txrx)
{
	unsigned int wed_cfg;
	unsigned int wed_wdma_cfg;
	unsigned int wed_wpdma_cfg;
	/*reset wed*/
	WHNAT_IO_READ32(wed, WED_GLO_CFG, &wed_cfg);
	wed_cfg &= ~((1 << WED_GLO_CFG_FLD_TX_DMA_EN) | (1 << WED_GLO_CFG_FLD_RX_DMA_EN));
	WHNAT_IO_READ32(wed, WED_WPDMA_GLO_CFG, &wed_wpdma_cfg);
	wed_wpdma_cfg &= ~((1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN) | (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN));
	WHNAT_IO_READ32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg &= ~((1 << WED_WDMA_GLO_CFG_FLD_TX_DRV_EN) | (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN));

	switch (txrx) {
	case WHNAT_DMA_TX: {
		WHNAT_DBG(WHNAT_DBG_INF, "%s(): %s DMA TX.\n", __func__, (txrx ? "ENABLE":"DISABLE"));
		wed_cfg |= (1 << WED_GLO_CFG_FLD_TX_DMA_EN);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN);
		wed_wdma_cfg |=  (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	}
	break;

	case WHNAT_DMA_RX: {
		WHNAT_DBG(WHNAT_DBG_INF, "%s(): %s DMA RX.\n", __func__, (txrx ? "ENABLE":"DISABLE"));
		wed_cfg |= (1 << WED_GLO_CFG_FLD_RX_DMA_EN);
		wed_wpdma_cfg |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN);
	}
	break;

	case WHNAT_DMA_TXRX: {
		WHNAT_DBG(WHNAT_DBG_INF, "%s(): %s DMA TXRX.\n", __func__, (txrx ? "ENABLE":"DISABLE"));
		wed_cfg |= ((1 << WED_GLO_CFG_FLD_TX_DMA_EN) | (1 << WED_GLO_CFG_FLD_RX_DMA_EN));
		wed_wpdma_cfg |= ((1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN) | (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN));
		wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
	}
	break;
	}

	WHNAT_IO_WRITE32(wed, WED_GLO_CFG, wed_cfg);
#ifdef WED_HW_TX_SUPPORT
	WHNAT_IO_WRITE32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
	WHNAT_IO_WRITE32(wed, WED_WPDMA_GLO_CFG, wed_wpdma_cfg);
}

/*
*
*/
static void wdma_dma_ctrl(struct wdma_entry *wdma, unsigned char txrx)
{
	unsigned int wdma_cfg = 0;

	WHNAT_IO_READ32(wdma, WDMA_GLO_CFG, &wdma_cfg);
	WHNAT_DBG(WHNAT_DBG_OFF, "%s(): WDMA_GLO_CFG=%x\n",  __func__, wdma_cfg);

	if (txrx) {
		/*reset wdma*/
		wdma_cfg |= (1 << WDMA_GLO_CFG_RX_INFO1_PRERESERVE);
		wdma_cfg |= (1 << WDMA_GLO_CFG_RX_INFO2_PRERESERVE);
		wdma_cfg |= (1 << WDMA_GLO_CFG_RX_INFO3_PRERESERVE);
		WHNAT_IO_WRITE32(wdma, WDMA_GLO_CFG, wdma_cfg);
	} else {
		wdma_cfg &= ~(1 << WDMA_GLO_CFG_RX_INFO1_PRERESERVE);
		wdma_cfg &= ~(1 << WDMA_GLO_CFG_RX_INFO2_PRERESERVE);
		WHNAT_IO_WRITE32(wdma, WDMA_GLO_CFG, wdma_cfg);
	}
}

/*
*
*/
#ifdef WED_HW_TX_SUPPORT
static void whnat_hal_bfm_init(struct whnat_entry *whnat)
{
	struct wed_entry *wed = &whnat->wed;
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	struct wed_tx_ring_ctrl *ring_ctrl = &wed->res_ctrl.tx_ctrl.ring_ctrl;
	unsigned int value = 0;
	/*PAUSE BUF MGMT*/
	value = (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	/*should be set before WED_MOD_RST is invoked*/
	value |= ((res->pkt_num/WED_TOKEN_UNIT) << WED_TX_BM_CTRL_FLD_VLD_GRP_NUM);
	value |= ((ring_ctrl->ring_len/256) << WED_TX_BM_CTRL_FLD_RSV_GRP_NUM);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_CTRL, value);
	/*TX BM_BASE*/
	value = res->des_buf.alloc_pa;
	WHNAT_IO_WRITE32(wed, WED_TX_BM_BASE, value);
	/*TX token cfg */
	value = ((WED_TOKEN_START) << WED_TX_BM_TKID_FLD_START_ID);
	value |= ((WED_TOKEN_END) << WED_TX_BM_TKID_FLD_END_ID);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_TKID, value);
	/*TX packet len*/
	value = (res->pkt_len & WED_TX_BM_BLEN_FLD_BYTE_LEN_MASK);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_BLEN, value);
	/*dynamic adjust*/
#ifdef WED_DYNAMIC_BM_SUPPORT
	value = (WED_TOKEN_LOW << WED_TX_BM_DYN_TH_FLD_LOW_GRP_NUM);
	value |= (WED_TOKEN_HIGH << WED_TX_BM_DYN_TH_FLD_HI_GRP_NUM);
#else
	value = (1 << WED_TX_BM_DYN_TH_FLD_LOW_GRP_NUM);
	value |= (WED_TX_BM_DYN_TH_FLD_HI_GRP_MASK << WED_TX_BM_DYN_TH_FLD_HI_GRP_NUM);
#endif /*WED_DYNAMIC_BW_SUPPORT*/
	WHNAT_IO_WRITE32(wed, WED_TX_BM_DYN_TH, value);
	/*Reset Buf mgmt for ready to start*/
	value = 1 << WED_MOD_RST_FLD_TX_BM;
	WHNAT_RESET(wed, WED_MOD_RST, value);
	/*Enable agent for BM*/
	WHNAT_IO_READ32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_BM_EN);
	value |= (1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN);
	WHNAT_IO_WRITE32(wed, WED_CTRL, value);
	WHNAT_IO_READ32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_CTRL, value);
}

/*
*
*/
void whnat_hal_bfm_freecnt(struct wed_entry *wed, unsigned int *cnt)
{
	unsigned int value;
	unsigned int tcnt = 0;
	unsigned int fcnt = 0;
	unsigned int cr;
	unsigned int i;
	unsigned int j = 0;
	unsigned int k = 0;
	unsigned int grp = 0;
	char str[256] = "";
	struct wed_buf_res *buf_res = &wed->res_ctrl.tx_ctrl.res;

	grp = buf_res->pkt_num/WED_TOKEN_STATUS_UNIT;
	tcnt = buf_res->pkt_num%WED_TOKEN_STATUS_UNIT;
	value = 1 << WED_DBG_CTRL_FLD_TX_BM_MEM_DBG;
	WHNAT_IO_WRITE32(wed, WED_DBG_CTRL, value);

	for (cr = WED_BMF_VALID_TABLE_START; cr <= WED_BMF_VALID_TABLE_END; cr += 4) {
		if (j%4 == 0)
			sprintf(str, "%08x:\t", cr);

		WHNAT_IO_READ32(wed, cr, &value);

		if (k < grp) {
			for (i = 0; i < WED_TOKEN_STATUS_UNIT; i++) {
				if (value & (1 << i))
					fcnt++;
			}
		} else if (k == grp) {
			for (i = 0; i < tcnt; i++) {
				if (value & (1 << i))
					fcnt++;
			}
		}

		k++;
		sprintf(str+strlen(str), "%08x\t", value);

		if (j%4 == 3) {
			WHNAT_DBG(WHNAT_DBG_OFF, "%s\n", str);
			memset(str, 0, sizeof(str));
		}

		j++;
	}

	value = 0x14;
	WHNAT_IO_WRITE32(wed, WED_DBG_CTRL, value);
	WHNAT_IO_READ32(wed, WED_DBG_PRB1, &value);
	WHNAT_DBG(WHNAT_DBG_OFF, "Total Free Cnt(%d), CR: Free Cnt(%d), Usage Cnt(%d),Pkt Cnt(%d)\n",
			fcnt,
			((value >> 16) & 0xffff),
			(value & 0xffff),
			buf_res->pkt_num);
	*cnt = ((value >> 16) & 0xffff);
	/*Disable debug*/
	WHNAT_IO_WRITE32(wed, WED_DBG_CTRL, 0);
}

/*
*
*/
void whnat_hal_bfm_free(struct wed_entry *wed)
{
	unsigned int value;
	unsigned int cr = 0;
	unsigned int cnt = 0;
	unsigned int grp = 0;
	unsigned int i;
	struct wed_buf_res *buf_res = &wed->res_ctrl.tx_ctrl.res;
	/*PAUSE BUF MGMT*/
	WHNAT_IO_READ32(wed, WED_TX_BM_CTRL, &value);
	value |= (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_CTRL, value);
	/*POLL status bit*/
	WHNAT_IO_READ32(wed, WED_ST, &value);

	while ((value & (0xff << WED_ST_FLD_TX_ST)) && cnt < WED_POLL_MAX) {
		WHNAT_IO_READ32(wed, WED_ST, &value);
		cnt++;
	}

	/*Set SRAM enable*/
	value = 1 << WED_DBG_CTRL_FLD_TX_BM_MEM_DBG;
	WHNAT_IO_WRITE32(wed, WED_DBG_CTRL, value);
	grp = buf_res->pkt_num/WED_TOKEN_STATUS_UNIT;
	cnt = buf_res->pkt_num%WED_TOKEN_STATUS_UNIT;

	for (i = 0; i < grp; i++) {
		cr = WED_BMF_VALID_TABLE_START+4*i;
		WHNAT_IO_WRITE32(wed, cr, 0xffffffff);
	}

	value = 0;
	cr += 4;

	for (i = 0; i < cnt; i++)
		value |= (1 << cnt);

	WHNAT_IO_WRITE32(wed, cr, value);
	/*Disable SRAM Mapping*/
	WHNAT_IO_WRITE32(wed, WED_DBG_CTRL, 0);
	/*Disable PAUSE*/
	WHNAT_IO_READ32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_CTRL, value);
}
#endif /*WED_HW_TX_SUPPORT*/

#ifdef WED_DYNAMIC_BM_SUPPORT
/*
*
*/
void whnat_hal_bfm_update(struct wed_entry *wed, unsigned char reduce)
{
	struct wed_buf_res *res = &wed->res_ctrl.tx_ctrl.res;
	struct wed_tx_ring_ctrl *ring_ctrl = &wed->res_ctrl.tx_ctrl.ring_ctrl;
	unsigned int value = 0;
	unsigned int vld_num = 0;
	unsigned int reduce_grp = 0;
	unsigned int cnt = 0;
	/*PAUSE BUF MGMT*/
	WHNAT_IO_READ32(wed, WED_TX_BM_CTRL, &value);
	value |= (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	vld_num = (value & WED_TX_BM_CTRL_FLD_VLD_GRP_NUM_MASK);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_CTRL, value);
	/*POLL status bit*/
	WHNAT_IO_READ32(wed, WED_ST, &value);

	while ((value & (0xff << WED_ST_FLD_TX_ST)) && cnt < WED_POLL_MAX) {
		WHNAT_IO_READ32(wed, WED_ST, &value);
		cnt++;
	}

	if (cnt == WED_POLL_MAX) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): can't poll tx buf status to clear!\n", __func__);
		goto end;
	}

	/*check token can reduce or not*/
	if (reduce) {
		reduce_grp = (res->pkt_num/WED_TOKEN_UNIT) - vld_num;
		vld_num -= reduce_grp;

		if (vld_num > FREE_CR_SIZE)
			WHNAT_IO_READ32(wed, WED_TX_BM_VB_FREE_32_63, &value);
		else
			WHNAT_IO_READ32(wed, WED_TX_BM_VB_FREE_0_31, &value);

		vld_num = (vld_num-1)%FREE_CR_SIZE;

		if ((value & (1 << vld_num)) == 0) {
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): reduce too much packet buffer, buffer still inused!\n", __func__);
			goto end;
		}
	}

	/*update token*/
	value = (1 << WED_TX_BM_CTRL_FLD_PAUSE);
	value |= ((res->pkt_num/WED_TOKEN_UNIT) << WED_TX_BM_CTRL_FLD_VLD_GRP_NUM);
	value |= ((ring_ctrl->ring_len/256) << WED_TX_BM_CTRL_FLD_RSV_GRP_NUM);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_CTRL, value);
	WHNAT_DBG(WHNAT_DBG_INF, "%s(): update packet buffer done!\n", __func__);
end:
	/*Disable PAUSE BUF MGMT*/
	WHNAT_IO_READ32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_CTRL, value);
}
#endif /*WED_DYNAMIC_BM_SUPPORT*/

/*
*
*/
static int reset_wed_tx_dma(struct wed_entry *wed, unsigned int reset_type)
{
#ifdef WED_TX_SUPPORT
	unsigned int value;

	WHNAT_IO_READ32(wed, WED_GLO_CFG, &value);
	value &= ~(1 << WED_GLO_CFG_FLD_TX_DMA_EN);
	WHNAT_IO_WRITE32(wed, WED_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_GLO_CFG, WED_GLO_CFG_FLD_TX_DMA_BUSY) < 0)
		return -1;

	if (reset_type == WHNAT_RESET_IDX_ONLY) {
		value = (1 << WED_RST_IDX_FLD_DTX_IDX0);
		value |= (1 << WED_RST_IDX_FLD_DTX_IDX1);
		WHNAT_IO_WRITE32(wed, WED_RST_IDX, value);
		WHNAT_IO_WRITE32(wed, WED_RST_IDX, 0);
	} else {
		value = (1 << WED_MOD_RST_FLD_WED_TX_DMA);
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif /*WED_TX_SUPPORT*/
	return 0;
}

/*
*
*/
static int reset_wdma(struct wdma_entry *wdma)
{
	unsigned int value;
	/*Stop Frame Engin WDMA*/
	WHNAT_IO_READ32(wdma, WDMA_GLO_CFG, &value);
	value &= ~(1 << WDMA_GLO_CFG_RX_DMA_EN);
	WHNAT_IO_WRITE32(wdma, WDMA_GLO_CFG, value);

	if (wdma_agt_dis_ck(wdma, WDMA_GLO_CFG, WDMA_GLO_CFG_RX_DMA_BUSY) < 0)
		return -1;

	/*Reset Frame Engine WDMA DRX/CRX index*/
	value = 1 << WDMA_RST_IDX_RST_DRX_IDX0;
	value |= 1 << WDMA_RST_IDX_RST_DRX_IDX1;
	WHNAT_IO_WRITE32(wdma, WDMA_RST_IDX, value);
	WHNAT_IO_WRITE32(wdma, WDMA_RST_IDX, 0);
	WHNAT_IO_WRITE32(wdma, WDMA_RX_CRX_IDX_0, 0);
	WHNAT_IO_WRITE32(wdma, WDMA_RX_CRX_IDX_1, 0);
	return 0;
}

/*
*
*/
static int reset_wed_rx_drv(struct wed_entry *wed, unsigned int reset_type)
{
#ifdef WED_HW_TX_SUPPORT
	struct whnat_entry *whnat = (struct whnat_entry *)wed->whnat;
	struct wdma_entry *wdma = &whnat->wdma;
	unsigned int value;
	/*Stop Frame Engin WDMA*/
	WHNAT_IO_READ32(wdma, WDMA_GLO_CFG, &value);
	value &= ~(1 << WDMA_GLO_CFG_RX_DMA_EN);
	WHNAT_IO_WRITE32(wdma, WDMA_GLO_CFG, value);

	if (wdma_agt_dis_ck(wdma, WDMA_GLO_CFG, WDMA_GLO_CFG_RX_DMA_BUSY) < 0)
		return -1;

	/*Stop WED WDMA Rx Driver Engine*/
	WHNAT_IO_READ32(wed, WED_WDMA_GLO_CFG, &value);
	value |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_DISABLE_FSM_AUTO_IDLE);
	value &= ~(1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
#ifdef WED_WORK_AROUND_WDMA_RETURN_IDLE
	value |= (1 << WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE);
#endif /*WED_WORK_AROUND_WDMA_RETURN_IDLE*/
	WHNAT_IO_WRITE32(wed, WED_WDMA_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_WDMA_GLO_CFG, WED_WDMA_GLO_CFG_FLD_RX_DRV_BUSY) < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): rx drv can't return idle state!\n", __func__);
#ifdef WED_WORK_AROUND_WDMA_RETURN_IDLE
		value &= ~(1 << WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE);
		WHNAT_IO_WRITE32(wed, WED_WDMA_GLO_CFG, value);
#endif /*WED_WORK_AROUND_WDMA_RETURN_IDLE*/
		return -1;
	}

#ifdef WED_WORK_AROUND_WDMA_RETURN_IDLE
	value &= ~(1 << WED_WDMA_GLO_CFG_FLD_FSM_RETURN_IDLE);
	WHNAT_IO_WRITE32(wed, WED_WDMA_GLO_CFG, value);
#endif /*WED_WORK_AROUND_WDMA_RETURN_IDLE*/
	/*Reset Frame Engine WDMA DRX/CRX index*/
	value = 1 << WDMA_RST_IDX_RST_DRX_IDX0;
	value |= 1 << WDMA_RST_IDX_RST_DRX_IDX1;
	WHNAT_IO_WRITE32(wdma, WDMA_RST_IDX, value);
	WHNAT_IO_WRITE32(wdma, WDMA_RST_IDX, 0);
	WHNAT_IO_WRITE32(wdma, WDMA_RX_CRX_IDX_0, 0);
	WHNAT_IO_WRITE32(wdma, WDMA_RX_CRX_IDX_1, 0);

	/*Reset WED WDMA RX Driver Engin DRV/CRX index only*/
	if (reset_type == WHNAT_RESET_IDX_ONLY) {
		value = (1 << WED_WDMA_RST_IDX_FLD_CRX_IDX0);
		value |= (1 << WED_WDMA_RST_IDX_FLD_CRX_IDX1);
		value |= (1 << WED_WDMA_RST_IDX_FLD_DRV_IDX0);
		value |= (1 << WED_WDMA_RST_IDX_FLD_DRV_IDX1);
		WHNAT_IO_WRITE32(wed, WED_WDMA_RST_IDX, value);
		WHNAT_IO_WRITE32(wed, WED_WDMA_RST_IDX, 0);
		WHNAT_IO_READ32(wed, WED_WDMA_GLO_CFG, &value);
		value |= (1  << WED_WDMA_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG);
		WHNAT_IO_WRITE32(wed, WED_WDMA_GLO_CFG, value);
		value &= ~(1 << WED_WDMA_GLO_CFG_FLD_RST_INIT_COMPLETE_FLAG);
		WHNAT_IO_WRITE32(wed, WED_WDMA_GLO_CFG, value);
	} else {
		/*Reset WDMA Interrupt Agent*/
		value &= ~(1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
		WHNAT_IO_WRITE32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WDMA_INT_AGT_BUSY) < 0)
			return -1;

		value = 1 << WED_MOD_RST_FLD_WDMA_INT_AGT;
		WHNAT_RESET(wed, WED_MOD_RST, value);
		/*Reset WED RX Driver Engin*/
		value = 1 << WED_MOD_RST_FLD_WDMA_RX_DRV;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif /*WED_HW_TX_SUPPORT*/
	return 0;
}

/*
*
*/
static int reset_wed_tx_bm(struct wed_entry *wed)
{
#ifdef WED_HW_TX_SUPPORT
	unsigned int value;
	unsigned int cnt = 0;
	/*Tx Free Agent Reset*/
	WHNAT_IO_READ32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN);
	WHNAT_IO_WRITE32(wed, WED_CTRL, value);

	if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WED_TX_FREE_AGT_BUSY) < 0) {
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): tx free agent reset faild!\n", __func__);
		return -1;
	}

	WHNAT_IO_READ32(wed, WED_TX_BM_INTF, &value);

	while (((value >> 16) & 0xffff) != 0x40 && cnt < WED_POLL_MAX) {
		WHNAT_IO_READ32(wed, WED_TX_BM_INTF, &value);
		cnt++;
	}

	if (cnt >= WED_POLL_MAX) {
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): tx free agent fifo reset faild!\n", __func__);
		return -1;
	}

	value = 1 << WED_MOD_RST_FLD_TX_FREE_AGT;
	WHNAT_RESET(wed, WED_MOD_RST, value);
	/*Reset TX Buffer manager*/
	WHNAT_IO_READ32(wed, WED_CTRL, &value);
	value &= ~(1 << WED_CTRL_FLD_WED_TX_BM_EN);
	WHNAT_IO_WRITE32(wed, WED_CTRL, value);

	if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WED_TX_BM_BUSY) < 0) {
		WHNAT_DBG(WHNAT_DBG_OFF, "%s(): tx bm reset faild!\n", __func__);
		return -1;
	}

	value = 1 << WED_MOD_RST_FLD_TX_BM;
	WHNAT_RESET(wed, WED_MOD_RST, value);
#endif /*WED_HW_TX_SUPPORT*/
	return 0;
}

/*
*
*/
static int reset_wed_tx_drv(struct wed_entry *wed, unsigned int reset_type)
{
#ifdef WED_TX_SUPPORT
	unsigned int value;
	/*Disable TX driver*/
	WHNAT_IO_READ32(wed, WED_WPDMA_GLO_CFG, &value);
	value &= ~(1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN);
	WHNAT_IO_WRITE32(wed, WED_WPDMA_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_WPDMA_GLO_CFG, WED_WPDMA_GLO_CFG_FLD_TX_DRV_BUSY) < 0)
		return -1;

	if (reset_type == WHNAT_RESET_IDX_ONLY) {
		/*Reset TX Ring only*/
		value = (1 << WED_WPDMA_RST_IDX_FLD_CTX_IDX0);
		value |= (1 << WED_WPDMA_RST_IDX_FLD_CTX_IDX1);
		WHNAT_IO_WRITE32(wed, WED_WPDMA_RST_IDX, value);
		WHNAT_IO_WRITE32(wed, WED_WPDMA_RST_IDX, 0);
	} else {
		/*Reset TX Interrupt agent*/
		WHNAT_IO_READ32(wed, WED_CTRL, &value);
		value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		WHNAT_IO_WRITE32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WPDMA_INT_AGT_BUSY) < 0)
			return -1;

		value = 1 << WED_MOD_RST_FLD_WPDMA_INT_AGT;
		WHNAT_RESET(wed, WED_MOD_RST, value);
		/*Reset Tx driver*/
		value = 1 << WED_MOD_RST_FLD_WPDMA_TX_DRV;
		WHNAT_RESET(wed, WED_MOD_RST, value);
	}

#endif /*WED_TX_SUPPORT*/
	return 0;
}

/*
*
*/
static int reset_tx_traffic(struct wed_entry *wed, unsigned int reset_type)
{
	int ret = 0;

	ret = reset_wed_tx_dma(wed, reset_type);

	if (ret < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): wed_tx_dma reset fail,ret=%d\n", __func__, ret);
		return ret;
	}

	ret = reset_wed_rx_drv(wed, reset_type);

	if (ret < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): wed_tx_drv reset fail,ret=%d\n", __func__, ret);
		return ret;
	}

	ret = reset_wed_tx_bm(wed);

	if (ret < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): wed_tx_bm reset fail,ret=%d\n", __func__, ret);
		return ret;
	}

	ret = reset_wed_tx_drv(wed, reset_type);

	if (ret < 0) {
		WHNAT_DBG(WHNAT_DBG_ERR, "%s(): wed_tx_drv reset fail,ret=%d\n", __func__, ret);
		return ret;
	}

	return ret;
}

/*
*
*/
static int reset_rx_traffic(struct wed_entry *wed, unsigned int reset_type)
{
#ifdef WED_RX_SUPPORT
	unsigned int value;
	/*disable WPDMA RX Driver Engine*/
	WHNAT_IO_READ32(wed, WED_WPDMA_GLO_CFG, &value);
	value &= ~(1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN);
	WHNAT_IO_WRITE32(wed, WED_WPDMA_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_WPDMA_GLO_CFG, WED_WPDMA_GLO_CFG_FLD_RX_DRV_BUSY) < 0)
		return -1;

	WHNAT_IO_READ32(wed, WED_GLO_CFG, &value);
	value &= ~(1 << WED_GLO_CFG_FLD_RX_DMA_EN);
	WHNAT_IO_WRITE32(wed, WED_GLO_CFG, value);

	if (wed_agt_dis_ck(wed, WED_GLO_CFG, WED_GLO_CFG_FLD_RX_DMA_BUSY) < 0)
		return -1;

	if (reset_type == WHNAT_RESET_IDX_ONLY) {
		value = (1 << WED_WPDMA_RST_IDX_FLD_CRX_IDX1);
		WHNAT_IO_WRITE32(wed, WED_WPDMA_RST_IDX, value);
		value = (1 << WED_RST_IDX_FLD_DRX_IDX1);
		WHNAT_IO_WRITE32(wed, WED_RST_IDX, value);
		WHNAT_IO_WRITE32(wed, WED_WPDMA_RST_IDX, 0);
		WHNAT_IO_WRITE32(wed, WED_RST_IDX, 0);
		WHNAT_IO_WRITE32(wed, WED_RX1_CTRL2, 0);
	} else {
		/*WPDMA  interrupt agent*/
		WHNAT_IO_READ32(wed, WED_CTRL, &value);
		value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
		WHNAT_IO_WRITE32(wed, WED_CTRL, value);

		if (wed_agt_dis_ck(wed, WED_CTRL, WED_CTRL_FLD_WPDMA_INT_AGT_BUSY) < 0)
			return -1;

		value = (1 << WED_MOD_RST_FLD_WPDMA_INT_AGT);
		WHNAT_RESET(wed, WED_MOD_RST, value);
		/*WPDMA RX Driver Engin*/
		value = (1 << WED_MOD_RST_FLD_WPDMA_RX_DRV);
		WHNAT_RESET(wed, WED_MOD_RST, value);
		WHNAT_IO_WRITE32(wed, WED_RX1_CTRL2, 0);
	}

#endif /*WED_RX_SUPPORT*/
	return 0;
}

/*
*
*/
static void reset_all(struct whnat_entry *whnat)
{
	struct wed_entry *wed = &whnat->wed;
	struct wdma_entry *wdma = &whnat->wdma;
	unsigned int value;
	/*Reset WDMA*/
	reset_wdma(wdma);
	/*Reset WED*/
	value = 1 << WED_MOD_RST_FLD_WED;
	WHNAT_RESET(wed, WED_MOD_RST, value);
}

/*
*
*/
static void restore_tx_traffic(struct wed_entry *wed)
{
	unsigned int value;
	/*WPDMA*/
#ifdef WED_TX_SUPPORT
	/*Enable TX Driver*/
	WHNAT_IO_READ32(wed, WED_WPDMA_GLO_CFG, &value);
	value |= (1 << WED_WPDMA_GLO_CFG_FLD_TX_DRV_EN);
	WHNAT_IO_WRITE32(wed, WED_WPDMA_GLO_CFG, value);
#endif /*WED_TX_SUPPORT*/
	/*Enable TX interrupt agent*/
	WHNAT_IO_READ32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
	WHNAT_IO_WRITE32(wed, WED_CTRL, value);
#ifdef WED_HW_TX_SUPPORT
	/*TX BM*/
	/*Enable TX buffer mgnt*/
	WHNAT_IO_READ32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_BM_EN);
	WHNAT_IO_WRITE32(wed, WED_CTRL, value);
	WHNAT_IO_READ32(wed, WED_TX_BM_CTRL, &value);
	value &= ~(1 << WED_TX_BM_CTRL_FLD_PAUSE);
	WHNAT_IO_WRITE32(wed, WED_TX_BM_CTRL, value);
	/*Enable TX free agent*/
	WHNAT_IO_READ32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WED_TX_FREE_AGT_EN);
	WHNAT_IO_WRITE32(wed, WED_CTRL, value);
	/*WDMA*/
	{
		struct whnat_entry *whnat = (struct whnat_entry *)wed->whnat;
		struct wdma_entry *wdma = (struct wdma_entry *)&whnat->wdma;
		/*Enable WDMA RX Driver*/
		WHNAT_IO_READ32(wed, WED_WDMA_GLO_CFG, &value);
		value |= (1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_EN);
		WHNAT_IO_WRITE32(wed, WED_WDMA_GLO_CFG, value);
		/*Enable WDMA RX DMA*/
		WHNAT_IO_READ32(wdma, WDMA_GLO_CFG, &value);
		value |= (1 << WDMA_GLO_CFG_RX_DMA_EN);
		WHNAT_IO_WRITE32(wdma, WDMA_GLO_CFG, value);
		/*Enable WDMA interrupt agent*/
		WHNAT_IO_READ32(wed, WED_CTRL, &value);
		value |= (1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
		WHNAT_IO_WRITE32(wed, WED_CTRL, value);
		/*Enable WED WDMA RX Engine*/
		WHNAT_IO_READ32(wed, WED_GLO_CFG, &value);
		value |= (1 << WED_GLO_CFG_FLD_TX_DMA_EN);
		WHNAT_IO_WRITE32(wed, WED_GLO_CFG, value);
	}
#endif /*WED_HW_TX_SUPPORT*/
}

/*
*
*/
static void restore_rx_traffic(struct wed_entry *wed)
{
#ifdef WED_RX_SUPPORT
	unsigned int value;
	/*Enable WED RX DMA*/
	WHNAT_IO_READ32(wed, WED_GLO_CFG, &value);
	value |= (1 << WED_GLO_CFG_FLD_RX_DMA_EN);
	WHNAT_IO_WRITE32(wed, WED_GLO_CFG, value);
	/*Enable WPDMA Rx Driver*/
	WHNAT_IO_READ32(wed, WED_WPDMA_GLO_CFG, &value);
	value |= (1 << WED_WPDMA_GLO_CFG_FLD_RX_DRV_EN);
	WHNAT_IO_WRITE32(wed, WED_WPDMA_GLO_CFG, value);
	/*Enable WPDAM RX interrupt agent*/
	WHNAT_IO_READ32(wed, WED_CTRL, &value);
	value |= (1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
	WHNAT_IO_WRITE32(wed, WED_CTRL, value);
#endif /*WED_RX_SUPPORT*/
}

/*
*
*/
static void wed_ctr_intr_set(struct wed_entry *wed, unsigned char enable)
{
	unsigned int value = 0;

	WHNAT_IO_READ32(wed, WED_CTRL, &value);

	/*whnat extra interrupt*/
	if (enable) {
		/*whnat interrupt agent*/
		value |= (1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
#ifdef WED_HW_TX_SUPPORT
		value |= (1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
#endif /*WED_HW_TX_SUPPORT*/
	} else {
		value &= ~(1 << WED_CTRL_FLD_WPDMA_INT_AGT_EN);
#ifdef WED_HW_TX_SUPPORT
		value &= ~(1 << WED_CTRL_FLD_WDMA_INT_AGT_EN);
#endif /*WED_HW_TX_SUPPORT*/
	}

	WHNAT_IO_WRITE32(wed, WED_CTRL, value);
#ifdef WED_DELAY_INT_SUPPORT
	WHNAT_IO_WRITE32(wed, WED_DLY_INT_CFG, WED_DLY_INT_VALUE);
#endif /*WED_DELAY_INT_SUPPORT*/
}

/*
*
*/
unsigned int whnat_wifi_cr_get(char type, unsigned int cr)
{
	unsigned int i = 0;

	while (cr_map_list[i].wifi_cr != 0) {
		if ((cr_map_list[i].whnat_cr == cr) && (cr_map_list[i].whnat_type == type))
			return cr_map_list[i].wifi_cr;

		i++;
	}

	WHNAT_DBG(WHNAT_DBG_ERR, "%s(): can't get wifi cr from whnat cr %x, type=%d\n", __func__, cr, type);
	return 0;
}

/*
*
*/
unsigned int whnat_cr_search(unsigned int cr)
{
	unsigned int i = 0;

	while (cr_map_list[i].wifi_cr != 0) {
		if (cr_map_list[i].wifi_cr == cr)
			return cr_map_list[i].whnat_cr;

		i++;
	}

	WHNAT_DBG(WHNAT_DBG_LOU, "%s(): can't get wifi cr from whnat cr %x\n", __func__, cr);
	return 0;
}

/*
* assign tx descripton to hw cr
*/
void whnat_hal_cr_handle(
	struct whnat_entry *entry,
	char type,
	char is_write,
	unsigned long addr,
	unsigned int *cr_value)
{
	struct wed_entry *wed = &entry->wed;
	struct wdma_entry *wdma = &entry->wdma;
	struct wifi_entry *wifi = &entry->wifi;

	switch (type) {
	case WHNAT_CR_WED:
		if (is_write)
			WHNAT_IO_WRITE32(wed, addr, *cr_value);
		else
			WHNAT_IO_READ32(wed, addr, cr_value);

		break;

	case WHNAT_CR_WDMA:
		if (is_write)
			WHNAT_IO_WRITE32(wdma, addr, *cr_value);
		else
			WHNAT_IO_READ32(wdma, addr, cr_value);

		break;

	case WHNAT_CR_WIFI:
		if (is_write)
			WHNAT_IO_WRITE32(wifi, addr, *cr_value);
		else
			WHNAT_IO_READ32(wifi, addr, cr_value);

		break;

	default:
		break;
	}
}

/*
*
*/
void whnat_hal_cr_mirror_set(struct whnat_entry *whnat, unsigned char enable)
{
	struct whnat_ctrl *ctrl = whnat_ctrl_get();
	struct whnat_hif_cfg *hif = &ctrl->hif_cfg;
	struct wifi_entry *wifi = &whnat->wifi;
	unsigned int value;
	unsigned int cr;

	cr = (whnat->idx) ? PCIE1_MAP : PCIE0_MAP;
	/*Disable CR mirror mode*/
	WHNAT_IO_READ32(hif, cr, &value);
	value &= ~(0xfffff << PCIE_MAP_FLD_PCIE_ADDR_REMAP);
	value |= ((wifi->wpdma_base & 0xfffff000));

	if (enable)
		value |= (1 << PCIE_MAP_FLD_PCIE_REMAP_EN);
	else
		value &= ~(1 << PCIE_MAP_FLD_PCIE_REMAP_EN);

	WHNAT_IO_WRITE32(hif, cr, value);
}

/*
* interrupt control
*/
void whnat_hal_int_ctrl(struct whnat_entry *entry, unsigned char enable)
{
	struct wed_entry *wed = &entry->wed;
	struct wifi_entry *wifi = &entry->wifi;
	unsigned int value = 0;
	/*wed control cr set*/
	wed_ctr_intr_set(wed, enable);
	/*pcie interrupt status trigger register*/
	WHNAT_IO_WRITE32(wed, WED_PCIE_INTS_TRIG, PCIE_INT_STA_OFFSET);
	/*WPDMA interrupt triger*/
	value = 0;

	if (enable) {
#ifdef WED_RX_SUPPORT
		value |= (1 << WED_WPDMA_INT_TRIG_FLD_RX_DONE_INT1);
#endif /*WED_RX_SUPPORT*/
#ifdef WED_TX_SUPPORT
		value |= (1 << WED_WPDMA_INT_TRIG_FLD_TX_DONE_INT0);
		value |= (1 << WED_WPDMA_INT_TRIG_FLD_TX_DONE_INT1);
#endif /*WED_TX_SUPPORT*/
	}

	WHNAT_IO_WRITE32(wed, WED_WPDMA_INT_TRIG, value);
	WHNAT_IO_READ32(wed, WED_WPDMA_INT_CTRL, &value);
	value |= (1 << WED_WPDMA_INT_CTRL_FLD_SUBRT_ADV);
	/*disable due to interrupr lost issue should not happen in ASIC*/
	/*value |= (1 << WED_WPDMA_INT_CTRL_FLD_MSK_ON_DLY);*/
	WHNAT_IO_WRITE32(wed, WED_WPDMA_INT_CTRL, value);
#ifdef WED_HW_TX_SUPPORT
	{
		struct wdma_entry *wdma = &entry->wdma;
		/*WED_WDMA Interrupt agent */
		value = 0;

		if (enable) {
			value |= (1 << WED_WDMA_INT_TRIG_FLD_RX_DONE0);
			value |= (1 << WED_WDMA_INT_TRIG_FLD_RX_DONE1);
		}

		WHNAT_IO_WRITE32(wed, WED_WDMA_INT_TRIG, value);
		/*WED_WDMA_SRC SEL */
		WHNAT_IO_READ32(wed, WED_WDMA_INT_CTRL, &value);
		/*setting for wdma_int0->wdma0, wdma_int1->wdma1*/
		value &= ~(WED_WDMA_INT_CTRL_FLD_POLL_SRC_SEL_MASK << WED_WDMA_INT_CTRL_FLD_POLL_SRC_SEL);
		WHNAT_IO_WRITE32(wed, WED_WDMA_INT_CTRL, value);
		/*WDMA interrupt enable*/
		value = 0;

		if (enable) {
			value |= (1 << WDMA_INT_MSK_RX_DONE_INT1);
			value |= (1 << WDMA_INT_MSK_RX_DONE_INT0);
		}

		WHNAT_IO_WRITE32(wdma, WDMA_INT_MSK, value);
		WHNAT_IO_WRITE32(wdma, WDMA_INT_GRP2, value);
	}
#endif /*WED_HW_TX_SUPPORT*/

	if (enable)
		WHNAT_IO_WRITE32(wifi, WIFI_INT_MSK, *wifi->int_mask);
	else
		WHNAT_IO_WRITE32(wifi, WIFI_INT_MSK, 0);
}

/*
*
*/
void whnat_hal_eint_init(struct whnat_entry *entry)
{
	struct wed_entry *wed = &entry->wed;
	unsigned int value = 0;
#ifdef WED_DYNAMIC_BM_SUPPORT
	value |= (1 << WED_EX_INT_STA_FLD_TX_FBUF_HTH);
	value |= (1 << WED_EX_INT_STA_FLD_TX_FBUF_LTH);
#endif /*WED_DYNAMIC_BM_SUPPORT*/
	value |= (1 << WED_EX_INT_STA_FLD_TF_LEN_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TF_TKID_WO_PYLD);
#ifdef WED_DYNAMIC_RX_BM_SUPPORT
	value |= (1 << WED_EX_INT_STA_FLD_RX_FBUF_HTH);
	value |= (1 << WED_EX_INT_STA_FLD_RX_FBUF_LTH);
#endif /*WED_DYNAMIC_RX_BM_SUPPORT*/
	value |= (1 << WED_EX_INT_STA_FLD_TX_DMA_W_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TX_DMA_R_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_INTI_WDMA_ENABLE);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_COHERENT);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_W_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_R_RESP_ERR);
	value |= (1 << WED_EX_INT_STA_FLD_TF_TKID_TITO_INVLD);
#ifdef WED_WDMA_RECYCLE
	value |= (1 << WED_EX_INT_STA_FLD_RX_DRV_DMA_RECYCLE);
#endif /*WED_WDMA_RECYCLE*/
	wed->ext_int_mask = value;
}

/*
*
*/
void whnat_hal_eint_ctrl(struct whnat_entry *entry, unsigned char enable)
{
	struct wed_entry *wed = &entry->wed;
	unsigned int value = 0;

	if (enable)
		value = wed->ext_int_mask;

	WHNAT_IO_WRITE32(wed, WED_EX_INT_MSK, value);
	WHNAT_IO_READ32(wed, WED_EX_INT_MSK, &value);
}

/*
*
*/
void whnat_hal_eint_stat_get(struct whnat_entry *entry, unsigned int *state)
{
	struct wed_entry *wed = &entry->wed;
	/*read stat*/
	WHNAT_IO_READ32(wed, WED_EX_INT_STA, state);
	/*write 1 clear*/
	WHNAT_IO_WRITE32(wed, WED_EX_INT_STA, *state);
}

/*
*
*/
void whnat_hal_dma_ctrl(struct whnat_entry *entry, unsigned char txrx)
{
	struct wed_entry *wed = &entry->wed;

	wed_dma_ctrl(wed, txrx);
}

/*
*
*/
void whnat_hal_wed_init(struct whnat_entry *entry)
{
	struct wed_entry *wed = &entry->wed;
	unsigned int wed_wdma_cfg;
#ifdef WED_WDMA_RECYCLE
	struct wdma_entry *wdma = &entry->wdma;
	unsigned int value;
	/*set wdma recycle threshold*/
	value = (WED_WDMA_RECYCLE_TIME & 0xffff) << WED_WDMA_RX_THRES_CFG_FLD_WAIT_BM_CNT_MAX;
	value |= (((wdma->res_ctrl.rx_ctrl.rx_ring_ctrl.ring_len-3) & 0xfff) << WED_WDMA_RX_THRES_CFG_FLD_DRX_CRX_DISTANCE_THRES);
	WHNAT_IO_WRITE32(wed, WED_WDMA_RX0_THRES_CFG, value);
	WHNAT_IO_WRITE32(wed, WED_WDMA_RX1_THRES_CFG, value);
#endif /*WED_WDMA_RECYCLE*/
	/*cfg wdma recycle*/
	WHNAT_IO_READ32(wed, WED_WDMA_GLO_CFG, &wed_wdma_cfg);
	wed_wdma_cfg &= ~(WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE_MASK << WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE);
	wed_wdma_cfg &= ~((1 << WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN) |
					  (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE) |
					  (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_DMAD_RECYCLE));
	/*disable auto idle*/
	wed_wdma_cfg &= ~(1 << WED_WDMA_GLO_CFG_FLD_RX_DRV_DISABLE_FSM_AUTO_IDLE);
	/*Set to 16 DWORD for 64bytes*/
	wed_wdma_cfg |= (0x2 << WED_WDMA_GLO_CFG_FLD_WDMA_BT_SIZE);
	/*enable skip state for fix dma busy issue*/
	wed_wdma_cfg |= ((1 << WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN) |
					 (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE));
#ifdef WED_WDMA_RECYCLE
	wed_wdma_cfg |= (1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_DMAD_RECYCLE) |
					(1 << WED_WDMA_GLO_CFG_FLD_DYNAMIC_SKIP_DMAD_PREPARE) |
					(1 << WED_WDMA_GLO_CFG_FLD_IDLE_STATE_DMAD_SUPPLY_EN);
#endif /*WED_WDMA_RECYCLE*/
#ifdef WED_HW_TX_SUPPORT
	WHNAT_IO_WRITE32(wed, WED_WDMA_GLO_CFG, wed_wdma_cfg);
#endif /*WED_HW_TX_SUPPORT*/
}

/*
*
*/
int whnat_hal_io_read(
	void *cookie,
	unsigned int addr,
	unsigned int *value)
{
	struct whnat_entry *entry = whnat_entry_search(cookie);
	struct wed_entry *wed;
	unsigned int whnat_cr = 0;

	if (!entry)
		return -1;

	wed = &entry->wed;
	whnat_cr = whnat_cr_search(addr);

	if (whnat_cr == 0)
		return -1;

	WHNAT_IO_READ32(wed, whnat_cr, value);
	WHNAT_DBG(WHNAT_DBG_LOU, "%s(): Read addr (%x)=%x\n",
			__func__,
			addr,
			*value);
	return 0;
}

/*
*
*/
int whnat_hal_io_write(
	void *cookie,
	unsigned int addr,
	unsigned int value)
{
	struct whnat_entry *entry = whnat_entry_search(cookie);
	struct wed_entry *wed;
	unsigned int whnat_cr = 0;

	if (!entry)
		return -1;

	wed = &entry->wed;
	whnat_cr = whnat_cr_search(addr);

	if (whnat_cr == 0)
		return -1;

#ifdef WED_DELAY_INT_SUPPORT

	if (whnat_cr == WED_INT_MSK) {
		value &= ~((1 << WED_INT_MSK_FLD_TX_DONE_INT0) | (1 << WED_INT_MSK_FLD_TX_DONE_INT1));
		value &= ~(1 << WED_INT_MSK_FLD_RX_DONE_INT1);
		value |= (1 << WED_INT_MSK_FLD_TX_DLY_INT);
		value |= (1 << WED_INT_MSK_FLD_RX_DLY_INT);
	}

#endif /*WED_DELAY_INT_SUPPORT*/
	WHNAT_IO_WRITE32(wed, whnat_cr, value);
	WHNAT_DBG(WHNAT_DBG_LOU, "%s(): Write addr (%x)=%x\n",
			__func__,
			addr,
			value);
	return 0;
}

/*
*
*/
void whnat_hal_wdma_init(struct whnat_entry *whnat)
{
	unsigned int value = 0;
	struct wed_entry *wed = &whnat->wed;
	/*Apply WDMA  related setting*/
	wdma_dma_ctrl(&whnat->wdma, WHNAT_DMA_TXRX);
	/*offset 0*/
	value = (whnat->idx) ? WDMA1_OFST0 : WDMA0_OFST0;
	WHNAT_IO_WRITE32(wed, WED_WDMA_OFST0, value);
	/*offset 1*/
	value = (whnat->idx) ? WDMA1_OFST1 : WDMA0_OFST1;
	WHNAT_IO_WRITE32(wed, WED_WDMA_OFST1, value);
}

/*
*
*/
void whnat_hal_wdma_ring_init(struct whnat_entry *entry)
{
#ifdef WED_HW_TX_SUPPORT
	struct wdma_entry *wdma = &entry->wdma;
	struct wed_entry *wed = &entry->wed;
	struct wdma_rx_ring_ctrl *ring_ctrl = &wdma->res_ctrl.rx_ctrl.rx_ring_ctrl;
	struct whnat_ring *ring;
	unsigned int offset;
	int i;

	/*set PDMA & WED_WPDMA Ring, wifi driver will configure WDMA ring by whnat_hal_tx_ring_ctrl */
	/*tx ring*/
	for (i = 0; i < ring_ctrl->ring_num; i++) {
		offset = i*WIFI_RING_OFFSET;
		ring = &ring_ctrl->ring[i];
		WHNAT_DBG(WHNAT_DBG_INF, "%s(): configure ring %d setting\n", __func__, i);
		WHNAT_DBG(WHNAT_DBG_INF, "%s(): wed:%p,wdma:%p: %x=%lx,%x=%d,%x=%d\n", __func__,
				wed, wdma,
				ring->hw_desc_base, (unsigned long)ring->cell[0].pkt_pa,
				ring->hw_cnt_addr, ring_ctrl->ring_len,
				ring->hw_cidx_addr, 0);
		/*PDMA*/
		WHNAT_IO_WRITE32(wdma, ring->hw_desc_base, ring->cell[0].alloc_pa);
		WHNAT_IO_WRITE32(wdma, ring->hw_cnt_addr, ring_ctrl->ring_len);
		WHNAT_IO_WRITE32(wdma, ring->hw_cidx_addr, 0);
		/*WED_WPDMA*/
		WHNAT_IO_WRITE32(wed, WED_WDMA_RX0_BASE+offset, ring->cell[0].alloc_pa);
		WHNAT_IO_WRITE32(wed, WED_WDMA_RX0_CNT+offset, ring_ctrl->ring_len);
	}

#endif /*WED_HW_TX_SUPPORT*/
}

/*
*
*/
void whnat_hal_wpdma_ring_init(struct whnat_entry *entry)
{
#ifdef WED_TX_SUPPORT
	struct wifi_entry *wifi = &entry->wifi;
	struct wed_entry *wed = &entry->wed;
	struct wed_tx_ring_ctrl *ring_ctrl = &wed->res_ctrl.tx_ctrl.ring_ctrl;
	struct whnat_ring *ring;
	unsigned int offset = (WIFI_TX_RING0_BASE-WED_WPDMA_TX0_CTRL0);
	int i;

	/*set PDMA & WED_WPDMA Ring, wifi driver will configure WDMA ring by whnat_hal_tx_ring_ctrl */
	for (i = 0; i < ring_ctrl->ring_num; i++) {
		ring = &ring_ctrl->ring[i];
		WHNAT_DBG(WHNAT_DBG_INF, "%s(): configure ring %d setting\n", __func__, i);
		WHNAT_DBG(WHNAT_DBG_INF, "%s(): wed:%p wifi:%p: %x=%lx,%x=%d,%x=%d\n", __func__,
				wed, wifi,
				ring->hw_desc_base, (unsigned long)ring->cell[0].alloc_pa,
				ring->hw_cnt_addr, ring_ctrl->ring_len,
				ring->hw_cidx_addr, 0);
		/*WPDMA*/
		WHNAT_IO_WRITE32(wifi, ring->hw_desc_base, ring->cell[0].alloc_pa);
		WHNAT_IO_WRITE32(wifi, ring->hw_cnt_addr, ring_ctrl->ring_len);
		WHNAT_IO_WRITE32(wifi, ring->hw_cidx_addr, 0);
		/*WED_WPDMA*/
		WHNAT_IO_WRITE32(wed, (ring->hw_desc_base-offset), ring->cell[0].alloc_pa);
		WHNAT_IO_WRITE32(wed, (ring->hw_cnt_addr-offset), ring_ctrl->ring_len);
		WHNAT_IO_WRITE32(wed, (ring->hw_cidx_addr-offset), 0);
	}

#endif /*WED_TX_SUPPORT*/
#ifdef WED_RX_SUPPORT
	{
		unsigned int value;
		struct wed_entry *wed = &entry->wed;
		struct wifi_entry *wifi = &entry->wifi;
		/*Rx Ring base */
		WHNAT_IO_READ32(wed, WED_RX1_CTRL0, &value);
		WHNAT_IO_WRITE32(wed, WED_WPDMA_RX1_CTRL0, value);
		WHNAT_IO_WRITE32(wifi, WIFI_RX_RING1_BASE, value);
		/*Rx CNT*/
		WHNAT_IO_READ32(wed, WED_RX1_CTRL1, &value);
		WHNAT_IO_WRITE32(wed, WED_WPDMA_RX1_CTRL1, value);
		WHNAT_IO_WRITE32(wifi, WIFI_RX_RING1_CNT, value);
		/*cpu idx*/
		WHNAT_IO_READ32(wed, WED_RX1_CTRL2, &value);
		WHNAT_IO_WRITE32(wed, WED_WPDMA_RX1_CTRL2, value);
		WHNAT_IO_WRITE32(wifi, WIFI_RX_RING1_CIDX, value);
	}
#endif /*WED_RX_SUPPORT*/
#ifdef WED_HW_TX_SUPPORT
	/*initial buf mgmt setting*/
	whnat_hal_bfm_init(entry);
#endif /*WED_HW_TX_SUPPORT*/
}

/*
*
*/
void whnat_hal_hif_init(struct whnat_hif_cfg *hif)
{
	unsigned int v1 = 0;
	unsigned int v2 = 0;
	/*debug only*/
	WHNAT_IO_READ32(hif, WED0_MAP, &v1);
	WHNAT_IO_READ32(hif, WED1_MAP, &v2);
	WHNAT_DBG(WHNAT_DBG_INF, "%s(): WED0_MAP: %x,WED1_MAP:%x\n", __func__, v1, v2);
	/*default remap pcie0 to wed0*/
	v1 = hif->wpdma_base[0] & 0xfffff000;
	v1 &=  ~(1 << PCIE_MAP_FLD_PCIE_REMAP_EN);
	v1 &=  ~(1 << PCIE_MAP_FLD_PCIE_REMAP);
	/*default remap pcie1 to wed1*/
	v2 = hif->wpdma_base[1] & 0xfffff000;
	v2 &=  ~(1 << PCIE_MAP_FLD_PCIE_REMAP_EN);
	v2 |= (1 << PCIE_MAP_FLD_PCIE_REMAP);
	WHNAT_DBG(WHNAT_DBG_INF, "%s(): PCIE0_MAP: %x,PCIE1_MAP:%x\n", __func__, v1, v2);
	WHNAT_IO_WRITE32(hif, PCIE0_MAP, v1);
	WHNAT_IO_WRITE32(hif, PCIE1_MAP, v2);
}

/*
*
*/
void whnat_hal_trace_set(struct whnat_cputracer *tracer)
{
	unsigned int value = 0;
	/*enable cpu tracer*/
	WHNAT_IO_WRITE32(tracer, CPU_TRACER_WP_ADDR, tracer->trace_addr);
	WHNAT_IO_WRITE32(tracer, CPU_TRACER_WP_ADDR, tracer->trace_mask);

	if (tracer->trace_en) {
		value = (1 << CPU_TRACER_CON_BUS_DBG_EN) |
				(1 << CPU_TRACER_CON_WP_EN)		 |
				(1 << CPU_TRACER_CON_IRQ_WP_EN);
	}

	WHNAT_IO_WRITE32(tracer, CPU_TRACER_CFG, value);
}

/*
*
*/
int whnat_hal_hw_reset(struct whnat_entry *whnat, unsigned int reset_type)
{
	struct wed_entry *wed = &whnat->wed;
	int ret = 0;

	switch (reset_type) {
	case WHNAT_RESET_IDX_ONLY:
	case WHNAT_RESET_IDX_MODULE:
		ret = reset_tx_traffic(wed, reset_type);

		if (ret < 0) {
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): wed_tx reset fail,ret=%d\n", __func__, ret);
			return ret;
		}

		ret = reset_rx_traffic(wed, reset_type);

		if (ret < 0) {
			WHNAT_DBG(WHNAT_DBG_ERR, "%s(): wed_rx reset fail,ret=%d\n", __func__, ret);
			return ret;
		}

		break;

	case WHNAT_RESET_ALL:
		reset_all(whnat);
		break;
	}

	return ret;
}

/*
*
*/
void whnat_hal_hw_restore(struct whnat_entry *whnat)
{
	struct wed_entry *wed = &whnat->wed;

	restore_rx_traffic(wed);
	restore_tx_traffic(wed);
}

/*
*
*/
void whnat_hal_pcie_map(struct whnat_entry *whnat)
{
	struct whnat_ctrl *ctrl = whnat_ctrl_get();
	struct wed_entry *wed = &whnat->wed;
	struct wifi_entry *wifi = &whnat->wifi;
	unsigned int value = ctrl->hif_cfg.pcie_base[whnat->idx];

	WHNAT_IO_WRITE32(wed, WED_PCIE_CFG_BASE, value);
	/*pcie interrupt agent control*/
#ifdef WED_WORK_AROUND_INT_POLL
	value = (PCIE_POLL_MODE_ALWAYS << WED_PCIE_INT_CTRL_FLD_POLL_EN);
	WHNAT_IO_WRITE32(wed, WED_PCIE_INT_CTRL, value);
#endif /*WED_WORK_AROUND_INT_POLL*/
	value = wifi->wpdma_base;
	WHNAT_IO_WRITE32(wed, WED_WPDMA_CFG_BASE, value);
}

/*
*
*/
void whnat_hal_pdma_mask_set(struct whnat_entry *whnat)
{
	struct wed_entry *wed = &whnat->wed;

	WHNAT_IO_WRITE32(wed, WED_WPDMA_INT_MSK, WIFI_IMR_VAL);
}

#ifdef ERR_RECOVERY
/*
*
*/
void whnat_hal_ser_trigger(struct whnat_entry *whnat)
{
	struct wifi_entry *wifi = &whnat->wifi;
	WHNAT_IO_WRITE32(wifi, WIFI_MCU_INT_EVENT, WIFI_TRIGGER_SER);
}

/*
*
*/
void whnat_hal_ser_update(struct wed_entry *wed, struct wed_ser_state *state)
{
	/*WED_TX_DMA*/
	WHNAT_IO_READ32(wed, WED_ST, &state->tx_dma_stat);
	state->tx_dma_stat = (state->tx_dma_stat >> WED_ST_FLD_TX_ST) & 0xff;
	WHNAT_IO_READ32(wed, WED_TX0_MIB, &state->tx0_mib);
	WHNAT_IO_READ32(wed, WED_TX1_MIB, &state->tx1_mib);
	WHNAT_IO_READ32(wed, WED_TX0_CTRL2, &state->tx0_cidx);
	WHNAT_IO_READ32(wed, WED_TX1_CTRL2, &state->tx1_cidx);
	WHNAT_IO_READ32(wed, WED_TX0_CTRL3, &state->tx0_didx);
	WHNAT_IO_READ32(wed, WED_TX1_CTRL3, &state->tx1_didx);
	/*WED_WDMA*/
	WHNAT_IO_READ32(wed, WED_WDMA_ST, &state->wdma_stat);
	state->wdma_stat = state->wdma_stat & 0xff;
	WHNAT_IO_READ32(wed, WED_WDMA_RX0_MIB, &state->wdma_rx0_mib);
	WHNAT_IO_READ32(wed, WED_WDMA_RX1_MIB, &state->wdma_rx1_mib);
	WHNAT_IO_READ32(wed, WED_WDMA_RX0_RECYCLE_MIB, &state->wdma_rx0_recycle_mib);
	WHNAT_IO_READ32(wed, WED_WDMA_RX1_RECYCLE_MIB, &state->wdma_rx1_recycle_mib);
	/*WED_WPDMA*/
	WHNAT_IO_READ32(wed, WED_WPDMA_ST, &state->wpdma_stat);
	state->wpdma_stat = (state->wpdma_stat >> WED_WPDMA_ST_FLD_TX_DRV_ST) & 0xff;
	WHNAT_IO_READ32(wed, WED_WPDMA_TX0_MIB, &state->wpdma_tx0_mib);
	WHNAT_IO_READ32(wed, WED_WPDMA_TX1_MIB, &state->wpdma_tx1_mib);
	/*WED_BM*/
	WHNAT_IO_READ32(wed, WED_TX_BM_STS, &state->bm_tx_stat);
	state->bm_tx_stat = state->bm_tx_stat & 0xffff;
	WHNAT_IO_READ32(wed, WED_TX_FREE_TO_TX_BM_TKID_MIB, &state->txfree_to_bm_mib);
	WHNAT_IO_READ32(wed, WED_TX_BM_TO_WDMA_RX_DRV_TKID_MIB, &state->txbm_to_wdma_mib);
	return;
}
#endif /*ERR_RECOVERY*/
