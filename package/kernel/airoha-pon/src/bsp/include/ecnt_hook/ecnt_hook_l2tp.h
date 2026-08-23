/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/
#ifndef _ECNT_HOOK_L2TP_H
#define _ECNT_HOOK_L2TP_H

/************************************************************************
*               I N C L U D E S
*************************************************************************
*/
#include <ecnt_hook/ecnt_hook.h>



/************************************************************************
*               D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/


/************************************************************************
*               M A C R O S
*************************************************************************
*/
#ifndef uint8
#define uint8	unsigned char
#endif
		
#ifndef uint16
#define uint16	unsigned short
#endif
		
#ifndef uint32
#define uint32	unsigned int
#endif


/************************************************************************
*               D A T A   T Y P E S
*************************************************************************
*/
enum ECNT_L2TP_SUBTYPE
{
    ECNT_L2TP_RX_HOOK,
};

typedef struct
{
	struct sk_buff *skb;
}l2tp_data_s;

/************************************************************************
*               D A T A   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*               F U N C T I O N   D E C L A R A T I O N S
                I N L I N E  F U N C T I O N  D E F I N I T I O N S
*************************************************************************
*/


/*___________________________________________________________________________
**      function name: ecnt_l2tp_api_rx_handle
**      descriptions:
**      	handle l2tp pkt comes from half hwnat
**      input parameters:
**		skb: data flow
**      output parameters:
**      	N/A
**
**      return:
**      	success:	> 0
**		failure:	-1
**___________________________________________________________________________
*/
int ecnt_l2tp_api_rx_handle(struct sk_buff *skb)
{
	int ret = 0;
	l2tp_data_s in_data;
	
	in_data.skb = skb;
	
    ret = __ECNT_HOOK(ECNT_L2TP,ECNT_L2TP_RX_HOOK,(struct ecnt_data *)&in_data);

	return ret;
}

#endif /* _ECNT_HOOK_L2TP_H */
