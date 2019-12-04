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

	Module Name:
	mt_io.c
*/
#include	"rt_config.h"



#ifdef MT7615
UINT32 mt7615_mac_cr_range[] = {
	0x82060000, 0x8000, 0x450, /* WF_PLE */
	0x82068000, 0xc000, 0x450, /* WF_PSE */
	0x8206c000, 0xe000, 0x300, /* PP */
	0x820d0000, 0x20000, 0x200, /* WF_AON */
	0x820f0000, 0x20200, 0x400, /* WF_CFG */
	0x820f0800, 0x20600, 0x200, /* WF_CFGOFF */
	0x820f1000, 0x20800, 0x200, /* WF_TRB */
	0x820f2000, 0x20a00, 0x200, /* WF_AGG */
	0x820f3000, 0x20c00, 0x400, /* WF_ARB */
	0x820f4000, 0x21000, 0x200, /* WF_TMAC */
	0x820f5000, 0x21200, 0x400, /* WF_RMAC */
	0x820f6000, 0x21600, 0x200, /* WF_SEC */
	0x820f7000, 0x21800, 0x200, /* WF_DMA */

	0x820f8000, 0x22000, 0x1000, /* WF_PF */
	0x820f9000, 0x23000, 0x400, /* WF_WTBLON */
	0x820f9800, 0x23400, 0x200, /* WF_WTBLOFF */

	0x820fa000, 0x24000, 0x200, /* WF_ETBF */
	0x820fb000, 0x24200, 0x400, /* WF_LPON */
	0x820fc000, 0x24600, 0x200, /* WF_INT */
	0x820fd000, 0x24800, 0x400, /* WF_MIB */

	0x820fe000, 0x25000, 0x2000, /* WF_MU */

	0x820e0000, 0x30000, 0x10000, /* WF_WTBL */

	0x80020000, 0x00000, 0x2000, /* TOP_CFG */
	0x80000000, 0x02000, 0x2000, /* MCU_CFG */
	0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
	0x82070000, 0x10000, 0x10000, /* WF_PHY */

	0x0, 0x0, 0x0,
};
#endif /* MT7615 */


#ifdef MT7622
UINT32 mt7622_mac_cr_range[] = {
	0x82060000, 0x8000, 0x450, /* WF_PLE */
	0x82068000, 0xc000, 0x450, /* WF_PSE */
	0x8206c000, 0xe000, 0x300, /* PP */
	0x820d0000, 0x20000, 0x200, /* WF_AON */
	0x820f0000, 0x20200, 0x400, /* WF_CFG */
	0x820f0800, 0x20600, 0x200, /* WF_CFGOFF */
	0x820f1000, 0x20800, 0x200, /* WF_TRB */
	0x820f2000, 0x20a00, 0x200, /* WF_AGG */
	0x820f3000, 0x20c00, 0x400, /* WF_ARB */
	0x820f4000, 0x21000, 0x200, /* WF_TMAC */
	0x820f5000, 0x21200, 0x400, /* WF_RMAC */
	0x820f6000, 0x21600, 0x200, /* WF_SEC */
	0x820f7000, 0x21800, 0x200, /* WF_DMA */

	0x820f8000, 0x22000, 0x1000, /* WF_PF */
	0x820f9000, 0x23000, 0x400, /* WF_WTBLON */
	0x820f9800, 0x23400, 0x200, /* WF_WTBLOFF */

	0x820fa000, 0x24000, 0x200, /* WF_ETBF */
	0x820fb000, 0x24200, 0x400, /* WF_LPON */
	0x820fc000, 0x24600, 0x200, /* WF_INT */
	0x820fd000, 0x24800, 0x400, /* WF_MIB */

	0x820fe000, 0x25000, 0x2000, /* WF_MU */

	0x820e0000, 0x30000, 0x10000, /* WF_WTBL */

	0x80020000, 0x00000, 0x2000, /* TOP_CFG */
	0x80000000, 0x02000, 0x2000, /* MCU_CFG */
	0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
	0x82070000, 0x10000, 0x10000, /* WF_PHY */

	0x0, 0x0, 0x0,
};
#endif /* MT7622 */



BOOLEAN mt_mac_cr_range_mapping(RTMP_ADAPTER *pAd, UINT32 *mac_addr)
{
	UINT32 mac_addr_hif = *mac_addr;
	INT idx = 0;
	BOOLEAN IsFound = 0;
	UINT32 *mac_cr_range = NULL;
#ifdef MT7615

	if (IS_MT7615(pAd))
		mac_cr_range = &mt7615_mac_cr_range[0];

#endif /* MT7615 */
#ifdef MT7622

	if (IS_MT7622(pAd))
		mac_cr_range = &mt7622_mac_cr_range[0];

#endif /* MT7622 */
	if (!mac_cr_range) {
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupported Chip for this function!\n", __func__));
		return IsFound;
	}

	if (mac_addr_hif >= 0x40000) {
		do {
			if (mac_addr_hif >= mac_cr_range[idx] &&
				mac_addr_hif < (mac_cr_range[idx] + mac_cr_range[idx + 2])) {
				mac_addr_hif -= mac_cr_range[idx];
				mac_addr_hif += mac_cr_range[idx + 1];
				IsFound = 1;
				break;
			}

			idx += 3;
		} while (mac_cr_range[idx] != 0);
	} else
		IsFound = 1;

	*mac_addr = mac_addr_hif;
	return IsFound;
}


UINT32 mt_physical_addr_map(RTMP_ADAPTER *pAd, UINT32 addr)
{
	UINT32 global_addr = 0x0, idx = 1;
	struct _RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 wtbl_2_base = cap->WtblPseAddr;

	if (addr < 0x2000)
		global_addr = 0x80020000 + addr;
	else if ((addr >= 0x2000) && (addr < 0x4000))
		global_addr = 0x80000000 + addr - 0x2000;
	else if ((addr >= 0x4000) && (addr < 0x8000))
		global_addr = 0x50000000 + addr - 0x4000;
	else if ((addr >= 0x8000) && (addr < 0x10000))
		global_addr = 0xa0000000 + addr - 0x8000;
	else if ((addr >= 0x10000) && (addr < 0x20000))
		global_addr = 0x60200000 + addr - 0x10000;
	else if ((addr >= 0x20000) && (addr < 0x40000)) {
		UINT32 *mac_cr_range = NULL;
#ifdef MT7615

		if (IS_MT7615(pAd))
			mac_cr_range = &mt7615_mac_cr_range[0];

#endif /* MT7615 */
#ifdef MT7622

		if (IS_MT7622(pAd))
			mac_cr_range = &mt7622_mac_cr_range[0];

#endif /* MT7622 */

		if (!mac_cr_range) {
			MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupported Chip for this function!\n", __func__));
			return global_addr;
		}

		do {
			if ((addr >= mac_cr_range[idx]) && (addr < (mac_cr_range[idx] + mac_cr_range[idx + 1]))) {
				global_addr = mac_cr_range[idx - 1] + (addr - mac_cr_range[idx]);
				break;
			}

			idx += 3;
		} while (mac_cr_range[idx] != 0);

		if (mac_cr_range[idx] == 0)
			MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("unknow addr range = %x\n", addr));
	} else if ((addr >= 0x40000) && (addr < 0x80000)) { /* WTBL Address */
		global_addr = wtbl_2_base + addr - 0x40000;
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr1=0x%x\n", global_addr));
	} else if ((addr >= 0xc0000) && (addr < 0xc0100)) { /* PSE Client */
		global_addr = 0x800c0000 + addr - 0xc0000;
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr2=0x%x\n", global_addr));
	} else {
		global_addr = addr;
		MTWF_LOG(DBG_CAT_HIF, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("==>global_addr3=0x%x\n", global_addr));
	}

	return global_addr;
}


