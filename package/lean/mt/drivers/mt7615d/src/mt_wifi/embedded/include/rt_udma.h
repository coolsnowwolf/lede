/****************************************************************************

    Module Name:
	rt_udma.h

	Abstract:
	For Puma6 UDMA implementation.

    Revision History:
    Who          When          What
    ---------    ----------    ----------------------------------------------
	Anjan Chanda	05-05-2015		Initial creation

***************************************************************************/

#ifndef __RT_UDMA_H__
#define __RT_UDMA_H__

int mt_udma_pkt_send(VOID *ctx, PNDIS_PACKET pRxPkt);

int mt_udma_register(VOID *pAd, PNET_DEV main_dev_p);
int mt_udma_unregister(VOID *pAd);


#endif /* __RT_UDMA_H__ */
