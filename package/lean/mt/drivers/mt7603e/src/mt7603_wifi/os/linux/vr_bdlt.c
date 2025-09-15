/****************************************************************************

    Module Name:
    vr_brlt.c
 
    Abstract:
    Only for BroadLight 2348 platform.
 
    Revision History:
    Who        When          What
    ---------  ----------    ----------------------------------------------
    Sample Lin	01-12-2010    Created

***************************************************************************/

#define RTMP_MODULE_OS
#define RTMP_MODULE_OS_UTIL

#define MODULE_BDLT

/*#include "rt_config.h" */
#include "rtmp_comm.h"
#include "rt_os_util.h"
#include "rtmp_osabl.h"


#if defined(PLATFORM_BL2348) || defined(PLATFORM_BL23570)

/* global variables */
int (*pToUpperLayerPktSent)(struct sk_buff *pSkb) = netif_rx ;




/*
========================================================================
Routine Description:
	Assign the briding function.

Arguments:
	xi_destination_ptr	- bridging function

Return Value:
	None

Note:
	The function name must be replace_upper_layer_packet_destination.
========================================================================
*/
VOID replace_upper_layer_packet_destination(VOID *pXiDestination)
{
	DBGPRINT(RT_DEBUG_TRACE, ("ralink broad light> replace_upper_layer_packet_destination\n"));
	pToUpperLayerPktSent = pXiDestination ;
} /* End of replace_upper_layer_packet_destination */


EXPORT_SYMBOL(pToUpperLayerPktSent);
EXPORT_SYMBOL(replace_upper_layer_packet_destination);

#endif /* PLATFORM_BL2348 */


/* End of vr_bdlt.c */
