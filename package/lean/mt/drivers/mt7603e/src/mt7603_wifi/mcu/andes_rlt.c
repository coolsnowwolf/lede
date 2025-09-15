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
	andes_rlt.c

	Abstract:
	on-chip CPU related codes

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#include	"rt_config.h"
#ifdef RTMP_PCI_SUPPORT
INT32 AndesRltPciLoadRomPatch(RTMP_ADAPTER *ad)
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
	
	if (cap->load_code_method == BIN_FILE_METHOD) {
		DBGPRINT(RT_DEBUG_OFF, ("load rom patch from /lib/firmware/%s\n", cap->rom_patch_bin_file_name));
		OS_LOAD_CODE_FROM_BIN(&cap->rom_patch, cap->rom_patch_bin_file_name, obj->pci_dev, &cap->rom_patch_len); 
	} else {
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

	/* get rom patch information */
	DBGPRINT(RT_DEBUG_OFF, ("build time = \n")); 
	
	for (loop = 0; loop < 16; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->rom_patch + loop)));

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
	
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("platform = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->rom_patch + 16 + loop)));
	
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("hw/sw version = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->rom_patch + 20 + loop)));
	
	DBGPRINT(RT_DEBUG_OFF, ("\n"));

	DBGPRINT(RT_DEBUG_OFF, ("patch version = \n"));

	for (loop = 0; loop < 4; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->rom_patch + 24 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));
		

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


INT32 AndesRltPciEraseRomPatch(RTMP_ADAPTER *ad)
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


INT32 AndesRltPciEraseFw(RTMP_ADAPTER *ad)
{
	int ret = NDIS_STATUS_SUCCESS;
	UINT32 ilm_len, dlm_len;
	UINT16 fw_ver, build_ver;
	UINT32 loop = 0, idx = 0, val = 0;
	UINT32 mac_value;
	UINT32 start_offset, end_offset;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	
	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));

	if (!ad->chipCap.ram_code_protect) {

		ilm_len = (*(cap->FWImageName + 3) << 24) | (*(cap->FWImageName + 2) << 16) |
				 (*(cap->FWImageName + 1) << 8) | (*cap->FWImageName);

		dlm_len = (*(cap->FWImageName + 7) << 24) | (*(cap->FWImageName + 6) << 16) |
				 (*(cap->FWImageName + 5) << 8) | (*(cap->FWImageName + 4));

		fw_ver = (*(cap->FWImageName + 11) << 8) | (*(cap->FWImageName + 10));

		build_ver = (*(cap->FWImageName + 9) << 8) | (*(cap->FWImageName + 8));
	
		DBGPRINT(RT_DEBUG_TRACE, ("FW Version:%d.%d.%02d ", (fw_ver & 0xf000) >> 8,
							(fw_ver & 0x0f00) >> 8, fw_ver & 0x00ff));
		DBGPRINT(RT_DEBUG_TRACE, ("Build:%x\n", build_ver));
		DBGPRINT(RT_DEBUG_TRACE, ("Build Time:"));

		for (loop = 0; loop < 16; loop++)
			DBGPRINT(RT_DEBUG_TRACE, ("%c", *(cap->FWImageName + 16 + loop)));

		DBGPRINT(RT_DEBUG_TRACE, ("\n"));

		DBGPRINT(RT_DEBUG_TRACE, ("ILM Length = %d(bytes)\n", ilm_len));
		DBGPRINT(RT_DEBUG_TRACE, ("DLM Length = %d(bytes)\n", dlm_len));
	
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
				val = (*(cap->FWImageName + idx)) +
				   (*(cap->FWImageName + idx + 3) << 24) +
				   (*(cap->FWImageName + idx + 2) << 16) +
				   (*(cap->FWImageName + idx + 1) << 8);

				RTMP_IO_WRITE32(ad, 0x80000 + (idx - FW_INFO_SIZE), 0);
			}

			if (cap->ram_code_protect) {
				/* Loading IV part into last 64 bytes of ILM */
				start_offset = FW_INFO_SIZE;
				end_offset = FW_INFO_SIZE + IV_SIZE;
	
				for (idx = start_offset; idx < end_offset; idx += 4)
				{
					val = (*(cap->FWImageName + idx)) +
						(*(cap->FWImageName + idx + 3) << 24) +
						(*(cap->FWImageName + idx + 2) << 16) +
						(*(cap->FWImageName + idx + 1) << 8);

					RTMP_IO_WRITE32(ad, 0x80000 + (0x54000 - IV_SIZE) + (idx - FW_INFO_SIZE), 0);
				}
			}

			RTMP_IO_WRITE32(ad, PCIE_REMAP_BASE4, cap->dlm_offset);

			start_offset = 32 + ilm_len;
			end_offset = 32 + ilm_len + dlm_len;
	
			/* erase dlm */
			for (idx = start_offset; idx < end_offset; idx += 4) {
				val = (*(cap->FWImageName + idx)) +
					(*(cap->FWImageName + idx + 3) << 24) +
					(*(cap->FWImageName + idx + 2) << 16) +
					(*(cap->FWImageName + idx + 1) << 8);

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


NDIS_STATUS AndesRltPciLoadFw(RTMP_ADAPTER *ad)
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
	
	if (cap->load_code_method == BIN_FILE_METHOD) {
		DBGPRINT(RT_DEBUG_OFF, ("load fw image from /lib/firmware/%s\n", cap->fw_bin_file_name));
		OS_LOAD_CODE_FROM_BIN(&cap->FWImageName, cap->fw_bin_file_name, obj->pci_dev, &cap->fw_len);
	} else {
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
	
	DBGPRINT(RT_DEBUG_OFF, ("FW Version:%d.%d.%02d ", (fw_ver & 0xf000) >> 8,
						(fw_ver & 0x0f00) >> 8, fw_ver & 0x00ff));
	DBGPRINT(RT_DEBUG_OFF, ("Build:%x\n", build_ver));
	DBGPRINT(RT_DEBUG_OFF, ("Build Time:"));

	for (loop = 0; loop < 16; loop++)
		DBGPRINT(RT_DEBUG_OFF, ("%c", *(cap->FWImageName + 16 + loop)));

	DBGPRINT(RT_DEBUG_OFF, ("\n"));
	
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




INT32 AndesRltUsbEraseFw(RTMP_ADAPTER *ad)
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


VOID AndesRltRxProcessCmdMsg(RTMP_ADAPTER *ad, struct cmd_msg *rx_msg)
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
				_AndesUnlinkCmdMsg(msg, &ctl->ackq);

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
					AndesFreeCmdMsg(msg);
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
#ifdef RLT_MAC
VOID PciRxCmdMsgComplete(RTMP_ADAPTER *ad, RXFCE_INFO *fce_info)
{
	struct cmd_msg *msg;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		return;
	}
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s\n", __FUNCTION__));

	msg = AndesAllocCmdMsg(ad, sizeof(*fce_info) + fce_info->pkt_len);
	
	if (!msg)
		return;

	AndesAppendCmdMsg(msg, (char *)fce_info, sizeof(*fce_info) + fce_info->pkt_len);
	
	AndesRxProcessCmdMsg(ad, msg);

	AndesFreeCmdMsg(msg);
}
#endif /* RLT_MAC */
#endif /* RTMP_PCI_SUPPORT */


#ifdef RLT_MAC
static UCHAR *txinfo_type_str[]={"PKT", "CMD", "RSV"};
static UCHAR *txinfo_d_port_str[]={"WLAN", "CPU_RX", "CPU_TX", "HOST", "VIRT_RX", "VIRT_TX", "DROP"};

VOID DumpCmdTxInfo(RTMP_ADAPTER *ad, TXINFO_STRUC *pTxInfo)
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


VOID AndesRltFillCmdHeader(struct cmd_msg *msg, PNDIS_PACKET net_pkt)
{
	TXINFO_NMAC_CMD *tx_info;

	tx_info = (TXINFO_NMAC_CMD *)OS_PKT_HEAD_BUF_EXTEND(net_pkt, sizeof(*tx_info));
	tx_info->info_type = CMD_PACKET;
	tx_info->d_port = msg->pq_id;
	tx_info->cmd_type = msg->cmd_type;
	tx_info->cmd_seq = msg->seq;
	tx_info->pkt_len = GET_OS_PKT_LEN(net_pkt) - sizeof(*tx_info);

#ifdef RT_BIG_ENDIAN
	RTMPDescriptorEndianChange((PUCHAR)tx_info, TYPE_TXINFO);
#endif

} 


#ifdef RTMP_PCI_SUPPORT
INT32 AndesRltPciKickOutCmdMsg(
	PRTMP_ADAPTER ad,
	struct cmd_msg *msg)
{
	int ret = NDIS_STATUS_SUCCESS;
	unsigned long flags = 0;
	ULONG FreeNum;
	PNDIS_PACKET net_pkt = msg->net_pkt;
	UINT32 SwIdx = 0, SrcBufPA;
	UCHAR *pSrcBufVA;
	UINT SrcBufLen = 0;
	PACKET_INFO PacketInfo;
	TXD_STRUC *pTxD;
	TXINFO_STRUC *pTxInfo;
	struct MCU_CTRL *ctl = &ad->MCUCtrl;
#ifdef RT_BIG_ENDIAN
	TXD_STRUC *pDestTxD;
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
#else
	pTxD  = (PTXD_STRUC)ad->CtrlRing.Cell[SwIdx].AllocVa;
#endif

	pTxInfo = (TXINFO_STRUC *)((UCHAR *)pTxD + sizeof(TXD_STRUC));
	NdisMoveMemory(pTxInfo, pSrcBufVA, TXINFO_SIZE);

	ad->CtrlRing.Cell[SwIdx].pNdisPacket = net_pkt;
	ad->CtrlRing.Cell[SwIdx].pNextNdisPacket = NULL;

	SrcBufPA = PCI_MAP_SINGLE(ad, (pSrcBufVA) + 4, (SrcBufLen) - 4, 0, RTMP_PCI_DMA_TODEVICE);

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
		AndesQueueTailCmdMsg(&ctl->ackq, msg, wait_cmd_out_and_ack);
	else
		AndesQueueTailCmdMsg(&ctl->kickq, msg, wait_cmd_out);

	if (!OS_TEST_BIT(MCU_INIT, &ctl->flags)) {
		RTMP_SPIN_UNLOCK_IRQRESTORE(&ad->CtrlRingLock, &flags);
		return -1;
	}
	
	RTMP_IO_WRITE32(ad, ad->CtrlRing.hw_cidx_addr, ad->CtrlRing.TxCpuIdx);

	RTMP_SPIN_UNLOCK_IRQRESTORE(&ad->CtrlRingLock, &flags);

	return ret;
}
#endif /* RTMP_PCI_SUPPORT */
#endif /* RLT_MAC */




static VOID AndesRltPwrEventHandler(RTMP_ADAPTER *ad, char *payload, UINT16 payload_len)
{


}


static VOID AndesRltWowEventHandler(RTMP_ADAPTER *ad, char *payload, UINT16 payload_len)
{


}


static VOID AndesRltCarrierDetectEventHandler(RTMP_ADAPTER *ad, char *payload, UINT16 payload_len)
{



}


static VOID AndesRltDfsDetectEventHandler(PRTMP_ADAPTER ad, char *payload, UINT16 payload_len)
{



}


MSG_EVENT_HANDLER msg_event_handler_tb[] =
{
	AndesRltPwrEventHandler,
	AndesRltWowEventHandler,
	AndesRltCarrierDetectEventHandler,
	AndesRltDfsDetectEventHandler,
};


INT32 AndesRltBurstWrite(RTMP_ADAPTER *ad, UINT32 offset, UINT32 *data, UINT32 cnt)
{
	struct cmd_msg *msg;
	unsigned int var_len, offset_num, cur_len = 0, sent_len;
	UINT32 value, i, cur_index = 0;
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

		msg = AndesAllocCmdMsg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}

		if (last_packet) {
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_BURST_WRITE, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		}else {
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_BURST_WRITE, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
		}
			
	
		value = cpu2le32(offset + cap->WlanMemmapOffset + cur_index * 4);
		AndesAppendCmdMsg(msg, (char *)&value, 4);

		for (i = 0; i < ((sent_len - 4) / 4); i++) {
			value = cpu2le32(data[i + cur_index]);
			AndesAppendCmdMsg(msg, (char *)&value, 4);
		}
		
		ret = AndesSendCmdMsg(ad, msg);

		
		cur_index += ((sent_len - 4) / 4);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}


static VOID AndesRltBurstReadCallback(struct cmd_msg *msg, char *rsp_payload, UINT16 rsp_payload_len)
{
	UINT32 i;
	UINT32 *data;
	NdisMoveMemory(msg->rsp_payload, rsp_payload + 4, rsp_payload_len - 4);

	for (i = 0; i < (msg->rsp_payload_len - 4) / 4; i++) {
		data = (UINT32 *)(msg->rsp_payload + i * 4);
		*data = le2cpu32(*data);
	}
}


INT32 AndesRltBurstRead(RTMP_ADAPTER *ad, UINT32 offset, UINT32 cnt, UINT32 *data)
{
	struct cmd_msg *msg;
	unsigned int cur_len = 0, rsp_len, offset_num, receive_len;
	UINT32 value, cur_index = 0;
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

		msg = AndesAllocCmdMsg(ad, 8);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
	
		AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_BURST_READ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, receive_len, 
									(char *)(&data[cur_index]), AndesRltBurstReadCallback);

		value = cpu2le32(offset + cap->WlanMemmapOffset + cur_index * 4);
		AndesAppendCmdMsg(msg, (char *)&value, 4);

		value = cpu2le32((receive_len - 4) / 4);
		AndesAppendCmdMsg(msg, (char *)&value, 4);

		ret = AndesSendCmdMsg(ad, msg);

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


static VOID AndesRltRandomReadCallback(struct cmd_msg *msg, char *rsp_payload, 
											UINT16 rsp_payload_len)
{
	UINT32 i;
	RTMP_REG_PAIR *reg_pair = (RTMP_REG_PAIR *)msg->rsp_payload;
	
	for (i = 0; i < msg->rsp_payload_len / 8; i++) {
		NdisMoveMemory(&reg_pair[i].Value, rsp_payload + 8 * i + 4, 4);
		reg_pair[i].Value = le2cpu32(reg_pair[i].Value);
	}
}


INT32 AndesRltRandomRead(RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, UINT32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, receive_len;
	UINT32 i, value, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;

	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		receive_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									   ? cap->InbandPacketMaxLen 
									   : (var_len - cur_len);

		msg = AndesAllocCmdMsg(ad, receive_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_RANDOM_READ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, receive_len, 
									(char *)&reg_pair[cur_index], AndesRltRandomReadCallback);

		for (i = 0; i < receive_len / 8; i++) {
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			AndesAppendCmdMsg(msg, (char *)&value, 4);
			value = 0;
			AndesAppendCmdMsg(msg, (char *)&value, 4);
		}
	

		ret = AndesSendCmdMsg(ad, msg);

		
		cur_index += receive_len / 8;
		cur_len += cap->InbandPacketMaxLen;
	}

error:	
	return ret;
}


static VOID AndesRltRfRandomReadCallback(struct cmd_msg *msg, char *rsp_payload, UINT16 rsp_payload_len)
{
	UINT32 i;
	BANK_RF_REG_PAIR *reg_pair = (BANK_RF_REG_PAIR *)msg->rsp_payload;
			
	for (i = 0; i < msg->rsp_payload_len / 8; i++) {
		NdisMoveMemory(&reg_pair[i].Value, rsp_payload + 8 * i + 4, 1);
	}
}


INT32 AndesRltRfRandomRead(RTMP_ADAPTER *ad, BANK_RF_REG_PAIR *reg_pair, UINT32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, receive_len;
	UINT32 i, value, cur_index = 0;
	RTMP_CHIP_CAP *cap = &ad->chipCap;
	int ret = 0;
	
	if (!reg_pair)
		return -1;

	while (cur_len < var_len)
	{
		receive_len = (var_len - cur_len) > cap->InbandPacketMaxLen 
									   ? cap->InbandPacketMaxLen 
									   : (var_len - cur_len);

		msg = AndesAllocCmdMsg(ad, receive_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_RANDOM_READ, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, receive_len, 
									(char *)&reg_pair[cur_index], AndesRltRfRandomReadCallback);

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
			AndesAppendCmdMsg(msg, (char *)&value, 4);
			value = 0;
			AndesAppendCmdMsg(msg, (char *)&value, 4);
		}

		ret = AndesSendCmdMsg(ad, msg);
	
	
		cur_index += receive_len / 8;
		cur_len += cap->InbandPacketMaxLen;
	}
	
error:
	return ret;
}


INT32 AndesRltReadModifyWrite(RTMP_ADAPTER *ad, R_M_W_REG *reg_pair, UINT32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 12, cur_len = 0, sent_len;
	UINT32 value, i, cur_index = 0;
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
		
		msg = AndesAllocCmdMsg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_READ_MODIFY_WRITE, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_READ_MODIFY_WRITE, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < (sent_len / 12); i++)
		{
			/* Address */
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			AndesAppendCmdMsg(msg, (char *)&value, 4);

			/* ClearBitMask */
			value = cpu2le32(reg_pair[i + cur_index].ClearBitMask);
			AndesAppendCmdMsg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			AndesAppendCmdMsg(msg, (char *)&value, 4);
		}

		ret = AndesSendCmdMsg(ad, msg);
	
	
		cur_index += (sent_len / 12);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}


INT32 AndesRltRfReadModifyWrite(RTMP_ADAPTER *ad, RF_R_M_W_REG *reg_pair, UINT32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 12, cur_len = 0, sent_len;
	UINT32 value, i, cur_index = 0;
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

		msg = AndesAllocCmdMsg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_READ_MODIFY_WRITE, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_READ_MODIFY_WRITE, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
		
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
			AndesAppendCmdMsg(msg, (char *)&value, 4);
			
			value = 0;
			/* ClearBitMask */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].ClearBitMask;
			value = cpu2le32(value);
			AndesAppendCmdMsg(msg, (char *)&value, 4);

			value = 0;
			/* UpdateData */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Value;
			value = cpu2le32(value);
			AndesAppendCmdMsg(msg, (char *)&value, 4);
		}
	
		ret = AndesSendCmdMsg(ad, msg);
	
		cur_index += (sent_len / 12);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}


INT32 AndesRltRandomWrite(RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, UINT32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, sent_len;
	UINT32 value, i, cur_index = 0;
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
	
		msg = AndesAllocCmdMsg(ad, sent_len);
		
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_RANDOM_WRITE, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_RANDOM_WRITE, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < (sent_len / 8); i++)
		{
			/* Address */
			value = cpu2le32(reg_pair[i + cur_index].Register + cap->WlanMemmapOffset);
			AndesAppendCmdMsg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			AndesAppendCmdMsg(msg, (char *)&value, 4);
		};

		ret = AndesSendCmdMsg(ad, msg);


		cur_index += (sent_len / 8);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}


INT32 AndesRltRfRandomWrite(RTMP_ADAPTER *ad, BANK_RF_REG_PAIR *reg_pair, UINT32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, sent_len;
	UINT32 value, i, cur_index = 0;
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

		msg = AndesAllocCmdMsg(ad, sent_len);

		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_RANDOM_WRITE, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_RANDOM_WRITE, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
		
		for (i = 0; i < (sent_len / 8); i++) {
			value = 0;
			/* RF selection */
			value = (value & ~0x80000000) | 0x80000000;

			/* RF bank */
			value = (value & ~0x00ff0000) | (reg_pair[i + cur_index].Bank << 16);

			/* RF Index */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Register;
			
			value = cpu2le32(value);
			AndesAppendCmdMsg(msg, (char *)&value, 4);

			value = 0;
			/* UpdateData */
			value = (value & ~0x000000ff) | reg_pair[i + cur_index].Value;
			value = cpu2le32(value);
			AndesAppendCmdMsg(msg, (char *)&value, 4);
		}

		ret = AndesSendCmdMsg(ad, msg);


		cur_index += (sent_len / 8);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}


#ifdef CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT
INT32 AndesBbpRandomWrite(RTMP_ADAPTER *ad, RTMP_REG_PAIR *reg_pair, UINT32 num)
{
	struct cmd_msg *msg;
	unsigned int var_len = num * 8, cur_len = 0, sent_len;
	UINT32 value, i, cur_index = 0;
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
	
		msg = AndesAllocCmdMsg(ad, sent_len);
		
		if (!msg) {
			ret = NDIS_STATUS_RESOURCES;
			goto error;
		}
		
		if (last_packet)
			AndesInitCmdMsg(msg, CMD_RANDOM_WRITE, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
		else
			AndesInitCmdMsg(msg, CMD_RANDOM_WRITE, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);

		for (i = 0; i < (sent_len / 8); i++)
		{
			/* BBP selection */
			value = 0x40000000;
		
			/* Address */
			value |= reg_pair[i + cur_index].Register;
			value = cpu2le32(value);

			AndesAppendCmdMsg(msg, (char *)&value, 4);

			/* UpdateData */
			value = cpu2le32(reg_pair[i + cur_index].Value);
			AndesAppendCmdMsg(msg, (char *)&value, 4);
		};

		ret = AndesSendCmdmsg(ad, msg);


		cur_index += (sent_len / 8);
		cur_len += cap->InbandPacketMaxLen;
	}

error:
	return ret;
}
#endif /* CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT */


INT32 AndesRltScRandomWrite(RTMP_ADAPTER *ad, CR_REG *table, UINT32 nums, UINT32 flags)
{
	UINT32 varlen = 0, i, j;
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

	AndesRltRandomWrite(ad, sw_ch_table, varlen / sizeof(RTMP_REG_PAIR));

	os_free_mem(NULL, sw_ch_table);

	return 0;
}


INT32 AndesRltScRfRandomWrite(RTMP_ADAPTER *ad, BANK_RF_CR_REG *table, UINT32 nums, UINT32 flags)
{
	UINT32 varlen = 0, i, j;
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

	AndesRltRfRandomWrite(ad, sw_ch_table, varlen / sizeof(BANK_RF_REG_PAIR));

	os_free_mem(NULL, sw_ch_table);

	return 0;
}


INT32 AndesRltPwrSaving(RTMP_ADAPTER *ad, UINT32 op, UINT32 level, 
						 UINT32 listen_interval, UINT32 pre_tbtt_lead_time,
						 UINT8 tim_byte_offset, UINT8 tim_byte_pattern)
{
	struct cmd_msg *msg;
	unsigned int var_len;
	UINT32 value;
	int ret = 0;

	/* Power operation and Power Level */
	var_len = 8;

	if (op == RADIO_OFF_ADVANCE)
	{
		/* Listen interval, Pre-TBTT, TIM info */
		var_len += 12;
	}
	
	msg = AndesAllocCmdMsg(ad, var_len);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_POWER_SAVING_OP, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
	
	/* Power operation */
	value = cpu2le32(op);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	/* Power Level */
	value = cpu2le32(level);

	AndesAppendCmdMsg(msg, (char *)&value, 4);

	if (op == RADIO_OFF_ADVANCE)
	{
		/* Listen interval */
		value = cpu2le32(listen_interval);
		AndesAppendCmdMsg(msg, (char *)&value, 4);


		/* Pre TBTT lead time */
		value = cpu2le32(pre_tbtt_lead_time);
		AndesAppendCmdMsg(msg, (char*)&value, 4);

		/* TIM Info */
		value = (value & ~0x000000ff) | tim_byte_pattern;
		value = (value & ~0x0000ff00) | (tim_byte_offset << 8);
		value = cpu2le32(value);
		AndesAppendCmdMsg(msg, (char *)&value, 4);
	}

	ret = AndesSendCmdMsg(ad, msg);
	
error:
	return ret;
}


INT32 AndesRltFunSet(RTMP_ADAPTER *ad, UINT32 fun_id, UINT32 param)
{
	struct cmd_msg *msg;
	UINT32 value;
	int ret = 0;

	/* Function ID and Parameter */
	msg = AndesAllocCmdMsg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	if (fun_id != Q_SELECT)
		AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_FUN_SET_OP, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
	else 
		AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_FUN_SET_OP, CMD_NA, EXT_CMD_NA, FALSE, 0, FALSE, FALSE, 0, NULL, NULL);
	
	/* Function ID */
	value = cpu2le32(fun_id);
	AndesAppendCmdMsg(msg, (char *)&value, 4);
	
	/* Parameter */
	value = cpu2le32(param);
	AndesAppendCmdMsg(msg, (char *)&value, 4);	
		
	ret = AndesSendCmdMsg(ad, msg);

error:
	return ret;
}


INT32 AndesRltCalibration(RTMP_ADAPTER *ad, UINT32 cal_id, ANDES_CALIBRATION_PARAM *param)
{
	struct cmd_msg *msg;
	UINT32 value;
	int ret = 0;

	DBGPRINT(RT_DEBUG_INFO, ("%s:cal_id(%d)\n ", __FUNCTION__, cal_id));


		msg = AndesAllocCmdMsg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_CALIBRATION_OP, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);
	
	/* Calibration ID */
	value = cpu2le32(cal_id);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	/* Parameter */
	{
		value = cpu2le32(param->generic);
		AndesAppendCmdMsg(msg, (char *)&value, 4);
	}

	ret = AndesSendCmdMsg(ad, msg);


error:
	return ret;
}


INT32 AndesRltLoadCr(RTMP_ADAPTER *ad, UINT32 cr_type, UINT8 temp_level, UINT8 channel)
{
	struct cmd_msg *msg;
	UINT32 value = 0;
	int ret = 0;

	DBGPRINT(RT_DEBUG_OFF, ("%s:cr_type(%d), channel(%d)\n", __FUNCTION__, cr_type, temp_level, channel));
	
	msg = AndesAllocCmdMsg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_LOAD_CR, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

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
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	value = 0x80000000;
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] >> 8) & 0xFF);
	value |= ((ad->EEPROMDefaultValue[EEPROM_NIC_CFG2_OFFSET] & 0xFF) << 8 );
	value = cpu2le32(value);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	ret = AndesSendCmdMsg(ad, msg);

error:
	return ret;
}


INT32 AndesRltSwitchChannel(RTMP_ADAPTER *ad, UINT8 channel, BOOLEAN scan, unsigned int bw, unsigned int tx_rx_setting, UINT8 bbp_ch_idx)
{
	struct cmd_msg *msg;
	UINT32 value = 0;
	int ret;

	DBGPRINT(RT_DEBUG_INFO, ("%s:channel(%d),scan(%d),bw(%d),trx(0x%x)\n", __FUNCTION__, channel, scan, bw, tx_rx_setting));

	msg = AndesAllocCmdMsg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_SWITCH_CHANNEL_OP, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

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
	AndesAppendCmdMsg(msg, (char *)&value, 4);
	
	value = 0;
	value |= SC_PARAM2_TR_SETTING(tx_rx_setting);
	value = cpu2le32(value);
	AndesAppendCmdMsg(msg, (char *)&value, 4);

	ret = AndesSendCmdMsg(ad, msg);
	
	mdelay(5);

	msg = AndesAllocCmdMsg(ad, 8);

	if (!msg) {
		ret = NDIS_STATUS_RESOURCES;
		goto error;
	}

	AndesInitCmdMsg(msg, CPU_TX_PORT, CMD_SWITCH_CHANNEL_OP, CMD_NA, EXT_CMD_NA, TRUE, 0, TRUE, TRUE, 0, NULL, NULL);

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
	AndesAppendCmdMsg(msg, (char *)&value, 4);
	
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
	AndesAppendCmdMsg(msg, (char *)&value, 4);
	
	ret = AndesSendCmdMsg(ad, msg);

error:
	return ret;
}


#ifdef RTMP_PCI_SUPPORT
VOID AndesRltPciFwInit(RTMP_ADAPTER *ad)
{
	UINT32 value;

	DBGPRINT(RT_DEBUG_OFF, ("%s\n", __FUNCTION__));
#ifdef RLT_MAC
	/* Enable Interrupt*/
	RTMP_IRQ_ENABLE(ad);
	RTMPEnableRxTx(ad);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_START_UP);
	RTMP_SET_FLAG(ad, fRTMP_ADAPTER_MCU_SEND_IN_BAND_CMD);
	/* clear garbage interrupts*/
	RTMP_IO_READ32(ad, 0x1300, &value);
	DBGPRINT(RT_DEBUG_OFF, ("0x1300 = %08x\n", value));

#ifdef HDR_TRANS_SUPPORT
	RTMP_IO_WRITE32(ad, HEADER_TRANS_CTRL_REG, 0X2);
	RTMP_IO_WRITE32(ad, TSO_CTRL, 0x7050);
#else
	RTMP_IO_WRITE32(ad, HEADER_TRANS_CTRL_REG, 0x0);
	RTMP_IO_WRITE32(ad, TSO_CTRL, 0x0);
#endif

	AndesRltFunSet(ad, Q_SELECT, ad->chipCap.CmdRspRxRing);
	PWR_SAVING_OP(ad, RADIO_ON, 0, 0, 0, 0, 0);
#endif /* RLT_MAC */
}
#endif /* RTMP_PCI_SUPPORT */



