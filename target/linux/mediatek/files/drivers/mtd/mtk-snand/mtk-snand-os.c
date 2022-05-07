// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2020 MediaTek Inc. All Rights Reserved.
 *
 * Author: Weijie Gao <weijie.gao@mediatek.com>
 */

#include "mtk-snand-def.h"

int mtk_snand_log(struct mtk_snand_plat_dev *pdev,
		  enum mtk_snand_log_category cat, const char *fmt, ...)
{
	const char *catname = "";
	va_list ap;
	char *msg;

	switch (cat) {
	case SNAND_LOG_NFI:
		catname = "NFI";
		break;
	case SNAND_LOG_SNFI:
		catname = "SNFI";
		break;
	case SNAND_LOG_ECC:
		catname = "ECC";
		break;
	default:
		break;
	}

	va_start(ap, fmt);
	msg = kvasprintf(GFP_KERNEL, fmt, ap);
	va_end(ap);

	if (!msg) {
		dev_warn(pdev->dev, "unable to print log\n");
		return -1;
	}

	if (*catname)
		dev_warn(pdev->dev, "%s: %s", catname, msg);
	else
		dev_warn(pdev->dev, "%s", msg);

	kfree(msg);

	return 0;
}
