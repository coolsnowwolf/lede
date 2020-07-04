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

UINT32 mt_mac_cr_range[] = {
	0x60000000, 0x20000, 0x200, /* WF_CFG */
	0x60100000, 0x21000, 0x200, /* WF_TRB */
	0x60110000, 0x21200, 0x200, /* WF_AGG */
	0x60120000, 0x21400, 0x200, /* WF_ARB */
	0x60130000, 0x21600, 0x200, /* WF_TMAC */
	0x60140000, 0x21800, 0x200, /* WF_RMAC */
	0x60150000, 0x21A00, 0x200, /* WF_SEC */
	0x60160000, 0x21C00, 0x200, /* WF_DMA */
	0x60170000, 0x21E00, 0x200, /* WF_CFGOFF */
	0x60180000, 0x22000, 0x1000, /* WF_PF */
	0x60190000, 0x23000, 0x200, /* WF_WTBLOFF */
	0x601A0000, 0x23200, 0x200, /* WF_ETBF */
	
	0x60300000, 0x24000, 0x400, /* WF_LPON */
	0x60310000, 0x24400, 0x200, /* WF_INT */
	0x60320000, 0x28000, 0x4000, /* WF_WTBLON */
	0x60330000, 0x2C000, 0x200, /* WF_MIB */
	0x60400000, 0x2D000, 0x200, /* WF_AON */

	0x80020000, 0x00000, 0x2000, /* TOP_CFG */	
	0x80000000, 0x02000, 0x2000, /* MCU_CFG */
	0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
#if defined(MT7603_FPGA) || defined(MT7628_FPGA)
	0xA0000000, 0x80000, 0x10000, /* PSE_CFG after remap 2 */
#else
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
#endif /* MT7603_FPGA */
	0x60200000, 0x10000, 0x10000, /* WF_PHY */
	
	0x0, 0x0, 0x0,
};


BOOLEAN mt_mac_cr_range_mapping(UINT32 *mac_addr)
{
	UINT32 mac_addr_hif = *mac_addr;
	INT idx = 0;
	BOOLEAN IsFound = 0;
	
	if (mac_addr_hif > 0x4ffff)
	{
		do
		{
			if (mac_addr_hif >= mt_mac_cr_range[idx] && 
				mac_addr_hif < (mt_mac_cr_range[idx] + mt_mac_cr_range[idx + 2]))
			{
				mac_addr_hif -= mt_mac_cr_range[idx];
				mac_addr_hif += mt_mac_cr_range[idx + 1];
				IsFound = 1;
				break;
			}
			idx += 3;
		} while (mt_mac_cr_range[idx] != 0);
	} 
	else
	{
		IsFound = 1;
	}

	*mac_addr = mac_addr_hif;

	return IsFound;
}


UINT32 mt_physical_addr_map(UINT32 addr)
{
	UINT32 global_addr = 0x0, idx = 1;

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
	else if ((addr >= 0x20000) && (addr < 0x40000))
	{
		do {
			if ((addr >= mt_mac_cr_range[idx]) && (addr < (mt_mac_cr_range[idx]+mt_mac_cr_range[idx+1]))) {
				global_addr = mt_mac_cr_range[idx-1]+(addr-mt_mac_cr_range[idx]);
				break;
			}
			idx += 3;
		} while (mt_mac_cr_range[idx] != 0);

		if (mt_mac_cr_range[idx] == 0)
		{
			DBGPRINT(RT_DEBUG_OFF, ("unknow addr range = %x\n", addr));
		}
	}
	else if ((addr >= 0x40000) && (addr < 0x80000)) //WTBL Address
	{
		global_addr = 0xa5000000 + addr - 0x40000; 
		DBGPRINT(RT_DEBUG_INFO, ("==>global_addr1=0x%x\n", global_addr));
	}
	else if ((addr >= 0xc0000) && (addr < 0xc0100)) //PSE Client
	{
		global_addr = 0x800c0000 + addr - 0xc0000;
		DBGPRINT(RT_DEBUG_INFO, ("==>global_addr2=0x%x\n", global_addr));
	}
	else
	{
		global_addr = addr;
		DBGPRINT(RT_DEBUG_INFO, ("==>global_addr3=0x%x\n", global_addr));
	}

	return global_addr;
}


