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
	mtusb_io.c
*/

#include	"rt_config.h"

INT32 MTSDIORead32(RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 *Value)
{

	INT32 Ret = 0;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;

	sdio_claim_host(dev_func);
	*Value = sdio_readl(dev_func, Offset, &Ret);
	sdio_release_host(dev_func);

	if (Ret) {
		DBGPRINT(RT_DEBUG_ERROR, ("RTSDIORead32 failure!\n"));
	}

	return Ret;   
}


INT32 MTSDIOWrite32(RTMP_ADAPTER *pAd, UINT32 Offset, UINT32 Value)
{
	INT32 Ret = 0;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;

	sdio_claim_host(dev_func);
	sdio_writel(dev_func, Value, Offset, &Ret);
	sdio_release_host(dev_func);

	return Ret;
}


INT32 MTSDIOMultiRead(RTMP_ADAPTER *pAd, UINT32 Offset, 
										UCHAR *Buf,UINT32 Length)
{
	UINT32 Count = Length;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;
	UINT32 bNum = 0;
	INT32 Ret;

	while (Count >= pAd->BlockSize) {
		Count -= pAd->BlockSize;
		bNum++;
	}

	if (Count > 0 && bNum > 0) {
		bNum++;
	}
   
	sdio_claim_host(dev_func);
	
	if (bNum > 0) {
		Ret = sdio_readsb(dev_func, Buf, Offset, pAd->BlockSize * bNum);
	}
	else 
	{
		Ret = sdio_readsb(dev_func, Buf, Offset, Count);
	}

	sdio_release_host(dev_func);

	return Ret;
}


INT32 MTSDIOMultiWrite(RTMP_ADAPTER *pAd, UINT32 Offset, 
										UCHAR *Buf,UINT32 Length)
{

	UINT32 Count = Length;
	void* handle = pAd->OS_Cookie;
	struct sdio_func *dev_func = ((POS_COOKIE)handle)->sdio_dev;
	INT32 Ret = 0;
	UINT32 bNum = 0;

	while (Count >= pAd->BlockSize) {
		Count -= pAd->BlockSize;
		bNum++;
	}
   
	if (Count > 0 && bNum > 0) {
		bNum++;
	}
	
	sdio_claim_host(dev_func);

	if (bNum > 0) 
	{
    	Ret = sdio_writesb(dev_func, Offset, Buf, pAd->BlockSize * bNum);
	}
	else 
	{
		Ret = sdio_writesb(dev_func, Offset, Buf, Count);
	}

	sdio_release_host(dev_func);

	return Ret;
}


