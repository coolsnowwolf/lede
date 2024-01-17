/******************************************************************************
 *
 * Copyright(c) 2007 - 2017 Realtek Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 *****************************************************************************/

/*@===========================================================
 * include files
 *============================================================
 */
#include "mp_precomp.h"
#include "phydm_precomp.h"

u64 _sqrt(u64 x)
{
	u64 i = 0;
	u64 j = (x >> 1) + 1;

	while (i <= j) {
		u64 mid = (i + j) >> 1;

		u64 sq = mid * mid;

		if (sq == x)
			return mid;
		else if (sq < x)
			i = mid + 1;
		else
			j = mid - 1;
	}

	return j;
}

u32 halrf_get_psd_data(
	struct dm_struct *dm,
	u32 point)
{
	struct _hal_rf_ *rf = &(dm->rf_table);
	struct _halrf_psd_data *psd = &(rf->halrf_psd_data);
	u32 psd_val = 0, psd_reg, psd_report, psd_point, psd_start, i, delay_time = 0;

#if (DEV_BUS_TYPE == RT_USB_INTERFACE) || (DEV_BUS_TYPE == RT_SDIO_INTERFACE)
	if (dm->support_interface == ODM_ITRF_USB || dm->support_interface == ODM_ITRF_SDIO) {
		if (psd->average == 0)
			delay_time = 100;
		else
			delay_time = 0;
	}
#endif
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
	if (dm->support_interface == ODM_ITRF_PCIE) {
		if (psd->average == 0)
			delay_time = 1000;
		else
			delay_time = 100;
	}
#endif

	if (dm->support_ic_type & (ODM_RTL8812 | ODM_RTL8821 | ODM_RTL8814A | ODM_RTL8822B | ODM_RTL8821C)) {
		psd_reg = R_0x910;
		psd_report = R_0xf44;
	} else {
		psd_reg = R_0x808;
		psd_report = R_0x8b4;
	}

	if (dm->support_ic_type & ODM_RTL8710B) {
		psd_point = 0xeffffc00;
		psd_start = 0x10000000;
	} else {
		psd_point = 0xffbffc00;
		psd_start = 0x00400000;
	}

	psd_val = odm_get_bb_reg(dm, psd_reg, MASKDWORD);

	psd_val &= psd_point;
	psd_val |= point;

	odm_set_bb_reg(dm, psd_reg, MASKDWORD, psd_val);

	psd_val |= psd_start;

	odm_set_bb_reg(dm, psd_reg, MASKDWORD, psd_val);

	for (i = 0; i < delay_time; i++)
		ODM_delay_us(1);

	psd_val = odm_get_bb_reg(dm, psd_report, MASKDWORD);

	if (dm->support_ic_type & (ODM_RTL8821C | ODM_RTL8710B)) {
		psd_val &= MASKL3BYTES;
		psd_val = psd_val / 32;
	} else {
		psd_val &= MASKLWORD;
	}

	return psd_val;
}

void halrf_psd(
	struct dm_struct *dm,
	u32 point,
	u32 start_point,
	u32 stop_point,
	u32 average)
{
	struct _hal_rf_ *rf = &(dm->rf_table);
	struct _halrf_psd_data *psd = &(rf->halrf_psd_data);

	u32 i = 0, j = 0, k = 0;
	u32 psd_reg, avg_org, point_temp, average_tmp, mode;
	u64 data_tatal = 0, data_temp[64] = {0};

	psd->buf_size = 256;

	mode = average >> 16;
	
	if (mode == 2)
		average_tmp = 1;
	else
		average_tmp = average & 0xffff;

	if (dm->support_ic_type & (ODM_RTL8812 | ODM_RTL8821 | ODM_RTL8814A | ODM_RTL8822B | ODM_RTL8821C))
		psd_reg = R_0x910;
	else
		psd_reg = R_0x808;

#if 0
	dbg_print("[PSD]point=%d, start_point=%d, stop_point=%d, average=%d, average_tmp=%d, buf_size=%d\n",
		point, start_point, stop_point, average, average_tmp, psd->buf_size);
#endif

	for (i = 0; i < psd->buf_size; i++)
		psd->psd_data[i] = 0;

	if (dm->support_ic_type & ODM_RTL8710B)
		avg_org = odm_get_bb_reg(dm, psd_reg, 0x30000);
	else
		avg_org = odm_get_bb_reg(dm, psd_reg, 0x3000);

	if (mode == 1) {
		if (dm->support_ic_type & ODM_RTL8710B)
			odm_set_bb_reg(dm, psd_reg, 0x30000, 0x1);
		else
			odm_set_bb_reg(dm, psd_reg, 0x3000, 0x1);
	}

#if 0
	if (avg_temp == 0)
		avg = 1;
	else if (avg_temp == 1)
		avg = 8;
	else if (avg_temp == 2)
		avg = 16;
	else if (avg_temp == 3)
		avg = 32;
#endif

	i = start_point;
	while (i < stop_point) {
		data_tatal = 0;

		if (i >= point)
			point_temp = i - point;
		else
			point_temp = i;

		for (k = 0; k < average_tmp; k++) {
			data_temp[k] = halrf_get_psd_data(dm, point_temp);
			data_tatal = data_tatal + (data_temp[k] * data_temp[k]);

#if 0
			if ((k % 20) == 0)
				dbg_print("\n ");

			dbg_print("0x%x ", data_temp[k]);
#endif
		}
#if 0
		/*dbg_print("\n");*/
#endif

		data_tatal = phydm_division64((data_tatal * 100), average_tmp);
		psd->psd_data[j] = (u32)_sqrt(data_tatal);

		i++;
		j++;
	}

#if 0
	for (i = 0; i < psd->buf_size; i++) {
		if ((i % 20) == 0)
			dbg_print("\n ");

		dbg_print("0x%x ", psd->psd_data[i]);
	}
	dbg_print("\n\n");
#endif

	if (dm->support_ic_type & ODM_RTL8710B)
		odm_set_bb_reg(dm, psd_reg, 0x30000, avg_org);
	else
		odm_set_bb_reg(dm, psd_reg, 0x3000, avg_org);
}

void backup_bb_register(struct dm_struct *dm, u32 *bb_backup, u32 *backup_bb_reg, u32 counter)
{
	u32 i ;

	for (i = 0; i < counter; i++)
		bb_backup[i] = odm_get_bb_reg(dm, backup_bb_reg[i], MASKDWORD);
}

void restore_bb_register(struct dm_struct *dm, u32 *bb_backup, u32 *backup_bb_reg, u32 counter)
{
	u32 i ;

	for (i = 0; i < counter; i++)
		odm_set_bb_reg(dm, backup_bb_reg[i], MASKDWORD, bb_backup[i]);
}



void _halrf_psd_iqk_init(struct dm_struct *dm)
{
	odm_set_bb_reg(dm, 0x1b04, MASKDWORD, 0x0);
	odm_set_bb_reg(dm, 0x1b08, MASKDWORD, 0x80);
	odm_set_bb_reg(dm, 0x1b0c, 0xc00, 0x3);
	odm_set_bb_reg(dm, 0x1b14, MASKDWORD, 0x0);
	odm_set_bb_reg(dm, 0x1b18, BIT(0), 0x1);

	if (dm->support_ic_type & ODM_RTL8197G)
		odm_set_bb_reg(dm, 0x1b20, MASKDWORD, 0x00040008);
	if (dm->support_ic_type & ODM_RTL8198F)
		odm_set_bb_reg(dm, 0x1b20, MASKDWORD, 0x00000000);

	if (dm->support_ic_type & (ODM_RTL8197G | ODM_RTL8198F)) {
		odm_set_bb_reg(dm, 0x1b24, MASKDWORD, 0x00030000);
		odm_set_bb_reg(dm, 0x1b28, MASKDWORD, 0x00000000);
		odm_set_bb_reg(dm, 0x1b2c, MASKDWORD, 0x00180018);
		odm_set_bb_reg(dm, 0x1b30, MASKDWORD, 0x20000000);
		/*odm_set_bb_reg(dm, 0x1b38, MASKDWORD, 0x20000000);*/
		/*odm_set_bb_reg(dm, 0x1b3c, MASKDWORD, 0x20000000);*/
	}

	odm_set_bb_reg(dm, 0x1b1c, 0xfff, 0xd21);
	odm_set_bb_reg(dm, 0x1b1c, 0xfff00000, 0x821);
	odm_set_bb_reg(dm, 0x1b28, MASKDWORD, 0x0);
	odm_set_bb_reg(dm, 0x1bcc, 0x3f, 0x3f);	
}

void _halrf_iqk_psd_init_8723f(void *dm_void,	 boolean onoff)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	u8 s;

	s = (u8)odm_get_bb_reg(dm, 0x1884, BIT(20));

	if (onoff) {
		/*01_8723F_AFE_ON_BB_settings.txt*/
		odm_set_bb_reg(dm, 0x1c38, MASKDWORD, 0x0);
		odm_set_bb_reg(dm, R_0x1830, BIT(30), 0x0);
		odm_set_bb_reg(dm, R_0x1860, 0xF0000000, 0xf);
		odm_set_bb_reg(dm, R_0x1860, 0x0FFFF000, 0x0041);
		odm_set_bb_reg(dm, 0x09f0, 0x0000FFFF, 0xbbbb);
		odm_set_bb_reg(dm, 0x1d40, BIT(3), 0x1);
		odm_set_bb_reg(dm, 0x1d40, 0x00000007, 0x3);
		odm_set_bb_reg(dm, 0x09b4, 0x00000700, 0x3);
		odm_set_bb_reg(dm, 0x09b4, 0x00003800, 0x3);
		odm_set_bb_reg(dm, 0x09b4, 0x0001C000, 0x3);
		odm_set_bb_reg(dm, 0x09b4, 0x000E0000, 0x3);
		odm_set_bb_reg(dm, R_0x1c20, BIT(5), 0x1);
		odm_set_bb_reg(dm, R_0x1e24, BIT(31), 0x0);
		odm_set_bb_reg(dm, R_0x1e28, 0x0000000F, 0x1);
		odm_set_bb_reg(dm, R_0x824, 0x000F0000, 0x1);
		odm_set_bb_reg(dm, R_0x1cd0, 0xF0000000, 0x7);
		odm_set_bb_reg(dm, R_0x2a24, BIT(13), 0x1);
		odm_set_bb_reg(dm, R_0x1c68, BIT(24), 0x1);
		odm_set_bb_reg(dm, R_0x1864, BIT(31), 0x1);
		odm_set_bb_reg(dm, R_0x180c, BIT(27), 0x1);
		odm_set_bb_reg(dm, R_0x180c, BIT(30), 0x1);
		odm_set_bb_reg(dm, R_0x1e24, BIT(17), 0x1);
		odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x0);
		odm_set_bb_reg(dm, R_0x1c38, MASKDWORD, 0xffffffff);
		/*02_IQK_Preset.txt*/
		//odm_set_rf_reg(dm, RF_PATH_A, 0x05, BIT(0), 0x0);
		//odm_set_rf_reg(dm, RF_PATH_B, 0x05, BIT(0), 0x0);
		odm_set_bb_reg(dm, R_0x1b08, MASKDWORD, 0x00000080);
		//odm_set_bb_reg(dm, R_0x1bd8, MASKDWORD, 0x00000002);
		//switch path  10 od 0x1b38 0x1/0x3 [1:0]
		if (s == 0)
			odm_set_bb_reg(dm, R_0x1b00, MASKDWORD, 0x00000008);
		else
			odm_set_bb_reg(dm, R_0x1b00, MASKDWORD, 0x0000000a);

		odm_set_bb_reg(dm, R_0x1b18, MASKDWORD, 0x40010101);
		odm_set_bb_reg(dm, R_0x1b14, MASKDWORD, 0x40010100);
		//odm_set_bb_reg(dm, R_0x1b1c, MASKDWORD, 0xA2103C00);
		odm_set_bb_reg(dm, R_0x1b0c, 0x00000C00, 0x2);
		odm_set_bb_reg(dm, R_0x1bcc, 0x0000003F, 0x3f);	
		//DbgPrint("[PSD][8723F]iqkpsd init!\n");
	} else {
		/*10_IQK_Reg_PSD_Restore.txt*/
		//odm_set_bb_reg(dm, R_0x1b1c, MASKDWORD, 0xA2103C00);
		odm_set_bb_reg(dm, R_0x1b08, MASKDWORD, 0x00000000);
		odm_set_bb_reg(dm, R_0x1b38, BIT(0), 0x0);
		odm_set_bb_reg(dm, R_0x1bcc, 0x0000003F, 0x0);	
		//odm_set_rf_reg(dm, RF_PATH_A, 0x05, BIT(0), 0x1);
		//odm_set_rf_reg(dm, RF_PATH_B, 0x05, BIT(0), 0x1);
		/*11_8723F_restore_AFE_BB_settings.txt*/
		odm_set_bb_reg(dm, 0x1c38, MASKDWORD, 0x0);
		odm_set_bb_reg(dm, R_0x1830, BIT(30), 0x1);
		odm_set_bb_reg(dm, R_0x1e24, BIT(31), 0x1);
		odm_set_bb_reg(dm, R_0x2a24, BIT(13), 0x0);
		odm_set_bb_reg(dm, R_0x1c68, BIT(24), 0x0);
		odm_set_bb_reg(dm, R_0x1864, BIT(31), 0x0);
		odm_set_bb_reg(dm, R_0x180c, BIT(27), 0x0);
		odm_set_bb_reg(dm, R_0x180c, BIT(30), 0x0);
		odm_set_bb_reg(dm, R_0x1880, BIT(21), 0x0);
		odm_set_bb_reg(dm, R_0x1c38, MASKDWORD, 0xffa1005e);
		//DbgPrint("[PSD][8723F]iqkpsd resotre!\n");
	}
}

void _halrf_psd_iqk_init_8814c(struct dm_struct *dm)
{
	odm_set_bb_reg(dm, 0x1b04, MASKDWORD, 0x0);
	odm_set_bb_reg(dm, 0x1b08, MASKDWORD, 0x80);
	odm_set_bb_reg(dm, 0x1b0c, 0xc00, 0x3);
	odm_set_bb_reg(dm, 0x1b14, MASKDWORD, 0x0);
	odm_set_bb_reg(dm, 0x1b18, BIT(0), 0x1);
	odm_set_bb_reg(dm, 0x1b28, MASKDWORD, 0x0);
	odm_set_bb_reg(dm, 0x1b30, MASKDWORD, 0x40000000);
	odm_set_bb_reg(dm, 0x1bcc, 0x3f, 0x3f);
}


u64 halrf_get_iqk_psd_data(void *dm_void, u32 point)
{
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &(dm->rf_table);
	struct _halrf_psd_data *psd = &(rf->halrf_psd_data);
	u64 psd_val, psd_val1, psd_val2;
	u32 psd_point, i, delay_time = 0;

#if (DEV_BUS_TYPE == RT_USB_INTERFACE) || (DEV_BUS_TYPE == RT_SDIO_INTERFACE)
	if (dm->support_interface == ODM_ITRF_USB || dm->support_interface == ODM_ITRF_SDIO) {
		if (dm->support_ic_type & (ODM_RTL8822C | ODM_RTL8723F | ODM_RTL8814C))
			delay_time = 1000;
		else
			delay_time = 0;
	}
#endif
#if (DEV_BUS_TYPE == RT_PCI_INTERFACE)
	if (dm->support_interface == ODM_ITRF_PCIE) {
		if (dm->support_ic_type & (ODM_RTL8822C | ODM_RTL8814C))
			delay_time = 1000;
		else
			delay_time = 150;
	}
#endif
	psd_point = odm_get_bb_reg(dm, R_0x1b2c, MASKDWORD);

	psd_point &= 0xF000FFFF;

	point &= 0xFFF;

	psd_point = psd_point | (point << 16);

	odm_set_bb_reg(dm, R_0x1b2c, MASKDWORD, psd_point);

	odm_set_bb_reg(dm, R_0x1b34, BIT(0), 0x1);

	odm_set_bb_reg(dm, R_0x1b34, BIT(0), 0x0);

	for (i = 0; i < delay_time; i++)
		ODM_delay_us(1);

	if (dm->support_ic_type & (ODM_RTL8197G | ODM_RTL8198F)) {
		if (dm->support_ic_type & ODM_RTL8197G)
			odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x001a0001);
		else
			odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x00250001);

		psd_val1 = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);

		psd_val1 = (psd_val1 & 0x001f0000) >> 16;

		if (dm->support_ic_type & ODM_RTL8197G)
			odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x001b0001);
		else
			odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x002e0001);

		psd_val2 = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);

		psd_val = (psd_val1 << 27) + (psd_val2 >> 5);
	} else if (dm->support_ic_type & ODM_RTL8723F) {
		odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x00210001);
		psd_val1 = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);
		psd_val1 = (psd_val1 & 0x00FF0000) >> 16;
		odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x00220001);
		psd_val2 = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);
		//psd_val = (psd_val1 << 27) + (psd_val2 >> 5);
		psd_val = (psd_val1 << 32) + psd_val2;
	} else {
		odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x00250001);

		psd_val1 = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);

		psd_val1 = (psd_val1 & 0x07FF0000) >> 16;

		odm_set_bb_reg(dm, R_0x1bd4, MASKDWORD, 0x002e0001);

		psd_val2 = odm_get_bb_reg(dm, R_0x1bfc, MASKDWORD);

		psd_val = (psd_val1 << 21) + (psd_val2 >> 11);
	}

	return psd_val;
}

void halrf_iqk_psd(
	struct dm_struct *dm,
	u32 point,
	u32 start_point,
	u32 stop_point,
	u32 average)
{
	struct _hal_rf_ *rf = &(dm->rf_table);
	struct _halrf_psd_data *psd = &(rf->halrf_psd_data);

	u32 i = 0, j = 0, k = 0;
	u32 psd_reg, avg_org, point_temp, average_tmp = 32, mode, reg_tmp = 5;
	u64 data_tatal = 0, data_temp[64] = {0};
	s32 s_point_tmp;

	psd->buf_size = 256;

	mode = average >> 16;

	if (mode == 2) {
		if (dm->support_ic_type & (ODM_RTL8822C | ODM_RTL8723F | ODM_RTL8814C))
			average_tmp = 1; //HW average
		else {
			reg_tmp = odm_get_bb_reg(dm, R_0x1b1c, 0x000e0000);
			if (reg_tmp == 0)
				average_tmp = 1;
			else if (reg_tmp == 3)
				average_tmp = 8;
			else if (reg_tmp == 4)
				average_tmp = 16;
			else if (reg_tmp == 5)
				average_tmp = 32;
			odm_set_bb_reg(dm, R_0x1b1c, 0x000e0000, 0x0);
		}
	} else {
		reg_tmp = odm_get_bb_reg(dm, R_0x1b1c, 0x000e0000);
		if (reg_tmp == 0)
			average_tmp = 1;
		else if (reg_tmp == 3)
			average_tmp = 8;
		else if (reg_tmp == 4)
			average_tmp = 16;
		else if (reg_tmp == 5)
			average_tmp = 32;
		if (!(dm->support_ic_type & ODM_RTL8723F))
			odm_set_bb_reg(dm, R_0x1b1c, 0x000e0000, 0x0);
	}

#if 0
	DbgPrint("[PSD]point=%d, start_point=%d, stop_point=%d, average=0x%x, average_tmp=%d, buf_size=%d, mode=%d\n",
				point, start_point, stop_point, average, average_tmp, psd->buf_size, mode);
#endif

	for (i = 0; i < psd->buf_size; i++)
		psd->psd_data[i] = 0;

	i = start_point;

	if (dm->support_ic_type & ODM_RTL8723F) {
		while (i < stop_point) {
			data_tatal = 0;

			if (i >= point)
				point_temp = i - point;
			else
				point_temp = i + 0xB00;
			//-640:0xD80,640:0x280,0x280+0xB00 =0xD80
				//point_temp = i + 0xC00;
			//-512:0xE00,512:0x200,0x200+0xC00 = 0xE00

			data_temp[k] = halrf_get_iqk_psd_data(dm, point_temp);
			data_tatal = data_temp[k];
			psd->psd_data[j] = (u32)data_tatal;
			i++;
			j++;
		}
	} else {
		while (i < stop_point) {
			data_tatal = 0;

			if (i >= point)
				point_temp = i - point;
			else
			{
				if (dm->support_ic_type & (ODM_RTL8814B | ODM_RTL8814C))
				{
					s_point_tmp = i - point - 1;
					point_temp = s_point_tmp & 0xfff;
				}
				else
					point_temp = i;
			}

			for (k = 0; k < average_tmp; k++) {
				data_temp[k] = halrf_get_iqk_psd_data(dm, point_temp);
				/*data_tatal = data_tatal + (data_temp[k] * data_temp[k]);*/
				data_tatal = data_tatal + data_temp[k];

#if 0
				if ((k % 20) == 0)
					DbgPrint("\n ");

				DbgPrint("0x%x ", data_temp[k]);
#endif
			}

			data_tatal = phydm_division64((data_tatal * 10), average_tmp);
			psd->psd_data[j] = (u32)data_tatal;

			i++;
			j++;
		}

		if (dm->support_ic_type & (ODM_RTL8814B | ODM_RTL8198F | ODM_RTL8197G))
			odm_set_bb_reg(dm, R_0x1b1c, 0x000e0000, reg_tmp);
	} 
	
#if 0
	DbgPrint("\n [iqk psd]psd result:\n");

	for (i = 0; i < psd->buf_size; i++) {
		if ((i % 20) == 0)
		DbgPrint("\n ");

		DbgPrint("0x%x ", psd->psd_data[i]);
	}
	DbgPrint("\n\n");
#endif
}


u32
halrf_psd_init(
	void *dm_void)
{
	enum rt_status ret_status = RT_STATUS_SUCCESS;
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &(dm->rf_table);
	struct _halrf_psd_data *psd = &(rf->halrf_psd_data);

	u32 bb_backup[18];
	u32 backup_bb_addr[18] = {0};
	u32 bk_counter = 18;
	
	u32 backup_bb_reg[12] = {0x1b04, 0x1b08, 0x1b0c, 0x1b14, 0x1b18,
				0x1b1c, 0x1b28, 0x1bcc, 0x1b2c, 0x1b34,
				0x1bd4, 0x1bfc};

	u32 backup_bb_reg_8723f[11] = {0x09f0, 0x09b4, 0x1c38, 0x1860, 0x1cd0,
				 	0x824, 0x2a24, 0x1d40, 0x1c20, 0x1880,
				 	0x180c};

	u32 backup_bb_reg_8814c[18] = {0x1e24, 0x1cd0, 0x1b08, 0x1d58, 0x1834,
					0x4134, 0x5234, 0x5334, 0x180c, 0x410c,
					0x520c, 0x530c, 0x186c, 0x416c, 0x526c,
					0x536c, 0x1a00, 0x1c38};

	if (dm->support_ic_type & ODM_RTL8723F) {
		odm_move_memory(dm, backup_bb_addr, backup_bb_reg_8723f,
			sizeof(backup_bb_addr));
		bk_counter = 11;
	} else if (dm->support_ic_type & ODM_RTL8814C) {
		odm_move_memory(dm, backup_bb_addr, backup_bb_reg_8814c,
			sizeof(backup_bb_addr));
		bk_counter = 18;
	} else {
		odm_move_memory(dm, backup_bb_addr, backup_bb_reg,
			sizeof(backup_bb_addr));
		bk_counter = 12;
	}

	if (psd->psd_progress) {
		ret_status = RT_STATUS_PENDING;
	} else {
		psd->psd_progress = 1;
		if (dm->support_ic_type & ODM_RTL8723F) {
			backup_bb_register(dm, bb_backup, backup_bb_addr, bk_counter);
			_halrf_iqk_psd_init_8723f(dm, true);
			halrf_iqk_psd(dm, psd->point, psd->start_point, psd->stop_point, psd->average);
			_halrf_iqk_psd_init_8723f(dm, false);
			restore_bb_register(dm, bb_backup, backup_bb_addr, bk_counter);
		} else if (dm->support_ic_type & ODM_RTL8814C) {
			backup_bb_register(dm, bb_backup, backup_bb_addr, bk_counter);
			_halrf_psd_iqk_init_8814c(dm);
			halrf_iqk_psd(dm, psd->point, psd->start_point, psd->stop_point, psd->average);
			restore_bb_register(dm, bb_backup, backup_bb_addr, bk_counter);
		} else if (dm->support_ic_type & 
			(ODM_RTL8822C | ODM_RTL8814B | ODM_RTL8198F | ODM_RTL8197G)) {
			backup_bb_register(dm, bb_backup, backup_bb_addr, bk_counter);
			_halrf_psd_iqk_init(dm);
			halrf_iqk_psd(dm, psd->point, psd->start_point, psd->stop_point, psd->average);
			restore_bb_register(dm, bb_backup, backup_bb_addr, bk_counter);
		} else
			halrf_psd(dm, psd->point, psd->start_point, psd->stop_point, psd->average);
		psd->psd_progress = 0;
	}
	return ret_status;
}

u32
halrf_psd_query(
	void *dm_void,
	u32 *outbuf,
	u32 buf_size)
{
	enum rt_status ret_status = RT_STATUS_SUCCESS;
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &(dm->rf_table);
	struct _halrf_psd_data *psd = &(rf->halrf_psd_data);

	if (psd->psd_progress)
		ret_status = RT_STATUS_PENDING;
	else
		odm_move_memory(dm, outbuf, psd->psd_data,
				sizeof(u32) * psd->buf_size);

	return ret_status;
}

u32
halrf_psd_init_query(
	void *dm_void,
	u32 *outbuf,
	u32 point,
	u32 start_point,
	u32 stop_point,
	u32 average,
	u32 buf_size)
{
	enum rt_status ret_status = RT_STATUS_SUCCESS;
	struct dm_struct *dm = (struct dm_struct *)dm_void;
	struct _hal_rf_ *rf = &(dm->rf_table);
	struct _halrf_psd_data *psd = &(rf->halrf_psd_data);

	psd->point = point;
	psd->start_point = start_point;
	psd->stop_point = stop_point;
	psd->average = average;

	if (psd->psd_progress) {
		ret_status = RT_STATUS_PENDING;
	} else {
		psd->psd_progress = 1;
		halrf_psd(dm, psd->point, psd->start_point, psd->stop_point, psd->average);
		odm_move_memory(dm, outbuf, psd->psd_data, 0x400);
		psd->psd_progress = 0;
	}

	return ret_status;
}
