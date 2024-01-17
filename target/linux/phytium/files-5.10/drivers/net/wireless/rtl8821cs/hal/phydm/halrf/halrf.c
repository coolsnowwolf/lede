/******************************************************************************
 *
 * Copyright(c) 2007 - 2017  Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in the
 * file called LICENSE.
 *
 * Contact Information:
 * wlanfae <wlanfae@realtek.com>
 * Realtek Corporation, No. 2, Innovation Road II, Hsinchu Science Park,
 * Hsinchu 300, Taiwan.
 *
 * Larry Finger <Larry.Finger@lwfinger.net>
 *
 *****************************************************************************/

/*@************************************************************
 * include files
 * ************************************************************
 */

#include "mp_precomp.h"
#include "phydm_precomp.h"

#if (RTL8822B_SUPPORT == 1 || RTL8821C_SUPPORT == 1 ||\
	RTL8195B_SUPPORT == 1 || RTL8198F_SUPPORT == 1 ||\
	RTL8814B_SUPPORT == 1 || RTL8822C_SUPPORT == 1 ||\
	RTL8812F_SUPPORT == 1 || RTL8710C_SUPPORT == 1 ||\
	RTL8197G_SUPPORT == 1 || RTL8814C_SUPPORT == 1 )

void _iqk_check_if_reload(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	iqk_info->is_reload = (boolean)odm_get_bb_reg(dm, R_0x1bf0, BIT(16));
}

void _iqk_page_switch(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (dm->support_ic_type == ODM_RTL8821C)
		odm_write_4byte(dm, 0x1b00, 0xf8000008);
	else
		odm_write_4byte(dm, 0x1b00, 0xf800000a);
}

u32 halrf_psd_log2base(u32 val)
{
	u8 j;
	u32 tmp, tmp2, val_integerd_b = 0, tindex, shiftcount = 0;
	u32 result, val_fractiond_b = 0;
	u32 table_fraction[21] = {
		0, 432, 332, 274, 232, 200, 174, 151, 132, 115,
		100, 86, 74, 62, 51, 42, 32, 23, 15, 7, 0};

	if (val == 0)
		return 0;

	tmp = val;

	while (1) {
		if (tmp == 1)
			break;

		tmp = (tmp >> 1);
		shiftcount++;
	}

	val_integerd_b = shiftcount + 1;

	tmp2 = 1;
	for (j = 1; j <= val_integerd_b; j++)
		tmp2 = tmp2 * 2;

	tmp = (val * 100) / tmp2;
	tindex = tmp / 5;

	if (tindex > 20)
		tindex = 20;

	val_fractiond_b = table_fraction[tindex];

	result = val_integerd_b * 100 - val_fractiond_b;

	return result;
}
#if (RTL8822B_SUPPORT == 1 || RTL8821C_SUPPORT == 1 ||\
	RTL8814B_SUPPORT == 1 || RTL8822C_SUPPORT == 1 ||\
	RTL8814C_SUPPORT == 1)
void halrf_iqk_xym_enable(struct dm_struct *dm, u8 xym_enable)
{
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	if (xym_enable == 0)
		iqk_info->xym_read = false;
	else
		iqk_info->xym_read = true;

	RF_DBG(dm, DBG_RF_IQK, "[IQK]%-20s %s\n", "xym_read = ",
	       (iqk_info->xym_read ? "true" : "false"));
}

/*xym_type => 0: rx_sym; 1: tx_xym; 2:gs1_xym; 3:gs2_sym; 4: rxk1_xym*/
void halrf_iqk_xym_read(void *dm_void, u8 path, u8 xym_type)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 i, start, num;
	u32 tmp1, tmp2;

	if (!iqk_info->xym_read)
		return;

	if (*dm->band_width == 0) {
		start = 3;
		num = 4;
	} else if (*dm->band_width == 1) {
		start = 2;
		num = 6;
	} else {
		start = 0;
		num = 10;
	}

	odm_write_4byte(dm, 0x1b00, 0xf8000008);
	tmp1 = odm_read_4byte(dm, 0x1b1c);
	odm_write_4byte(dm, 0x1b1c, 0xa2193c32);

	odm_write_4byte(dm, 0x1b00, 0xf800000a);
	tmp2 = odm_read_4byte(dm, 0x1b1c);
	odm_write_4byte(dm, 0x1b1c, 0xa2193c32);

	for (path = 0; path < 2; path++) {
		odm_write_4byte(dm, 0x1b00, 0xf8000008 | path << 1);
		switch (xym_type) {
		case 0:
			for (i = 0; i < num; i++) {
				odm_write_4byte(dm, 0x1b14, 0xe6 + start + i);
				odm_write_4byte(dm, 0x1b14, 0x0);
				iqk_info->rx_xym[path][i] =
						odm_read_4byte(dm, 0x1b38);
			}
			break;
		case 1:
			for (i = 0; i < num; i++) {
				odm_write_4byte(dm, 0x1b14, 0xe6 + start + i);
				odm_write_4byte(dm, 0x1b14, 0x0);
				iqk_info->tx_xym[path][i] =
						odm_read_4byte(dm, 0x1b38);
			}
			break;
		case 2:
			for (i = 0; i < 6; i++) {
				odm_write_4byte(dm, 0x1b14, 0xe0 + i);
				odm_write_4byte(dm, 0x1b14, 0x0);
				iqk_info->gs1_xym[path][i] =
						odm_read_4byte(dm, 0x1b38);
			}
			break;
		case 3:
			for (i = 0; i < 6; i++) {
				odm_write_4byte(dm, 0x1b14, 0xe0 + i);
				odm_write_4byte(dm, 0x1b14, 0x0);
				iqk_info->gs2_xym[path][i] =
						odm_read_4byte(dm, 0x1b38);
			}
			break;
		case 4:
			for (i = 0; i < 6; i++) {
				odm_write_4byte(dm, 0x1b14, 0xe0 + i);
				odm_write_4byte(dm, 0x1b14, 0x0);
				iqk_info->rxk1_xym[path][i] =
						odm_read_4byte(dm, 0x1b38);
			}
			break;
		}
		odm_write_4byte(dm, 0x1b38, 0x20000000);
		odm_write_4byte(dm, 0x1b00, 0xf8000008);
		odm_write_4byte(dm, 0x1b1c, tmp1);
		odm_write_4byte(dm, 0x1b00, 0xf800000a);
		odm_write_4byte(dm, 0x1b1c, tmp2);
		_iqk_page_switch(dm);
	}
}

/*xym_type => 0: rx_sym; 1: tx_xym; 2:gs1_xym; 3:gs2_sym; 4: rxk1_xym*/
void halrf_iqk_xym_show(struct dm_struct *dm, u8 xym_type)
{
	u8 num, path, path_num, i;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	if (dm->rf_type == RF_1T1R)
		path_num = 0x1;
	else if (dm->rf_type == RF_2T2R)
		path_num = 0x2;
	else
		path_num = 0x4;

	if (*dm->band_width == CHANNEL_WIDTH_20)
		num = 4;
	else if (*dm->band_width == CHANNEL_WIDTH_40)
		num = 6;
	else
		num = 10;

	for (path = 0; path < path_num; path++) {
		switch (xym_type) {
		case 0:
			for (i = 0; i < num; i++)
				RF_DBG(dm, DBG_RF_IQK,
				       "[IQK]%-20s %-2d: 0x%x\n",
				       (path == 0) ? "PATH A RX-XYM " :
				       "PATH B RX-XYM", i,
				       iqk_info->rx_xym[path][i]);
			break;
		case 1:
			for (i = 0; i < num; i++)
				RF_DBG(dm, DBG_RF_IQK,
				       "[IQK]%-20s %-2d: 0x%x\n",
				       (path == 0) ? "PATH A TX-XYM " :
				       "PATH B TX-XYM", i,
				       iqk_info->tx_xym[path][i]);
			break;
		case 2:
			for (i = 0; i < 6; i++)
				RF_DBG(dm, DBG_RF_IQK,
				       "[IQK]%-20s %-2d: 0x%x\n",
				       (path == 0) ? "PATH A GS1-XYM " :
				       "PATH B GS1-XYM", i,
				       iqk_info->gs1_xym[path][i]);
			break;
		case 3:
			for (i = 0; i < 6; i++)
				RF_DBG(dm, DBG_RF_IQK,
				       "[IQK]%-20s %-2d: 0x%x\n",
				       (path == 0) ? "PATH A GS2-XYM " :
				       "PATH B GS2-XYM", i,
				       iqk_info->gs2_xym[path][i]);
			break;
		case 4:
			for (i = 0; i < 6; i++)
				RF_DBG(dm, DBG_RF_IQK,
				       "[IQK]%-20s %-2d: 0x%x\n",
				       (path == 0) ? "PATH A RXK1-XYM " :
				       "PATH B RXK1-XYM", i,
				       iqk_info->rxk1_xym[path][i]);
			break;
		}
	}
}

void halrf_iqk_xym_dump(void *dm_void)
{
	u32 tmp1, tmp2;
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	odm_write_4byte(dm, 0x1b00, 0xf8000008);
	tmp1 = odm_read_4byte(dm, 0x1b1c);
	odm_write_4byte(dm, 0x1b00, 0xf800000a);
	tmp2 = odm_read_4byte(dm, 0x1b1c);
#if 0
	/*halrf_iqk_xym_read(dm, xym_type);*/
#endif
	odm_write_4byte(dm, 0x1b00, 0xf8000008);
	odm_write_4byte(dm, 0x1b1c, tmp1);
	odm_write_4byte(dm, 0x1b00, 0xf800000a);
	odm_write_4byte(dm, 0x1b1c, tmp2);
	_iqk_page_switch(dm);
}
#endif
void halrf_iqk_info_dump(void *dm_void, u32 *_used, char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 used = *_used;
	u32 out_len = *_out_len;
	u8 rf_path, j, reload_iqk = 0;
	u32 tmp;
	/*two channel, PATH, TX/RX, 0:pass 1 :fail*/
	boolean iqk_result[2][NUM][2];
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	if (!(dm->support_ic_type & (ODM_RTL8822B | ODM_RTL8821C)))
		return;

	/* IQK INFO */
	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s\n",
		 "% IQK Info %");
	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s\n",
		 (dm->fw_offload_ability & PHYDM_RF_IQK_OFFLOAD) ? "FW-IQK" :
		 "Driver-IQK");

	reload_iqk = (u8)odm_get_bb_reg(dm, R_0x1bf0, BIT(16));
	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s: %s\n",
		 "reload", (reload_iqk) ? "True" : "False");

	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s: %s\n",
		 "rfk_forbidden", (iqk_info->rfk_forbidden) ? "True" : "False");
#if (RTL8814A_SUPPORT == 1 || RTL8822B_SUPPORT == 1 || \
	RTL8821C_SUPPORT == 1 || RTL8195B_SUPPORT == 1 ||\
	RTL8814B_SUPPORT == 1 || RTL8822C_SUPPORT == 1||\
	RTL8814C_SUPPORT == 1 )
	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s: %s\n",
		 "segment_iqk", (iqk_info->segment_iqk) ? "True" : "False");
#endif

	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s:%d %d\n",
		 "iqk count / fail count", dm->n_iqk_cnt, dm->n_iqk_fail_cnt);

	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s: %d\n",
		 "channel", *dm->channel);

	if (*dm->band_width == CHANNEL_WIDTH_20)
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "%-20s: %s\n", "bandwidth", "BW_20");
	else if (*dm->band_width == CHANNEL_WIDTH_40)
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "%-20s: %s\n", "bandwidth", "BW_40");
	else if (*dm->band_width == CHANNEL_WIDTH_80)
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "%-20s: %s\n", "bandwidth", "BW_80");
	else if (*dm->band_width == CHANNEL_WIDTH_160)
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "%-20s: %s\n", "bandwidth", "BW_160");
	else if (*dm->band_width == CHANNEL_WIDTH_80_80)
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "%-20s: %s\n", "bandwidth", "BW_80_80");
	else
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "%-20s: %s\n", "bandwidth", "BW_UNKNOWN");

	PDM_SNPF(out_len, used, output + used, out_len - used,
		 "%-20s: %llu %s\n", "progressing_time",
		 dm->rf_calibrate_info.iqk_total_progressing_time, "(ms)");

	tmp = odm_read_4byte(dm, 0x1bf0);
	for (rf_path = RF_PATH_A; rf_path <= RF_PATH_B; rf_path++)
		for (j = 0; j < 2; j++)
			iqk_result[0][rf_path][j] = (boolean)
			(tmp & (BIT(rf_path + (j * 4)) >> (rf_path + (j * 4))));

	PDM_SNPF(out_len, used, output + used, out_len - used,
		 "%-20s: 0x%08x\n", "Reg0x1bf0", tmp);
	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s: %s\n",
		 "PATH_A-Tx result",
		 (iqk_result[0][RF_PATH_A][0]) ? "Fail" : "Pass");
	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s: %s\n",
		 "PATH_A-Rx result",
		 (iqk_result[0][RF_PATH_A][1]) ? "Fail" : "Pass");
#if (RTL8822B_SUPPORT == 1)
	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s: %s\n",
		 "PATH_B-Tx result",
		 (iqk_result[0][RF_PATH_B][0]) ? "Fail" : "Pass");
	PDM_SNPF(out_len, used, output + used, out_len - used, "%-20s: %s\n",
		 "PATH_B-Rx result",
		 (iqk_result[0][RF_PATH_B][1]) ? "Fail" : "Pass");
#endif
	*_used = used;
	*_out_len = out_len;
}

void halrf_get_fw_version(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	rf->fw_ver = (dm->fw_version << 16) | dm->fw_sub_version;
}

void halrf_iqk_dbg(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u8 rf_path, j;
	u32 tmp;
	/*two channel, PATH, TX/RX, 0:pass 1 :fail*/
	boolean iqk_result[2][NUM][2];
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	/* IQK INFO */
	RF_DBG(dm, DBG_RF_IQK, "%-20s\n", "====== IQK Info ======");

	RF_DBG(dm, DBG_RF_IQK, "%-20s\n",
	       (dm->fw_offload_ability & PHYDM_RF_IQK_OFFLOAD) ? "FW-IQK" :
	       "Driver-IQK");

	if (dm->fw_offload_ability & PHYDM_RF_IQK_OFFLOAD) {
		halrf_get_fw_version(dm);
		RF_DBG(dm, DBG_RF_IQK, "%-20s: 0x%x\n", "FW_VER", rf->fw_ver);
	} else {
		RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "IQK_VER", HALRF_IQK_VER);
	}

	RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "reload",
	       (iqk_info->is_reload) ? "True" : "False");

	RF_DBG(dm, DBG_RF_IQK, "%-20s: %d %d\n", "iqk count / fail count",
	       dm->n_iqk_cnt, dm->n_iqk_fail_cnt);

	RF_DBG(dm, DBG_RF_IQK, "%-20s: %d\n", "channel", *dm->channel);

	if (*dm->band_width == CHANNEL_WIDTH_20)
		RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "bandwidth", "BW_20");
	else if (*dm->band_width == CHANNEL_WIDTH_40)
		RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "bandwidth", "BW_40");
	else if (*dm->band_width == CHANNEL_WIDTH_80)
		RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "bandwidth", "BW_80");
	else if (*dm->band_width == CHANNEL_WIDTH_160)
		RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "bandwidth", "BW_160");
	else if (*dm->band_width == CHANNEL_WIDTH_80_80)
		RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "bandwidth", "BW_80_80");
	else
		RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "bandwidth",
		       "BW_UNKNOWN");
#if 0
/*
 *	RF_DBG(dm, DBG_RF_IQK, "%-20s: %llu %s\n",
 *	       "progressing_time",
 *	       dm->rf_calibrate_info.iqk_total_progressing_time, "(ms)");
 */
#endif
	RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "rfk_forbidden",
	       (iqk_info->rfk_forbidden) ? "True" : "False");
#if (RTL8814A_SUPPORT == 1 || RTL8822B_SUPPORT == 1 || \
	RTL8821C_SUPPORT == 1 || RTL8195B_SUPPORT == 1 ||\
	RTL8814B_SUPPORT == 1 || RTL8822C_SUPPORT == 1||\
	RTL8814C_SUPPORT == 1)
	RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "segment_iqk",
	       (iqk_info->segment_iqk) ? "True" : "False");
#endif

	RF_DBG(dm, DBG_RF_IQK, "%-20s: %llu %s\n", "progressing_time",
	       dm->rf_calibrate_info.iqk_progressing_time, "(ms)");

	tmp = odm_read_4byte(dm, 0x1bf0);
	for (rf_path = RF_PATH_A; rf_path <= RF_PATH_B; rf_path++)
		for (j = 0; j < 2; j++)
			iqk_result[0][rf_path][j] = (boolean)
			(tmp & (BIT(rf_path + (j * 4)) >> (rf_path + (j * 4))));

	RF_DBG(dm, DBG_RF_IQK, "%-20s: 0x%08x\n", "Reg0x1bf0", tmp);
	RF_DBG(dm, DBG_RF_IQK, "%-20s: 0x%08x\n", "Reg0x1be8",
	       odm_read_4byte(dm, 0x1be8));
	RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "PATH_A-Tx result",
	       (iqk_result[0][RF_PATH_A][0]) ? "Fail" : "Pass");
	RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "PATH_A-Rx result",
	       (iqk_result[0][RF_PATH_A][1]) ? "Fail" : "Pass");
#if (RTL8822B_SUPPORT == 1)
	RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "PATH_B-Tx result",
	       (iqk_result[0][RF_PATH_B][0]) ? "Fail" : "Pass");
	RF_DBG(dm, DBG_RF_IQK, "%-20s: %s\n", "PATH_B-Rx result",
	       (iqk_result[0][RF_PATH_B][1]) ? "Fail" : "Pass");
#endif
}

void halrf_lck_dbg(struct dm_struct *dm)
{
	RF_DBG(dm, DBG_RF_IQK, "%-20s\n", "====== LCK Info ======");
#if 0
	/*RF_DBG(dm, DBG_RF_IQK, "%-20s\n",
	 *	 (dm->fw_offload_ability & PHYDM_RF_IQK_OFFLOAD) ? "LCK" : "RTK"));
	 */
#endif
	RF_DBG(dm, DBG_RF_IQK, "%-20s: %llu %s\n", "progressing_time",
	       dm->rf_calibrate_info.lck_progressing_time, "(ms)");
}
void phydm_get_iqk_cfir(void *dm_void, u8 idx, u8 path, boolean debug)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	switch (dm->support_ic_type) {
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		phy_get_iqk_cfir_8822b(dm, idx, path, debug);
	break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:		
		phy_get_iqk_cfir_8822c(dm, idx, path, debug);
	break;
#endif
#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:		
		phy_get_iqk_cfir_8814b(dm, idx, path, debug);
	break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:		
		phy_get_iqk_cfir_8814c(dm, idx, path, debug);
	break;
#endif

	default:
	break;
	}
}


void halrf_iqk_dbg_cfir_backup(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 path, idx, i;

	switch (dm->support_ic_type) {
#if (RTL8822B_SUPPORT == 1)
		case ODM_RTL8822B:
			phy_iqk_dbg_cfir_backup_8822b(dm);
				break;
#endif
#if (RTL8822C_SUPPORT == 1)
		case ODM_RTL8822C:			
			phy_iqk_dbg_cfir_backup_8822c(dm);
				break;
#endif
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:			
			phy_iqk_dbg_cfir_backup_8814b(dm);
				break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:			
			phy_iqk_dbg_cfir_backup_8814c(dm);
				break;
#endif

	default:
	break;
	}

}

void halrf_iqk_dbg_cfir_backup_update(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk = &dm->IQK_info;
	u8 i, path, idx;
	u32 bmask13_12 = BIT(13) | BIT(12);
	u32 bmask20_16 = BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16);
	u32 data;

	switch (dm->support_ic_type) {
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		phy_iqk_dbg_cfir_backup_update_8822b(dm);
	break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:		
		phy_iqk_dbg_cfir_backup_update_8822c(dm);
	break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		phy_iqk_dbg_cfir_backup_update_8814c(dm);
	break;
#endif

	default:
	break;
	}
}

void halrf_iqk_dbg_cfir_reload(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk = &dm->IQK_info;
	u8 i, path, idx;
	u32 bmask13_12 = BIT(13) | BIT(12);
	u32 bmask20_16 = BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16);
	u32 data;
	
	switch (dm->support_ic_type) {
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		phy_iqk_dbg_cfir_reload_8822b(dm);
	break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:		
		phy_iqk_dbg_cfir_reload_8822c(dm);
	break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		phy_iqk_dbg_cfir_reload_8814c(dm);
	break;
#endif

	default:
	break;
	}
}

void halrf_iqk_dbg_cfir_write(void *dm_void, u8 type, u32 path, u32 idx,
			      u32 i, u32 data)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	switch (dm->support_ic_type) {
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		phy_iqk_dbg_cfir_write_8822b(dm, type, path, idx, i, data);
	break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:		
		phy_iqk_dbg_cfir_write_8822c(dm, type, path, idx, i, data);
		break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
		phy_iqk_dbg_cfir_write_8814c(dm, type, path, idx, i, data);
		break;
#endif

	default:
	break;
	}
}

void halrf_iqk_dbg_cfir_backup_show(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 path, idx, i;

	switch (dm->support_ic_type) {
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		phy_iqk_dbg_cfir_backup_8822b(dm);
	break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:		
		phy_iqk_dbg_cfir_backup_8822c(dm);
		break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:		
		phy_iqk_dbg_cfir_backup_8814c(dm);
	break;
#endif

	default:
	break;
	}
}

void halrf_do_imr_test(void *dm_void, u8 flag_imr_test)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (flag_imr_test != 0x0)
		switch (dm->support_ic_type) {
#if (RTL8822B_SUPPORT == 1)
		case ODM_RTL8822B:
			do_imr_test_8822b(dm);
			break;
#endif
#if (RTL8821C_SUPPORT == 1)
		case ODM_RTL8821C:
			do_imr_test_8821c(dm);
			break;
#endif
		default:
			break;
		}
}

#if (RTL8822B_SUPPORT == 1 || RTL8821C_SUPPORT == 1 || RTL8822C_SUPPORT == 1 ||\
	RTL8814B_SUPPORT == 1 || RTL8814C_SUPPORT == 1)
void halrf_iqk_debug(void *dm_void, u32 *const dm_value, u32 *_used,
		     char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if 0
	/*dm_value[0]=0x0: backup from SRAM & show*/
	/*dm_value[0]=0x1: write backup CFIR to SRAM*/
	/*dm_value[0]=0x2: reload default CFIR to SRAM*/
	/*dm_value[0]=0x3: show backup*/
	/*dm_value[0]=0x10: write backup CFIR real part*/
	/*--> dm_value[1]:path, dm_value[2]:tx/rx, dm_value[3]:index, dm_value[4]:data*/
	/*dm_value[0]=0x11: write backup CFIR imag*/
	/*--> dm_value[1]:path, dm_value[2]:tx/rx, dm_value[3]:index, dm_value[4]:data*/
	/*dm_value[0]=0x20 :xym_read enable*/
	/*--> dm_value[1]:0:disable, 1:enable*/
	/*if dm_value[0]=0x20 = enable, */
	/*0x1:show rx_sym; 0x2: tx_xym; 0x3:gs1_xym; 0x4:gs2_sym; 0x5:rxk1_xym*/
#endif
	if (dm_value[0] == 0x0)
		halrf_iqk_dbg_cfir_backup(dm);
	else if (dm_value[0] == 0x1)
		halrf_iqk_dbg_cfir_backup_update(dm);
	else if (dm_value[0] == 0x2)
		halrf_iqk_dbg_cfir_reload(dm);
	else if (dm_value[0] == 0x3)
		halrf_iqk_dbg_cfir_backup_show(dm);
	else if (dm_value[0] == 0x10)
		halrf_iqk_dbg_cfir_write(dm, 0, dm_value[1], dm_value[2],
					 dm_value[3], dm_value[4]);
	else if (dm_value[0] == 0x11)
		halrf_iqk_dbg_cfir_write(dm, 1, dm_value[1], dm_value[2],
					 dm_value[3], dm_value[4]);
	else if (dm_value[0] == 0x20)
		halrf_iqk_xym_enable(dm, (u8)dm_value[1]);
	else if (dm_value[0] == 0x21)
		halrf_iqk_xym_show(dm, (u8)dm_value[1]);
	else if (dm_value[0] == 0x30)
		halrf_do_imr_test(dm, (u8)dm_value[1]);
}
#endif

void halrf_iqk_hwtx_check(void *dm_void, boolean is_check)
{
#if 0
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u32 tmp_b04;

	if (is_check) {
		iqk_info->is_hwtx = (boolean)odm_get_bb_reg(dm, R_0xb00, BIT(8));
	} else {
		if (iqk_info->is_hwtx) {
			tmp_b04 = odm_read_4byte(dm, 0xb04);
			odm_set_bb_reg(dm, R_0xb04, BIT(3) | BIT(2), 0x0);
			odm_write_4byte(dm, 0xb04, tmp_b04);
		}
	}
#endif
}
#endif

u8 halrf_match_iqk_version(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u32 iqk_version = 0;
	char temp[10] = {0};

	odm_move_memory(dm, temp, HALRF_IQK_VER, sizeof(temp));
	PHYDM_SSCANF(temp + 2, DCMD_HEX, &iqk_version);

	if (dm->support_ic_type == ODM_RTL8822B) {
		if (iqk_version >= 0x24 && (odm_get_hw_img_version(dm) >= 72))
			return 1;
		else if ((iqk_version <= 0x23) &&
			 (odm_get_hw_img_version(dm) <= 71))
			return 1;
		else
			return 0;
	}

	if (dm->support_ic_type == ODM_RTL8821C) {
		if (iqk_version >= 0x18 && (odm_get_hw_img_version(dm) >= 37))
			return 1;
		else
			return 0;
	}

	return 1;
}

void halrf_rf_lna_setting(void *dm_void, enum halrf_lna_set type)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	switch (dm->support_ic_type) {
#if (RTL8188E_SUPPORT == 1)
	case ODM_RTL8188E:
		halrf_rf_lna_setting_8188e(dm, type);
		break;
#endif
#if (RTL8192E_SUPPORT == 1)
	case ODM_RTL8192E:
		halrf_rf_lna_setting_8192e(dm, type);
		break;
#endif
#if (RTL8192F_SUPPORT == 1)
	case ODM_RTL8192F:
		halrf_rf_lna_setting_8192f(dm, type);
		break;
#endif

#if (RTL8723B_SUPPORT == 1)
	case ODM_RTL8723B:
		halrf_rf_lna_setting_8723b(dm, type);
		break;
#endif
#if (RTL8812A_SUPPORT == 1)
	case ODM_RTL8812:
		halrf_rf_lna_setting_8812a(dm, type);
		break;
#endif
#if ((RTL8821A_SUPPORT == 1) || (RTL8881A_SUPPORT == 1))
	case ODM_RTL8881A:
	case ODM_RTL8821:
		halrf_rf_lna_setting_8821a(dm, type);
		break;
#endif
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		halrf_rf_lna_setting_8822b(dm_void, type);
		break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		halrf_rf_lna_setting_8822c(dm_void, type);
		break;
#endif
#if (RTL8812F_SUPPORT == 1)
	case ODM_RTL8812F:
		halrf_rf_lna_setting_8812f(dm_void, type);
		break;
#endif
#if (RTL8821C_SUPPORT == 1)
	case ODM_RTL8821C:
		halrf_rf_lna_setting_8821c(dm_void, type);
		break;
#endif
#if (RTL8710C_SUPPORT == 1)
	case ODM_RTL8710C:
		halrf_rf_lna_setting_8710c(dm_void, type);
		break;
#endif
#if (RTL8721D_SUPPORT == 1)
	case ODM_RTL8721D:
		halrf_rf_lna_setting_8721d(dm, type);
		break;
#endif
#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		break;
#endif

	default:
		break;
	}
}

void halrf_support_ability_debug(void *dm_void, char input[][16], u32 *_used,
				 char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	u32 dm_value[10] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
	u8 i;

	for (i = 0; i < 5; i++)
		if (input[i + 1])
			PHYDM_SSCANF(input[i + 2], DCMD_DECIMAL, &dm_value[i]);

	if (dm_value[0] == 100) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "\n[RF Supportability]\n");
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "00. (( %s ))Power Tracking\n",
			 ((rf->rf_supportability & HAL_RF_TX_PWR_TRACK) ?
			 ("V") : (".")));
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "01. (( %s ))IQK\n",
			 ((rf->rf_supportability & HAL_RF_IQK) ? ("V") :
			 (".")));
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "02. (( %s ))LCK\n",
			 ((rf->rf_supportability & HAL_RF_LCK) ? ("V") :
			 (".")));
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "03. (( %s ))DPK\n",
			 ((rf->rf_supportability & HAL_RF_DPK) ? ("V") :
			 (".")));
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "04. (( %s ))HAL_RF_TXGAPK\n",
			 ((rf->rf_supportability & HAL_RF_TXGAPK) ? ("V") :
			 (".")));
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "05. (( %s ))HAL_RF_DACK\n",
			 ((rf->rf_supportability & HAL_RF_DACK) ? ("V") :
			 (".")));
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "06. (( %s ))DPK_TRACK\n",
			 ((rf->rf_supportability & HAL_RF_DPK_TRACK) ? ("V") :
			 (".")));
#ifdef CONFIG_2G_BAND_SHIFT
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "07. (( %s ))HAL_2GBAND_SHIFT\n",
			 ((rf->rf_supportability & HAL_2GBAND_SHIFT) ? ("V") :
			 (".")));
#endif
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "08. (( %s ))HAL_RF_RXDCK\n",
			 ((rf->rf_supportability & HAL_RF_RXDCK) ? ("V") :
			 (".")));

	} else {
		if (dm_value[1] == 1) /* enable */
			rf->rf_supportability |= BIT(dm_value[0]);
		else if (dm_value[1] == 2) /* disable */
			rf->rf_supportability &= ~(BIT(dm_value[0]));
		else
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "[Warning!!!]  1:enable,  2:disable\n");
	}
	PDM_SNPF(out_len, used, output + used, out_len - used,
		 "\nCurr-RF_supportability =  0x%x\n\n", rf->rf_supportability);

	*_used = used;
	*_out_len = out_len;
}

#ifdef CONFIG_2G_BAND_SHIFT
void halrf_support_band_shift_debug(void *dm_void, char input[][16], u32 *_used,
				    char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	//u32 band_value[2] = {00};
	u32 dm_value[10] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
	u8 i;

#if (RTL8192F_SUPPORT == 1)
	for (i = 0; i < 7; i++)
		if (input[i + 1])
			PHYDM_SSCANF(input[i + 2], DCMD_DECIMAL, &dm_value[i]);

	if (!(rf->rf_supportability & HAL_2GBAND_SHIFT)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "\nCurr-RF_supportability[07. (( . ))HAL_2GBAND_SHIFT]\nNo RF Band Shift,default: 2.4G!\n");
	} else {
		if (dm_value[0] == 01) {
			rf->rf_shift_band = HAL_RF_2P3;
			halrf_lck_trigger(dm);
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "\n[rf_shift_band] = %d\nRF Band Shift to 2.3G!\n",
				 rf->rf_shift_band);
		} else if (dm_value[0] == 02) {
			rf->rf_shift_band = HAL_RF_2P5;
			halrf_lck_trigger(dm);
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "\n[rf_shift_band] = %d\nRF Band Shift to 2.5G!\n",
				 rf->rf_shift_band);
		} else {
			rf->rf_shift_band = HAL_RF_2P4;
			halrf_lck_trigger(dm);
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "\n[rf_shift_band] = %d\nNo RF Band Shift,default: 2.4G!\n",
				 rf->rf_shift_band);
		}
	}
	*_used = used;
	*_out_len = out_len;
#endif
}
#endif

void halrf_cmn_info_init(void *dm_void, enum halrf_cmninfo_init cmn_info,
			 u32 value)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	switch (cmn_info) {
	case HALRF_CMNINFO_EEPROM_THERMAL_VALUE:
		rf->eeprom_thermal = (u8)value;
		break;
	case HALRF_CMNINFO_PWT_TYPE:
		rf->pwt_type = (u8)value;
		break;
	case HALRF_CMNINFO_MP_POWER_TRACKING_TYPE:
		rf->mp_pwt_type = (u8)value;
		break;
	default:
		break;
	}
}

void halrf_cmn_info_hook(void *dm_void, enum halrf_cmninfo_hook cmn_info,
			 void *value)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	switch (cmn_info) {
	case HALRF_CMNINFO_CON_TX:
		rf->is_con_tx = (boolean *)value;
		break;
	case HALRF_CMNINFO_SINGLE_TONE:
		rf->is_single_tone = (boolean *)value;
		break;
	case HALRF_CMNINFO_CARRIER_SUPPRESSION:
		rf->is_carrier_suppresion = (boolean *)value;
		break;
	case HALRF_CMNINFO_MP_RATE_INDEX:
		rf->mp_rate_index = (u8 *)value;
		break;
	case HALRF_CMNINFO_MANUAL_RF_SUPPORTABILITY:
		rf->manual_rf_supportability = (u32 *)value;
		break;
	default:
		/*do nothing*/
		break;
	}
}

void halrf_cmn_info_set(void *dm_void, u32 cmn_info, u64 value)
{
	/* This init variable may be changed in run time. */
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_rf_calibration_struct *cali_info = &dm->rf_calibrate_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	switch (cmn_info) {
	case HALRF_CMNINFO_ABILITY:
		rf->rf_supportability = (u32)value;
		break;

	case HALRF_CMNINFO_DPK_EN:
		rf->dpk_en = (u8)value;
		break;
	case HALRF_CMNINFO_RFK_FORBIDDEN:
		dm->IQK_info.rfk_forbidden = (boolean)value;
		break;
#if (RTL8814A_SUPPORT == 1 || RTL8822B_SUPPORT == 1 || \
	RTL8821C_SUPPORT == 1 || RTL8195B_SUPPORT == 1 ||\
	RTL8814B_SUPPORT == 1 || RTL8822C_SUPPORT == 1||\
	RTL8814C_SUPPORT == 1 )
	case HALRF_CMNINFO_IQK_SEGMENT:
		dm->IQK_info.segment_iqk = (boolean)value;
		break;
#endif
	case HALRF_CMNINFO_RATE_INDEX:
		rf->p_rate_index = (u32)value;
		break;
#if !(DM_ODM_SUPPORT_TYPE & ODM_IOT)
	case HALRF_CMNINFO_MP_PSD_POINT:
		rf->halrf_psd_data.point = (u32)value;
		break;
	case HALRF_CMNINFO_MP_PSD_START_POINT:
		rf->halrf_psd_data.start_point = (u32)value;
		break;
	case HALRF_CMNINFO_MP_PSD_STOP_POINT:
		rf->halrf_psd_data.stop_point = (u32)value;
		break;
	case HALRF_CMNINFO_MP_PSD_AVERAGE:
		rf->halrf_psd_data.average = (u32)value;
		break;
#endif
	case HALRF_CMNINFO_POWER_TRACK_CONTROL:
		cali_info->txpowertrack_control = (u8)value;
		break;
	default:
		/* do nothing */
		break;
	}
}

u64 halrf_cmn_info_get(void *dm_void, u32 cmn_info)
{
	/* This init variable may be changed in run time. */
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	u64 return_value = 0;

	switch (cmn_info) {
	case HALRF_CMNINFO_ABILITY:
		return_value = (u32)rf->rf_supportability;
		break;
	case HALRF_CMNINFO_RFK_FORBIDDEN:
		return_value = dm->IQK_info.rfk_forbidden;
		break;
#if (RTL8814A_SUPPORT == 1 || RTL8822B_SUPPORT == 1 || \
	RTL8821C_SUPPORT == 1 || RTL8195B_SUPPORT == 1 ||\
	RTL8814B_SUPPORT == 1  || RTL8822C_SUPPORT == 1||\
	RTL8814C_SUPPORT == 1)
	case HALRF_CMNINFO_IQK_SEGMENT:
		return_value = dm->IQK_info.segment_iqk;
		break;
	case HALRF_CMNINFO_IQK_TIMES:
		return_value = dm->IQK_info.iqk_times;
		break;
#endif
	default:
		/* do nothing */
		break;
	}

	return return_value;
}

void halrf_supportability_init_mp(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	switch (dm->support_ic_type) {
	case ODM_RTL8814B:
#if (RTL8814B_SUPPORT == 1)
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			/*HAL_RF_TXGAPK |*/
			HAL_RF_DPK_TRACK |
			0;
#endif
		break;
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*@HAL_RF_DPK |*/
			0;
		break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_TXGAPK |
			0;
		break;
#endif
#if (RTL8821C_SUPPORT == 1)
	case ODM_RTL8821C:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*@HAL_RF_DPK |*/
			/*@HAL_RF_TXGAPK |*/
			0;
		break;
#endif
#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			/*HAL_RF_TXGAPK |*/
			HAL_RF_DPK_TRACK |
			0;
		break;
#endif
#if (RTL8812F_SUPPORT == 1)
	case ODM_RTL8812F:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_DPK_TRACK |
			0;
		break;
#endif

#if (RTL8198F_SUPPORT == 1)
	case ODM_RTL8198F:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			/*@HAL_RF_TXGAPK |*/
			0;
		break;
#endif

#if (RTL8192F_SUPPORT == 1)
	case ODM_RTL8192F:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			/*@HAL_RF_TXGAPK |*/
#ifdef CONFIG_2G_BAND_SHIFT
			/*@HAL_2GBAND_SHIFT |*/
#endif
			0;
		break;
#endif

#if (RTL8197F_SUPPORT == 1)
	case ODM_RTL8197F:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			/*@HAL_RF_TXGAPK |*/
			0;
		break;
#endif
#if (RTL8197G_SUPPORT == 1)
	case ODM_RTL8197G:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			/*HAL_RF_LCK |*/
			HAL_RF_DPK |
			/*@HAL_RF_TXGAPK |*/
			HAL_RF_DPK_TRACK |
			0;
		break;
#endif
#if (RTL8721D_SUPPORT == 1)
	case ODM_RTL8721D:
		rf->rf_supportability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DPK_TRACK |
			/*@HAL_RF_TXGAPK |*/
			0;
		break;
#endif
#if (RTL8723F_SUPPORT == 1)
	case ODM_RTL8723F:
		rf->rf_supportability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			0;
		break;
#endif
	case ODM_RTL8814C:
#if (RTL8814C_SUPPORT == 1)
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_TXGAPK |
			HAL_RF_DPK_TRACK |
			0;
#endif
	break;

	default:
		rf->rf_supportability =
			/*HAL_RF_TX_PWR_TRACK |*/
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*@HAL_RF_DPK |*/
			/*@HAL_RF_TXGAPK |*/
			0;
		break;
	}

	RF_DBG(dm, DBG_RF_INIT,
	       "IC = ((0x%x)), RF_Supportability Init MP = ((0x%x))\n",
	       dm->support_ic_type, rf->rf_supportability);
}

void halrf_supportability_init(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	switch (dm->support_ic_type) {
	case ODM_RTL8814B:
#if (RTL8814B_SUPPORT == 1)
		rf->rf_supportability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_DPK_TRACK |
			0;
#endif
		break;
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		rf->rf_supportability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*@HAL_RF_DPK |*/
			0;
		break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		rf->rf_supportability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			HAL_RF_DACK |
			HAL_RF_DPK_TRACK |
			HAL_RF_RXDCK |
			HAL_RF_TXGAPK |
			0;
		break;
#endif
#if (RTL8821C_SUPPORT == 1)
	case ODM_RTL8821C:
		rf->rf_supportability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*@HAL_RF_DPK |*/
			/*@HAL_RF_TXGAPK |*/
			0;
		break;
#endif
#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		rf->rf_supportability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			HAL_RF_DPK |
			/*HAL_RF_TXGAPK |*/
			HAL_RF_DPK_TRACK |
			0;
		break;
#endif
#if (RTL8812F_SUPPORT == 1)
		case ODM_RTL8812F:
			rf->rf_supportability =
				HAL_RF_TX_PWR_TRACK |
				HAL_RF_IQK |
				HAL_RF_LCK |
				HAL_RF_DPK |
				HAL_RF_DACK |
				HAL_RF_DPK_TRACK |
				0;
			break;
#endif

#if (RTL8198F_SUPPORT == 1)
		case ODM_RTL8198F:
			rf->rf_supportability =
				HAL_RF_TX_PWR_TRACK |
				HAL_RF_IQK |
				HAL_RF_LCK |
				HAL_RF_DPK |
				/*@HAL_RF_TXGAPK |*/
				0;
			break;
#endif

#if (RTL8192F_SUPPORT == 1)
		case ODM_RTL8192F:
			rf->rf_supportability =
				HAL_RF_TX_PWR_TRACK |
				HAL_RF_IQK |
				HAL_RF_LCK |
				HAL_RF_DPK |
				/*@HAL_RF_TXGAPK |*/
#ifdef CONFIG_2G_BAND_SHIFT
				/*@HAL_2GBAND_SHIFT |*/
#endif
				0;
			break;
#endif

#if (RTL8197F_SUPPORT == 1)
		case ODM_RTL8197F:
			rf->rf_supportability =
				HAL_RF_TX_PWR_TRACK |
				HAL_RF_IQK |
				HAL_RF_LCK |
				HAL_RF_DPK |
				/*@HAL_RF_TXGAPK |*/
				0;
			break;
#endif
#if (RTL8197G_SUPPORT == 1)
		case ODM_RTL8197G:
			rf->rf_supportability =
				HAL_RF_TX_PWR_TRACK |
				HAL_RF_IQK |
				/*HAL_RF_LCK |*/
				HAL_RF_DPK |
				/*@HAL_RF_TXGAPK |*/
				HAL_RF_DPK_TRACK |
#ifdef CONFIG_2G_BAND_SHIFT
				HAL_2GBAND_SHIFT |
#endif
			0;
		break;
#endif
#if (RTL8721D_SUPPORT == 1)
		case ODM_RTL8721D:
			rf->rf_supportability =
				HAL_RF_TX_PWR_TRACK |
				HAL_RF_IQK |
				HAL_RF_LCK |
				HAL_RF_DPK |
				HAL_RF_DPK_TRACK |
				/*@HAL_RF_TXGAPK |*/
				0;
			break;
#endif
#if (RTL8723F_SUPPORT == 1)
		case ODM_RTL8723F:
			rf->rf_supportability =
				HAL_RF_TX_PWR_TRACK |
				HAL_RF_IQK |
				HAL_RF_LCK |
				HAL_RF_DPK |
				HAL_RF_TXGAPK |
				HAL_RF_DPK_TRACK |
				0;
			break;
#endif
		case ODM_RTL8814C:
#if (RTL8814C_SUPPORT == 1)
			rf->rf_supportability =
				HAL_RF_TX_PWR_TRACK |
				HAL_RF_IQK |
				HAL_RF_LCK |
				HAL_RF_DPK |
				HAL_RF_DACK |
				HAL_RF_DPK_TRACK |
				HAL_RF_TXGAPK |
				0;
#endif
			break;

	default:
		rf->rf_supportability =
			HAL_RF_TX_PWR_TRACK |
			HAL_RF_IQK |
			HAL_RF_LCK |
			/*@HAL_RF_DPK |*/
			0;
		break;
	}

	RF_DBG(dm, DBG_RF_INIT,
	       "IC = ((0x%x)), RF_Supportability Init = ((0x%x))\n",
	       dm->support_ic_type, rf->rf_supportability);
}

void halrf_watchdog(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
#if 0
	/*RF_DBG(dm, DBG_RF_TMP, "%s\n", __func__);*/
#endif
	if (rf->is_dpk_in_progress || dm->rf_calibrate_info.is_iqk_in_progress ||
		rf->is_tssi_in_progress)
		return;

	if (!(dm->support_ic_type & ODM_RTL8723F))
		phydm_rf_watchdog(dm);

	halrf_dpk_track(dm);
#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8723F){
		halrf_xtal_thermal_track(dm);
		halrf_powertracking_thermal(dm);
	}
#endif
}

#if 0
void
halrf_iqk_init(
	void			*dm_void
)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	switch (dm->support_ic_type) {
#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		break;
#endif
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
		_iq_calibrate_8822b_init(dm);
		break;
#endif
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		_iq_calibrate_8822c_init(dm);
		break;
#endif
#if (RTL8821C_SUPPORT == 1)
	case ODM_RTL8821C:
		break;
#endif

	default:
		break;
	}
}
#endif

void halrf_rfk_power_save(void *dm_void, boolean is_power_save)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:		
		halrf_rfk_power_save_8822c(dm, is_power_save);
	break;
#endif

#if (RTL8723F_SUPPORT == 1)
	case ODM_RTL8723F:
		halrf_rfk_power_save_8723f(dm, is_power_save);
	break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
		break;
#endif

	default:
	break;
	}
}



void halrf_reload_iqk(void *dm_void, boolean reset)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	u8 i, ch;
	u32 tmp;
	u32 bit_mask_20_16 = BIT(20) | BIT(19) | BIT(18) | BIT(17) | BIT(16);

	halrf_rfk_power_save(dm, false);
	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		iqk_reload_iqk_8822c(dm, reset);
	break;
#endif
#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		iqk_reload_iqk_8195b(dm, reset);
	break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		iqk_reload_iqk_8814c(dm, reset);
	break;
#endif

	default:
	break;
	}
	halrf_rfk_power_save(dm, true);
}

void halrf_rfk_handshake(void *dm_void, boolean is_before_k)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (!dm->mp_mode)
		return;

	if (*dm->mp_mode)
		return;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
		case ODM_RTL8822C:
			halrf_rfk_handshake_8822c(dm, is_before_k);
			break;
#endif
#if (RTL8710C_SUPPORT == 1)
		case ODM_RTL8710C:
			halrf_rfk_handshake_8710c(dm, is_before_k);
			break;
#endif
#if (RTL8723F_SUPPORT == 1)
		case ODM_RTL8723F:
			halrf_rfk_handshake_8723f(dm, is_before_k);
			break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
			break;
#endif

		default:
			break;
	}
}

void halrf_bbreset(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;


	switch (dm->support_ic_type) {
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			phydm_bb_reset_8814b(dm);
			break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
			phydm_bb_reset_8814c(dm);
			break;
#endif

		default:
			break;
	}
}

void halrf_rf_k_connect_trigger(void *dm_void, boolean is_recovery,
				enum halrf_k_segment_time seg_time)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;	
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (!dm->mp_mode)
		return;

	if (dm->mp_mode && rf->is_con_tx && rf->is_single_tone &&
		rf->is_carrier_suppresion) {
		if (*dm->mp_mode & 
			(*rf->is_con_tx || *rf->is_single_tone ||
			*rf->is_carrier_suppresion))
			return;
	}
#if (RTL8723F_SUPPORT == 1)
	if(dm->support_ic_type == ODM_RTL8723F)
		halrf_dis_cca_8723f(dm, true);
#endif
	/*[TX GAP K]*/
	halrf_txgapk_trigger(dm);

	/*[LOK, IQK]*/
	halrf_segment_iqk_trigger(dm, true, seg_time);

	/*[TSSI Trk]*/
	halrf_tssi_trigger(dm);
	/*[DPK]*/
#if 1
	if(dpk_info->is_dpk_by_channel == true)
		halrf_dpk_trigger(dm);
	else
		halrf_dpk_reload(dm);
#endif
	//ADDA restore to MP_UI setting;
	config_halrf_path_adda_setting_trigger(dm);

	halrf_spur_compensation(dm);

	halrf_bbreset(dm);
#if (RTL8723F_SUPPORT == 1)
	if(dm->support_ic_type == ODM_RTL8723F)
		halrf_dis_cca_8723f(dm, false);
#endif
}

void config_halrf_path_adda_setting_trigger(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	
#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		config_phydm_path_adda_setting_8814b(dm);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		config_phydm_path_adda_setting_8814c(dm);
#endif

	
}

void halrf_dack_restore(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (!(rf->rf_supportability & HAL_RF_DACK))
		return;
	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		halrf_dack_restore_8822c(dm);
		break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		//halrf_dack_restore_8814c(dm);
	break;
#endif

	default:
		break;
	}
}
void halrf_dack_trigger(void *dm_void, boolean force)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	u64 start_time;

	if (!(rf->rf_supportability & HAL_RF_DACK))
		return;

	start_time = odm_get_current_time(dm);

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		halrf_dac_cal_8822c(dm, force);
		break;
#endif
#if (RTL8812F_SUPPORT == 1)
	case ODM_RTL8812F:
		halrf_dac_cal_8812f(dm);
		break;
#endif
#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		halrf_dac_cal_8814b(dm);
		break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		halrf_dac_cal_8814c(dm);
	break;
#endif

	default:
		break;
	}
	rf->dpk_progressing_time = odm_get_progressing_time(dm, start_time);
	RF_DBG(dm, DBG_RF_DACK, "[DACK]DACK progressing_time = %lld ms\n",
	       rf->dpk_progressing_time);
}


void halrf_dack_dbg(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	u64 start_time;

	if (!(rf->rf_supportability & HAL_RF_DACK))
		return;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		halrf_dack_dbg_8822c(dm);
		break;
#endif
	default:
		break;
	}
}


void halrf_segment_iqk_trigger(void *dm_void, boolean clear,
			       boolean segment_iqk)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	struct _hal_rf_ *rf = &dm->rf_table;
	u64 start_time;

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
	if (odm_check_power_status(dm) == false)
		return;
#endif

	if (!dm->mp_mode)
		return;

	if (dm->mp_mode && rf->is_con_tx && rf->is_single_tone &&
		rf->is_carrier_suppresion) {
		if (*dm->mp_mode & 
			(*rf->is_con_tx || *rf->is_single_tone ||
			*rf->is_carrier_suppresion))
			return;
	}

	if (!(rf->rf_supportability & HAL_RF_IQK))
		return;

#if DISABLE_BB_RF
	return;
#endif
	if (iqk_info->rfk_forbidden)
		return;

	rf->rfk_type = RF01_IQK;
	halrf_rfk_handshake(dm, true);

	if (!dm->rf_calibrate_info.is_iqk_in_progress) {
		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		dm->rf_calibrate_info.is_iqk_in_progress = true;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);
		start_time = odm_get_current_time(dm);
		dm->IQK_info.segment_iqk = segment_iqk;

		halrf_rfk_power_save(dm, false);
		switch (dm->support_ic_type) {
#if (RTL8822B_SUPPORT == 1)
		case ODM_RTL8822B:
			phy_iq_calibrate_8822b(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8822C_SUPPORT == 1)
		case ODM_RTL8822C:
			phy_iq_calibrate_8822c(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8821C_SUPPORT == 1)
		case ODM_RTL8821C:
			phy_iq_calibrate_8821c(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			phy_iq_calibrate_8814b(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8195B_SUPPORT == 1)
		case ODM_RTL8195B:
			phy_iq_calibrate_8195b(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8710C_SUPPORT == 1)
		case ODM_RTL8710C:
			phy_iq_calibrate_8710c(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8198F_SUPPORT == 1)
		case ODM_RTL8198F:
			phy_iq_calibrate_8198f(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8812F_SUPPORT == 1)
		case ODM_RTL8812F:
			phy_iq_calibrate_8812f(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8197G_SUPPORT == 1)
		case ODM_RTL8197G:
			phy_iq_calibrate_8197g(dm, clear, segment_iqk);
			break;
#endif
#if (RTL8188E_SUPPORT == 1)
		case ODM_RTL8188E:
			phy_iq_calibrate_8188e(dm, false);
			break;
#endif
#if (RTL8188F_SUPPORT == 1)
		case ODM_RTL8188F:
			phy_iq_calibrate_8188f(dm, false);
			break;
#endif
#if (RTL8192E_SUPPORT == 1)
		case ODM_RTL8192E:
			phy_iq_calibrate_8192e(dm, false);
			break;
#endif
#if (RTL8197F_SUPPORT == 1)
		case ODM_RTL8197F:
			phy_iq_calibrate_8197f(dm, false);
			break;
#endif
#if (RTL8192F_SUPPORT == 1)
		case ODM_RTL8192F:
			phy_iq_calibrate_8192f(dm, false);
			break;
#endif
#if (RTL8703B_SUPPORT == 1)
		case ODM_RTL8703B:
			phy_iq_calibrate_8703b(dm, false);
			break;
#endif
#if (RTL8710B_SUPPORT == 1)
		case ODM_RTL8710B:
			phy_iq_calibrate_8710b(dm, false);
			break;
#endif
#if (RTL8723B_SUPPORT == 1)
		case ODM_RTL8723B:
			phy_iq_calibrate_8723b(dm, false);
			break;
#endif
#if (RTL8723D_SUPPORT == 1)
		case ODM_RTL8723D:
			phy_iq_calibrate_8723d(dm, false);
			break;
#endif
#if (RTL8721D_SUPPORT == 1)
		case ODM_RTL8721D:
			phy_iq_calibrate_8721d(dm, false);
			break;
#endif
#if (RTL8812A_SUPPORT == 1)
		case ODM_RTL8812:
			phy_iq_calibrate_8812a(dm, false);
			break;
#endif
#if (RTL8821A_SUPPORT == 1)
		case ODM_RTL8821:
			phy_iq_calibrate_8821a(dm, false);
			break;
#endif
#if (RTL8814A_SUPPORT == 1)
		case ODM_RTL8814A:
			phy_iq_calibrate_8814a(dm, false);
			break;
#endif
#if (RTL8723F_SUPPORT == 1)
		case ODM_RTL8723F:
			phy_iq_calibrate_8723f(dm, false);
			break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
			phy_iq_calibrate_8814c(dm, clear, segment_iqk);
			break;
#endif


		default:
			break;
		}
		
		halrf_rfk_power_save(dm, true);
		dm->rf_calibrate_info.iqk_progressing_time =
				odm_get_progressing_time(dm, start_time);
		RF_DBG(dm, DBG_RF_IQK, "[IQK]IQK progressing_time = %lld ms\n",
		       dm->rf_calibrate_info.iqk_progressing_time);

		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		dm->rf_calibrate_info.is_iqk_in_progress = false;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);

		halrf_rfk_handshake(dm, false);
	} else {
		RF_DBG(dm, DBG_RF_IQK,
		       "== Return the IQK CMD, because RFKs in Progress ==\n");
	}
}


void halrf_iqk_trigger(void *dm_void, boolean is_recovery)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &dm->rf_table;
	u64 start_time;

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
	if (odm_check_power_status(dm) == false)
		return;
#endif

	if (!dm->mp_mode)
		return;

	if (dm->mp_mode && rf->is_con_tx && rf->is_single_tone &&
		rf->is_carrier_suppresion) {
		if (*dm->mp_mode & 
			(*rf->is_con_tx || *rf->is_single_tone ||
			*rf->is_carrier_suppresion))
			return;
	}

	if (!(rf->rf_supportability & HAL_RF_IQK))
		return;

#if DISABLE_BB_RF
	return;
#endif

	if (iqk_info->rfk_forbidden)
		return;

	rf->rfk_type = RF01_IQK;
	halrf_rfk_handshake(dm, true);

	if (!dm->rf_calibrate_info.is_iqk_in_progress) {
		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		dm->rf_calibrate_info.is_iqk_in_progress = true;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);
		start_time = odm_get_current_time(dm);
		halrf_rfk_power_save(dm, false);
		switch (dm->support_ic_type) {
#if (RTL8188E_SUPPORT == 1)
		case ODM_RTL8188E:
			phy_iq_calibrate_8188e(dm, is_recovery);
			break;
#endif
#if (RTL8188F_SUPPORT == 1)
		case ODM_RTL8188F:
			phy_iq_calibrate_8188f(dm, is_recovery);
			break;
#endif
#if (RTL8192E_SUPPORT == 1)
		case ODM_RTL8192E:
			phy_iq_calibrate_8192e(dm, is_recovery);
			break;
#endif
#if (RTL8197F_SUPPORT == 1)
		case ODM_RTL8197F:
			phy_iq_calibrate_8197f(dm, is_recovery);
			break;
#endif
#if (RTL8192F_SUPPORT == 1)
		case ODM_RTL8192F:
			phy_iq_calibrate_8192f(dm, is_recovery);
			break;
#endif
#if (RTL8703B_SUPPORT == 1)
		case ODM_RTL8703B:
			phy_iq_calibrate_8703b(dm, is_recovery);
			break;
#endif
#if (RTL8710B_SUPPORT == 1)
		case ODM_RTL8710B:
			phy_iq_calibrate_8710b(dm, is_recovery);
			break;
#endif
#if (RTL8723B_SUPPORT == 1)
		case ODM_RTL8723B:
			phy_iq_calibrate_8723b(dm, is_recovery);
			break;
#endif
#if (RTL8723D_SUPPORT == 1)
		case ODM_RTL8723D:
			phy_iq_calibrate_8723d(dm, is_recovery);
			break;
#endif
#if (RTL8721D_SUPPORT == 1)
		case ODM_RTL8721D:
			phy_iq_calibrate_8721d(dm, is_recovery);
			break;
#endif
#if (RTL8812A_SUPPORT == 1)
		case ODM_RTL8812:
			phy_iq_calibrate_8812a(dm, is_recovery);
			break;
#endif
#if (RTL8821A_SUPPORT == 1)
		case ODM_RTL8821:
			phy_iq_calibrate_8821a(dm, is_recovery);
			break;
#endif
#if (RTL8814A_SUPPORT == 1)
		case ODM_RTL8814A:
			phy_iq_calibrate_8814a(dm, is_recovery);
			break;
#endif
#if (RTL8822B_SUPPORT == 1)
		case ODM_RTL8822B:
			phy_iq_calibrate_8822b(dm, false, false);
			break;
#endif
#if (RTL8822C_SUPPORT == 1)
		case ODM_RTL8822C:
			phy_iq_calibrate_8822c(dm, false, false);
			break;
#endif
#if (RTL8821C_SUPPORT == 1)
		case ODM_RTL8821C:
			phy_iq_calibrate_8821c(dm, false, false);
			break;
#endif
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			phy_iq_calibrate_8814b(dm, false, false);
			break;
#endif
#if (RTL8195B_SUPPORT == 1)
		case ODM_RTL8195B:
			phy_iq_calibrate_8195b(dm, false, false);
			break;
#endif
#if (RTL8710C_SUPPORT == 1)
		case ODM_RTL8710C:
			phy_iq_calibrate_8710c(dm, false, false);
			break;
#endif
#if (RTL8198F_SUPPORT == 1)
		case ODM_RTL8198F:
			phy_iq_calibrate_8198f(dm, false, false);
			break;
#endif
#if (RTL8812F_SUPPORT == 1)
		case ODM_RTL8812F:
			phy_iq_calibrate_8812f(dm, false, false);
			break;
#endif
#if (RTL8197G_SUPPORT == 1)
		case ODM_RTL8197G:
			phy_iq_calibrate_8197g(dm, false, false);
			break;
#endif
#if (RTL8723F_SUPPORT == 1)
		case ODM_RTL8723F:
			phy_iq_calibrate_8723f(dm, is_recovery);
			break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
			phy_iq_calibrate_8814c(dm, false, false);
			break;
#endif

		default:
			break;
		}

	halrf_rfk_power_save(dm, true);
	rf->iqk_progressing_time = odm_get_progressing_time(dm, start_time);
	RF_DBG(dm, DBG_RF_LCK, "[IQK]Trigger IQK progressing_time = %lld ms\n",
	       rf->iqk_progressing_time);
		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		dm->rf_calibrate_info.is_iqk_in_progress = false;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);

		halrf_rfk_handshake(dm, false);
	} else {
		RF_DBG(dm, DBG_RF_IQK,
		       "== Return the IQK CMD, because RFKs in Progress ==\n");
	}
}

void halrf_lck_trigger(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	struct _hal_rf_ *rf = &dm->rf_table;
	u64 start_time;

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
	if (odm_check_power_status(dm) == false)
		return;
#endif

	if (!dm->mp_mode)
		return;

	if (dm->mp_mode && rf->is_con_tx && rf->is_single_tone &&
		rf->is_carrier_suppresion) {
		if (*dm->mp_mode & 
			(*rf->is_con_tx || *rf->is_single_tone ||
			*rf->is_carrier_suppresion))
			return;
	}

	if (!(rf->rf_supportability & HAL_RF_LCK))
		return;

#if DISABLE_BB_RF
	return;
#endif
	if (iqk_info->rfk_forbidden)
		return;
	while (*dm->is_scan_in_process) {
		RF_DBG(dm, DBG_RF_LCK, "[LCK]scan is in process, bypass LCK\n");
		return;
	}

	if (!dm->rf_calibrate_info.is_lck_in_progress) {
		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		dm->rf_calibrate_info.is_lck_in_progress = true;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);
		start_time = odm_get_current_time(dm);
		switch (dm->support_ic_type) {
#if (RTL8188E_SUPPORT == 1)
		case ODM_RTL8188E:
			phy_lc_calibrate_8188e(dm);
			break;
#endif
#if (RTL8188F_SUPPORT == 1)
		case ODM_RTL8188F:
			phy_lc_calibrate_8188f(dm);
			break;
#endif
#if (RTL8192E_SUPPORT == 1)
		case ODM_RTL8192E:
			phy_lc_calibrate_8192e(dm);
			break;
#endif
#if (RTL8197F_SUPPORT == 1)
		case ODM_RTL8197F:
			phy_lc_calibrate_8197f(dm);
			break;
#endif
#if (RTL8192F_SUPPORT == 1)
		case ODM_RTL8192F:
			phy_lc_calibrate_8192f(dm);
			break;
#endif
#if (RTL8703B_SUPPORT == 1)
		case ODM_RTL8703B:
			phy_lc_calibrate_8703b(dm);
			break;
#endif
#if (RTL8710B_SUPPORT == 1)
		case ODM_RTL8710B:
			phy_lc_calibrate_8710b(dm);
			break;
#endif
#if (RTL8721D_SUPPORT == 1)
		case ODM_RTL8721D:
			phy_lc_calibrate_8721d(dm);
			break;
#endif
#if (RTL8723B_SUPPORT == 1)
		case ODM_RTL8723B:
			phy_lc_calibrate_8723b(dm);
			break;
#endif
#if (RTL8723D_SUPPORT == 1)
		case ODM_RTL8723D:
			phy_lc_calibrate_8723d(dm);
			break;
#endif
#if (RTL8812A_SUPPORT == 1)
		case ODM_RTL8812:
			phy_lc_calibrate_8812a(dm);
			break;
#endif
#if (RTL8821A_SUPPORT == 1)
		case ODM_RTL8821:
			phy_lc_calibrate_8821a(dm);
			break;
#endif
#if (RTL8814A_SUPPORT == 1)
		case ODM_RTL8814A:
			phy_lc_calibrate_8814a(dm);
			break;
#endif
#if (RTL8822B_SUPPORT == 1)
		case ODM_RTL8822B:
			phy_lc_calibrate_8822b(dm);
			break;
#endif
#if (RTL8822C_SUPPORT == 1)
		case ODM_RTL8822C:
			phy_lc_calibrate_8822c(dm);
			break;
#endif
#if (RTL8812F_SUPPORT == 1)
		case ODM_RTL8812F:
			phy_lc_calibrate_8812f(dm);
			break;
#endif
#if (RTL8821C_SUPPORT == 1)
		case ODM_RTL8821C:
			phy_lc_calibrate_8821c(dm);
			break;
#endif
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			phy_lc_calibrate_8814b(dm);
			break;
#endif
#if (RTL8197G_SUPPORT == 1)
		case ODM_RTL8197G:
			phy_lc_calibrate_8197g(dm);
			break;
#endif
#if (RTL8198F_SUPPORT == 1)
		case ODM_RTL8198F:
			phy_lc_calibrate_8198f(dm);
			break;
#endif
#if (RTL8710C_SUPPORT == 1)
		case ODM_RTL8710C:
			phy_lc_calibrate_8710c(dm);
			break;
#endif
#if (RTL8723F_SUPPORT == 1)
		case ODM_RTL8723F:
			phy_lc_calibrate_8723f(dm);
			break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
			phy_lc_calibrate_8814c(dm);
			break;
#endif


		default:
			break;
		}
		dm->rf_calibrate_info.lck_progressing_time =
				odm_get_progressing_time(dm, start_time);
		RF_DBG(dm, DBG_RF_LCK, "[LCK]LCK progressing_time = %lld ms\n",
		       dm->rf_calibrate_info.lck_progressing_time);
#if (RTL8822B_SUPPORT == 1 || RTL8821C_SUPPORT == 1)
		halrf_lck_dbg(dm);
#endif
		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		dm->rf_calibrate_info.is_lck_in_progress = false;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);
	} else {
		RF_DBG(dm, DBG_RF_LCK,
		       "[LCK]= Return the LCK CMD, because RFK is in Progress =\n");
	}
}

void halrf_aac_check(struct dm_struct *dm)
{
	switch (dm->support_ic_type) {
#if (RTL8821C_SUPPORT == 1)
	case ODM_RTL8821C:
#if 0
		aac_check_8821c(dm);
#endif
		break;
#endif
#if (RTL8822B_SUPPORT == 1)
	case ODM_RTL8822B:
#if 1
		aac_check_8822b(dm);
#endif
		break;
#endif
	default:
		break;
	}
}

void halrf_rxdck(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (!(rf->rf_supportability & HAL_RF_RXDCK))
		return;

	switch (dm->support_ic_type) {
	case ODM_RTL8822C:
#if (RTL8822C_SUPPORT == 1)
		halrf_rxdck_8822c(dm);
		break;
#endif
	default:
		break;
	}
}

void halrf_x2k_check(struct dm_struct *dm)
{

	switch (dm->support_ic_type) {
	case ODM_RTL8821C:
#if (RTL8821C_SUPPORT == 1)
#endif
		break;
	case ODM_RTL8822C:
#if (RTL8822C_SUPPORT == 1)
		phy_x2_check_8822c(dm);
		break;
#endif
	case ODM_RTL8812F:
#if (RTL8812F_SUPPORT == 1)
		phy_x2_check_8812f(dm);
		break;
#endif
	case ODM_RTL8723F:
#if (RTL8723F_SUPPORT == 1)
		phy_x2_check_8723f(dm);
		break;
#endif

	default:
		break;
	}
}

void halrf_set_rfsupportability(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (!dm->mp_mode)
		return;

	if (rf->manual_rf_supportability &&
	    *rf->manual_rf_supportability != 0xffffffff) {
		rf->rf_supportability = *rf->manual_rf_supportability;
	} else if (*dm->mp_mode) {
		halrf_supportability_init_mp(dm);
	} else {
		halrf_supportability_init(dm);
	}
}

void halrf_rfe_definition(struct dm_struct *dm)
{
	struct _hal_rf_ *rf = &dm->rf_table;

	switch (dm->support_ic_type) {
	case ODM_RTL8822C:
#if (RTL8822C_SUPPORT == 1)
		if (dm->rfe_type == 21 || dm->rfe_type == 22) {
			rf->ext_pa_5g = 1;
			rf->ext_lna_5g = 1;
			}
		break;
#endif
	default:
		break;
	}
}

void halrf_init(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	RF_DBG(dm, DBG_RF_INIT, "HALRF_Init\n");
	rf->aac_checked = false;
	halrf_init_debug_setting(dm);
	halrf_set_rfsupportability(dm);
	halrf_rfe_definition(dm);
#if 1
	/*Init all RF funciton*/
	halrf_aac_check(dm);
	halrf_dack_trigger(dm, false);
	halrf_x2k_check(dm);
#endif

	/*power trim, thrmal trim, pa bias*/
	phydm_config_new_kfree(dm);

	/*TSSI Init*/
	halrf_tssi_dck(dm, true);
	halrf_tssi_get_efuse(dm);
	halrf_tssi_set_de(dm);
#if (RTL8723F_SUPPORT == 1)
	halrf_do_tssi(dm);
	halrf_rx_port_ctl_8723f(dm);
#endif

	/*TX Gap K*/
	halrf_txgapk_write_gain_table(dm);
}

void halrf_dpk_trigger(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	u64 start_time;

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN))
	if (odm_check_power_status(dm) == false)
		return;
#endif

	if (!dm->mp_mode)
		return;

	if (dm->mp_mode && rf->is_con_tx && rf->is_single_tone &&
		rf->is_carrier_suppresion) {
		if (*dm->mp_mode &
			(*rf->is_con_tx || *rf->is_single_tone ||
			*rf->is_carrier_suppresion))
			return;
	}

	if (!(rf->rf_supportability & HAL_RF_DPK))
		return;

#if DISABLE_BB_RF
	return;
#endif

	if (iqk_info->rfk_forbidden)
		return;

	rf->rfk_type = RF03_DPK;
	halrf_rfk_handshake(dm, true);

	if (!rf->is_dpk_in_progress) {
		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		rf->is_dpk_in_progress = true;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);
		start_time = odm_get_current_time(dm);
		halrf_rfk_power_save(dm, false);
		switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
		case ODM_RTL8822C:
			do_dpk_8822c(dm);
		break;
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
			do_dpk_8814c(dm);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
#if (RTL8197F_SUPPORT == 1)
		case ODM_RTL8197F:
			do_dpk_8197f(dm);
			break;
#endif
#if (RTL8192F_SUPPORT == 1)
		case ODM_RTL8192F:
			do_dpk_8192f(dm);
			break;
#endif

#if (RTL8198F_SUPPORT == 1)
		case ODM_RTL8198F:
			do_dpk_8198f(dm);
			break;
#endif
#if (RTL8812F_SUPPORT == 1)
		case ODM_RTL8812F:
			do_dpk_8812f(dm);
			break;
#endif
#if (RTL8197G_SUPPORT == 1)
		case ODM_RTL8197G:
			do_dpk_8197g(dm);
			break;
#endif

#endif

#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			do_dpk_8814b(dm);
			break;
#endif
#if (RTL8723F_SUPPORT == 1)
		case ODM_RTL8723F:
			do_dpk_8723f(dm);
			break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_IOT))
#if (RTL8195B_SUPPORT == 1)
		case ODM_RTL8195B:
			do_dpk_8195b(dm);
		break;
#endif
#if (RTL8721D_SUPPORT == 1)
		case ODM_RTL8721D:
			do_dpk_8721d(dm);
			break;
#endif

#endif
		default:
			break;
	}
	halrf_rfk_power_save(dm, true);
	rf->dpk_progressing_time = odm_get_progressing_time(dm, start_time);
	RF_DBG(dm, DBG_RF_DPK, "[DPK]DPK progressing_time = %lld ms\n",
	       rf->dpk_progressing_time);

		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		rf->is_dpk_in_progress = false;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);

		halrf_rfk_handshake(dm, false);
	} else {
		RF_DBG(dm, DBG_RF_DPK,
		       "== Return the DPK CMD, because RFKs in Progress ==\n");
	}
}

void halrf_set_dpkbychannel(void *dm_void, boolean dpk_by_ch)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	
	switch (dm->support_ic_type) {
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			dpk_set_dpkbychannel_8814b(dm, dpk_by_ch);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_IOT))
#if (RTL8195B_SUPPORT == 1)
		case ODM_RTL8195B:
			dpk_set_dpkbychannel_8195b(dm,dpk_by_ch);
		break;
#endif
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
			dpk_set_dpkbychannel_8814c(dm, dpk_by_ch);
		break;
#endif

		default:
			if (dpk_by_ch)
				dpk_info->is_dpk_by_channel = 1;
			else
				dpk_info->is_dpk_by_channel = 0;
		break;
	}

}

void halrf_set_dpkenable(void *dm_void, boolean is_dpk_enable)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;

	
	switch (dm->support_ic_type) {
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			dpk_set_is_dpk_enable_8814b(dm, is_dpk_enable);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_IOT))
#if (RTL8195B_SUPPORT == 1)
		case ODM_RTL8195B:
			dpk_set_is_dpk_enable_8195b(dm, is_dpk_enable);
	break;
#endif

#if (RTL8721D_SUPPORT == 1)
	case ODM_RTL8721D:
		dpk_set_is_dpk_enable_8721d(dm, is_dpk_enable);
	break;
#endif

#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		dpk_set_is_dpk_enable_8814c(dm, is_dpk_enable);
		break;
#endif

	default:
	break;
	}

}
boolean halrf_get_dpkbychannel(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	boolean is_dpk_by_channel = true;
	
	switch (dm->support_ic_type) {
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			is_dpk_by_channel = dpk_get_dpkbychannel_8814b(dm);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_IOT))
#if (RTL8195B_SUPPORT == 1)
		case ODM_RTL8195B:
			is_dpk_by_channel = dpk_get_dpkbychannel_8195b(dm);
		break;
#endif
#endif

#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		is_dpk_by_channel = dpk_get_dpkbychannel_8814c(dm);
	break;
#endif


	default:
	break;
	}
	return is_dpk_by_channel;

}


boolean halrf_get_dpkenable(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct dm_iqk_info *iqk_info = &dm->IQK_info;
	boolean is_dpk_enable = true;

	
	switch (dm->support_ic_type) {
#if (RTL8814B_SUPPORT == 1)
		case ODM_RTL8814B:
			is_dpk_enable = dpk_get_is_dpk_enable_8814b(dm);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_IOT))
#if (RTL8195B_SUPPORT == 1)
		case ODM_RTL8195B:
			is_dpk_enable = dpk_get_is_dpk_enable_8195b(dm);
		break;
#endif
#endif
#if (RTL8814C_SUPPORT == 1)
		case ODM_RTL8814C:
			is_dpk_enable = dpk_get_is_dpk_enable_8814c(dm);
		break;
#endif

		default:
		break;
	}
	return is_dpk_enable;

}

u8 halrf_dpk_result_check(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	u8 result = 0;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		if (dpk_info->dpk_path_ok == 0x3)
			result = 1;
		else
			result = 0;
		break;
#endif

#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		if (dpk_info->dpk_path_ok == 0x1)
			result = 1;
		else
			result = 0;
		break;
#endif

#if (RTL8721D_SUPPORT == 1)
	case ODM_RTL8721D:
		if (dpk_info->dpk_path_ok == 0x1)
			result = 1;
		else
			result = 0;
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#if (RTL8197F_SUPPORT == 1)
	case ODM_RTL8197F:
		if (dpk_info->dpk_path_ok == 0x3)
			result = 1;
		else
			result = 0;
		break;
#endif

#if (RTL8192F_SUPPORT == 1)
	case ODM_RTL8192F:
		if (dpk_info->dpk_path_ok == 0x3)
			result = 1;
		else
			result = 0;
		break;
#endif

#if (RTL8198F_SUPPORT == 1)
	case ODM_RTL8198F:
		if (dpk_info->dpk_path_ok == 0xf)
			result = 1;
		else
			result = 0;
		break;
#endif

#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		if (dpk_info->dpk_path_ok == 0xf)
			result = 1;
		else
			result = 0;
		break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		if (dpk_info->dpk_path_ok == 0xf)
			result = 1;
		else
			result = 0;
		break;
#endif


#if (RTL8812F_SUPPORT == 1)
	case ODM_RTL8812F:
		if (dpk_info->dpk_path_ok == 0x3)
			result = 1;
		else
			result = 0;
		break;
#endif

#if (RTL8197G_SUPPORT == 1)
	case ODM_RTL8197G:
		if (dpk_info->dpk_path_ok == 0x3)
			result = 1;
		else
			result = 0;
		break;
#endif

#endif
	default:
		break;
	}
	return result;
}

void halrf_dpk_sram_read(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u8 path, group;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		dpk_coef_read_8822c(dm);
		break;
#endif

#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		dpk_sram_read_8195b(dm);
		break;
#endif

#if (RTL8721D_SUPPORT == 1)
	case ODM_RTL8721D:
		dpk_sram_read_8721d(dm);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#if (RTL8197F_SUPPORT == 1)
	case ODM_RTL8197F:
		dpk_sram_read_8197f(dm);
		break;
#endif

#if (RTL8192F_SUPPORT == 1)
	case ODM_RTL8192F:
		dpk_sram_read_8192f(dm);
		break;
#endif

#if (RTL8198F_SUPPORT == 1)
	case ODM_RTL8198F:
		dpk_sram_read_8198f(dm);
		break;
#endif

#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		dpk_sram_read_8814b(dm);
		break;
#endif

#if (RTL8812F_SUPPORT == 1)
	case ODM_RTL8812F:
		dpk_coef_read_8812f(dm);
		break;
#endif

#if (RTL8197G_SUPPORT == 1)
	case ODM_RTL8197G:
		dpk_sram_read_8197g(dm);
		break;
#endif

#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		dpk_sram_read_8814c(dm);
		break;
#endif

#endif

	default:
		break;
	}
}

void halrf_dpk_enable_disable(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (!(rf->rf_supportability & HAL_RF_DPK))
		return;

	if (!rf->is_dpk_in_progress) {
		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		rf->is_dpk_in_progress = true;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		dpk_enable_disable_8822c(dm);
		break;
#endif
#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		dpk_enable_disable_8195b(dm);
		break;
#endif
#if (RTL8721D_SUPPORT == 1)
		case ODM_RTL8721D:
			phy_dpk_enable_disable_8721d(dm);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#if (RTL8197F_SUPPORT == 1)
	case ODM_RTL8197F:
		phy_dpk_enable_disable_8197f(dm);
		break;
#endif
#if (RTL8192F_SUPPORT == 1)
	case ODM_RTL8192F:
		phy_dpk_enable_disable_8192f(dm);
		break;
#endif

#if (RTL8198F_SUPPORT == 1)
	case ODM_RTL8198F:
		dpk_enable_disable_8198f(dm);
		break;
#endif

#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		dpk_enable_disable_8814b(dm);
		break;
#endif

#if (RTL8812F_SUPPORT == 1)
	case ODM_RTL8812F:
		dpk_enable_disable_8812f(dm);
		break;
#endif

#if (RTL8197G_SUPPORT == 1)
	case ODM_RTL8197G:
		dpk_enable_disable_8197g(dm);
		break;
#endif

#if (RTL8723F_SUPPORT == 1)
	case ODM_RTL8723F:
		dpk_enable_disable_8723f(dm);
		break;
#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		dpk_enable_disable_8814c(dm);
		break;
#endif

#endif
	default:
		break;
	}

		odm_acquire_spin_lock(dm, RT_IQK_SPINLOCK);
		rf->is_dpk_in_progress = false;
		odm_release_spin_lock(dm, RT_IQK_SPINLOCK);
	} else {
		RF_DBG(dm, DBG_RF_DPK,
		       "== Return the DPK CMD, because RFKs in Progress ==\n");
	}
}

void halrf_dpk_track(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (rf->is_dpk_in_progress || dm->rf_calibrate_info.is_iqk_in_progress ||
	    dm->is_psd_in_process || (dpk_info->dpk_path_ok == 0) ||
	    !(rf->rf_supportability & HAL_RF_DPK_TRACK) || rf->is_tssi_in_progress
	    || rf->is_txgapk_in_progress)
		return;

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	if (*dm->is_fcs_mode_enable)
		return;
#endif

	switch (dm->support_ic_type) {
#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		dpk_track_8814b(dm);
		break;
#endif

#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		dpk_track_8822c(dm);
		break;
#endif

#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		dpk_track_8195b(dm);
		break;
#endif

#if (RTL8721D_SUPPORT == 1)
	case ODM_RTL8721D:
		phy_dpk_track_8721d(dm);
		break;
#endif

#if (RTL8723F_SUPPORT == 1)
	case ODM_RTL8723F:
		dpk_track_8723f(dm);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#if (RTL8197F_SUPPORT == 1)
	case ODM_RTL8197F:
		phy_dpk_track_8197f(dm);
		break;
#endif

#if (RTL8192F_SUPPORT == 1)
	case ODM_RTL8192F:
		phy_dpk_track_8192f(dm);
		break;
#endif

#if (RTL8198F_SUPPORT == 1)
	case ODM_RTL8198F:
		dpk_track_8198f(dm);
		break;
#endif

#if (RTL8812F_SUPPORT == 1)
	case ODM_RTL8812F:
		dpk_track_8812f(dm);
		break;
#endif

#if (RTL8197G_SUPPORT == 1)
	case ODM_RTL8197G:
		dpk_track_8197g(dm);
		break;
#endif

#endif
#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		dpk_track_8814c(dm);
	break;
#endif

	default:
		break;
	}
}

void halrf_set_dpk_track(void *dm_void, u8 enable)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &(dm->rf_table);

	if (enable)
		rf->rf_supportability = rf->rf_supportability | HAL_RF_DPK_TRACK;
	else
		rf->rf_supportability = rf->rf_supportability & ~HAL_RF_DPK_TRACK;
}

void halrf_dpk_reload(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	switch (dm->support_ic_type) {
#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		if (dpk_info->dpk_path_ok > 0)
			dpk_reload_8195b(dm);
		break;
#endif
#if (RTL8721D_SUPPORT == 1)
	case ODM_RTL8721D:
		if (dpk_info->dpk_path_ok > 0)
			dpk_reload_8721d(dm);
		break;
#endif

#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))

#if (RTL8197F_SUPPORT == 1)
	case ODM_RTL8197F:
		if (dpk_info->dpk_path_ok > 0)
			dpk_reload_8197f(dm);
		break;
#endif

#if (RTL8192F_SUPPORT == 1)
	case ODM_RTL8192F:
		if (dpk_info->dpk_path_ok > 0)
			dpk_reload_8192f(dm);

		break;
#endif

#if (RTL8198F_SUPPORT == 1)
	case ODM_RTL8198F:
		if (dpk_info->dpk_path_ok > 0)
			dpk_reload_8198f(dm);
		break;		
#endif

#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		if (dpk_info->dpk_path_ok > 0)
			dpk_reload_8814b(dm);
		break;		
#endif

#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		if (dpk_info->dpk_path_ok > 0)
			dpk_reload_8814c(dm);
		break;		
#endif


#endif
	default:
		break;
	}
}

void halrf_dpk_switch(void *dm_void, u8 enable)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (enable) {
		rf->rf_supportability = rf->rf_supportability | HAL_RF_DPK;
		dpk_info->is_dpk_enable = true;
		halrf_dpk_enable_disable(dm);
		halrf_dpk_trigger(dm);
		halrf_set_dpk_track(dm, 1);
	} else {
		halrf_set_dpk_track(dm, 0);
		dpk_info->is_dpk_enable = false;
		halrf_dpk_enable_disable(dm);
		rf->rf_supportability = rf->rf_supportability & ~HAL_RF_DPK;
	}
}

void _halrf_dpk_info_by_chip(void *dm_void, u32 *_used, char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u32 used = *_used;
	u32 out_len = *_out_len;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		dpk_info_by_8822c(dm, &used, output, &out_len);
		break;
#endif

#if (RTL8812F_SUPPORT == 1)
	case ODM_RTL8812F:
		dpk_info_by_8812f(dm, &used, output, &out_len);
		break;
#endif

#if (RTL8197G_SUPPORT == 1)
	case ODM_RTL8197G:
		dpk_info_by_8197g(dm, &used, output, &out_len);
		break;
#endif

	default:
		break;
	}

	*_used = used;
	*_out_len = out_len;
}

void _halrf_display_dpk_info(void *dm_void, u32 *_used, char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &(dm->rf_table);

	u32 used = *_used;
	u32 out_len = *_out_len;
	char *ic_name = NULL;
	u8 path;

	switch (dm->support_ic_type) {

#if (RTL8822C_SUPPORT)
	case ODM_RTL8822C:
		ic_name = "8822C";
		break;
#endif

#if (RTL8814B_SUPPORT)
	case ODM_RTL8814B:
		ic_name = "8814B";
		break;
#endif

#if (RTL8812F_SUPPORT)
	case ODM_RTL8812F:
		ic_name = "8812F";
		break;
#endif

#if (RTL8198F_SUPPORT)
	case ODM_RTL8198F:
		ic_name = "8198F";
		break;
#endif

#if (RTL8197F_SUPPORT)
	case ODM_RTL8197F:
		ic_name = "8197F";
		break;
#endif

#if (RTL8192F_SUPPORT)
	case ODM_RTL8192F:
		ic_name = "8192F";
		break;
#endif

#if (RTL8197G_SUPPORT)
	case ODM_RTL8197G:
		ic_name = "8197G";
		break;
#endif

#if (RTL8710B_SUPPORT)
	case ODM_RTL8721D:
		ic_name = "8721D";
		break;
#endif

#if (RTL8195B_SUPPORT)
	case ODM_RTL8195B:
		ic_name = "8195B";
		break;
#endif
#if (RTL8814C_SUPPORT)
	case ODM_RTL8814C:
		ic_name = "8814C";
		break;
#endif
	default:
	break;
	}

	PDM_SNPF(out_len, used, output + used, out_len - used,
		 "\n===============[ DPK info %s ]===============\n", ic_name);

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s %s\n",
		 "DPK type", (dm->fw_offload_ability & PHYDM_RF_DPK_OFFLOAD) ? "FW" : "Driver",
		 (dpk_info->is_dpk_by_channel) ? "(By channel)" : "(By group)");

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %d (%d)\n",
		 "FW Ver (Sub Ver)", dm->fw_version, dm->fw_sub_version);

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s\n",
		 "DPK Ver", HALRF_DPK_VER);

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s\n",
		 "RFK init ver", HALRF_RFK_INIT_VER);

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %d / %d (RFE type:%d)\n",
		 "Ext_PA 2G / 5G", dm->ext_pa, dm->ext_pa_5g, dm->rfe_type);

	if ((dpk_info->dpk_ch == 0) && (dpk_info->thermal_dpk[0] == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used, "\n %-25s\n",
			 "No DPK had been done before!!!");
		return;
	}

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %d / %d / %d\n",
		 "DPK Cal / OK / Reload", dpk_info->dpk_cal_cnt, dpk_info->dpk_ok_cnt,
		 dpk_info->dpk_reload_cnt);

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s\n",
		 "RFK H2C timeout", (rf->is_rfk_h2c_timeout) ? "Yes" : "No");

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s\n",
		 "DPD Reload", (dpk_info->dpk_status & BIT(0)) ? "Yes" : "No");

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s\n",
		 "DPD status", dpk_info->is_dpk_enable ? "Enable" : "Disable");

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s\n",
		 "DPD track status", (rf->rf_supportability & HAL_RF_DPK_TRACK) ? "Enable" : "Disable");

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s / %s / %d / %s\n",
		 "TSSI / Band / CH / BW", dpk_info->is_tssi_mode == 1 ? "On" : "Off",
		 dpk_info->dpk_band == 0 ? "2G" : "5G", dpk_info->dpk_ch,
		 dpk_info->dpk_bw == 3 ? "20M" : (dpk_info->dpk_bw == 2 ? "40M" : "80M"));

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %s / %s / %s / %s\n",
		 "DPK result (path)", dpk_info->dpk_path_ok & BIT(0) ? "OK" : "Fail",
		 (dm->support_ic_type & ODM_IC_2SS) ? ((dpk_info->dpk_path_ok & BIT(1)) >> 1 ? "OK" : "Fail") : "NA",
		 (dm->support_ic_type & ODM_IC_3SS) ? ((dpk_info->dpk_path_ok & BIT(2)) >> 2 ? "OK" : "Fail") : "NA",
		 (dm->support_ic_type & ODM_IC_4SS) ? ((dpk_info->dpk_path_ok & BIT(3)) >> 3 ? "OK" : "Fail") : "NA");
#if 0
	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = %d / %d / %d / %d\n",
		 "DPK thermal (path)", dpk_info->thermal_dpk[0], dpk_info->thermal_dpk[1],
		 dpk_info->thermal_dpk[2], dpk_info->thermal_dpk[3]);
#endif
	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = ",
		 "DPK thermal (path)");
	for (path = 0; path < KPATH; path++) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 path == (KPATH - 1) ? "%d\n" : "%d / ",
			 dpk_info->thermal_dpk[path]);
	}

	PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = 0x%x\n",
		 "DPK bkup GNT control", dpk_info->gnt_control);

		PDM_SNPF(out_len, used, output + used, out_len - used, " %-25s = 0x%x\n",
		 "DPK bkup GNT value", dpk_info->gnt_value);

	_halrf_dpk_info_by_chip(dm, &used, output, &out_len);

	*_used = used;
	*_out_len = out_len;
}

void halrf_dpk_debug_cmd(void *dm_void, char input[][16], u32 *_used,
				char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;
	struct _hal_rf_ *rf = &(dm->rf_table);

	char *cmd[5] = {"-h", "on", "off", "info", "switch"};
	u32 used = *_used;
	u32 out_len = *_out_len;
	u8 i;

	if ((strcmp(input[2], cmd[4]) != 0)) {
		if (!(rf->rf_supportability & HAL_RF_DPK)) {
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "DPK is Unsupported!!!\n");
			return;
		}
	}

	if ((strcmp(input[2], cmd[0]) == 0)) {
		for (i = 1; i < 4; i++) {
			PDM_SNPF(out_len, used, output + used, out_len - used,
				 "  %s\n", cmd[i]);
		}
	} else if ((strcmp(input[2], cmd[1]) == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "DPK is Enabled!!\n");
		dpk_info->is_dpk_enable = true;
		halrf_dpk_enable_disable(dm);
	} else if ((strcmp(input[2], cmd[2]) == 0)){
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "DPK is Disabled!!\n");
		dpk_info->is_dpk_enable = false;
		halrf_dpk_enable_disable(dm);
	} else if ((strcmp(input[2], cmd[3]) == 0))
		_halrf_display_dpk_info(dm, &used, output, &out_len);
	else if ((strcmp(input[2], cmd[4]) == 0) && (strcmp(input[3], cmd[1]) == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "DPK Switch on!!\n");
		halrf_dpk_switch(dm, 1);
	} else if ((strcmp(input[2], cmd[4]) == 0) && (strcmp(input[3], cmd[2]) == 0)) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "DPK Switch off!!\n");
		halrf_dpk_switch(dm, 0);
	} else {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "DPK Trigger start!!\n");
		halrf_dpk_trigger(dm);
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "DPK Trigger finish!!\n");
	}
}

void halrf_dpk_c2h_report_transfer(void	*dm_void, boolean is_ok, u8 *buf, u8 buf_size)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	if (!(rf->rf_supportability & HAL_RF_DPK))
		return;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		dpk_c2h_report_transfer_8822c(dm, is_ok, buf, buf_size);
		break;
#endif
	default:
		break;
	}
}

void halrf_dpk_info_rsvd_page(void *dm_void, u8 *buf, u32 *buf_size)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct dm_dpk_info *dpk_info = &dm->dpk_info;

	if (!(rf->rf_supportability & HAL_RF_DPK) || rf->is_dpk_in_progress)
		return;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		dpk_info_rsvd_page_8822c(dm, buf, buf_size);
		break;
#endif
	default:
		break;
	}
}

void halrf_iqk_info_rsvd_page(void *dm_void, u8 *buf, u32 *buf_size)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	if (!(rf->rf_supportability & HAL_RF_IQK))
		return;

	if (dm->rf_calibrate_info.is_iqk_in_progress)
		return;

	switch (dm->support_ic_type) {
#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		iqk_info_rsvd_page_8822c(dm, buf, buf_size);
		break;
#endif
#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		iqk_info_rsvd_page_8195b(dm, buf, buf_size);
		break;
#endif

	default:
		break;
	}
}

enum hal_status
halrf_config_rfk_with_header_file(void *dm_void, u32 config_type)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	enum hal_status result = HAL_STATUS_SUCCESS;
#if 0
#if (RTL8822B_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8822B) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8822b_cal_init(dm);
	}
#endif
#endif
#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8197G) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8197g_cal_init(dm);
	}
#endif
#if (RTL8198F_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8198F) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8198f_cal_init(dm);
	}
#endif
#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8812F) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8812f_cal_init(dm);
	}
#endif
#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8822C) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8822c_cal_init(dm);
	}
#endif
#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814B) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8814b_cal_init(dm);
	}
#endif
#if (RTL8195B_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8195B) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8195b_cal_init(dm);
	}
#endif
#if (RTL8721D_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8721D) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8721d_cal_init(dm);
	}
#endif
#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8723F) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8723f_cal_init(dm);
	}
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814C) {
		if (config_type == CONFIG_BB_RF_CAL_INIT)
			odm_read_and_config_mp_8814c_cal_init(dm);
	}
#endif


#if 1
	if (dm->fw_offload_ability & PHYDM_PHY_PARAM_OFFLOAD) {
		result = phydm_set_reg_by_fw(dm, PHYDM_HALMAC_CMD_END, 0, 0, 0, (enum rf_path)0, 0);
		RF_DBG(dm, DBG_RF_IQK,"phy param offload end!result = %d", result);
	}
#endif
	return result;
}

void halrf_txgapk_trigger(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	u64 start_time = 0x0;

	if (!(rf->rf_supportability & HAL_RF_TXGAPK))
		return;

	rf->rfk_type = RF04_TXGAPK;
	halrf_rfk_handshake(dm, true);

	start_time = odm_get_current_time(dm);
	rf->is_txgapk_in_progress = true;
	halrf_rfk_power_save(dm, false);

	switch (dm->support_ic_type) {
		
#if (DM_ODM_SUPPORT_TYPE & (ODM_IOT))
#if (RTL8195B_SUPPORT == 1)
	case ODM_RTL8195B:
		/*phy_txgap_calibrate_8195b(dm, false);*/
	break;
#endif
#if (RTL8721D_SUPPORT == 1)
	case ODM_RTL8721D:
		/*phy_txgap_calibrate_8721d(dm, false);*/
	break;
#endif

#endif

#if (RTL8814B_SUPPORT == 1)
	case ODM_RTL8814B:
		/*phy_txgap_calibrate_8814b(dm, false);*/
	break;
#endif

#if (RTL8814C_SUPPORT == 1)
	case ODM_RTL8814C:
		halrf_txgapk_8814c(dm);
	break;
#endif


#if (RTL8822C_SUPPORT == 1)
	case ODM_RTL8822C:
		halrf_txgapk_8822c(dm);
	break;
#endif

#if (RTL8723F_SUPPORT == 1)
	case ODM_RTL8723F:
		halrf_txgapk_8723f(dm);
	break;
#endif

	default:
		break;
	}	
	halrf_rfk_power_save(dm, true);
	rf->is_txgapk_in_progress = false;

	halrf_rfk_handshake(dm, false);

	rf->dpk_progressing_time =
		odm_get_progressing_time(dm_void, start_time);
	RF_DBG(dm, DBG_RF_TXGAPK, "[TGGC]TXGAPK progressing_time = %lld ms\n",
	       rf->dpk_progressing_time);
}

void halrf_spur_compensation(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	switch (dm->support_ic_type) {
#if (RTL8723F_SUPPORT == 1)
	case ODM_RTL8723F:
		halrf_rfk_power_save(dm, false);
		halrf_spur_compensation_8723f(dm);
		halrf_rfk_power_save(dm, true);
		break;
#endif
	default:
		break;
	}
}

void halrf_tssi_get_efuse(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	
#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C) {
		halrf_tssi_get_efuse_8822c(dm);
		halrf_get_efuse_thermal_pwrtype_8822c(dm);
	}
#endif

#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8812F) {
		halrf_tssi_get_efuse_8812f(dm);
	}
#endif

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B) {
		halrf_tssi_get_efuse_8814b(dm);
		halrf_get_efuse_thermal_pwrtype_8814b(dm);
	}
#endif

#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8197G) {
		halrf_tssi_get_efuse_8197g(dm);
	}
#endif

#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8723F) {
		halrf_tssi_get_efuse_8723f(dm);
	}
#endif

#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C) {
		halrf_tssi_get_efuse_8814c(dm);
		halrf_get_efuse_thermal_pwrtype_8814c(dm);
	}
#endif

}

void halrf_do_rxbb_dck(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;


#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814B)
		halrf_do_rxbb_dck_8814b(dm);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814C)
		halrf_do_rxbb_dck_8814c(dm);
#endif


}

void halrf_do_tssi(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8822C)
		halrf_do_tssi_8822c(dm);
#endif

#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8812F)
		halrf_do_tssi_8812f(dm);
#endif

#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8197G)
		halrf_do_tssi_8197g(dm);
#endif

#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8723F) {
		halrf_rfk_power_save(dm, false);
		halrf_do_tssi_8723f(dm);
		halrf_rfk_power_save(dm, true);
		}
#endif

#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8814C)
		halrf_tssi_trigger_bit_reset_8814c(dm);
#endif

}

u8 halrf_do_tssi_by_manual(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		return halrf_do_tssi_8814b(dm, path);
#endif

#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		return halrf_do_tssi_8814c(dm, path);
#endif

	return 0;
}

void halrf_set_tssi_enable(void *dm_void, boolean enable)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &(dm->rf_table);

	if (enable == 1) {
		rf->power_track_type = 4;
		odm_set_bb_reg(dm, R_0x1e7c, 0x40000000, 0x1);
	} else {
		rf->power_track_type = 0;
		odm_set_bb_reg(dm, R_0x1e7c, 0x40000000, 0x0);
	}
}


void halrf_do_thermal(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_do_thermal_8822c(dm);
#endif
}



u32 halrf_set_tssi_value(void *dm_void, u32 tssi_value)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		return halrf_set_tssi_value_8822c(dm, tssi_value);
#endif

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		return halrf_set_tssi_value_8814b(dm, tssi_value);
#endif
#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8723F)
		return halrf_tssi_set_de_8723f(dm, tssi_value);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		return halrf_set_tssi_value_8814c(dm, tssi_value);
#endif

	return 0;
}

void halrf_set_tssi_power(void *dm_void, s8 power)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	/*halrf_set_tssi_poewr_8822c(dm, power);*/
#endif
}

void halrf_tssi_set_de_for_tx_verify(void *dm_void, u32 tssi_de, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_tssi_set_de_for_tx_verify_8822c(dm, tssi_de, path);
#endif

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		halrf_tssi_set_de_for_tx_verify_8814b(dm, tssi_de, path);
#endif

#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8812F)
		halrf_tssi_set_de_for_tx_verify_8812f(dm, tssi_de, path);
#endif

#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8197G)
		halrf_tssi_set_de_for_tx_verify_8197g(dm, tssi_de, path);
#endif

#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8723F)
		halrf_tssi_set_de_for_tx_verify_8723f(dm, tssi_de, path);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		halrf_tssi_set_de_for_tx_verify_8814c(dm, tssi_de, path);
#endif

}

u32 halrf_tssi_turn_target_power(void *dm_void, s16 power_offset, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 pout = 0;

    if (dm->mp_mode) {
        if (*dm->mp_mode) { 
#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8723F)
		pout = halrf_tssi_set_powerbyrate_pout_8723f(dm, power_offset, path);
#endif
            }
        }
	return pout;
}

void halrf_tssi_set_power_offset(void *dm_void, s16 power_offset, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8723F)
		 _halrf_tssi_set_powerlevel_8723f(dm, power_offset, path);
#endif

}

u32 halrf_query_tssi_value(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		return halrf_query_tssi_value_8822c(dm);
#endif

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		return halrf_query_tssi_value_8814b(dm);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		return halrf_query_tssi_value_8814c(dm);
#endif

	return 0;
}

void halrf_tssi_cck(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	/*halrf_tssi_cck_8822c(dm);*/
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_thermal_cck_8822c(dm);
#endif

}

void halrf_thermal_cck(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_thermal_cck_8822c(dm);
#endif

}

void halrf_tssi_set_de(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	
#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		halrf_tssi_set_de_8814b(dm);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		halrf_tssi_set_de_8814c(dm);
#endif

}

void halrf_tssi_dck(void *dm_void, u8 direct_do)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	
	halrf_rfk_handshake(dm, true);

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
		if (dm->rfe_type == 1 || dm->rfe_type == 4 || dm->rfe_type == 5)
			return;
#else
		if (dm->rfe_type == 1 || dm->rfe_type == 6)
			return;
#endif
		halrf_tssi_dck_8814b(dm, direct_do);
	}
#endif

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_tssi_dck_8822c(dm);
#endif

#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8812F)
		halrf_tssi_dck_8812f(dm);
#endif

#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type == ODM_RTL8197G)
		halrf_tssi_dck_8197g(dm);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	if (dm->rfe_type == 1 || dm->rfe_type == 4 || dm->rfe_type == 5)
		return;
#else
	if (dm->rfe_type == 1 || dm->rfe_type == 6)
		return;
#endif
	halrf_tssi_dck_8814c(dm, direct_do);
	}
#endif


	halrf_rfk_handshake(dm, false);

}

void halrf_calculate_tssi_codeword(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	
#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		halrf_calculate_tssi_codeword_8814b(dm, RF_PATH_A);
#endif

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_calculate_tssi_codeword_8822c(dm);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		halrf_calculate_tssi_codeword_8814c(dm, RF_PATH_A);
#endif

}

void halrf_set_tssi_codeword(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
#if !(DM_ODM_SUPPORT_TYPE & ODM_IOT)
	struct _halrf_tssi_data *tssi = &rf->halrf_tssi_data;
#endif
	
#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		halrf_set_tssi_codeword_8814b(dm, tssi->tssi_codeword);
#endif

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_set_tssi_codeword_8822c(dm, tssi->tssi_codeword);
#endif	

#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		halrf_set_tssi_codeword_8814c(dm, tssi->tssi_codeword);
#endif

}

u8 halrf_get_tssi_codeword_for_txindex(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
		return 80;
#else
		return 60;
#endif
	}
#endif

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		return 64;
#endif

#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8812F)
		return 100;
#endif

#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8197G)
		return 100;
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C) {
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
		return 80;
#else
		return 60;
#endif
	}
#endif
	return 60;
}

void halrf_tssi_clean_de(
	void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8812F)
		halrf_tssi_clean_de_8812f(dm);
#endif

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		halrf_tssi_clean_de_8814b(dm);
#endif

#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8197G)
		halrf_tssi_clean_de_8197g(dm);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		halrf_tssi_clean_de_8814c(dm);
#endif
}

u32 halrf_tssi_trigger_de(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8812F)
		return halrf_tssi_trigger_de_8812f(dm, path);
#endif

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		return halrf_tssi_trigger_de_8814b(dm, path);
#endif

#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8197G)
		return halrf_tssi_trigger_de_8197g(dm, path);
#endif

#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		return halrf_tssi_trigger_de_8814c(dm, path);
#endif
	return 0;
}

u32 halrf_tssi_get_de(void *dm_void, u8 path)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		return halrf_tssi_get_de_8822c(dm, path);
#endif
	
#if (RTL8812F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8812F)
		return halrf_tssi_get_de_8812f(dm, path);
#endif

#if (RTL8814B_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814B)
		return halrf_tssi_get_de_8814b(dm, path);
#endif

#if (RTL8197G_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8197G)
		return halrf_tssi_get_de_8197g(dm, path);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		return halrf_tssi_get_de_8814c(dm, path);
#endif
	return 0;
}

u32 halrf_get_online_tssi_de(void *dm_void, u8 path, s32 pout)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8723F_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8723F)
		return halrf_get_online_tssi_de_8723f(dm, path, pout);
#endif
	return 0;
}

void halrf_tssi_trigger(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_rf_calibration_struct *cali_info = &(dm->rf_calibrate_info);
	struct _hal_rf_ *rf = &(dm->rf_table);

#if (DM_ODM_SUPPORT_TYPE & (ODM_WIN | ODM_CE))
	if (*dm->mp_mode == 1) {
		if (cali_info->txpowertrack_control == 0 ||
			cali_info->txpowertrack_control == 1) {
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
				"[TSSI]======>%s MP Mode UI chose thermal tracking. return !!!\n", __func__);
			return;
		}
	} else {
		if (rf->power_track_type >= 0 && rf->power_track_type <= 3) {
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
				"[TSSI]======>%s Normal Mode efues is thermal tracking. return !!!\n", __func__);
			return;
		}	
	}
#endif

	halrf_calculate_tssi_codeword(dm);
	halrf_set_tssi_codeword(dm);
	halrf_tssi_dck(dm, false);
#if (DM_ODM_SUPPORT_TYPE & (ODM_AP))
	halrf_tssi_get_efuse(dm);
#endif
	halrf_tssi_set_de(dm);
	halrf_do_tssi(dm);
}

void halrf_txgapk_write_gain_table(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_txgapk_save_all_tx_gain_table_8822c(dm);
#endif
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		halrf_txgapk_save_all_tx_gain_table_8814c(dm);
#endif

}

void halrf_txgapk_reload_tx_gain(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

#if (RTL8822C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8822C)
		halrf_txgapk_reload_tx_gain_8822c(dm);
#endif
/*
#if (RTL8814C_SUPPORT == 1)
	if (dm->support_ic_type & ODM_RTL8814C)
		halrf_txgapk_reload_tx_gain_8814c(dm);
#endif
*/
}

void halrf_txgap_enable_disable(void *dm_void, u8 enable)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &(dm->rf_table);

	if (enable) {
		rf->rf_supportability = rf->rf_supportability | HAL_RF_TXGAPK;
		halrf_txgapk_trigger(dm);
	} else {
		rf->rf_supportability = rf->rf_supportability & ~HAL_RF_TXGAPK;
		halrf_txgapk_reload_tx_gain(dm);
	}
}

#if (RTL8723F_SUPPORT == 1)
s8 _halrf_get_power_offset_by_thermal_8723f(void *dm_void, u8 path, s8 thermal_detla)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct dm_rf_calibration_struct *cali_info = &dm->rf_calibrate_info;
	u8 channel = *dm->channel;
	u8 tx_rate = phydm_get_tx_rate(dm);
	s8 power_offset = 0;
	u8 i = 0;
	u8 thermal_up_a[DELTA_SWINGIDX_SIZE] = {0}, thermal_down_a[DELTA_SWINGIDX_SIZE] = {0};
	u8 thermal_up_b[DELTA_SWINGIDX_SIZE] = {0}, thermal_down_b[DELTA_SWINGIDX_SIZE] = {0};
	u8 txagc_offset_2g_cck_a_p[] = {
		0, 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5,
		 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
	u8 txagc_offset_2g_cck_a_n[] = {
		0, 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5,
		 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
	u8 txagc_offset_2g_cck_b_p[] = {
		0, 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5, 5,
		 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
	u8 txagc_offset_2g_cck_b_n[] = {
		0, 0, 0, 1, 1, 2, 2, 2, 3, 3, 4, 4, 4, 5,
		 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
	u8 txagc_offset_2ga_p[] = {
		0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6,
		 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
	u8 txagc_offset_2ga_n[] = {
		0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,
		 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
	u8 txagc_offset_2gb_p[] = {
		0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 5,
		 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
	u8 txagc_offset_2gb_n[] = {
		0, 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4,
		 5, 5, 5, 5, 5, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6};
	u8 txagc_offset_5ga_n[][30] = {
		{0, 0, 2, 2, 2, 2, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6,
			6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
		{0, 0, 2, 2, 2, 2, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6,
			6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
		{0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 5, 5, 5, 6, 6,
			6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
	};
	u8 txagc_offset_5ga_p[][30] = {
		{0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 6, 6, 6, 6, 8,
			8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
		{0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 4, 6, 6, 6, 6, 8,
			8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
		{0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 6, 6, 6, 6, 7, 7, 8,
			8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9},
	};
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,"[RF][TSSI] ======>%s\n", __func__);

	if (channel >= 1 && channel <= 14) {
		if (IS_CCK_RATE(tx_rate)) {
			odm_move_memory(dm, thermal_up_a, txagc_offset_2g_cck_a_p, sizeof(thermal_up_a));
			odm_move_memory(dm, thermal_down_a, txagc_offset_2g_cck_a_n, sizeof(thermal_down_a));
			odm_move_memory(dm, thermal_up_b, txagc_offset_2g_cck_b_p, sizeof(thermal_up_b));
			odm_move_memory(dm, thermal_down_b, txagc_offset_2g_cck_b_n, sizeof(thermal_down_b));
		} else {
			odm_move_memory(dm, thermal_up_a, txagc_offset_2ga_p, sizeof(thermal_up_a));
			odm_move_memory(dm, thermal_down_a, txagc_offset_2ga_n, sizeof(thermal_down_a));
			odm_move_memory(dm, thermal_up_b, txagc_offset_2gb_p, sizeof(thermal_up_b));
			odm_move_memory(dm, thermal_down_b, txagc_offset_2gb_n, sizeof(thermal_down_b));
		}
	}

	if (channel >= 36 && channel <= 64) {
		odm_move_memory(dm, thermal_up_a, txagc_offset_5ga_p[0], sizeof(thermal_up_a));
		odm_move_memory(dm, thermal_down_a, txagc_offset_5ga_n[0], sizeof(thermal_down_a));
	} else if (channel >= 100 && channel <= 144) {
		 odm_move_memory(dm, thermal_up_a, txagc_offset_5ga_p[1], sizeof(thermal_up_a));
		odm_move_memory(dm, thermal_down_a, txagc_offset_5ga_n[1], sizeof(thermal_down_a));
	} else if (channel >= 149 && channel <= 177) {
		odm_move_memory(dm, thermal_up_a, txagc_offset_5ga_p[2], sizeof(thermal_up_a));
		odm_move_memory(dm, thermal_down_a, txagc_offset_5ga_n[2], sizeof(thermal_down_a));
	}

	if(thermal_detla < 0) {
		if (thermal_detla < -29)
			i = 29;
		else
			i = (u8)(-1 * thermal_detla);

		if (path ==0 )
			power_offset = thermal_down_a[i];
		else
			power_offset = thermal_down_b[i];

		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,"[RF][TSSI] Temp is lower\n");
	} else {
		if (thermal_detla >= 30)
			i = 29;
		else
			i = thermal_detla;

		if (path ==0 )
			power_offset = thermal_up_a[i];
		else
			power_offset = thermal_up_b[i];
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,"[RF][TSSI] Temp is higher\n");
	}

	return power_offset;
}

void halrf_powertracking_thermal(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct _halrf_tssi_data *tssi = &rf->halrf_tssi_data;
	struct dm_rf_calibration_struct *cali_info = &(dm->rf_calibrate_info);
	s8 thermal_value = 0, thermal_detla = 0;
	s8 power_offset0 = 0, power_offset1 = 0;
	u8 thermal_base = 0, temp[2] = {0};
	u8 path;
	u8 rate = phydm_get_tx_rate(dm);
	//RF_DBG(dm, DBG_RF_TX_PWR_TRACK,"[RF][TSSI] ======>%s\n", __func__);

	if(rf->is_tssi_in_progress == 1)
		return;
	if (*dm->mp_mode == 1) {
		if (cali_info->txpowertrack_control <= 2) {
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,"[RF][TSSI] return!! txpowertrack_control = %d\n",
				cali_info->txpowertrack_control);
			return;
		}
	} else {
		if (!(rf->rf_supportability & HAL_RF_TX_PWR_TRACK)) {
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
				"[RF][TSSI] ===>is_txpowertracking is false, return!!\n");
			return;
		}
	}

	if (odm_get_bb_reg(dm, R_0x4318, BIT30) == 1){
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,"[RF][TSSI] return!! txpowertrack_control = %d, 0x4318 = 0x%x\n",
			cali_info->txpowertrack_control,odm_get_bb_reg(dm, R_0x4318, MASKDWORD));
		return;
	}
	path = (u8)odm_get_bb_reg(dm, 0x1884, BIT(20));
	thermal_base = tssi->thermal_cal;
	thermal_value = (s8)odm_get_rf_reg(dm, 0, RF_0x42, 0x7E);/*path0*/
	thermal_detla = (s8)(thermal_value - thermal_base);
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][TSSI] thermal_value = 0x%x, thermal_base = 0x%x\n",
		thermal_value, thermal_base);
	power_offset0 = _halrf_get_power_offset_by_thermal_8723f(dm, 0, thermal_detla);
	power_offset1 = _halrf_get_power_offset_by_thermal_8723f(dm, 1, thermal_detla);

	if (rate == ODM_MGN_1M || rate == ODM_MGN_2M || rate == ODM_MGN_5_5M || rate == ODM_MGN_11M){
		temp[0] = tssi->txagc_offset_thermaltrack[0] -0x18 + 4*(power_offset0);/*s0*/
		temp[1] = tssi->txagc_offset_thermaltrack[1] - 0x8 + 4*(power_offset1);/*s1*/
	} else {
		temp[0] = tssi->txagc_offset_thermaltrack[0] + 4*(power_offset0);
		temp[1] = tssi->txagc_offset_thermaltrack[1] + 4*(power_offset1);
	}
	/*S0:in the same index,cck_pwr-ofdm_pwr=7dB*/
	odm_set_bb_reg(dm, R_0x4388, MASKBYTE0, temp[0]);/*s0*/
	/*S1:in the same index,cck_pwr-ofdm_pwr=5dB*/
	odm_set_bb_reg(dm, R_0x4388, MASKBYTE2, temp[1]);/*s1*/

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][TSSI] current_Path = %d, tx_rate = 0x%x\n", path, rate);
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][TSSI] txagc_offset = 0x%x\n",
		tssi->txagc_offset_thermaltrack[path]);
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		"[RF][TSSI] thermal_detla = %d, thermal_offset_s0 = 0x%x, thermal_offset_s1 = 0x%x\n",
		thermal_detla, power_offset0, power_offset1);
	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,"[RF][TSSI] 0x4388 = 0x%x\n",
		odm_get_bb_reg(dm, R_0x4388, MASKDWORD));


}
void halrf_xtal_thermal_track(void *dm_void)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;
	struct _halrf_tssi_data *tssi = &rf->halrf_tssi_data;
	struct dm_rf_calibration_struct *cali_info = &(dm->rf_calibrate_info);
	s8	*delta_swing_table_xtal_up = NULL;
	u8	xtal_offset_eanble = 0, i =0;
	s8	thermal_value = 0, thermal_detla = 0;
	u8  	thermal_base = 0;
	s8  	xtal_table_up[DELTA_SWINGIDX_SIZE] = {0};
	s8  	xtal_table_down[DELTA_SWINGIDX_SIZE] = {0};
	s32 	reg_val = 0, crystal_cap = 0;

	RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][xtal] ======>%s\n", __func__);

	if ( dm->support_ic_type == ODM_RTL8723F) {	
		if (rf->is_dpk_in_progress || dm->rf_calibrate_info.is_iqk_in_progress ||
		    dm->is_psd_in_process || rf->is_tssi_in_progress ||
		    !(rf->rf_supportability & HAL_RF_DPK_TRACK) || 
		    rf->is_txgapk_in_progress)
			return;

		if(tssi->thermal[0] == 0xff) {
			//RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][xtal] thermal 0xFF, return!\n");
			return;
		} else {
			thermal_base = tssi->thermal[0];
			//RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][xtal] thermal_base = 0x%x\n", thermal_base);
		}

		RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][xtal] thermal_base = 0x%x\n", thermal_base);
		
		thermal_value = (s8)odm_get_rf_reg(dm, RF_PATH_A, RF_0x42, 0x7E);	/* 0x42: RF Reg[6:1]*/

		thermal_detla = (s8)(thermal_value - thermal_base);
		
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "[RF][xtal] cali_info->xtal_offset = 0x%x\n", cali_info->xtal_offset);

		cali_info->xtal_offset_last = cali_info->xtal_offset;
		/*
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
						   "[RF][Xtal] cali_info->delta_swing_table_xtal_p = %d\n", cali_info->delta_swing_table_xtal_p[2]);
		*/
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF][Xtal] thermal_value = 0x%x, thermal_detla = 0x%x, xtal_offset_last = 0x%x\n", 
		       thermal_value, thermal_detla, cali_info->xtal_offset_last);
		odm_move_memory(dm, xtal_table_up, cali_info->delta_swing_table_xtal_p, sizeof(xtal_table_up));//(void *)
		odm_move_memory(dm, xtal_table_down, cali_info->delta_swing_table_xtal_n, sizeof(xtal_table_down));
		/*
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
						   "[RF][Xtal] xtal_table_up[1] = %d\n", xtal_table_up[1]);*/
		if(thermal_detla < 0) {
			
			if (thermal_detla < -29)
				i = 29;
			else
				i = (u8)(-1 * thermal_detla);
			cali_info->xtal_offset = xtal_table_down[i];
		} else {

			if (thermal_detla >= 30)
				i = 29;
			else
				i = thermal_detla;

			cali_info->xtal_offset = xtal_table_up[i];
		}
		
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF][Xtal] xtal_offset = %d\n", cali_info->xtal_offset);
		if (cali_info->xtal_offset_last == cali_info->xtal_offset)
			xtal_offset_eanble = 0;
		else
			xtal_offset_eanble = 1;
		RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF][Xtal] xtal_offset_eanble = %d\n", xtal_offset_eanble);
		if (xtal_offset_eanble != 0) {
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "**********Enter Xtal Tracking**********\n");
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF][Xtal] R_0x103c[16:10] = 0x%x\n", odm_get_mac_reg(dm, R_0x103c, 0x0001FC00));

			crystal_cap = dm->dm_cfo_track.crystal_cap_default & 0x7F;
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF][Xtal] DEFAULT crystal_cap = 0x%x\n", crystal_cap);
			reg_val = crystal_cap + cali_info->xtal_offset;
			if (reg_val > 0x7F)
				reg_val = 0x7F;
			else if (reg_val < 0)
				reg_val = 0x0;

			//reg_val = (u32)(odm_get_mac_reg(dm, R_0x103c, 0x0001FC00) + cali_info->xtal_offset);
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF][Xtal] reg_val = 0x%x\n", reg_val);
			/* write 0x103c[23:17] = 0x103c[16:10] = crystal_cap */
			crystal_cap = reg_val | (reg_val << 7);
			odm_set_mac_reg(dm, R_0x103c, 0x00FFFC00, crystal_cap);
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF][Xtal] R_0x103c[16:10] = 0x%x\n", odm_get_mac_reg(dm, R_0x103c, 0x0001FC00));
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
		       "[RF][Xtal] R_0x103c[23:17] = 0x%x\n", odm_get_mac_reg(dm, R_0x103c, 0x00FE0000));
			RF_DBG(dm, DBG_RF_TX_PWR_TRACK, "**********End Xtal Tracking**********\n");
		}
		//odm_set_rf_reg(dm, RF_PATH_A, RF_0x42, 0x30000, 0x3);
		//delay
	}
	/*RF_DBG(dm, DBG_RF_TX_PWR_TRACK,
	       "[RF][xtal] <======%s\n", __func__);*/
}
#endif

void _halrf_dump_subpage(void *dm_void, u32 *_used, char *output, u32 *_out_len, u8 page)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	u32 used = *_used;
	u32 out_len = *_out_len;
	u32 addr;

	PDM_SNPF(out_len, used, output + used, out_len - used,
		 "\n===============[ Subpage_%d start]===============\n", page);

	RF_DBG(dm, DBG_RF_RFK, " ===============[ Subpage_%d start]===============\n", page);

	odm_set_bb_reg(dm, R_0x1b00, BIT(2) | BIT(1), page);

	for (addr = 0x1b00; addr < 0x1c00; addr += 0x10) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 " 0x%x : 0x%08x  0x%08x  0x%08x  0x%08x\n", addr,
			odm_get_bb_reg(dm, addr, MASKDWORD),
		 	odm_get_bb_reg(dm, addr + 0x4, MASKDWORD),
		 	odm_get_bb_reg(dm, addr + 0x8, MASKDWORD),
		 	odm_get_bb_reg(dm, addr + 0xc, MASKDWORD));
		RF_DBG(dm, DBG_RF_RFK, " 0x%x : 0x%08x  0x%08x  0x%08x  0x%08x\n", addr,
		       odm_get_bb_reg(dm, addr, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 0x4, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 0x8, MASKDWORD),
		       odm_get_bb_reg(dm, addr + 0xc, MASKDWORD));
	}

	*_used = used;
	*_out_len = out_len;
}

void halrf_dump_rfk_reg(void *dm_void, char input[][16], u32 *_used,
			      char *output, u32 *_out_len)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &dm->rf_table;

	char help[] = "-h";
	u32 var1[10] = {0};
	u32 used = *_used;
	u32 out_len = *_out_len;
	u32 reg_1b00, supportability;
	u8 page;

	if (!(dm->support_ic_type & (ODM_IC_11AC_SERIES |  ODM_IC_JGR3_SERIES))) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "CMD is Unsupported due to IC type!!!\n");
		RF_DBG(dm, DBG_RF_RFK, "[RFK] CMD is Unsupported due to IC type!!!\n");
		return;
	} else if (rf->is_dpk_in_progress || dm->rf_calibrate_info.is_iqk_in_progress ||
	    dm->is_psd_in_process || rf->is_tssi_in_progress || rf->is_txgapk_in_progress) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "Bypass CMD due to RFK is doing!!!\n");
		RF_DBG(dm, DBG_RF_RFK, "[RFK] Bypass CMD due to RFK is doing!!!\n");
		return;
	}

#if (DM_ODM_SUPPORT_TYPE == ODM_WIN)
	if (*dm->is_fcs_mode_enable) {
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "Bypass CMD due to FCS mode!!!\n");
		RF_DBG(dm, DBG_RF_RFK, "[RFK] Bypass CMD due to FCS mode!!!\n");
		return;
	}
#endif
	supportability = rf->rf_supportability;

	/*to avoid DPK track interruption*/
	rf->rf_supportability = rf->rf_supportability & ~HAL_RF_DPK_TRACK;

	reg_1b00 = odm_get_bb_reg(dm, R_0x1b00, MASKDWORD);

	if (input[2])
		PHYDM_SSCANF(input[2], DCMD_DECIMAL, &var1[0]);

	if ((strcmp(input[2], help) == 0))
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "dump subpage {0:Page0, 1:Page1, 2:Page2, 3:Page3, 4:all}\n");
	else if (var1[0] > 4)
		PDM_SNPF(out_len, used, output + used, out_len - used,
			 "Wrong subpage number!!\n");
	else if (var1[0] == 4) {
		for (page = 0; page < 4; page++)
			_halrf_dump_subpage(dm, &used, output, &out_len, page);
	} else
		_halrf_dump_subpage(dm, &used, output, &out_len, (u8)var1[0]);

	odm_set_bb_reg(dm, R_0x1b00, MASKDWORD, reg_1b00);

	rf->rf_supportability = supportability;

	*_used = used;
	*_out_len = out_len;
}

/*Golbal function*/
void halrf_reload_bp(void *dm_void, u32 *bp_reg, u32 *bp, u32 num)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 i;

	for (i = 0; i < num; i++)
		odm_write_4byte(dm, bp_reg[i], bp[i]);
}

void halrf_reload_bprf(void *dm_void, u32 *bp_reg, u32 bp[][4], u32 num,
		       u8 ss)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 i, path;

	for (i = 0; i < num; i++) {
		for (path = 0; path < ss; path++)
			odm_set_rf_reg(dm, (enum rf_path)path, bp_reg[i],
				       MASK20BITS, bp[i][path]);
	}
}

void halrf_bp(void *dm_void, u32 *bp_reg, u32 *bp, u32 num)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 i;

	for (i = 0; i < num; i++)
		bp[i] = odm_read_4byte(dm, bp_reg[i]);
}

void halrf_bprf(void *dm_void, u32 *bp_reg, u32 bp[][4], u32 num, u8 ss)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 i, path;

	for (i = 0; i < num; i++) {
		for (path = 0; path < ss; path++) {
			bp[i][path] =
				odm_get_rf_reg(dm, (enum rf_path)path,
					       bp_reg[i], MASK20BITS);
		}
	}
}

void halrf_swap(void *dm_void, u32 *v1, u32 *v2)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 temp;

	temp = *v1;
	*v1 = *v2;
	*v2 = temp;
}

void halrf_bubble(void *dm_void, u32 *v1, u32 *v2)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 temp;

	if (*v1 >= 0x200 && *v2 >= 0x200) {
		if (*v1 > *v2)
			halrf_swap(dm, v1, v2);
	} else if (*v1 < 0x200 && *v2 < 0x200) {
		if (*v1 > *v2)
			halrf_swap(dm, v1, v2);
	} else if (*v1 < 0x200 && *v2 >= 0x200) {
		halrf_swap(dm, v1, v2);
	}
}

void halrf_b_sort(void *dm_void, u32 *iv, u32 *qv)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 temp;
	u32 i, j;

	RF_DBG(dm, DBG_RF_DACK, "[DACK]bubble!!!!!!!!!!!!");
	for (i = 0; i < SN - 1; i++) {
		for (j = 0; j < (SN - 1 - i) ; j++) {
			halrf_bubble(dm, &iv[j], &iv[j + 1]);
			halrf_bubble(dm, &qv[j], &qv[j + 1]);
		}
	}
}

void halrf_minmax_compare(void *dm_void, u32 value, u32 *min,
			  u32 *max)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (value >= 0x200) {
		if (*min >= 0x200) {
			if (*min > value)
				*min = value;
		} else {
			*min = value;
		}
		if (*max >= 0x200) {
			if (*max < value)
				*max = value;
		}
	} else {
		if (*min < 0x200) {
			if (*min > value)
				*min = value;
		}

		if (*max  >= 0x200) {
			*max = value;
		} else {
			if (*max < value)
				*max = value;
		}
	}
}

u32 halrf_delta(void *dm_void, u32 v1, u32 v2)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	if (v1 >= 0x200 && v2 >= 0x200) {
		if (v1 > v2)
			return v1 - v2;
		else
			return v2 - v1;
	} else if (v1 >= 0x200 && v2 < 0x200) {
		return v2 + (0x400 - v1);
	} else if (v1 < 0x200 && v2 >= 0x200) {
		return v1 + (0x400 - v2);
	}

	if (v1 > v2)
		return v1 - v2;
	else
		return v2 - v1;
}

boolean halrf_compare(void *dm_void, u32 value)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;

	boolean fail = false;

	if (value >= 0x200 && (0x400 - value) > 0x64)
		fail = true;
	else if (value < 0x200 && value > 0x64)
		fail = true;

	if (fail)
		RF_DBG(dm, DBG_RF_DACK, "[DACK]overflow!!!!!!!!!!!!!!!");
	return fail;
}

void halrf_mode(void *dm_void, u32 *i_value, u32 *q_value)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u32 iv[SN], qv[SN], im[SN], qm[SN], temp, temp1, temp2;
	u32 p, m, t;
	u32 i_max = 0, q_max = 0, i_min = 0x0, q_min = 0x0, c = 0x0;
	u32 i_delta, q_delta;
	u8 i, j, ii = 0, qi = 0;
	boolean fail = false;

	ODM_delay_ms(10);
	for (i = 0; i < SN; i++) {
		im[i] = 0;
		qm[i] = 0;
	}
	i = 0;
	c = 0;
	while (i < SN && c < 1000) {
		c++;
		temp = odm_get_bb_reg(dm, 0x2dbc, 0x3fffff);
		iv[i] = (temp & 0x3ff000) >> 12;
		qv[i] = temp & 0x3ff;

		fail = false;
		if (halrf_compare(dm, iv[i]))
			fail = true;
		if (halrf_compare(dm, qv[i]))
			fail = true;
		if (!fail)
			i++;
	}
	c = 0;
	do {
		c++;
		i_min = iv[0];
		i_max = iv[0];
		q_min = qv[0];
		q_max = qv[0];
		for (i = 0; i < SN; i++) {
			halrf_minmax_compare(dm, iv[i], &i_min, &i_max);
			halrf_minmax_compare(dm, qv[i], &q_min, &q_max);
		}
		RF_DBG(dm, DBG_RF_DACK, "[DACK]i_min=0x%x, i_max=0x%x",
		       i_min, i_max);
		RF_DBG(dm, DBG_RF_DACK, "[DACK]q_min=0x%x, q_max=0x%x",
		       q_min, q_max);
		if (i_max < 0x200 && i_min < 0x200)
			i_delta = i_max - i_min;
		else if (i_max >= 0x200 && i_min >= 0x200)
			i_delta = i_max - i_min;
		else
			i_delta = i_max + (0x400 - i_min);

		if (q_max < 0x200 && q_min < 0x200)
			q_delta = q_max - q_min;
		else if (q_max >= 0x200 && q_min >= 0x200)
			q_delta = q_max - q_min;
		else
			q_delta = q_max + (0x400 - q_min);
		RF_DBG(dm, DBG_RF_DACK, "[DACK]i_delta=0x%x, q_delta=0x%x",
		       i_delta, q_delta);
		halrf_b_sort(dm, iv, qv);
		if (i_delta > 5 || q_delta > 5) {
			temp = odm_get_bb_reg(dm, 0x2dbc, 0x3fffff);
			iv[0] = (temp & 0x3ff000) >> 12;
			qv[0] = temp & 0x3ff;
			temp = odm_get_bb_reg(dm, 0x2dbc, 0x3fffff);
			iv[SN - 1] = (temp & 0x3ff000) >> 12;
			qv[SN - 1] = temp & 0x3ff;
		} else {
			break;
		}
	} while (c < 100);
#if 1
#if 0
	for (i = 0; i < SN; i++)
		RF_DBG(dm, DBG_RF_DACK, "[DACK]iv[%d] = 0x%x\n", i, iv[i]);
	for (i = 0; i < SN; i++)
		RF_DBG(dm, DBG_RF_DACK, "[DACK]qv[%d] = 0x%x\n", i, qv[i]);
#endif
	/*i*/
	m = 0;
	p = 0;
	for (i = 10; i < SN - 10; i++) {
		if (iv[i] > 0x200)
			m = (0x400 - iv[i]) + m;
		else
			p = iv[i] + p;
	}

	if (p > m) {
		t = p - m;
		t = t / (SN - 20);
	} else {
		t = m - p;
		t = t / (SN - 20);
		if (t != 0x0)
			t = 0x400 - t;
	}
	*i_value = t;
	/*q*/
	m = 0;
	p = 0;
	for (i = 10; i < SN - 10; i++) {
		if (qv[i] > 0x200)
			m = (0x400 - qv[i]) + m;
		else
			p = qv[i] + p;
	}
	if (p > m) {
		t = p - m;
		t = t / (SN - 20);
	} else {
		t = m - p;
		t = t / (SN - 20);
		if (t != 0x0)
			t = 0x400 - t;
	}
	*q_value = t;
#endif
}
void halrf_delay_10us(u16 v1)
{	
	u16 i = 0;
	
	for (i = 0; i < v1; i++)
		ODM_delay_us(10);
}

