/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	mcu_and.c

	Abstract:
	on-chip CPU related codes

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include	"rt_config.h"

#ifdef BTCOEX_CONCURRENT
extern int CoexCenctralChannel;
extern int CoexChannel;
extern int CoexChannelBw;
#endif

#ifdef RTMP_PCI_SUPPORT
int andes_pci_load_rom_patch(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	UINT32 start_offset, end_offset;
	UINT32 loop = 0, idx = 0, val = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	UINT32 mac_value;
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;
	
	if (cap->rom_code_protect) {
loadfw_protect:
		RTMP_IO_READ32(ad, SEMAPHORE_03, &mac_value);
		loop++;

		if (((mac_value & 0x01) == 0x00) && (loop < GET_SEMAPHORE_RETRY_MAX)) {
			RtmpOsMsDelay(1);
			goto loadfw_protect;
		}
		
		if (loop >= GET_SEMAPHORE_RETRY_MAX) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s: can not get the hw semaphore\n", __FUNCTION__));
			return NDIS_STATUS_FAILURE;
		}
	}

	/* check rom patch if ready */
	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		RTMP_IO_READ32(ad, CLOCK_CTL, &mac_value);
	} else {
		RTMP_IO_READ32(ad, COM_REG0, &mac_value);
	}

	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		if (((mac_value & 0x01) == 0x01) && (cap->rom_code_protect)) {
			goto done;
		}
	} else {
		if (((mac_value & 0x02) == 0x02) && (cap->rom_code_protect)) {
			goto done;
		}
	}
	
#ifndef BB_SOC	
	if (cap->load_code_method == BIN_FILE_METHOD) {
		DBGPRINT(RT_DEBUG_OFF, ("load rom patch from /lib/firmware/%s\n", cap->rom_patch_bin_file_name));
		OS_LOAD_CODE_FROM_BIN(&cap->rom_patch, cap->rom_patch_bin_file_name, obj->pci_dev, &cap->rom_patch_len); 
	} else
#endif /* !BB_SOC */	
	{
		cap->rom_patch = cap->rom_patch_header_image;
	}

	if (!cap->rom_patch) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a rom patch(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->rom_patch_bin_file_name, cap->load_code_method));
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a rom patch, load_method(%d)\n", 
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

#ifdef DBG
	/* get rom patch information */
	DBGPRINT(RT_DEBUG_OFF, ("build time = "));
	for (loop = 0; loop < 16; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->rom_patch + loop)));
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
#endif

	if (IS_MT76x2(ad)) {
		if (((strncmp(cap->rom_patch, "20130809", 8) >= 0)) && (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))) {
			DBGPRINT(RT_DEBUG_OFF, ("rom patch for E3 IC\n"));
		} else if (((strncmp(cap->rom_patch, "20130809", 8) < 0)) && (MT_REV_LT(ad, MT76x2, REV_MT76x2E3))){
			DBGPRINT(RT_DEBUG_OFF, ("rom patch for E2 IC\n"));
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("rom patch do not match IC version\n"));
			RTMP_IO_READ32(ad, 0x0, &mac_value);
			DBGPRINT(RT_DEBUG_OFF, ("IC version(%x)\n", mac_value));
			ret = NDIS_STATUS_FAILURE;
			goto done;
		}
	}
	
#ifdef DBG
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("platform = "));
	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->rom_patch + 16 + loop)));
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("hw/sw version = "));
	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%x", *(cap->rom_patch + 20 + loop)));
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("patch version = "));
	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%x", *(cap->rom_patch + 24 + loop)));
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
#endif

	RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, cap->rom_patch_offset - 10000);
		
	start_offset = PATCH_INFO_SIZE;
	end_offset = cap->rom_patch_len;
	
	/* Load rom patch code */
	for (idx = start_offset; idx < end_offset; idx += 4)
	{
		val = (*(cap->rom_patch + idx)) +
		   (*(cap->rom_patch + idx + 3) << 24) +
		   (*(cap->rom_patch + idx + 2) << 16) +
		   (*(cap->rom_patch + idx + 1) << 8);

		RTMP_IO_WRITE32(ad, 0x90000 + (idx - PATCH_INFO_SIZE), val);
	}
	
	RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, 0x0);

	/* Trigger rom */
	RTMP_IO_WRITE32(ad, INT_LEVEL, 0x04);
	
	/* check rom if ready */
	loop = 0;
	do
	{
		if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
			RTMP_IO_READ32(ad, CLOCK_CTL, &mac_value);
			if ((mac_value & 0x01)== 0x01)
				break;
		} else {
			RTMP_IO_READ32(ad, COM_REG0, &mac_value);
			if ((mac_value & 0x02)== 0x02)
				break;
		}

		RtmpOsMsDelay(10);
		loop++;
	} while (loop <= 200);

	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: CLOCK_CTL(0x%x) = 0x%x\n", __FUNCTION__, CLOCK_CTL, mac_value));
		if ((mac_value & 0x01) != 0x01)
		ret = NDIS_STATUS_FAILURE;
	} else {
		DBGPRINT(RT_DEBUG_TRACE, ("%s: COM_REG0(0x%x) = 0x%x\n", __FUNCTION__, COM_REG0, mac_value));
		if ((mac_value & 0x02) != 0x02)
			ret = NDIS_STATUS_FAILURE;
	}	

done:
	if (cap->rom_code_protect)
		RTMP_IO_WRITE32(ad, SEMAPHORE_03, 0x1);
	
	return ret;
}

int andes_pci_erase_rom_patch(RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	if (cap->load_code_method == BIN_FILE_METHOD) {
		if (cap->rom_patch)
			os_free_mem(NULL, cap->rom_patch);
			cap->rom_patch = NULL;
	}

	return 0;
}

int andes_pci_erasefw(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	u32 ilm_len, dlm_len;
	u16 fw_ver, build_ver;
	u32 loop = 0, idx = 0/*, val = 0*/;
	u32 mac_value;
	u32 start_offset, end_offset;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	if (!ad->chipCap.ram_code_protect) {

		ilm_len = (*(cap->FWImageName + 3) << 24) | (*(cap->FWImageName + 2) << 16) |
				 (*(cap->FWImageName + 1) << 8) | (*cap->FWImageName);

		dlm_len = (*(cap->FWImageName + 7) << 24) | (*(cap->FWImageName + 6) << 16) |
				 (*(cap->FWImageName + 5) << 8) | (*(cap->FWImageName + 4));

		fw_ver = (*(cap->FWImageName + 11) << 8) | (*(cap->FWImageName + 10));

		build_ver = (*(cap->FWImageName + 9) << 8) | (*(cap->FWImageName + 8));
	
#ifdef DBG
		DBGPRINT(RT_DEBUG_TRACE, ("FW Version:%d.%d.%02d ", (fw_ver & 0xf000) >> 8,
							(fw_ver & 0x0f00) >> 8, fw_ver & 0x00ff));
		DBGPRINT(RT_DEBUG_TRACE, ("Build:%x\n", build_ver));

		DBGPRINT(RT_DEBUG_TRACE, ("Build Time:"));
		for (loop = 0; loop < 16; loop++)
			DBGPRINT(RT_DEBUG_TRACE, ("%c", *(cap->FWImageName + 16 + loop)));
		DBGPRINT(RT_DEBUG_TRACE, ("\n"));

		DBGPRINT(RT_DEBUG_TRACE, ("ILM Length = %d(bytes)\n", ilm_len));
		DBGPRINT(RT_DEBUG_TRACE, ("DLM Length = %d(bytes)\n", dlm_len));
#endif
		RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, cap->ilm_offset);

		if (IS_MT76x2(ad)) {
			start_offset = FW_INFO_SIZE;
			RTMP_IO_WRITE32(ad, INT_LEVEL, 0x1);
			RtmpOsMsDelay(20);
		} else {
			if (cap->ram_code_protect) {
				RTMP_IO_WRITE32(ad, CPU_CTL, 0x0);
				RTMP_IO_WRITE32(ad, CPU_CTL, (0x1<<20));
				RTMP_IO_WRITE32(ad, RESET_CTL, 0x10);
				RTMP_IO_WRITE32(ad, RESET_CTL, 0x300);
				RTMP_IO_WRITE32(ad, COM_REG0, 0x0);
	
				start_offset = FW_INFO_SIZE + IV_SIZE;
			} else {
				RTMP_IO_WRITE32(ad, RESET_CTL, 0x10); /* reset fce */
				RTMP_IO_WRITE32(ad, RESET_CTL, 0x200); /* reset cpu */
				RTMP_IO_WRITE32(ad, COM_REG0, 0x0); /* clear mcu ready bit */

				start_offset = FW_INFO_SIZE;
			}
		}

		end_offset = FW_INFO_SIZE + ilm_len;

		if (!IS_MT76x2(ad)) {
			/* erase ilm */
			for (idx = start_offset; idx < end_offset; idx += 4) {
				RTMP_IO_WRITE32(ad, 0x80000 + (idx - FW_INFO_SIZE), 0);
			}

			if (cap->ram_code_protect) {
				/* Loading IV part into last 64 bytes of ILM */
				start_offset = FW_INFO_SIZE;
				end_offset = FW_INFO_SIZE + IV_SIZE;
	
				for (idx = start_offset; idx < end_offset; idx += 4)
				{
					RTMP_IO_WRITE32(ad, 0x80000 + (0x54000 - IV_SIZE) + (idx - FW_INFO_SIZE), 0);
				}
			}

			RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, cap->dlm_offset);

			start_offset = 32 + ilm_len;
			end_offset = 32 + ilm_len + dlm_len;
	
			/* erase dlm */
			for (idx = start_offset; idx < end_offset; idx += 4) {
				RTMP_IO_WRITE32(ad, 0x80000 + (0x54000 - IV_SIZE) + (idx - FW_INFO_SIZE), 0);
			}
		}
	
		RTMP_IO_READ32(ad, COM_REG0, &mac_value);
		DBGPRINT(RT_DEBUG_TRACE, ("%s: COM_REG0(0x%x) = 0x%x\n", __FUNCTION__, COM_REG0, mac_value));
		RTMP_IO_READ32(ad, RESET_CTL, &mac_value);
		DBGPRINT(RT_DEBUG_TRACE, ("%s: RESET_CTL(0x%x) = 0x%x\n", __FUNCTION__, RESET_CTL, mac_value));
	}
	
	if (cap->load_code_method == BIN_FILE_METHOD) {
		if (cap->FWImageName)
			os_free_mem(NULL, cap->FWImageName);
			cap->FWImageName = NULL;
	}

	return ret;
}

NDIS_STATUS andes_pci_loadfw(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	UINT32 ilm_len, dlm_len;
	USHORT fw_ver, build_ver;
	UINT32 loop = 0, idx = 0, val = 0;
	UINT32 mac_value;
	UINT32 start_offset, end_offset;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	POS_COOKIE obj = (POS_COOKIE)ad->OS_Cookie;

	if (cap->ram_code_protect) {
loadfw_protect:
		RTMP_IO_READ32(ad, SEMAPHORE_00, &mac_value);
		loop++;

		if (((mac_value & 0x01) == 0) && (loop < GET_SEMAPHORE_RETRY_MAX)) {
			RtmpOsMsDelay(1);
			goto loadfw_protect;
		}

		if (loop >= GET_SEMAPHORE_RETRY_MAX) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s: can not get the hw semaphore\n", __FUNCTION__));
			return NDIS_STATUS_FAILURE;
		}
	}

	/* check MCU if ready */
	RTMP_IO_READ32(ad, COM_REG0, &mac_value);

	if (((mac_value & 0x01) == 0x01) && (cap->ram_code_protect)) {
		goto done;
	}
	
#ifndef BB_SOC	
	if (cap->load_code_method == BIN_FILE_METHOD) {
		DBGPRINT(RT_DEBUG_OFF, ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
		OS_LOAD_CODE_FROM_BIN(&cap->FWImageName, cap->fw_bin_file_name, obj->pci_dev, &cap->fw_len);
	} else
#endif /* !BB_SOC */	
	{
		cap->FWImageName = cap->fw_header_image;
	}

	if (!cap->FWImageName) {
		if (cap->load_code_method == BIN_FILE_METHOD) {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a fw image(/lib/firmware/%s), load_method(%d)\n", __FUNCTION__, cap->fw_bin_file_name, cap->load_code_method));
		} else {
			DBGPRINT(RT_DEBUG_ERROR, ("%s:Please assign a fw image, load_method(%d)\n", 
				__FUNCTION__, cap->load_code_method));
		}
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}
	
	ilm_len = (*(cap->FWImageName + 3) << 24) | (*(cap->FWImageName + 2) << 16) |
			 (*(cap->FWImageName + 1) << 8) | (*cap->FWImageName);

	dlm_len = (*(cap->FWImageName + 7) << 24) | (*(cap->FWImageName + 6) << 16) |
			 (*(cap->FWImageName + 5) << 8) | (*(cap->FWImageName + 4));

	fw_ver = (*(cap->FWImageName + 11) << 8) | (*(cap->FWImageName + 10));

	build_ver = (*(cap->FWImageName + 9) << 8) | (*(cap->FWImageName + 8));
	
#ifdef DBG
	DBGPRINT(RT_DEBUG_OFF, ("FW Version:%d.%d.%02d ", (fw_ver & 0xf000) >> 8,
						(fw_ver & 0x0f00) >> 8, fw_ver & 0x00ff));
	DBGPRINT(RT_DEBUG_OFF, ("Build:%x\n", build_ver));

	DBGPRINT(RT_DEBUG_OFF, ("Build Time:"));
	for (loop = 0; loop < 16; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->FWImageName + 16 + loop)));
	DBGPRINT(RT_DEBUG_OFF, ("\n"));
#endif
	
	if (IS_MT76x2(ad)) {
		if (((strncmp(cap->FWImageName + 16, "20130811", 8) >= 0)) && (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))) {
			DBGPRINT(RT_DEBUG_OFF, ("fw for E3 IC\n"));
		} else if (((strncmp(cap->FWImageName + 16, "20130811", 8) < 0)) && (MT_REV_LT(ad, MT76x2, REV_MT76x2E3))){
			DBGPRINT(RT_DEBUG_OFF, ("fw for E2 IC\n"));
		} else {
			DBGPRINT(RT_DEBUG_OFF, ("fw do not match IC version\n"));
			RTMP_IO_READ32(ad, 0x0, &mac_value);
			DBGPRINT(RT_DEBUG_OFF, ("IC version(%x)\n", mac_value));
			ret = NDIS_STATUS_FAILURE;
			goto done;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("ILM Length = %d(bytes)\n", ilm_len));
	DBGPRINT(RT_DEBUG_TRACE, ("DLM Length = %d(bytes)\n", dlm_len));
	
	RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, cap->ilm_offset);

	if (cap->ram_code_protect)
		start_offset = FW_INFO_SIZE + IV_SIZE;
	else
		start_offset = FW_INFO_SIZE;

	end_offset = FW_INFO_SIZE + ilm_len;
	
	/* Load ILM code */
	for (idx = start_offset; idx < end_offset; idx += 4)
	{
		val = (*(cap->FWImageName + idx)) +
		   (*(cap->FWImageName + idx + 3) << 24) +
		   (*(cap->FWImageName + idx + 2) << 16) +
		   (*(cap->FWImageName + idx + 1) << 8);

		RTMP_IO_WRITE32(ad, 0x80000 + (idx - FW_INFO_SIZE), val);
	}

	if (cap->ram_code_protect)
	{
		/* Loading IV part into last 64 bytes of ILM */
		start_offset = FW_INFO_SIZE;
		end_offset = FW_INFO_SIZE + IV_SIZE;
	
		for (idx = start_offset; idx < end_offset; idx += 4)
		{
			val = (*(cap->FWImageName + idx)) +
				(*(cap->FWImageName + idx + 3) << 24) +
				(*(cap->FWImageName + idx + 2) << 16) +
				(*(cap->FWImageName + idx + 1) << 8);

			RTMP_IO_WRITE32(ad, 0x80000 + (0x54000 - IV_SIZE) + (idx - FW_INFO_SIZE), val);
		}
	}

	if (IS_MT76x2(ad)) {
		RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, cap->dlm_offset - 0x10000);
	} else
		RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, cap->dlm_offset);

	start_offset = FW_INFO_SIZE + ilm_len;
	end_offset = FW_INFO_SIZE + ilm_len + dlm_len;
	
	/* Load DLM code */
	for (idx = start_offset; idx < end_offset; idx += 4)
	{
		val = (*(cap->FWImageName + idx)) +
		   (*(cap->FWImageName + idx + 3) << 24) +
		   (*(cap->FWImageName + idx + 2) << 16) +
		   (*(cap->FWImageName + idx + 1) << 8);
	
		if (IS_MT76x2(ad)) {
			if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3))
				RTMP_IO_WRITE32(ad, 0x90800 + (idx - FW_INFO_SIZE - ilm_len), val);
			else
				RTMP_IO_WRITE32(ad, 0x90000 + (idx - FW_INFO_SIZE - ilm_len), val);
		} else
			RTMP_IO_WRITE32(ad, 0x80000 + (idx - FW_INFO_SIZE - ilm_len), val);
	}

	RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, 0x0);

#ifdef RTMP_FLASH_SUPPORT
#ifdef MT76x2
	if (IS_MT76x2(ad))
	{
		UCHAR e2p_type;
		e2p_type = ad->E2pAccessMode;
		e2p_type = ((e2p_type != 0) && (e2p_type < NUM_OF_E2P_MODE)) ? e2p_type : RtmpEepromGetDefault(ad);


		if (e2p_type == E2P_FLASH_MODE)
		{
			USHORT ee_val = 0;
			
			rtmp_nv_init(ad);
			rtmp_ee_flash_read(ad, EEPROM_NIC3_OFFSET, &ee_val);
			ad->NicConfig3.word = ee_val;
			if (ad->NicConfig3.field.XtalOption == 0x1)
			{
				/*
					MAC 730 bit [30] = 1: Co-Clock Enable
				*/
				RTMP_IO_READ32(ad, COM_REG0, &mac_value);
				mac_value |= (1 << 30);
				RTMP_IO_WRITE32(ad, COM_REG0, mac_value);
			}
		}
	}
#endif /* MT76x2 */
#endif /* RTMP_FLASH_SUPPORT */


	if (cap->ram_code_protect)
	{
		/* Trigger Firmware */
		RTMP_IO_WRITE32(ad, INT_LEVEL, 0x03);
	}
	else
	{
		if (IS_MT76x2(ad))
			RTMP_IO_WRITE32(ad, INT_LEVEL, 0x2);
		else
			RTMP_IO_WRITE32(ad, RESET_CTL, 0x300);
	}

	/* check MCU if ready */
	loop = 0;
	do
	{
		RTMP_IO_READ32(ad, COM_REG0, &mac_value);
		if ((mac_value & 0x01)== 0x1)
			break;
		RtmpOsMsDelay(10);
		loop++;
	} while (loop <= 200);

	DBGPRINT(RT_DEBUG_TRACE, ("%s: COM_REG0(0x%x) = 0x%x\n", __FUNCTION__, COM_REG0, mac_value));
	
	RTMP_IO_READ32(ad, COM_REG0, &mac_value);
	mac_value |= (1 << 1);
	RTMP_IO_WRITE32(ad, COM_REG0, mac_value);

	if ((mac_value & 0x01 ) != 0x1)
		ret = NDIS_STATUS_FAILURE;

done:
	if (cap->ram_code_protect)
		RTMP_IO_WRITE32(ad, SEMAPHORE_00, 0x1);
	
	return ret;
}
#endif


int andes_usb_erasefw(RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *cap = &ad->chipCap;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	if (cap->load_code_method == BIN_FILE_METHOD) {
		if (cap->FWImageName)
			os_free_mem(NULL, cap->FWImageName);
			cap->FWImageName = NULL;
	}

	return 0;
}

struct cmd_msg *andes_alloc_cmd_msg(RTMP_ADAPTER *ad, unsigned int length)
{
	struct cmd_msg *msg = NULL;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	PNDIS_PACKET net_pkt = NULL;

	net_pkt = RTMP_AllocateFragPacketBuffer(ad, cap->cmd_header_len + length + cap->cmd_padding_len);

	if (!net_pkt) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate net_pkt\n"));
		goto error0;
	}

	OS_PKT_RESERVE(net_pkt, cap->cmd_header_len);

	os_alloc_mem(NULL, (PUCHAR *)&msg, sizeof(*msg));

	if (!msg) {
		DBGPRINT(RT_DEBUG_ERROR, ("can not allocate cmd msg\n"));
		goto error1;
	}

	CMD_MSG_CB(net_pkt)->msg = msg;

	memset(msg, 0x00, sizeof(*msg));
	

	msg->priv = (void *)ad;
	msg->net_pkt = net_pkt;

	ctl->alloc_cmd_msg++;

	return msg;

	os_free_mem(NULL, msg);
error1:
	RTMPFreeNdisPacket(ad, net_pkt);
error0:
	return NULL;
}

void andes_init_cmd_msg(struct cmd_msg *msg, u8 type, BOOLEAN need_wait, u16 timeout, 
							   BOOLEAN need_retransmit, BOOLEAN need_rsp, u16 rsp_payload_len, 
							   char *rsp_payload, MSG_RSP_HANDLER rsp_handler)
{
	msg->type = type;
	msg->need_wait= FALSE;
	msg->timeout = timeout;

	if (need_wait) {
#ifdef RTMP_PCI_SUPPORT
		msg->ack_done = FALSE;
#endif

	}

	msg->need_retransmit = 0;

		msg->retransmit_times = 0;
	
	msg->need_rsp = FALSE;
	msg->rsp_payload_len = rsp_payload_len;
	msg->rsp_payload = rsp_payload;
	msg->rsp_handler = rsp_handler;
}

void andes_append_cmd_msg(struct cmd_msg *msg, char *data, unsigned int len)
{
	PNDIS_PACKET net_pkt = msg->net_pkt;

	if (data)
		memcpy(OS_PKT_TAIL_BUF_EXTEND(net_pkt, len), data, len);
}

void andes_free_cmd_msg(struct cmd_msg *msg)
{
	PNDIS_PACKET net_pkt = msg->net_pkt;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)(msg->priv);
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (msg->need_wait) {
#ifdef RTMP_PCI_SUPPORT
		msg->ack_done = FALSE;
#endif

	}


	os_free_mem(NULL, msg);
	
	RTMPFreeNdisPacket(ad, net_pkt);
	ctl->free_cmd_msg++;
}

BOOLEAN is_inband_cmd_processing(RTMP_ADAPTER *ad)
{
	BOOLEAN ret = 0;

	return ret;
}

UCHAR get_cmd_rsp_num(RTMP_ADAPTER *ad)
{
	UCHAR Num = 0;

	return Num;
}

static inline void andes_inc_error_count(struct MCU_CTRL *ctl, enum cmd_msg_error_type type)
{
	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		switch (type) {
		case error_tx_kickout_fail:
			ctl->tx_kickout_fail_count++;
		break;
		case error_tx_timeout_fail:
			ctl->tx_timeout_fail_count++;
		break;
		case error_rx_receive_fail:
			ctl->rx_receive_fail_count++;
		break;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("%s:unknown cmd_msg_error_type(%d)\n", __FUNCTION__, type));
		}
	}
}

static NDIS_SPIN_LOCK *andes_get_spin_lock(struct MCU_CTRL *ctl, DL_LIST *list)
{
	NDIS_SPIN_LOCK *lock = NULL;

	if (list == &ctl->txq)
		lock = &ctl->txq_lock;
	else if (list == &ctl->rxq)
		lock = &ctl->rxq_lock;
	else if (list == &ctl->ackq)
		lock = &ctl->ackq_lock;
	else if (list == &ctl->kickq)
		lock = &ctl->kickq_lock;
	else if (list == &ctl->tx_doneq)
		lock = &ctl->tx_doneq_lock;
	else if (list == &ctl->rx_doneq)
		lock = &ctl->rx_doneq_lock;
	else
		DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal list\n", __FUNCTION__));

	return lock;
} 

static inline UCHAR andes_get_cmd_msg_seq(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg;
	unsigned long flags;

	RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
get_seq:
	ctl->cmd_seq >= 0xf ? ctl->cmd_seq = 1 : ctl->cmd_seq++;
	DlListForEach(msg, &ctl->ackq, struct cmd_msg, list) {
		if (msg->seq == ctl->cmd_seq) {
			DBGPRINT(RT_DEBUG_ERROR, ("command(seq: %d) is still running\n", ctl->cmd_seq));
			DBGPRINT(RT_DEBUG_ERROR, ("command response nums = %d\n", get_cmd_rsp_num(ad)));
			goto get_seq;
		}
	}
	RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);

	return ctl->cmd_seq;
}

static void _andes_queue_tail_cmd_msg(DL_LIST *list, struct cmd_msg *msg, 
										enum cmd_msg_state state)
{
	msg->state = state;
	DlListAddTail(list, &msg->list);
}

static void andes_queue_tail_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	_andes_queue_tail_cmd_msg(list, msg, state);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

static void _andes_queue_head_cmd_msg(DL_LIST *list, struct cmd_msg *msg, 
										enum cmd_msg_state state)
{
	msg->state = state;
	DlListAdd(list, &msg->list);
}

static void andes_queue_head_cmd_msg(DL_LIST *list, struct cmd_msg *msg,
										enum cmd_msg_state state)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	
	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);	
	_andes_queue_head_cmd_msg(list, msg, state);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

static u32 andes_queue_len(struct MCU_CTRL *ctl, DL_LIST *list)
{
	u32 qlen;
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	
	lock = andes_get_spin_lock(ctl, list);
	
	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	qlen = DlListLen(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	return qlen;
}


static void andes_queue_init(struct MCU_CTRL *ctl, DL_LIST *list)
{

	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	
	lock = andes_get_spin_lock(ctl, list);
	
	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListInit(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

static void _andes_unlink_cmd_msg(struct cmd_msg *msg, DL_LIST *list)
{
	if (!msg)
		return;

	DlListDel(&msg->list);		
}

static void andes_unlink_cmd_msg(struct cmd_msg *msg, DL_LIST *list)
{
	unsigned long flags;
	NDIS_SPIN_LOCK *lock;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	
	lock = andes_get_spin_lock(ctl, list);
	
	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	_andes_unlink_cmd_msg(msg, list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

static struct cmd_msg *_andes_dequeue_cmd_msg(DL_LIST *list)
{
	struct cmd_msg *msg;

	msg = DlListFirst(list, struct cmd_msg, list);

	_andes_unlink_cmd_msg(msg, list);

	return msg;
}

static struct cmd_msg *andes_dequeue_cmd_msg(struct MCU_CTRL *ctl, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg;
	NDIS_SPIN_LOCK *lock;
	
	lock = andes_get_spin_lock(ctl, list);
	
	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	msg = _andes_dequeue_cmd_msg(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);

	return msg;
}

void andes_rx_process_cmd_msg(RTMP_ADAPTER *ad, struct cmd_msg *rx_msg)
{
	PNDIS_PACKET net_pkt = rx_msg->net_pkt;
	struct cmd_msg *msg, *msg_tmp;
	RXFCE_INFO_CMD *rx_info = (RXFCE_INFO_CMD *)GET_OS_PKT_DATAPTR(net_pkt);
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	unsigned long flags;
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)rx_info, TYPE_RXINFO);
#endif

	DBGPRINT(RT_DEBUG_INFO, ("(andex_rx_cmd)info_type=%d,evt_type=%d,d_port=%d,"
                                "qsel=%d,pcie_intr=%d,cmd_seq=%d,"
                                "self_gen=%d,pkt_len=%d\n",
                                rx_info->info_type, rx_info->evt_type,rx_info->d_port,
                                rx_info->qsel, rx_info->pcie_intr, rx_info->cmd_seq,
                                rx_info->self_gen, rx_info->pkt_len));

	if ((rx_info->info_type != CMD_PACKET)) {
		DBGPRINT(RT_DEBUG_ERROR, ("packet is not command response/self event\n"));
		return;
	} 
#ifdef DBG
   #define LOG2HOST 3
   if (rx_info->evt_type == LOG2HOST)
   {
       PUCHAR pRxRspEvtPayload = GET_OS_PKT_DATAPTR(net_pkt) + sizeof(RXFCE_INFO_CMD);
       DBGPRINT(RT_DEBUG_WARN, ("[FW]%s\n", pRxRspEvtPayload));
    }
#endif
	if (rx_info->self_gen) {
		/* if have callback function */
		RTEnqueueInternalCmd(ad, CMDTHREAD_RESPONSE_EVENT_CALLBACK, 
								GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*rx_info), rx_info->pkt_len);
	} else {

#ifdef RTMP_PCI_SUPPORT
		RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
#endif
		DlListForEachSafe(msg, msg_tmp, &ctl->ackq, struct cmd_msg, list) {
			if (msg->seq == rx_info->cmd_seq)
			{
				_andes_unlink_cmd_msg(msg, &ctl->ackq);

#ifdef RTMP_PCI_SUPPORT
				//RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);
#endif
				
				if ((msg->rsp_payload_len == rx_info->pkt_len) && (msg->rsp_payload_len != 0))
				{
					msg->rsp_handler(msg, GET_OS_PKT_DATAPTR(net_pkt) + sizeof(*rx_info), rx_info->pkt_len);
				}
				else if ((msg->rsp_payload_len == 0) && (rx_info->pkt_len == 8))
				{
					DBGPRINT(RT_DEBUG_INFO, ("command response(ack) success\n"));
				}
				else
				{
					DBGPRINT(RT_DEBUG_ERROR, ("expect response len(%d), command response len(%d) invalid\n", msg->rsp_payload_len, rx_info->pkt_len));
					msg->rsp_payload_len = rx_info->pkt_len;
				}

				if (msg->need_wait) {
#ifdef RTMP_PCI_SUPPORT
					msg->ack_done = TRUE;
#endif

				} else {
					andes_free_cmd_msg(msg);
				}

#ifdef RTMP_PCI_SUPPORT
				//RTMP_SPIN_LOCK_IRQSAVE(&ctl->ackq_lock, &flags);
#endif
				break;
			}
		}


#ifdef RTMP_PCI_SUPPORT
		RTMP_SPIN_UNLOCK_IRQRESTORE(&ctl->ackq_lock, &flags);
#endif
	}
}

#ifdef RTMP_PCI_SUPPORT
void pci_rx_cmd_msg_complete(RTMP_ADAPTER *ad, RXFCE_INFO *fce_info, PUCHAR payload)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		return;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	msg = andes_alloc_cmd_msg(ad, sizeof(*fce_info) + fce_info->pkt_len);
	
	if (!msg)
		return;
#ifdef MT76x2
	andes_append_cmd_msg(msg, (char *)fce_info, sizeof(*fce_info));
	andes_append_cmd_msg(msg, (char *)payload, fce_info->pkt_len);
#else
	andes_append_cmd_msg(msg, (char *)fce_info, sizeof(*fce_info) + fce_info->pkt_len);
#endif /* MT76x2 */
	
	andes_rx_process_cmd_msg(ad, msg);

	andes_free_cmd_msg(msg);
}

#endif


void andes_cmd_msg_bh(unsigned long param)
{
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)param;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	struct cmd_msg *msg = NULL;

	while ((msg = andes_dequeue_cmd_msg(ctl, &ctl->rx_doneq))) {
		switch (msg->state) {
		case rx_done:
			andes_rx_process_cmd_msg(ad, msg);
			andes_free_cmd_msg(msg);
			continue;
		case rx_receive_fail:
			andes_free_cmd_msg(msg);
			continue;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("unknow msg state(%d)\n", msg->state));
		}
	}

	while ((msg = andes_dequeue_cmd_msg(ctl, &ctl->tx_doneq))) {
		switch (msg->state) {
		case tx_done:
		case tx_kickout_fail:
		case tx_timeout_fail:
			andes_free_cmd_msg(msg);
			continue;
		default:
			DBGPRINT(RT_DEBUG_ERROR, ("unknow msg state(%d)\n", msg->state));
		}
	}

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		andes_bh_schedule(ad);
	}
}

void andes_bh_schedule(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;

	if (((andes_queue_len(ctl, &ctl->rx_doneq) > 0) 
							|| (andes_queue_len(ctl, &ctl->tx_doneq) > 0)) 
							&& OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifndef WORKQUEUE_BH
		RTMP_NET_TASK_DATA_ASSIGN(&ctl->cmd_msg_task, (unsigned long)(ad));	
		RTMP_OS_TASKLET_SCHE(&ctl->cmd_msg_task);
#else
		tasklet_hi_schedule(&ctl->cmd_msg_task);
#endif
	}
}

#ifdef RTMP_PCI_SUPPORT
#ifdef DBG
static UCHAR *txinfo_type_str[]={"PKT", "CMD", "RSV"};
static UCHAR *txinfo_d_port_str[]={"WLAN", "CPU_RX", "CPU_TX", "HOST", "VIRT_RX", "VIRT_TX", "DROP"};

VOID dump_cmd_txinfo(RTMP_ADAPTER *ad, TXINFO_STRUC *pTxInfo)
{
	DBGPRINT(RT_DEBUG_OFF, ("TxInfo:\n"));
	{
		struct _TXINFO_NMAC_CMD *cmd_txinfo = (struct _TXINFO_NMAC_CMD *)pTxInfo;
	
		hex_dump("Raw Data: ", (UCHAR *)pTxInfo, sizeof(TXINFO_STRUC));
		DBGPRINT(RT_DEBUG_OFF, ("\t Info_Type=%d(%s)\n", cmd_txinfo->info_type, txinfo_type_str[cmd_txinfo->info_type]));
		DBGPRINT(RT_DEBUG_OFF, ("\t d_port=%d(%s)\n", cmd_txinfo->d_port, txinfo_d_port_str[cmd_txinfo->d_port]));
		DBGPRINT(RT_DEBUG_OFF, ("\t cmd_type=%d\n", cmd_txinfo->cmd_type));
		DBGPRINT(RT_DEBUG_OFF, ("\t cmd_seq=%d\n", cmd_txinfo->cmd_seq));
		DBGPRINT(RT_DEBUG_OFF, ("\t pkt_len=0x%x\n", cmd_txinfo->pkt_len));
	}

	DBGPRINT(RT_DEBUG_OFF, ("\t"));
}
#endif

int pci_kick_out_cmd_msg(
	PRTMP_ADAPTER ad,
	struct cmd_msg *msg)
{
	int ret = NDIS_STATUS_SUCCESS;
	unsigned long flags = 0;
	ULONG FreeNum;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	u32 SwIdx = 0, SrcBufPA;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen = 0;
	PACKET_INFO PacketInfo;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
	UCHAR tx_hw_info[TXD_SIZE];
#endif

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return -1;
	
	FreeNum = GET_CTRLRING_FREENO(ad);
	
	if (FreeNum == 0) {
		DBGPRINT(RT_DEBUG_WARN, ("%s FreeNum == 0 (TxCpuIdx = %d, TxDmaIdx = %d, TxSwFreeIdx = %d)\n",
		__FUNCTION__, ad->CtrlRing.TxCpuIdx, ad->CtrlRing.TxDmaIdx, ad->CtrlRing.TxSwFreeIdx));
		return NDIS_STATUS_FAILURE;
	}

	RTMP_SPIN_LOCK_IRQSAVE(&ad->CtrlRingLock, &flags);	

	RTMP_QueryPacketInfo(net_pkt, &PacketInfo, &pSrcBufVA, &SrcBufLen);
	
	if (pSrcBufVA == NULL) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&ad->CtrlRingLock, &flags);
		return NDIS_STATUS_FAILURE;
	}

	SwIdx = ad->CtrlRing.TxCpuIdx;
#ifdef RT_BIG_ENDIAN
	pDestTxD  = (PTXD_STRUC)ad->CtrlRing.Cell[SwIdx].AllocVa;
	NdisMoveMemory(&tx_hw_info[0], (UCHAR *)pDestTxD, TXD_SIZE);
 	pTxD = (PTXD_STRUC)&tx_hw_info[0];
#else
	pTxD  = (PTXD_STRUC)ad->CtrlRing.Cell[SwIdx].AllocVa;
#endif

	pTxInfo = (TXINFO_STRUC *)((UCHAR *)pTxD + sizeof(TXD_STRUC));
	NdisMoveMemory(pTxInfo, pSrcBufVA, TXINFO_SIZE);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
#endif /* RT_BIG_ENDIAN */

	ad->CtrlRing.Cell[SwIdx].pNdisPacket = net_pkt;
	ad->CtrlRing.Cell[SwIdx].pNextNdisPacket = NULL;

#ifndef RT_SECURE_DMA
	SrcBufPA = PCI_MAP_SINGLE(ad, (pSrcBufVA) + 4, (SrcBufLen) - 4, 0, RTMP_PCI_DMA_TODEVICE);
#else
	NdisMoveMemory(ad->CtrlSecureDMA.AllocVa + (SwIdx * 4096), pSrcBufVA + 4, (SrcBufLen - TXINFO_SIZE));
	SrcBufPA = ad->CtrlSecureDMA.AllocPa + (SwIdx * 4096);
#endif

	pTxD->LastSec0 = 1;
	pTxD->LastSec1 = 0;
	pTxD->SDLen0 = (SrcBufLen - TXINFO_SIZE);
	pTxD->SDLen1 = 0;
	pTxD->SDPtr0 = SrcBufPA;
	pTxD->DMADONE = 0;
	
#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)pTxD, TYPE_TXD);
	WriteBackToDescriptor((PUCHAR)pDestTxD, (PUCHAR)pTxD, FALSE, TYPE_TXD);
#endif

	/* flush dcache if no consistent memory is supported */
	RTMP_DCACHE_FLUSH(SrcBufPA, SrcBufLen);
	RTMP_DCACHE_FLUSH(ad->CtrlRing.Cell[SwIdx].AllocPa, TXD_SIZE);

   	/* Increase TX_CTX_IDX, but write to register later.*/
	INC_RING_INDEX(ad->CtrlRing.TxCpuIdx, MGMT_RING_SIZE);

	if (msg->need_rsp)
		andes_queue_tail_cmd_msg(&ctl->ackq, msg, wait_cmd_out_and_ack);
	else
		andes_queue_tail_cmd_msg(&ctl->kickq, msg, wait_cmd_out);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&ad->CtrlRingLock, &flags);
		return -1;
	}
	
	RTMP_IO_WRITE32(ad, ad->CtrlRing.hw_cidx_addr,  ad->CtrlRing.TxCpuIdx);

	RTMP_SPIN_UNLOCK_IRQRESTORE(&ad->CtrlRingLock, &flags);

	return ret;
}

void pci_kick_out_cmd_msg_complete(PNDIS_PACKET net_pkt)
{
	struct cmd_msg *msg =CMD_MSG_CB(net_pkt)->msg;
	RTMP_ADAPTER *ad = (RTMP_ADAPTER *)msg->priv;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags))
		return;
		
	if (!msg->need_rsp) {
		andes_unlink_cmd_msg(msg, &ctl->kickq);
		andes_queue_tail_cmd_msg(&ctl->tx_doneq, msg, tx_done);
	} else {
		msg->state = wait_ack;
	}
	
	andes_bh_schedule(ad);
}
#endif



void andes_cleanup_cmd_msg(RTMP_ADAPTER *ad, DL_LIST *list)
{
	unsigned long flags;
	struct cmd_msg *msg, *msg_tmp;
	NDIS_SPIN_LOCK *lock;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	
	lock = andes_get_spin_lock(ctl, list);

	RTMP_SPIN_LOCK_IRQSAVE(lock, &flags);
	DlListForEachSafe(msg, msg_tmp, list, struct cmd_msg, list) {
		_andes_unlink_cmd_msg(msg, list);
		andes_free_cmd_msg(msg);
	}
	DlListInit(list);
	RTMP_SPIN_UNLOCK_IRQRESTORE(lock, &flags);
}

#ifdef RTMP_PCI_SUPPORT
static void andes_ctrl_pci_init(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	RTMP_SPIN_LOCK_IRQ(&ad->mcu_atomic);
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	ctl->cmd_seq = 0;
#ifndef WORKQUEUE_BH
	RTMP_OS_TASKLET_INIT(ad, &ctl->cmd_msg_task, andes_cmd_msg_bh, (unsigned long)ad);
#else
	tasklet_init(&ctl->cmd_msg_task, andes_cmd_msg_bh, (unsigned long)ad);
#endif
	NdisAllocateSpinLock(ad, &ctl->txq_lock);
	andes_queue_init(ctl, &ctl->txq);
	NdisAllocateSpinLock(ad, &ctl->rxq_lock);
	andes_queue_init(ctl, &ctl->rxq);
	NdisAllocateSpinLock(ad, &ctl->ackq_lock);
	andes_queue_init(ctl, &ctl->ackq);
	NdisAllocateSpinLock(ad, &ctl->kickq_lock);
	andes_queue_init(ctl, &ctl->kickq);
	NdisAllocateSpinLock(ad, &ctl->tx_doneq_lock);
	andes_queue_init(ctl, &ctl->tx_doneq);
	NdisAllocateSpinLock(ad, &ctl->rx_doneq_lock);
	andes_queue_init(ctl, &ctl->rx_doneq);
	ctl->tx_kickout_fail_count = 0;
	ctl->tx_timeout_fail_count = 0;
	ctl->rx_receive_fail_count = 0;	
	ctl->alloc_cmd_msg = 0;
	ctl->free_cmd_msg = 0;
	OS_SET_BIT(MCU_INIT, &ctl->flags);
	ctl->ad = ad;
	RTMP_SPIN_UNLOCK_IRQ(&ad->mcu_atomic);
}
#endif


void andes_ctrl_init(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifdef RTMP_PCI_SUPPORT
		andes_ctrl_pci_init(ad);
#endif

	}

	ctl->power_on = FALSE;
	ctl->dpd_on = FALSE;
}


#ifdef RTMP_PCI_SUPPORT
static void andes_ctrl_pci_exit(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	unsigned long flags;

	RTMP_SPIN_LOCK_IRQSAVE(&ad->mcu_atomic, &flags);
	RTMP_CLEAR_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	OS_CLEAR_BIT(MCU_INIT, &ctl->flags);
	RTMP_OS_TASKLET_KILL(&ctl->cmd_msg_task);
	andes_cleanup_cmd_msg(ad, &ctl->txq);
	NdisFreeSpinLock(&ctl->txq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->ackq);
	NdisFreeSpinLock(&ctl->ackq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->rxq);
	NdisFreeSpinLock(&ctl->rxq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->kickq);
	NdisFreeSpinLock(&ctl->kickq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->tx_doneq);
	NdisFreeSpinLock(&ctl->tx_doneq_lock);
	andes_cleanup_cmd_msg(ad, &ctl->rx_doneq);
	NdisFreeSpinLock(&ctl->rx_doneq_lock);
	DBGPRINT(RT_DEBUG_OFF, ("tx_kickout_fail_count = %ld\n", ctl->tx_kickout_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("tx_timeout_fail_count = %ld\n", ctl->tx_timeout_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("rx_receive_fail_count = %ld\n", ctl->rx_receive_fail_count));
	DBGPRINT(RT_DEBUG_OFF, ("alloc_cmd_msg = %ld\n", ctl->alloc_cmd_msg));
	DBGPRINT(RT_DEBUG_OFF, ("free_cmd_msg = %ld\n", ctl->free_cmd_msg));
	RTMP_SPIN_UNLOCK_IRQRESTORE(&ad->mcu_atomic, &flags);
}
#endif

void andes_ctrl_exit(RTMP_ADAPTER *ad)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
#ifdef RTMP_PCI_SUPPORT
		andes_ctrl_pci_exit(ad);
#endif

	}
	
	ctl->power_on = FALSE;
	ctl->dpd_on = FALSE;
}

static int andes_dequeue_and_kick_out_cmd_msgs(RTMP_ADAPTER *ad)
{
	struct cmd_msg *msg = NULL;
	PNDIS_PACKET net_pkt = NULL;	
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = NDIS_STATUS_SUCCESS;
	TXINFO_NMAC_CMD *tx_info;

	while ((msg = andes_dequeue_cmd_msg(ctl, &ctl->txq)) != NULL) {
		if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD) 
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST)
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) { 
			if (!msg->need_rsp)	
				andes_free_cmd_msg(msg);
			continue;
		}

		if (andes_queue_len(ctl, &ctl->ackq) > 0) {
			andes_queue_head_cmd_msg(&ctl->txq, msg, msg->state);
			ret = NDIS_STATUS_FAILURE;
			continue;
		}

		net_pkt = msg->net_pkt;

		if (msg->state != tx_retransmit) {
			if (msg->need_rsp)
				msg->seq = andes_get_cmd_msg_seq(ad);
			else
				msg->seq = 0;

			tx_info = (TXINFO_NMAC_CMD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(*tx_info));
			tx_info->info_type = CMD_PACKET;
			tx_info->d_port = CPU_TX_PORT;
			tx_info->cmd_type = msg->type;
			tx_info->cmd_seq = msg->seq;
			tx_info->pkt_len = GET_OS_PKT_LEN(net_pkt) - sizeof(*tx_info);

#ifdef RT_BIG_ENDIAN
			*(UINT32 *)tx_info = le2cpu32(*(UINT32 *)tx_info);
			//RTMPDescriptorEndianChange((PUCHAR)tx_info, TYPE_TXINFO);
#endif
		}
	


#ifdef RTMP_PCI_SUPPORT
		ret = pci_kick_out_cmd_msg(ad, msg);
#endif

		if (ret) {
			DBGPRINT(RT_DEBUG_ERROR, ("kick out msg fail\n"));
			break;
		}
	}
	
	andes_bh_schedule(ad);
	
	return ret;
}

static int andes_wait_for_complete_timeout(struct cmd_msg *msg, long timeout)
{
	int ret = 0;

#ifdef RTMP_PCI_SUPPORT
	timeout = CMD_MSG_TIMEOUT;

	do {
		if (msg->ack_done == TRUE)
			return -1;

		OS_WAIT(1);
		timeout --;
	} while (timeout != 0);
#endif


	return ret;
}

int andes_send_cmd_msg(PRTMP_ADAPTER ad, struct cmd_msg *msg)
{
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
	int ret = 0;
	BOOLEAN need_wait = msg->need_wait;

#ifdef RTMP_PCI_SUPPORT
	RTMP_SPIN_LOCK(&ad->mcu_atomic);
#endif
	

	if (!RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD) 
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_NIC_NOT_EXIST) 
				|| RTMP_TEST_FLAG(ad, fRTMP_ADAPTER_SUSPEND)) { 
		andes_free_cmd_msg(msg);
#ifdef RTMP_PCI_SUPPORT
		RTMP_SPIN_UNLOCK(&ad->mcu_atomic);
#endif
		
		return NDIS_STATUS_FAILURE;
	}

	andes_queue_tail_cmd_msg(&ctl->txq, msg, tx_start);

retransmit:
	andes_dequeue_and_kick_out_cmd_msgs(ad);

	/* Wait for response */
	if (need_wait) {
		enum cmd_msg_state state;
		if (!andes_wait_for_complete_timeout(msg, msg->timeout)) {
			ret = NDIS_STATUS_FAILURE;
			DBGPRINT(RT_DEBUG_ERROR, ("command (%d) timeout(%dms)\n", msg->type, CMD_MSG_TIMEOUT));
			DBGPRINT(RT_DEBUG_ERROR, ("txq qlen = %d\n", andes_queue_len(ctl, &ctl->txq)));
			DBGPRINT(RT_DEBUG_ERROR, ("rxq qlen = %d\n", andes_queue_len(ctl, &ctl->rxq)));
			DBGPRINT(RT_DEBUG_ERROR, ("kickq qlen = %d\n", andes_queue_len(ctl, &ctl->kickq)));
			DBGPRINT(RT_DEBUG_ERROR, ("ackq qlen = %d\n", andes_queue_len(ctl, &ctl->ackq)));
			DBGPRINT(RT_DEBUG_ERROR, ("tx_doneq.qlen = %d\n", andes_queue_len(ctl, &ctl->tx_doneq)));
			DBGPRINT(RT_DEBUG_ERROR, ("rx_done qlen = %d\n", andes_queue_len(ctl, &ctl->rx_doneq)));
			if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
				if (msg->state == wait_cmd_out_and_ack) {
				} else if (msg->state == wait_ack) {
					andes_unlink_cmd_msg(msg, &ctl->ackq);
				}
			}

			andes_inc_error_count(ctl, error_tx_timeout_fail);
			state = tx_timeout_fail;
			if (msg->retransmit_times > 0)
				msg->retransmit_times--;
			DBGPRINT(RT_DEBUG_ERROR, ("msg->retransmit_times = %d\n", msg->retransmit_times));
		} else {
			if (msg->state == tx_kickout_fail) {
				state = tx_kickout_fail;
				msg->retransmit_times--;
			} else {
				state = tx_done;
				msg->retransmit_times = 0;
			}
		}
	
		if (OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
			if (msg->need_retransmit && (msg->retransmit_times > 0)) {
#ifdef RTMP_PCI_SUPPORT
				msg->ack_done = FALSE;
#endif

				state = tx_retransmit;
				andes_queue_head_cmd_msg(&ctl->txq, msg, state);
				goto retransmit;
			} else {
				andes_queue_tail_cmd_msg(&ctl->tx_doneq, msg, state);
			}
		} else {
			andes_free_cmd_msg(msg);
		}
	}


#ifdef RTMP_PCI_SUPPORT
	RTMP_SPIN_UNLOCK(&ad->mcu_atomic);
	RtmpOsMsDelay(10);
#endif
	
	return ret;
}

static void andes_pwr_event_handler(RTMP_ADAPTER *ad, char *payload, u16 payload_len)
{


}


static void andes_wow_event_handler(RTMP_ADAPTER *ad, char *payload, u16 payload_len)
{


}

static void andes_carrier_detect_event_handler(RTMP_ADAPTER *ad, char *payload, u16 payload_len)
{



}

static void andes_dfs_detect_event_handler(PRTMP_ADAPTER ad, char *payload, u16 payload_len)
{



}

MSG_EVENT_HANDLER msg_event_handler_tb[] =
{
	andes_pwr_event_handler,
	andes_wow_event_handler,
	andes_carrier_detect_event_handler,
	andes_dfs_detect_event_handler,
};


int andes_burst_write(RTMP_ADAPTER *ad, u32 offset, u32 *data, u32 cnt)
{
	struct cmd_msg *msg;
	unsigned int var_len, offset_num, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!data)
		return -1;

	offset_num = cnt / ((cap->InbandPacketMaxLen - sizeof(offset)) / 4);

	if (cnt % ((cap->InbandPacketMaxLen - sizeof(offset)) / 4))
		var_len = sizeof(offset) * (offset_num + 1) + 4 * cnt;
	else
		var_len = sizeof(offset) * offset_num + 4 * cnt;

	while (cur_len < var_len) {
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									? cap->InbandPacketMaxLen : (var_len - cur_len);

		if (((sent_len < cap->InbandPacketMaxLen) || ((cur_len + cap->InbandPacketMaxLen) == var_len)))
			last_packet = TRUE;

		msg = andes_alloc_cmd_msg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet) {
			andes_init_cmd_msg(msg, CMD_BURST_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		}else {
			andes_init_cmd_msg(msg, CMD_BURST_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
		}
			
	
		value = cpu2le32(offset + cap->WlanMemmapOffset + cur_index * 4);
		andes_append_cmd_msg(msg, (char *)&value, 4);

		for (i = 0; i < ((sent_len - 4) / 4); i++) {
			value = cpu2le32(data[i + cur_index]);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}
		
		ret = andes_send_cmd_msg(ad, msg);

		
		cur_index += ((sent_len - 4) / 4);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

static void andes_burst_read_callback(struct cmd_msg *msg, char *rsp_payload, u16 rsp_payload_len)
{
	u32 i;
	u32 *data;
	NdisMoveMemory(msg->rsp_payload, rsp_payload + 4, rsp_payload_len - 4);

	for (i = 0; i < (msg->rsp_payload_len - 4) / 4; i++) {
		data = (u32 *)(msg->rsp_payload + i * 4);
		*data = le2cpu32(*data);
	}
}

int andes_burst_read(RTMP_ADAPTER *ad, u32 offset, u32 cnt, u32 *data)
{
	struct cmd_msg *msg;
	unsigned int cur_len = 0, rsp_len, offset_num, receive_len;
	u32 value, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	if (!data)
		return -1;

	offset_num = cnt / ((cap->InbandPacketMaxLen - sizeof(offset)) / 4);

	if (cnt % ((cap->InbandPacketMaxLen - sizeof(offset)) / 4))
		rsp_len = sizeof(offset) * (offset_num + 1) + 4 * cnt;
	else
		rsp_len = sizeof(offset) * offset_num + 4 * cnt;

	while (cur_len < rsp_len) {
		receive_len = (rsp_len - cur_len) > cap->InbandPacketMaxLen 
									   ? cap->InbandPacketMaxLen 
									   : (rsp_len - cur_len);

		msg = andes_alloc_cmd_msg(ad, 8);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
	
		andes_init_cmd_msg(msg, CMD_BURST_READ, TRUE, 0, TRUE, TRUE, receive_len, 
									(char *)(&data[cur_index]), andes_burst_read_callback);

		value = cpu2le32(offset + cap->WlanMemmapOffset + cur_index * 4);
		andes_append_cmd_msg(msg, (char *)&value, 4);

		value = cpu2le32((receive_len - 4) / 4);
		andes_append_cmd_msg(msg, (char *)&value, 4);

		ret = andes_send_cmd_msg(ad, msg);

		if (ret) {
			if (cnt == 1)
				*data = 0xffffffff;
		}
		
		cur_index += ((receive_len - 4) / 4);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

static void andes_random_read_callback(struct cmd_msg *msg, char *rsp_payload, u16 rsp_payload_len)
{
	u32 i;
	RTMP_REG_PAIR *reg_pair = (RTMP_REG_PAIR *)msg->rsp_payload;
	
	for (i = 0; i < msg->rsp_payload_len / 8; i++) {
		NdisMoveMemory(&reg_pair[i].Value, rsp_payload + 8 * i + 4, 4);
		reg_pair[i].Value = le2cpu32(reg_pair[i].Value);
	}
}

int andes_random_read(RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, receive_len;
	u32 i, value, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		receive_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									   ? cap->InbandPacketMaxLen 
									   : (var_len - cur_len);

		msg = andes_alloc_cmd_msg(ad, receive_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		andes_init_cmd_msg(msg, CMD_RANDOM_READ, TRUE, 0, TRUE, TRUE, receive_len, 
									(char *)&reg_pair[cur_index], andes_random_read_callback);

		for (i = 0; i < receive_len / 8; i++) {
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			andes_append_cmd_msg(msg, (char *)&value, 4);
			value = 0;
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}
	

		ret = andes_send_cmd_msg(ad, msg);

		
		cur_index += receive_len / 8;
		cur_len += cap->InbandPacketMaxLen;
	}

error:	
	return ret;
}

static void andes_rf_random_read_callback(struct cmd_msg *msg, char *rsp_payload, u16 rsp_payload_len)
{
	u32 i;
	BANK_RF_REG_PAIR *reg_pair = (BANK_RF_REG_PAIR *)msg->rsp_payload;
			
	for (i = 0; i < msg->rsp_payload_len / 8; i++) {
		NdisMoveMemory(&reg_pair[i].Value, rsp_payload + 8 * i + 4, 1);
	}
}

int andes_rf_random_read(RTMP_ADAPTER *ad, BANK_RF_REG_PAIR *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, receive_len;
	u32 i, value, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	
	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		receive_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									   ? cap->InbandPacketMaxLen 
									   : (var_len - cur_len);

		msg = andes_alloc_cmd_msg(ad, receive_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		andes_init_cmd_msg(msg, CMD_RANDOM_READ, TRUE, 0, TRUE, TRUE, receive_len, 
									(char *)&reg_pair[cur_index], andes_rf_random_read_callback);

		for (i = 0; i < (receive_len) / 8; i++)
		{
			value = 0;
	
			/* RF selection */
			value = (value & ~0x80000000) | 0x80000000;

			/* RF bank */
			value = (value & ~0x00ff0000) | (reg_pair[i + cur_index].Bank << 16);

			/* RF Index */
			value = (value & ~0x0000ffff) | reg_pair[i + cur_index].Register;

			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
			value = 0;
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}

		ret = andes_send_cmd_msg(ad, msg);
	
	
		cur_index += receive_len / 8;
		cur_len += cap->InbandPacketMaxLen;
	}
	
error:
	return ret;
}

int andes_read_modify_write(RTMP_ADAPTER *ad, R_M_W_REG *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 12, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									? cap->InbandPacketMaxLen : (var_len - cur_len);
		
		if ((sent_len < cap->InbandPacketMaxLen) || (cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = TRUE;
		
		msg = andes_alloc_cmd_msg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			andes_init_cmd_msg(msg, CMD_READ_MODIFY_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			andes_init_cmd_msg(msg, CMD_READ_MODIFY_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < (sent_len / 12); i++)
		{
			/* Address */
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			/* ClearBitMask */
			value = cpu2le32(reg_pair[i + cur_index].ClearBitMask);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}

		ret = andes_send_cmd_msg(ad, msg);
	
	
		cur_index += (sent_len / 12);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_rf_read_modify_write(RTMP_ADAPTER *ad, RF_R_M_W_REG *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 12, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;
	
	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									? cap->InbandPacketMaxLen : (var_len - cur_len);
		
		if ((sent_len < cap->InbandPacketMaxLen) || (cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = TRUE;

		msg = andes_alloc_cmd_msg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			andes_init_cmd_msg(msg, CMD_READ_MODIFY_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			andes_init_cmd_msg(msg, CMD_READ_MODIFY_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
		
		for (i = 0; i < sent_len / 12; i++)
		{
			value = 0;
			/* RF selection */
			value = (value & ~0x80000000) | 0x80000000;

			/* RF bank */
			value = (value & ~0x00ff0000) | (reg_pair[i + cur_index].Bank << 16);

			/* RF Index */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Register;
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
			
			value = 0;
			/* ClearBitMask */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].ClearBitMask;
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			value = 0;
			/* UpdateData */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Value;
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}
	
		ret = andes_send_cmd_msg(ad, msg);
	
		cur_index += (sent_len / 12);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_random_write(RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									? cap->InbandPacketMaxLen : (var_len - cur_len);
	
		if ((sent_len < cap->InbandPacketMaxLen) || (cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = TRUE;
	
		msg = andes_alloc_cmd_msg(ad, sent_len);
		
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			andes_init_cmd_msg(msg, CMD_RANDOM_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			andes_init_cmd_msg(msg, CMD_RANDOM_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < (sent_len / 8); i++)
		{
			/* Address */
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		};

		ret = andes_send_cmd_msg(ad, msg);


		cur_index += (sent_len / 8);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_rf_random_write(RTMP_ADAPTER *ad, BANK_RF_REG_PAIR *reg_pair, u32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, sent_len;
	u32 value, i, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	BOOLEAN last_packet = FALSE;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		sent_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									? cap->InbandPacketMaxLen : (var_len - cur_len);
	
		if ((sent_len < cap->InbandPacketMaxLen) || (cur_len + cap->InbandPacketMaxLen) == var_len)
			last_packet = TRUE;

		msg = andes_alloc_cmd_msg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			andes_init_cmd_msg(msg, CMD_RANDOM_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			andes_init_cmd_msg(msg, CMD_RANDOM_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
		
		for (i = 0; i < (sent_len / 8); i++) {
			value = 0;
			/* RF selection */
			value = (value & ~0x80000000) | 0x80000000;

			/* RF bank */
			value = (value & ~0x00ff0000) | (reg_pair[i + cur_index].Bank << 16);

			/* RF Index */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Register;
			
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);

			value = 0;
			/* UpdateData */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Value;
			value = cpu2le32(value);
			andes_append_cmd_msg(msg, (char *)&value, 4);
		}

		ret = andes_send_cmd_msg(ad, msg);


		cur_index += (sent_len / 8);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}

int andes_sc_random_write(RTMP_ADAPTER *ad, CR_REG *table, u32 nums, u32 flags)
{
	u32 varlen = 0, i, j;
	RTMP_REG_PAIR *sw_ch_table = NULL, temp;

	if (!table)
		return -1;

	for (i = 0; i < nums; i++) {
		if ((table[i].flags & (_BAND | _BW | _TX_RX_SETTING)) == flags) {
			varlen += sizeof(RTMP_REG_PAIR);
		}
	}
	
	os_alloc_mem(NULL, (UCHAR **)&sw_ch_table, varlen);

	if (!sw_ch_table) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: memory is not available for allocating switch channel table\n", __FUNCTION__));
		return -1;
	}

	for (i = 0, j = 0; i < nums; i++) {
		if ((table[i].flags & (_BAND | _BW | _TX_RX_SETTING)) == flags) {
			temp.Register = table[i].offset;
			temp.Value = table[i].value;
			NdisMoveMemory(&sw_ch_table[j], &temp, sizeof(temp));
			j++;
		}
	}

	andes_random_write(ad, sw_ch_table, varlen / sizeof(RTMP_REG_PAIR));

	os_free_mem(NULL, sw_ch_table);

	return 0;
}

int andes_sc_rf_random_write(RTMP_ADAPTER *ad, BANK_RF_CR_REG *table, u32 nums, u32 flags)
{
	u32 varlen = 0, i, j;
	BANK_RF_REG_PAIR *sw_ch_table = NULL, temp;

	if (!table)
		return -1;

	for (i = 0; i < nums; i++) {
		if ((table[i].flags & (_BAND | _BW | _TX_RX_SETTING)) == flags) {
			varlen += sizeof(BANK_RF_REG_PAIR);
		}
	}
	
	os_alloc_mem(NULL, (UCHAR **)&sw_ch_table, varlen);

	if (!sw_ch_table) {
		DBGPRINT(RT_DEBUG_ERROR, ("%s: memory is not available for allocating switch channel table\n", __FUNCTION__));
		return -1;
	}

	for (i = 0, j = 0; i < nums; i++) {
		if ((table[i].flags & (_BAND | _BW | _TX_RX_SETTING)) == flags) {
			temp.Bank = table[i].bank;
			temp.Register = table[i].offset;
			temp.Value = table[i].value;
			NdisMoveMemory(&sw_ch_table[j], &temp, sizeof(temp));
			j++;
		}
	}

	andes_rf_random_write(ad, sw_ch_table, varlen / sizeof(BANK_RF_REG_PAIR));

	os_free_mem(NULL, sw_ch_table);

	return 0;
}


int andes_pwr_saving(RTMP_ADAPTER *ad, u32 op, u32 level, 
					 u32 listen_interval, u32 pre_tbtt_lead_time,
					 u8 tim_byte_offset, u8 tim_byte_pattern)
{
	struct cmd_msg *msg;
	unsigned int var_len;
	u32 value;
	int ret = 0;
	BOOLEAN need_wait = FALSE;

#ifdef CONFIG_STA_SUPPORT
#ifdef RTMP_MAC_PCI
	need_wait = TRUE;
#endif /* RTMP_MAC_PCI */
#endif /* CONFIG_STA_SUPPORT */

	/* Power operation and Power Level */
	var_len = 8;

	if (op == RADIO_OFF_ADVANCE_PWR_SAVING)
	{
		/* Listen interval, Pre-TBTT, TIM info */
		var_len += 12;
	}
	
	msg = andes_alloc_cmd_msg(ad, var_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_POWER_SAVING_OP, need_wait, 0, FALSE, FALSE, 0, NULL, NULL);
	
	/* Power operation */
	value = cpu2le32(op);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* Power Level */
	value = cpu2le32(level);

	andes_append_cmd_msg(msg, (char *)&value, 4);

	if (op == RADIO_OFF_ADVANCE_PWR_SAVING)
	{
		/* Listen interval */
		value = cpu2le32(listen_interval);
		andes_append_cmd_msg(msg, (char *)&value, 4);


		/* Pre TBTT lead time */
		value = cpu2le32(pre_tbtt_lead_time);
		andes_append_cmd_msg(msg, (char*)&value, 4);

		/* TIM Info */
		value = (value & ~0x000000ff) | tim_byte_pattern;
		value = (value & ~0x0000ff00) | (tim_byte_offset << 8);
		value = cpu2le32(value);
		andes_append_cmd_msg(msg, (char *)&value, 4);
	}

	ret = andes_send_cmd_msg(ad, msg);
	
error:
	return ret;
}

int andes_fun_set(RTMP_ADAPTER *ad, u32 fun_id, u32 param)
{
	struct cmd_msg *msg;
	u32 value;
	int ret = 0;

	/* Function ID and Parameter */
	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	if (fun_id != Q_SELECT
#ifdef BTCOEX_CONCURRENT
		&& fun_id != 11
#endif
		)
		andes_init_cmd_msg(msg, CMD_FUN_SET_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
	else 
		andes_init_cmd_msg(msg, CMD_FUN_SET_OP, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
	
	/* Function ID */
	value = cpu2le32(fun_id);
	andes_append_cmd_msg(msg, (char *)&value, 4);
	
	/* Parameter */
	value = cpu2le32(param);
	andes_append_cmd_msg(msg, (char *)&value, 4);	
		
	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}


int andes_calibration(RTMP_ADAPTER *ad, u32 cal_id, ANDES_CALIBRATION_PARAM *param)
{
	struct cmd_msg *msg;
	u32 value;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:cal_id(%d)\n ", __FUNCTION__, cal_id));

#ifdef MT76x2
#ifdef RTMP_PCI_SUPPORT
	if (IS_MT76x2(ad) && (cal_id != TSSI_COMPENSATION_7662)) {
		ANDES_CALIBRATION_START(ad);
	}
#endif /* RTMP_PCI_SUPPORT */
#endif /* MT76x2 */

#ifdef MT76x2
	/* Calibration ID and Parameter */
	if (cal_id == TSSI_COMPENSATION_7662 && IS_MT76x2(ad))		
		msg = andes_alloc_cmd_msg(ad, 12);
	else
#endif /* MT76x2 */
		msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_CALIBRATION_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
	
	/* Calibration ID */
	value = cpu2le32(cal_id);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* Parameter */
#ifdef MT76x2
	if (cal_id == TSSI_COMPENSATION_7662 && IS_MT76x2(ad)) {
		value = cpu2le32(param->mt76x2_tssi_comp_param.pa_mode);
		andes_append_cmd_msg(msg, (char *)&value, 4);
		
		value = cpu2le32(param->mt76x2_tssi_comp_param.tssi_slope_offset);
		andes_append_cmd_msg(msg, (char *)&value, 4);
	} else
#endif /* MT76x2 */
	{
		value = cpu2le32(param->generic);
		andes_append_cmd_msg(msg, (char *)&value, 4);
	}

	ret = andes_send_cmd_msg(ad, msg);

#ifdef MT76x2
#ifdef RTMP_PCI_SUPPORT
	if (IS_MT76x2(ad) && ((cal_id != TSSI_COMPENSATION_7662))) {
		ANDES_WAIT_CALIBRATION_DONE(ad);
	}
#endif /* RTMP_PCI_SUPPORT */
#endif /* MT76x2 */

error:
	return ret;
}

int andes_load_cr(RTMP_ADAPTER *ad, u32 cr_type, UINT8 temp_level, UINT8 channel)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s:cr_type(%d)\n", __FUNCTION__, cr_type));
	
	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_LOAD_CR, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* CR type */
	value &= ~LOAD_CR_MODE_MASK;
	value |= LOAD_CR_MODE(cr_type);

	if (cr_type == HL_TEMP_CR_UPDATE) {
		value &= ~LOAD_CR_TEMP_LEVEL_MASK;
		value |= LOAD_CR_TEMP_LEVEL(temp_level); 

		value &= ~LOAD_CR_CHL_MASK;
		value |= LOAD_CR_CHL(channel); 
	} 

	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	value = 0x80000000;
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] >> 8) & 0xFF);
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] & 0xFF) << 8 );
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_switch_channel(RTMP_ADAPTER *ad, u8 channel, BOOLEAN scan, unsigned int bw, unsigned int tx_rx_setting, u8 bbp_ch_idx)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d),scan(%d),bw(%d),trx(0x%x)\n", __FUNCTION__, channel, scan, bw, tx_rx_setting));

	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_SWITCH_CHANNEL_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* 
     * switch channel related param
     * channel, scan, bw, tx_rx_setting
     */
	value &= ~SC_PARAM1_CHL_MASK;
	value |= SC_PARAM1_CHL(channel);
	value &= ~SC_PARAM1_SCAN_MASK;
	value |= SC_PARAM1_SCAN(scan);
	value &= ~SC_PARAM1_BW_MASK;
	value |= SC_PARAM1_BW(bw);
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);
	
	value = 0;
	value |= SC_PARAM2_TR_SETTING(tx_rx_setting);
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	ret = andes_send_cmd_msg(ad, msg);
	
	mdelay(5);

	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_SWITCH_CHANNEL_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* 
     * switch channel related param
     * channel, scan, bw, tx_rx_setting, extension channel
     */
	value &= ~SC_PARAM1_CHL_MASK;
	value |= SC_PARAM1_CHL(channel);
	value &= ~SC_PARAM1_SCAN_MASK;
	value |= SC_PARAM1_SCAN(scan);
	value &= ~SC_PARAM1_BW_MASK;
	value |= SC_PARAM1_BW(bw);
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);
	
	value = 0;
	value |= SC_PARAM2_TR_SETTING(tx_rx_setting);
	value &= ~SC_PARAM2_EXTENSION_CHL_MASK;
	
	if (bbp_ch_idx == 0)
		value |= SC_PARAM2_EXTENSION_CHL(0xe0);
	else if (bbp_ch_idx == 1)
		value |= SC_PARAM2_EXTENSION_CHL(0xe1);
	else if (bbp_ch_idx == 2)
		value |= SC_PARAM2_EXTENSION_CHL(0xe2);
	else if (bbp_ch_idx == 3)
		value |= SC_PARAM2_EXTENSION_CHL(0xe3);
		
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);
	
	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_init_gain(RTMP_ADAPTER *ad, UINT8 channel, BOOLEAN force_mode, unsigned int gain_from_e2p)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d), force mode(%d), init gain parameter(0x%08x)\n", 
		__FUNCTION__, channel, force_mode, gain_from_e2p));
	
	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_INIT_GAIN_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* init gain parameter#1 */
	if (force_mode == TRUE)
		value = 0x80000000;
		
	value |= channel;
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* init gain parameter#2 while force mode is enabled */
	value = gain_from_e2p;
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_dynamic_vga(RTMP_ADAPTER *ad, UINT8 channel, BOOLEAN mode, BOOLEAN ext, int rssi, unsigned int false_cca)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int rssi_val = 0, ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d), ap/sta mode(%d), extension(%d), rssi(%d), false cca count(%d)\n", 
		__FUNCTION__, channel, mode, ext, rssi, false_cca));
	
	msg = andes_alloc_cmd_msg(ad, 12);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_DYNC_VGA_OP, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

	/* dynamic VGA parameter#1: TRUE = AP mode ; FALSE = STA mode */
	if (mode == TRUE)
		value |= 0x80000000;

	if (ext == TRUE)
		value |= 0x40000000;

	if (ad->ed_chk == TRUE)
		value |= 0x20000000;
		
	value |= channel;
	value = cpu2le32(value);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* dynamic VGA parameter#2: RSSI (signed value) */
	rssi_val = cpu2le32(rssi);
	andes_append_cmd_msg(msg, (char *)&rssi_val, 4);

	/* dynamic VGA parameter#3: false CCA count */
	value = cpu2le32(false_cca);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	ret = andes_send_cmd_msg(ad, msg);

error:
	return ret;
}

int andes_led_op(RTMP_ADAPTER *ad, u32 led_idx, u32 link_status)
{
	struct cmd_msg *msg;
	u32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:led_idx(%d), link_status(%d)\n ", 
		__FUNCTION__, led_idx, link_status));
	
	msg = andes_alloc_cmd_msg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	andes_init_cmd_msg(msg, CMD_LED_MODE_OP, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
	
	/* Led index */
	value = cpu2le32(led_idx);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	/* Link status */
	value = cpu2le32(link_status);
	andes_append_cmd_msg(msg, (char *)&value, 4);

	ret = andes_send_cmd_msg(ad, msg);
	
error:
	return ret;
}


#ifdef RTMP_PCI_SUPPORT
void andes_pci_fw_init(RTMP_ADAPTER *ad)
{
	u32 value;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	/* Enable Interrupt*/
	RTMP_IRQ_ENABLE(ad);
	RTMPEnableRxTx(ad);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_START_UP);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	/* clear garbage interrupts*/
	RTMP_IO_READ32(ad, 0x1300, &value);
	if (value) {
	DBGPRINT(RT_DEBUG_OFF, ("0x1300 = %08x\n", value));
	}

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(ad, HEADER_TRANS_CTRL_REG, 0X2);
	RTMP_IO_WRITE32(ad, TSO_CTRL, 0x7050);
#else
	RTMP_IO_WRITE32(ad, HEADER_TRANS_CTRL_REG, 0x0);
	RTMP_IO_WRITE32(ad, TSO_CTRL, 0x0);
#endif

	andes_fun_set(ad, Q_SELECT, ad->chipCap.CmdRspRxRing);
	PWR_SAVING_OP(ad, RADIO_ON, 0, 0, 0, 0, 0);
}
#endif /* RTMP_PCI_SUPPORT */


#ifdef BTCOEX_CONCURRENT
void MT7662ReceCoexFromOtherCHip(
	IN UCHAR channel,
	IN UCHAR centralchannel,
	IN UCHAR channel_bw
	)
{	
	CoexChannel = channel;
	CoexCenctralChannel=centralchannel;
	CoexChannelBw=channel_bw;

	DBGPRINT(RT_DEBUG_TRACE, ("-->MT7662ReceCoexFromOtherCHip channel=%d centralchannel=%d channel_bw=%d\n",CoexChannel, CoexCenctralChannel, CoexChannelBw));
	
}
EXPORT_SYMBOL(MT7662ReceCoexFromOtherCHip);
#endif
