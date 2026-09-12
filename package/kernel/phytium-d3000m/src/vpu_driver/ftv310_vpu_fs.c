// SPDX-License-Identifier: GPL-2.0
/*
 *    ftv310 driver DMA_BUF fence operation.
 *
 *    Copyright (C) 2024-2025, Phytium Technology Co., Ltd.
 *
 *    This program is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU General Public License
 *    as published by the Free Software Foundation; either version 2
 *    of the License, or (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You may obtain a copy of the GNU General Public License
 *    Version 2 or later at the following locations:
 *    http://www.opensource.org/licenses/gpl-license.html
 *    http://www.gnu.org/copyleft/gpl.html
 */
#include "ftv310_vpu_priv.h"
#include "ftv310_vpu_dec.h"
#include "ftv310e.h"

static ssize_t bandwidthDecRead_show(struct device *kdev,
				     struct device_attribute *attr, char *buf)
{
	/* sys/bus/platform/drivers/ftv310_vpu/xxxxx.vpu/bandwidthDecRead
	 *  used to show bandwidth info to user space
	 *  all core' bandwidth might be exported in same string.
	 *  data is just an example. Real data should be read from HW registers
	 *  this file is read only.
	 */
	u32 bandwidth;
	int sliceidx = findslice_bydev(kdev);

	if (sliceidx < 0)
		return 0;
#ifdef HAS_VCD
	bandwidth = ftv310_vpu_dec_readbandwidth(sliceidx, 1);
#endif
	return snprintf(buf, PAGE_SIZE, "%u\n", bandwidth);
}

static ssize_t bandwidthDecWrite_show(struct device *kdev,
				      struct device_attribute *attr, char *buf)
{
	u32 bandwidth;
	int sliceidx = findslice_bydev(kdev);

	if (sliceidx < 0)
		return 0;
#ifdef HAS_VCD
	bandwidth = ftv310_vpu_dec_readbandwidth(sliceidx, 0);
#endif
	return snprintf(buf, PAGE_SIZE, "%u\n", bandwidth);
}

static ssize_t bandwidthEncRead_show(struct device *kdev,
				     struct device_attribute *attr, char *buf)
{
	u32 bandwidth;
	int sliceidx = findslice_bydev(kdev);

	if (sliceidx < 0)
		return 0;
#ifdef HAS_VCE
	bandwidth = ftv310_vpu_enc_readbandwidth(sliceidx, 1);
#endif
	return snprintf(buf, PAGE_SIZE, "%u\n", bandwidth);
}

static ssize_t bandwidthEncWrite_show(struct device *kdev,
				      struct device_attribute *attr, char *buf)
{
	u32 bandwidth;
	int sliceidx = findslice_bydev(kdev);

	if (sliceidx < 0)
		return 0;
#ifdef HAS_VCE
	bandwidth = ftv310_vpu_enc_readbandwidth(sliceidx, 0);
#endif
	return snprintf(buf, PAGE_SIZE, "%u\n", bandwidth);
}

int ftv310_vpu_createsysfsAPI(int sliceidx, struct device *dev)
{
	int result;
	char APIname[16] = { 0 };
	struct device_attribute APIattr = {
		.attr = {
				.mode = 0444,
			},
		.store = NULL,
	};
	memcpy(APIname, "BWDecRead", 9);
	APIname[9] = sliceidx + 0x30;
	APIattr.attr.name = APIname;
	APIattr.show = bandwidthDecRead_show;
	result = device_create_file(dev, &APIattr);
	if (result != 0)
		return result;

	memcpy(APIname, "BWEncRead", 9);
	APIname[9] = sliceidx + 0x30;
	APIattr.attr.name = APIname;
	APIattr.show = bandwidthEncRead_show;
	result = device_create_file(dev, &APIattr);
	if (result != 0)
		return result;

	memcpy(APIname, "BWDecWrite", 10);
	APIname[10] = sliceidx + 0x30;
	APIattr.attr.name = APIname;
	APIattr.show = bandwidthDecWrite_show;
	result = device_create_file(dev, &APIattr);
	if (result != 0)
		return result;

	memcpy(APIname, "BWEncWrite", 10);
	APIname[10] = sliceidx + 0x30;
	APIattr.attr.name = APIname;
	APIattr.show = bandwidthEncWrite_show;
	result = device_create_file(dev, &APIattr);

	return result;
}

int ftv310_vpu_unlinksysfsAPI(void)
{
	int i;
	char APIname[16] = { 0 };
	struct device_attribute APIattr = {
		.attr = {
				.mode = 0444,
			},
		.store = NULL,
	};

	for (i = 0; i < get_slicenumber(); i++) {
		struct slice_info *pslice = getslicenode(i);

		memcpy(APIname, "BWDecRead", 9);
		APIname[9] = i + 0x30;
		APIattr.attr.name = APIname;
		APIattr.show = bandwidthDecRead_show;
		device_remove_file(pslice->dev, &APIattr);

		memcpy(APIname, "BWEncRead", 9);
		APIname[9] = i + 0x30;
		APIattr.attr.name = APIname;
		APIattr.show = bandwidthEncRead_show;
		device_remove_file(pslice->dev, &APIattr);

		memcpy(APIname, "BWDecWrite", 10);
		APIname[10] = i + 0x30;
		APIattr.attr.name = APIname;
		APIattr.show = bandwidthDecWrite_show;
		device_remove_file(pslice->dev, &APIattr);

		memcpy(APIname, "BWEncWrite", 10);
		APIname[10] = i + 0x30;
		APIattr.attr.name = APIname;
		APIattr.show = bandwidthEncWrite_show;
		device_remove_file(pslice->dev, &APIattr);
	}
	return 0;
}
