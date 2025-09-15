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
	fwdl.c
*/

#ifdef COMPOS_WIN
#include "MtConfig.h"
#if defined(EVENT_TRACING)
#include "fwdl_mt.tmh"
#endif

struct MCU_CTRL;

#elif defined(COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

static VOID img_get_8bit(UINT8 *dest, UINT8 **src, UINT32 cnt)
{
	UINT32 i;

	for (i = 0; i < cnt; i++) {
		dest[i] = *(*src + i);
	}

	*src += cnt;
}

static VOID img_get_32bit(UINT32 *dest, UINT8 **src, UINT32 cnt)
{
	UINT32 i;

	for (i = 0; i < cnt; i++) {
		dest[i] = (UINT32)((*(*src + (i * 4) + 3) << 24) |
						   (*(*src + (i * 4) + 2) << 16) |
						   (*(*src + (i * 4) + 1) <<  8) |
						   (*(*src + (i * 4))     <<  0));
	}

	*src += cnt * 4;
}

static INT32 MtCmdFwScatters(RTMP_ADAPTER *ad, UINT8 *image, UINT32 image_len)
{
	UINT32 sent_len;
	UINT32 cur_len = 0, count = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(ad->hdev_ctrl);
	int ret = 0;

	while (1) {
		UINT32 sent_len_max = MT_UPLOAD_FW_UNIT - cap->cmd_header_len;

		sent_len = (image_len - cur_len) >=  sent_len_max ?
				   sent_len_max : (image_len - cur_len);

		if (sent_len > 0) {
			ret = MtCmdFwScatter(ad, image + cur_len, sent_len, count);
			count++;

			if (ret)
				goto error;

			cur_len += sent_len;
		} else
			break;
	}

error:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			 ("%s:(ret = %d)\n", __func__, ret));
	return ret;
}

static NDIS_STATUS load_code(struct _RTMP_ADAPTER *pAd, UINT32 method, struct img_source *src)
{
	NDIS_STATUS ret = NDIS_STATUS_FAILURE;

	if ((ret != NDIS_STATUS_SUCCESS) && (method & BIT(BIN_METHOD))) {
		if (src->bin_name)
			os_load_code_from_bin(pAd, &src->img_ptr, src->bin_name, &src->img_len);

		if (src->img_ptr) {
			src->applied_method = BIN_METHOD;
			ret =  NDIS_STATUS_SUCCESS;
		}
	}

	if ((ret != NDIS_STATUS_SUCCESS) && (method & BIT(HEADER_METHOD))) {
		if (src->header_ptr && src->header_len) {
			src->img_ptr = src->header_ptr;
			src->img_len = src->header_len;
			src->applied_method = HEADER_METHOD;
			ret = NDIS_STATUS_SUCCESS;
		}
	}

	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("load code method: cap (bitmap) = 0x%x, applied = %d\n", method, src->applied_method));

	return ret;
}

static NDIS_STATUS alloc_patch_target(struct _RTMP_ADAPTER *pAd, struct patch_dl_target *target, UINT32 count)
{
	if (count < 0)
		return NDIS_STATUS_FAILURE;

	target->num_of_region = count;
	return os_alloc_mem(pAd, (UCHAR **)&target->patch_region, count * sizeof(struct patch_dl_buf));
}

static VOID free_patch_target(struct _RTMP_ADAPTER *pAd, struct patch_dl_target *target)
{
	os_free_mem(target->patch_region);
	target->patch_region = NULL;
	target->num_of_region = 0;
}

static NDIS_STATUS alloc_fw_target(struct _RTMP_ADAPTER *pAd, struct fw_dl_target *target, UINT32 count)
{
	if (count < 0)
		return NDIS_STATUS_FAILURE;

	target->num_of_region = count;
	return os_alloc_mem(pAd, (UCHAR **)&target->fw_region, count * sizeof(struct fw_dl_buf));
}

static VOID free_fw_target(struct _RTMP_ADAPTER *pAd, struct fw_dl_target *target)
{
	os_free_mem(target->fw_region);
	target->fw_region = NULL;
	target->num_of_region = 0;
}

static VOID show_patch_info_cpu(struct patch_info *patch_info)
{
	FWDL_PRINT_CHAR(patch_info->built_date, 16, ("\tBuilt date: "));
	FWDL_PRINT_CHAR(patch_info->platform, 4, ("\tPlatform: "));
	FWDL_PRINT_HEX(patch_info->hw_sw_version, 4, ("\tHW/SW version: "));
	FWDL_PRINT_HEX(patch_info->patch_version, 4, ("\tPatch version: "));
}

VOID show_patch_info(struct _RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 need_load, i;
	struct patch_info *patch_info;

	need_load = chip_cap->need_load_patch;

	for (i = 0; i < MAX_CPU; i++) {
		if (need_load & BIT(i)) {
			patch_info = &pAd->MCUCtrl.fwdl_ctrl.patch_profile[i].patch_info;

			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CPU %d patch info\n", i));

			show_patch_info_cpu(patch_info);
		}
	}
}

static VOID show_fw_info_cpu(struct fw_info *fw_info)
{
	FWDL_PRINT_HEX(&fw_info->chip_id, 1, ("\tChip ID: "));
	FWDL_PRINT_HEX(&fw_info->eco_ver, 1, ("\tEco version: "));
	FWDL_PRINT_HEX(&fw_info->num_of_region, 1, ("\tRegion number: "));
	FWDL_PRINT_HEX(&fw_info->format_ver, 1, ("\tFormat version: "));
	FWDL_PRINT_CHAR(fw_info->ram_ver, 10, ("\tRam version: "));
	FWDL_PRINT_CHAR(fw_info->ram_built_date, 15, ("\tBuilt date: "));
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tCommon crc: 0x%x\n", fw_info->crc));
}

VOID show_fw_info(struct _RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	UINT32 need_load, i;
	struct fw_info *fw_info;

	need_load = chip_cap->need_load_fw;

	for (i = 0; i < MAX_CPU; i++) {
		if (need_load & BIT(i)) {
			fw_info = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[i].fw_info;

			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CPU %d fw info\n", i));

			show_fw_info_cpu(fw_info);
		}
	}
}

static NDIS_STATUS parse_patch_v1(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct patch_dl_target *target)
{
	NDIS_STATUS ret;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct img_source *src;
	struct patch_info *patch_info;
	UINT8 *img_ptr;
	UINT32 num_of_region, i;

	src = &pAd->MCUCtrl.fwdl_ctrl.patch_profile[cpu].source;
	patch_info = &pAd->MCUCtrl.fwdl_ctrl.patch_profile[cpu].patch_info;

	ret = load_code(pAd, cap->load_patch_method, src);
	if (ret)
		goto out;

	/* parse patch info */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Parsing patch header\n"));

	img_ptr = src->img_ptr;

	img_get_8bit(patch_info->built_date, &img_ptr, 16);
	img_get_8bit(patch_info->platform, &img_ptr, 4);
	img_get_8bit(patch_info->hw_sw_version, &img_ptr, 4);
	img_get_8bit(patch_info->patch_version, &img_ptr, 4);

	show_patch_info_cpu(patch_info);

	/* fix to one region for this rom patch format */
	num_of_region = 1;

	ret = alloc_patch_target(pAd, target, num_of_region);
	if (ret)
		goto out;

	for (i = 0; i < num_of_region; i++) {
		struct patch_dl_buf *region;

		region = &target->patch_region[i];

		region->img_dest_addr = cap->rom_patch_offset;
		region->img_size = src->img_len - PATCH_V1_INFO_SIZE;
		region->img_ptr = src->img_ptr + PATCH_V1_INFO_SIZE;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("\tTarget address: 0x%x, length: %d\n", region->img_dest_addr, region->img_size));
	}

	return NDIS_STATUS_SUCCESS;

out:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: parse patch fail\n", __func__));

	return ret;
}

static NDIS_STATUS load_patch_v1(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct patch_dl_target *target)
{
	NDIS_STATUS ret;
	UINT32 num_of_region, i;
	struct fwdl_ctrl *fwdl_ctrl;

	ret = NDIS_STATUS_SUCCESS;
	fwdl_ctrl = &pAd->MCUCtrl.fwdl_ctrl;
	num_of_region = target->num_of_region;

	if (num_of_region < 0) {
		ret = NDIS_STATUS_FAILURE;
		goto out;
	}

	for (i = 0; i < num_of_region; i++) {
		struct patch_dl_buf *region;

		region = &target->patch_region[i];

		fwdl_ctrl->stage = FWDL_STAGE_CMD_EVENT;

		/* 1. get semaphore */
		ret = MtCmdPatchSemGet(pAd, GET_PATCH_SEM);
		if (ret)
			goto out;

		switch (fwdl_ctrl->sem_status) {
		case PATCH_NOT_DL_SEM_FAIL:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("patch is not ready && get semaphore fail\n"));
			ret = NDIS_STATUS_FAILURE;
			goto out;
			break;
		case PATCH_IS_DL:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("patch is ready, continue to ILM/DLM DL\n"));
			ret = NDIS_STATUS_SUCCESS;
			goto out;
			break;
		case PATCH_NOT_DL_SEM_SUCCESS:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("patch is not ready && get semaphore success\n"));
			break;
		default:
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("get semaphore invalid status(%d)\n", fwdl_ctrl->sem_status));
			ret = NDIS_STATUS_FAILURE;
			goto out;
			break;
		}

		/* 2. config PDA */
		ret = MtCmdAddressLenReq(pAd, region->img_dest_addr, region->img_size, MODE_TARGET_ADDR_LEN_NEED_RSP);
		if (ret)
			goto out;

		/* 3. image scatter */
		fwdl_ctrl->stage = FWDL_STAGE_SCATTER;

		ret = MtCmdFwScatters(pAd, region->img_ptr, region->img_size);
		if (ret)
			goto out;

		/* 4. patch start */
		fwdl_ctrl->stage = FWDL_STAGE_CMD_EVENT;

		ret = MtCmdPatchFinishReq(pAd);
		if (ret)
			goto out;

		/* 5. release semaphore */
		ret = MtCmdPatchSemGet(pAd, REL_PATCH_SEM);
		if (ret)
			goto out;

		if (fwdl_ctrl->sem_status == SEM_RELEASE) {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("release patch semaphore\n"));
		} else {
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("release patch semaphore invalid status(%d)\n", fwdl_ctrl->sem_status));
			ret = NDIS_STATUS_FAILURE;
			goto out;
		}
	}

out:
	free_patch_target(pAd, target);

	if (ret) {
		fwdl_ctrl->stage = FWDL_STAGE_FW_NOT_DL;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: load patch fail\n", __func__));
	}

	return ret;
}

static NDIS_STATUS parse_fw_v1(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct fw_dl_target *target)
{
	NDIS_STATUS ret;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct img_source *src;
	struct fw_info *fw_info;
	UINT8 *img_ptr;
	UINT32 num_of_region, i;

	src = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[cpu].source;
	fw_info = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[cpu].fw_info;

	ret = load_code(pAd, cap->load_fw_method, src);
	if (ret)
		goto out;

	/* parse fw info */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Parsing CPU %d fw tailer\n", cpu));

	img_ptr = src->img_ptr + src->img_len - 29;

	img_get_8bit(fw_info->ram_ver, &img_ptr, 10);
	img_get_8bit(fw_info->ram_built_date, &img_ptr, 15);

	show_fw_info_cpu(fw_info);

	/* fix to 1 because they're all von-neumann architecture */
	num_of_region = 1;

	ret = alloc_fw_target(pAd, target, num_of_region);
	if (ret)
		goto out;

	for (i = 0; i < num_of_region; i++) {
		UINT32 dl_len;
		struct fw_dl_buf *region;

		img_ptr = src->img_ptr + src->img_len - 4;

		img_get_32bit(&dl_len, &img_ptr, 1);
		dl_len += 4; /* including 4 byte inverse-crc */

		region = &target->fw_region[i];
#define FW_CODE_START_ADDRESS1 0x100000
		region->img_dest_addr = FW_CODE_START_ADDRESS1; /* hard code because header don't have it */
		region->img_size = dl_len;
		region->img_ptr = src->img_ptr;
		region->feature_set = 0; /* no feature set field for this fw format */
		region->feature_set |= FW_FEATURE_OVERRIDE_RAM_ADDR; /* hard code this field to override ram starting address */
	}

	return NDIS_STATUS_SUCCESS;

out:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: parse fw fail\n", __func__));

	return ret;
}

static NDIS_STATUS parse_fw_v2(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct fw_dl_target *target)
{
	NDIS_STATUS ret;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct img_source *src;
	struct fw_info *fw_info;
	UINT8 *img_ptr;
	UINT32 num_of_region, i, offset;

	src = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[cpu].source;
	fw_info = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[cpu].fw_info;
	offset = 0;

	ret = load_code(pAd, cap->load_fw_method, src);
	if (ret)
		goto out;

	/* parse fw info */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Parsing CPU %d fw tailer\n", cpu));

	img_ptr = src->img_ptr + src->img_len - FW_V2_INFO_SIZE;

	img_ptr += 4; /* bypass destination address field */
	img_get_8bit(&fw_info->chip_id, &img_ptr, 1);
	img_ptr += 1; /* bypass feature set field */
	img_get_8bit(&fw_info->eco_ver, &img_ptr, 1);
	img_get_8bit(fw_info->ram_ver, &img_ptr, 10);
	img_get_8bit(fw_info->ram_built_date, &img_ptr, 15);

	show_fw_info_cpu(fw_info);

	if (cpu == WA_CPU)
		num_of_region = 1; /* for 7615 CR4, which is von-neumann architecture */
	else
		num_of_region = 2; /* fix to 2 because they're all harvard architecture */

	ret = alloc_fw_target(pAd, target, num_of_region);
	if (ret)
		goto out;

	/* first region first parsing */
	for (i = 0; i < num_of_region; i++) {
		UINT32 dl_addr, dl_len;
		UINT8 dl_feature_set;
		struct fw_dl_buf *region;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Parsing tailer region %d\n", i));

		img_ptr = src->img_ptr + src->img_len - (num_of_region - i) * FW_V2_INFO_SIZE;

		img_get_32bit(&dl_addr, &img_ptr, 1);

		img_ptr += 1; /* bypass chip id field */

		img_get_8bit(&dl_feature_set, &img_ptr, 1);
		FWDL_PRINT_HEX(&dl_feature_set, 1, ("\tFeature set: "));

		img_ptr += 26; /* jump to size field */

		img_get_32bit(&dl_len, &img_ptr, 1);
		dl_len += 4; /* including 4 byte inverse-crc */
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTarget address: 0x%x, Download size: %d\n", dl_addr, dl_len));

		region = &target->fw_region[i];
		region->img_dest_addr = dl_addr;
		region->img_size = dl_len;
		region->img_ptr = src->img_ptr + offset;
		offset += dl_len;
		region->feature_set = dl_feature_set;
		if (IS_MT7615(pAd) && (cpu == WM_CPU) && (i == 0)) { /* for 7615 N9 ILM */
			region->feature_set |= FW_FEATURE_OVERRIDE_RAM_ADDR; /* hard code this field to override ram starting address */
		}
	}

	return NDIS_STATUS_SUCCESS;

out:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: parse fw fail\n", __func__));

	return ret;
}

static NDIS_STATUS parse_fw_v3(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct fw_dl_target *target)
{
	NDIS_STATUS ret;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);
	struct img_source *src;
	UINT8 *img_ptr;
	struct fw_info *fw_info;
	UINT32 num_of_region, i, offset;

	src = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[cpu].source;
	fw_info = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[cpu].fw_info;
	offset = 0;

	ret = load_code(pAd, cap->load_fw_method, src);
	if (ret)
		goto out;

	/* parse fw info */
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Parsing CPU %d fw tailer\n", cpu));

	img_ptr = src->img_ptr + src->img_len - FW_V3_COMMON_TAILER_SIZE;

	img_get_8bit(&fw_info->chip_id, &img_ptr, 1);
	img_get_8bit(&fw_info->eco_ver, &img_ptr, 1);
	img_get_8bit(&fw_info->num_of_region, &img_ptr, 1);
	img_get_8bit(&fw_info->format_ver, &img_ptr, 1);
	img_ptr += 3; /* bypass reserved field */
	img_get_8bit(fw_info->ram_ver, &img_ptr, 10);
	img_get_8bit(fw_info->ram_built_date, &img_ptr, 15);
	img_get_32bit(&fw_info->crc, &img_ptr, 1);

	show_fw_info_cpu(fw_info);

	num_of_region = fw_info->num_of_region;

	ret = alloc_fw_target(pAd, target, num_of_region);
	if (ret)
		goto out;

	/* first region first parsing */
	for (i = 0; i < num_of_region; i++) {
		struct fw_dl_buf *region;
		UINT32 dl_addr, dl_len, decomp_crc, decomp_len, decomp_block_size;
		UINT8 dl_feature_set;

		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Parsing tailer region %d\n", i));

		img_ptr = src->img_ptr + src->img_len - FW_V3_COMMON_TAILER_SIZE - (num_of_region - i) * FW_V3_REGION_TAILER_SIZE;

		img_get_32bit(&decomp_crc, &img_ptr, 1);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDecomp crc: 0x%x\n", decomp_crc));

		img_get_32bit(&decomp_len, &img_ptr, 1);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDecomp size: 0x%x\n", decomp_len));

		img_get_32bit(&decomp_block_size, &img_ptr, 1);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDecomp block size: 0x%x\n", decomp_block_size));

		img_ptr += 4; /* bypass reserved field */

		img_get_32bit(&dl_addr, &img_ptr, 1);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tTarget address: 0x%x\n", dl_addr));

		img_get_32bit(&dl_len, &img_ptr, 1);
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\tDownload size: %d\n", dl_len));

		img_get_8bit(&dl_feature_set, &img_ptr, 1);
		FWDL_PRINT_HEX(&dl_feature_set, 1, ("\tFeature set: "));

		region = &target->fw_region[i];
		region->img_dest_addr = dl_addr;
		region->img_size = dl_len;
		region->feature_set = dl_feature_set;
		region->decomp_crc = decomp_crc;
		region->decomp_img_size = decomp_len;
		region->decomp_block_size = decomp_block_size;
		region->img_ptr = src->img_ptr + offset;
		offset += dl_len;
	}

	return NDIS_STATUS_SUCCESS;

out:
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: parse fw fail\n", __func__));

	return ret;
}

static NDIS_STATUS load_fw_v1(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct fw_dl_target *target)
{
	NDIS_STATUS ret;
	UINT32 num_of_region, i, override, override_addr;
	struct fwdl_ctrl *fwdl_ctrl;
	struct MCU_CTRL *mcu_ctrl;

	ret = NDIS_STATUS_SUCCESS;
	override = 0;
	override_addr = 0;
	fwdl_ctrl = &pAd->MCUCtrl.fwdl_ctrl;
	mcu_ctrl = &pAd->MCUCtrl;
	num_of_region = target->num_of_region;

	if (num_of_region < 0) {
		ret = NDIS_STATUS_FAILURE;
		goto out;
	}

	/* first parsing first download */
	for (i = 0; i < num_of_region; i++) {
		struct fw_dl_buf *region;

		region = &target->fw_region[i];

		if (region->feature_set & FW_FEATURE_OVERRIDE_RAM_ADDR) {
			override |= FW_START_OVERRIDE_START_ADDRESS;
			override_addr = region->img_dest_addr;
		}

		fwdl_ctrl->stage = FWDL_STAGE_CMD_EVENT;

		/* 1. config PDA */
		ret = MtCmdAddressLenReq(pAd, region->img_dest_addr, region->img_size,
								((region->feature_set & FW_FEATURE_SET_ENCRY) ? MODE_ENABLE_ENCRY : 0) |
								(MODE_SET_KEY(GET_FW_FEATURE_SET_KEY(region->feature_set))) |
								((region->feature_set & FW_FEATURE_SET_ENCRY) ? MODE_RESET_SEC_IV : 0) |
								((cpu == WA_CPU) ? MODE_WORKING_PDA_OPTION : 0) |
								MODE_TARGET_ADDR_LEN_NEED_RSP);
		if (ret)
			goto out;

		/* 2. image scatter */
		fwdl_ctrl->stage = FWDL_STAGE_SCATTER;

		ret = MtCmdFwScatters(pAd, region->img_ptr, region->img_size);
		if (ret)
			goto out;
	}

	/* 3. fw start negotiation */
	fwdl_ctrl->stage = FWDL_STAGE_CMD_EVENT;

	ret = MtCmdFwStartReq(pAd, override | ((cpu == WA_CPU) ? FW_START_WORKING_PDA_OPTION : 0), override_addr);

out:
	free_fw_target(pAd, target);

	if (ret) {
		fwdl_ctrl->stage = FWDL_STAGE_FW_NOT_DL;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: load fw fail\n", __func__));
	}

	return ret;
}

static NDIS_STATUS load_fw_v2_compressimg(struct _RTMP_ADAPTER *pAd, enum target_cpu cpu, struct fw_dl_target *target)
{
	NDIS_STATUS ret;
	UINT32 num_of_region, i, override, override_addr;
	UINT32 compress_region_num = 0;
	UINT32 block_idx = 0;
	UINT32 block_dest_addr;
	UINT32 remain_chunk_size = 0;
	UINT8 *img_ptr_pos = NULL;
	struct fwdl_ctrl *fwdl_ctrl;
	struct MCU_CTRL *mcu_ctrl;
	INIT_CMD_WIFI_START_WITH_DECOMPRESSION decompress_info;
	UINT8 do_compressed_dl = 0;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	ret = NDIS_STATUS_SUCCESS;
	override = 0;
	override_addr = 0;
	fwdl_ctrl = &pAd->MCUCtrl.fwdl_ctrl;
	mcu_ctrl = &pAd->MCUCtrl;
	num_of_region = target->num_of_region;

	if (num_of_region < 0) {
		ret = NDIS_STATUS_FAILURE;
		goto out;
	}

	/* first parsing first download */
	for (i = 0; i < num_of_region; i++) {
		struct fw_dl_buf *region;

		region = &target->fw_region[i];
		img_ptr_pos = region->img_ptr;
		remain_chunk_size = region->img_size;
		if (region->feature_set & FW_FEATURE_OVERRIDE_RAM_ADDR) {
			override |= FW_START_OVERRIDE_START_ADDRESS;
			override_addr = region->img_dest_addr;
		}
		if (region->feature_set & FW_FEATURE_COMPRESS_IMG) { /* Compressed image Process */
			do_compressed_dl = 1;
			block_idx = 0;
			compress_region_num += 1;
			MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("REGION[%d] COMPRESSED IMAGE DOWNLOAD\n", i));
			while (remain_chunk_size > 0) {
				UINT32 payload_size_per_chunk = 0;

				fwdl_ctrl->stage = FWDL_STAGE_CMD_EVENT;
				img_get_32bit(&payload_size_per_chunk, &img_ptr_pos, 1);
				remain_chunk_size -= 4;
				/* 1. config PDA */
				block_dest_addr = region->img_dest_addr + block_idx * region->decomp_block_size;
				ret = MtCmdAddressLenReq(pAd, block_dest_addr, payload_size_per_chunk,
					((region->feature_set & FW_FEATURE_SET_ENCRY) ? MODE_ENABLE_ENCRY : 0) |
					(MODE_SET_KEY(GET_FW_FEATURE_SET_KEY(region->feature_set))) |
					((region->feature_set & FW_FEATURE_SET_ENCRY) ? MODE_RESET_SEC_IV : 0) |
					((cpu == WA_CPU) ? MODE_WORKING_PDA_OPTION : 0) |
					MODE_TARGET_ADDR_LEN_NEED_RSP);
				if (ret)
					goto out;
				/* 2. image scatter */
				fwdl_ctrl->stage = FWDL_STAGE_SCATTER;
				ret = MtCmdFwScatters(pAd, img_ptr_pos, payload_size_per_chunk);
				if (ret)
					goto out;
				remain_chunk_size -= payload_size_per_chunk;
				img_ptr_pos += payload_size_per_chunk;
				block_idx++;
			}
			decompress_info.aucDecompRegion[i].u4RegionAddress = region->img_dest_addr;
			decompress_info.aucDecompRegion[i].u4Regionlength = region->decomp_img_size;
			decompress_info.aucDecompRegion[i].u4RegionCRC = region->decomp_crc;
			decompress_info.u4BlockSize = region->decomp_block_size;
		} else { /* Uncompressed Image Process*/
			fwdl_ctrl->stage = FWDL_STAGE_CMD_EVENT;
			/* 1. config PDA */
			ret = MtCmdAddressLenReq(pAd, region->img_dest_addr, region->img_size,
						((region->feature_set & FW_FEATURE_SET_ENCRY) ? MODE_ENABLE_ENCRY : 0) |
						(MODE_SET_KEY(GET_FW_FEATURE_SET_KEY(region->feature_set))) |
						((region->feature_set & FW_FEATURE_SET_ENCRY) ? MODE_RESET_SEC_IV : 0) |
						((cpu == WA_CPU) ? MODE_WORKING_PDA_OPTION : 0) |
						MODE_TARGET_ADDR_LEN_NEED_RSP);
			if (ret)
				goto out;
			/* 2. image scatter */
			fwdl_ctrl->stage = FWDL_STAGE_SCATTER;

			ret = MtCmdFwScatters(pAd, region->img_ptr, region->img_size);
			if (ret)
				goto out;
		}
	} /* num_of_region */
	fwdl_ctrl->stage = FWDL_STAGE_CMD_EVENT;
	if (do_compressed_dl) {
		/*if the override[5] is not set, the ROM CODE will use default WIFI ILM as start address */
		decompress_info.u4Address = override_addr;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Start CMD Jump Address 0x%x\n", decompress_info.u4Address));
		decompress_info.u4DecompressTmpAddress = cap->decompress_tmp_addr;
		override |= ((cpu == WA_CPU) ? FW_START_WORKING_PDA_OPTION : 0);
		override |= FW_CHANGE_DECOMPRESSION_TMP_ADDRESS;
		decompress_info.u4Override = override;
		decompress_info.u4RegionNumber = compress_region_num;
		/* 3. fw start negotiation */
		ret = MtCmdFwDecompressStart(pAd, &decompress_info);
	} else {
		/* 3. fw start negotiation */
		ret = MtCmdFwStartReq(pAd, override | ((cpu == WA_CPU) ? FW_START_WORKING_PDA_OPTION : 0), override_addr);
	}
out:
	free_fw_target(pAd, target);

	if (ret) {
		fwdl_ctrl->stage = FWDL_STAGE_FW_NOT_DL;
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: load fw fail\n", __func__));
	}

	return ret;
}

static VOID free_patch_buf(RTMP_ADAPTER *pAd)
{
	UINT32 i, loaded;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	loaded = cap->need_load_patch;

	for (i = 0; i < MAX_CPU; i++) {
		if (loaded & BIT(i)) {
			struct img_source *src;

			src = &pAd->MCUCtrl.fwdl_ctrl.patch_profile[i].source;
			if ((src->applied_method == BIN_METHOD) && (src->img_ptr)) {
				os_free_mem(src->img_ptr);
				src->img_ptr = NULL;
			}
		}
	}
}

static VOID free_fw_buf(RTMP_ADAPTER *pAd)
{
	UINT32 i, loaded;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	loaded = cap->need_load_fw;

	for (i = 0; i < MAX_CPU; i++) {
		if (loaded & BIT(i)) {
			struct img_source *src;

			src = &pAd->MCUCtrl.fwdl_ctrl.fw_profile[i].source;
			if ((src->applied_method == BIN_METHOD) && (src->img_ptr)) {
				os_free_mem(src->img_ptr);
				src->img_ptr = NULL;
			}
		}
	}
}


/*
 * setup fw wifi task to be in the right state for a specific fwdl stage
 */
static NDIS_STATUS ctrl_fw_state_v2(struct _RTMP_ADAPTER *pAd, enum fwdl_stage target_stage)
{
	NDIS_STATUS ret;
	UINT32 fw_sync, loop;
	RTMP_CHIP_CAP *cap = hc_get_chip_cap(pAd->hdev_ctrl);

	ret = NDIS_STATUS_SUCCESS;
	fw_sync = MtAsicGetFwSyncValue(pAd);

	switch (target_stage) {
	case FWDL_STAGE_FW_NOT_DL:
		/* could be in both state because of wifi default on/off */
		if (fw_sync == WIFI_TASK_STATE_INITIAL || fw_sync == WIFI_TASK_STATE_FW_DOWNLOAD) {
			ret = NDIS_STATUS_SUCCESS;
			break;
		}

		/* fw restart cmd*/
		ret = MtCmdRestartDLReq(pAd);
		if (ret)
			break;

		/* polling */
		loop = 0;
		ret = NDIS_STATUS_FAILURE;
		do {
			fw_sync = MtAsicGetFwSyncValue(pAd);

			if (fw_sync == WIFI_TASK_STATE_INITIAL || fw_sync == WIFI_TASK_STATE_FW_DOWNLOAD) {
				ret = NDIS_STATUS_SUCCESS;
				break;
			}

			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= WAIT_LOOP);

		break;
	case FWDL_STAGE_CMD_EVENT:
	case FWDL_STAGE_SCATTER:
		if (fw_sync == WIFI_TASK_STATE_FW_DOWNLOAD) {
			ret = NDIS_STATUS_SUCCESS;
			break;
		} else if (fw_sync == WIFI_TASK_STATE_INITIAL) {
			/* power on wifi sys */
			ret = MtCmdPowerOnWiFiSys(pAd);
			if (ret)
				break;
		} else {
			/* in RAM code, use restart cmd */
			ret = MtCmdRestartDLReq(pAd);
			if (ret)
				break;

			/* power on wifi sys */
			ret = MtCmdPowerOnWiFiSys(pAd);
			if (ret)
				break;
		}

		/* polling */
		loop = 0;
		ret = NDIS_STATUS_FAILURE;
		do {
			fw_sync = MtAsicGetFwSyncValue(pAd);

			if (fw_sync == WIFI_TASK_STATE_FW_DOWNLOAD) {
				ret = NDIS_STATUS_SUCCESS;
				break;
			}

			RtmpOsMsDelay(1);
			loop++;
		} while (loop <= WAIT_LOOP);

		break;
	case FWDL_STAGE_FW_RUNNING:
		{
			UINT32 target_fw_sync;

			target_fw_sync = (cap->need_load_fw & BIT(WA_CPU)) ? WIFI_TASK_STATE_WACPU_RDY : WIFI_TASK_STATE_NORMAL_TRX;

			if (fw_sync == target_fw_sync) {
				ret = NDIS_STATUS_SUCCESS;
				break;
			}

			/* polling */
			loop = 0;
			ret = NDIS_STATUS_FAILURE;
			do {
				fw_sync = MtAsicGetFwSyncValue(pAd);

				if (fw_sync == target_fw_sync) {
					ret = NDIS_STATUS_SUCCESS;
					break;
				}

				RtmpOsMsDelay(1);
				loop++;
			} while (loop <= WAIT_LOOP);
		}

		break;
	default:
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: invalid target stage\n", __func__));
		ret = NDIS_STATUS_FAILURE;
		break;
	}

	if (ret)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: fail, target stage = %d, current sync CR = %d \n", __func__, target_stage, fw_sync));

	return ret;
}


NDIS_STATUS mt_fwdl_hook_init(struct _RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret;
	struct fwdl_op *op = &pAd->MCUCtrl.fwdl_ctrl.fwdl_op;
	RTMP_CHIP_CAP *pChipCap = hc_get_chip_cap(pAd->hdev_ctrl);

	ret = NDIS_STATUS_SUCCESS;

	if (pChipCap->need_load_patch) {
		if (pChipCap->load_patch_flow == PATCH_FLOW_V1)
			op->load_patch = load_patch_v1;
		else
			ret = NDIS_STATUS_FAILURE;

		if (pChipCap->patch_format == PATCH_FORMAT_V1)
			op->parse_patch = parse_patch_v1;
		else
			ret = NDIS_STATUS_FAILURE;
	}

	if (pChipCap->need_load_fw) {
		if (pChipCap->load_fw_flow == FW_FLOW_V1)
			op->load_fw = load_fw_v1;
		else if (pChipCap->load_fw_flow == FW_FLOW_V2_COMPRESS_IMG)
			op->load_fw = load_fw_v2_compressimg; /* compress img must use v3 format*/
		else
			ret = NDIS_STATUS_FAILURE;

		if (pChipCap->fw_format == FW_FORMAT_V1)
			op->parse_fw = parse_fw_v1;
		else if (pChipCap->fw_format == FW_FORMAT_V2)
			op->parse_fw = parse_fw_v2;
		else if (pChipCap->fw_format == FW_FORMAT_V3)
			op->parse_fw = parse_fw_v3;
		else
			ret = NDIS_STATUS_FAILURE;
	}

		op->ctrl_fw_state = ctrl_fw_state_v2;
#ifdef CONFIG_RECOVERY_ON_INTERRUPT_MISS
#ifdef INTELP6_SUPPORT
		if (ret == NDIS_STATUS_FAILURE) {
			struct _RTMP_CHIP_OP *ops = hc_get_chip_ops(pAd->hdev_ctrl);

			pAd->ErrRecoveryCheck++;
			if (ops->heart_beat_check)
				ops->heart_beat_check(pAd);
		}
#endif
#endif
	if (ret)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: FWDL hook fail\n", __func__));

	return ret;
}

NDIS_STATUS mt_load_patch(struct _RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	RTMP_CHIP_OP *chip_op = hc_get_chip_ops(pAd->hdev_ctrl);
	struct fwdl_op *fwdl_op = &pAd->MCUCtrl.fwdl_ctrl.fwdl_op;
	struct patch_dl_target target;
	NDIS_STATUS ret;
	UINT32 i, need_load;

	ret = NDIS_STATUS_SUCCESS;

	if (chip_op->fwdl_datapath_setup)
		chip_op->fwdl_datapath_setup(pAd, TRUE);

	if (fwdl_op->ctrl_fw_state) {
		ret = fwdl_op->ctrl_fw_state(pAd, FWDL_STAGE_CMD_EVENT);
		if (ret)
			goto done;
	}

	need_load = chip_cap->need_load_patch;
	if (need_load && (!fwdl_op->parse_patch || !fwdl_op->load_patch)) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: no hook function available\n", __func__));
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	for (i = 0; i < MAX_CPU; i++) {
		if (need_load & BIT(i)) {
			ret = fwdl_op->parse_patch(pAd, i, &target);
			if (ret)
				goto done;
			ret = fwdl_op->load_patch(pAd, i, &target);
			if (ret)
				goto done;
		}
	}

done:

	if (chip_op->fwdl_datapath_setup)
		chip_op->fwdl_datapath_setup(pAd, FALSE);

	free_patch_buf(pAd);

	if (ret)
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: patch download fail\n", __func__));

	return ret;
}

NDIS_STATUS mt_load_fw(struct _RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *chip_cap = hc_get_chip_cap(pAd->hdev_ctrl);
	RTMP_CHIP_OP *chip_op = hc_get_chip_ops(pAd->hdev_ctrl);
	struct fwdl_op *fwdl_op = &pAd->MCUCtrl.fwdl_ctrl.fwdl_op;
	struct fw_dl_target target;
	NDIS_STATUS ret;
	UINT32 i, need_load;

	ret = NDIS_STATUS_SUCCESS;

	if (chip_op->fwdl_datapath_setup)
		chip_op->fwdl_datapath_setup(pAd, TRUE);

	if (fwdl_op->ctrl_fw_state) {
		ret = fwdl_op->ctrl_fw_state(pAd, FWDL_STAGE_CMD_EVENT);
		if (ret)
			goto done;
	}

	need_load = chip_cap->need_load_fw;
	if (need_load && (!fwdl_op->parse_fw || !fwdl_op->load_fw)) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: no hook function available\n", __func__));
		ret = NDIS_STATUS_FAILURE;
		goto done;
	}

	for (i = 0; i < MAX_CPU; i++) {
		if (need_load & BIT(i)) {
			ret = fwdl_op->parse_fw(pAd, i, &target);
			if (ret)
				goto done;
			ret = fwdl_op->load_fw(pAd, i, &target);
			if (ret)
				goto done;
		}
	}

	if (fwdl_op->ctrl_fw_state) {
		ret = fwdl_op->ctrl_fw_state(pAd, FWDL_STAGE_FW_RUNNING);
	}

done:

	if (chip_op->fwdl_datapath_setup)
		chip_op->fwdl_datapath_setup(pAd, FALSE);

	free_fw_buf(pAd);

	if (ret) {
		MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: fw download fail\n", __func__));
		pAd->MCUCtrl.fwdl_ctrl.stage = FWDL_STAGE_FW_NOT_DL;
	} else {
		pAd->MCUCtrl.fwdl_ctrl.stage = FWDL_STAGE_FW_RUNNING;
	}

	return ret;
}

NDIS_STATUS mt_restart_fw(struct _RTMP_ADAPTER *pAd)
{
	NDIS_STATUS ret;
	struct fwdl_op *fwdl_op = &pAd->MCUCtrl.fwdl_ctrl.fwdl_op;

	ret = NDIS_STATUS_SUCCESS;

	if (fwdl_op->ctrl_fw_state)
		ret = fwdl_op->ctrl_fw_state(pAd, FWDL_STAGE_FW_NOT_DL);

	return ret;
}

