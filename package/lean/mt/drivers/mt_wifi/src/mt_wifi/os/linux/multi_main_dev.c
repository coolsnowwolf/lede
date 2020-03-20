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

	Abstract:

	Revision History:
	Who		When			What
	--------	----------		----------------------------------------------
*/


#ifdef MULTI_INF_SUPPORT

#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rt_os_net.h"
#include <linux/pci.h>

/* Index 0 for Card_1, Index 1 for Card_2 */
VOID *adapt_list[MAX_NUM_OF_INF] = {NULL};

int multi_inf_adapt_reg(VOID *pAd)
{
	int status = 0;

	if (adapt_list[0] == NULL)
		adapt_list[0] = pAd;
	else if (adapt_list[1] == NULL)
		adapt_list[1] = pAd;
	else if (adapt_list[2] == NULL)
		adapt_list[2] = pAd;
	else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): adapt_list assign error !\n", __func__));
		status = NDIS_STATUS_FAILURE;
	}

	return status;
}

int multi_inf_adapt_unreg(VOID *pAd)
{
	int status = 0;

	if (adapt_list[0] == pAd)
		adapt_list[0] = NULL;
	else if (adapt_list[1] == pAd)
		adapt_list[1] = NULL;
	else if (adapt_list[2] == pAd)
		adapt_list[2] = NULL;
	else {
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("%s(): adapt_list assign error !\n", __func__));
		status = NDIS_STATUS_FAILURE;
	}

	return status;
}


int multi_inf_get_idx(VOID *pAd)
{
	int idx = 0; /* use index 0 as default */

	for (idx = 0; idx < MAX_NUM_OF_INF; idx++) {
		if (pAd == adapt_list[idx])
			return idx;
	}

	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("%s(): failed to find the index in adapt_list!\n", __func__));
	return idx;
}
EXPORT_SYMBOL(multi_inf_get_idx);

/* Driver module load/unload function */
static int __init wifi_drv_init_module(void)
{
	int status = 0;

#ifdef RTMP_RBUS_SUPPORT
	status = wbsys_module_init();

	if (status)
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Register RBUS device driver failed(%d)!\n", status));

#endif /* RTMP_RBUS_SUPPORT */

#ifdef RTMP_PCI_SUPPORT
	status = rt_pci_init_module();

	if (status)
		MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				 ("Register PCI device driver failed(%d)!\n", status));

#endif /* RTMP_PCI_SUPPORT */

	/* Add out-of-memory notifier */

	return status;
}


static void __exit wifi_drv_cleanup_module(void)
{
	/* Del out-of-memory notifier */

#ifdef RTMP_PCI_SUPPORT
	rt_pci_cleanup_module();
	MTWF_LOG(DBG_CAT_INIT, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			 ("Unregister PCI device driver\n"));
#endif /* RTMP_PCI_SUPPORT */
#ifdef RTMP_RBUS_SUPPORT
	wbsys_module_exit();
#endif /* RTMP_RBUS_SUPPORT */
}


module_init(wifi_drv_init_module);
module_exit(wifi_drv_cleanup_module);

#endif /* MULTI_INF_SUPPORT */

