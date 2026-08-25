/*
** $Id: tc3162.h,v 1.7 2011/01/07 06:05:58 pork Exp $
*/
/************************************************************************
 *
 *	Copyright (C) 2006 Trendchip Technologies, Corp.
 *	All Rights Reserved.
 *
 * Trendchip Confidential; Need to Know only.
 * Protected as an unpublished work.
 *
 * The computer program listings, specifications and documentation
 * herein are the property of Trendchip Technologies, Co. and shall
 * not be reproduced, copied, disclosed, or used in whole or in part
 * for any reason without the prior express written permission of
 * Trendchip Technologeis, Co.
 *
 *************************************************************************/
/*
** $Log: tc3162.h,v $
** Revision 1.7  2011/01/07 06:05:58  pork
** add the definition of INT!16,INT32,SINT15,SINT7
**
** Revision 1.6  2010/09/20 07:08:02  shnwind
** decrease nf_conntrack buffer size
**
** Revision 1.5  2010/09/03 16:43:07  here
** [Ehance] TC3182 GMAC Driver is support TC-Console & WAN2LAN function & update the tc3182 dmt version (3.12.8.83)
**
** Revision 1.4  2010/09/02 07:04:50  here
** [Ehance] Support TC3162U/TC3182 Auto-Bench
**
** Revision 1.3  2010/08/30 07:53:02  lino
** add power saving mode kernel module support
**
** Revision 1.2  2010/06/05 05:40:29  lino
** add tc3182 asic board support
**
** Revision 1.1.1.1  2010/04/09 09:39:21  feiyan
** New TC Linux Make Flow Trunk
**
** Revision 1.4  2010/01/14 10:56:42  shnwind
** recommit
**
** Revision 1.3  2010/01/14 08:00:10  shnwind
** add TC3182 support
**
** Revision 1.2  2010/01/10 15:27:26  here
** [Ehancement]TC3162U MAC EEE is operated at 100M-FD, SAR interface is accroding the SAR_CLK to calculate atm rate.
**
** Revision 1.1.1.1  2009/12/17 01:42:47  josephxu
** 20091217, from Hinchu ,with VoIP
**
** Revision 1.2  2006/07/06 07:24:57  lino
** update copyright year
**
** Revision 1.1.1.1  2005/11/02 05:45:38  lino
** no message
**
** Revision 1.5  2005/09/27 08:01:38  bread.hsu
** adding IMEM support for Tc3162L2
**
** Revision 1.4  2005/09/14 11:06:20  bread.hsu
** new definition for TC3162L2
**
** Revision 1.3  2005/06/17 16:26:16  jasonlin
** Remove redundant code to gain extra 100K bytes free memory.
** Add "CODE_REDUCTION" definition to switch
**
** Revision 1.2  2005/06/14 10:02:01  jasonlin
** Merge TC3162L2 source code into new main trunk
**
** Revision 1.1.1.1  2005/03/30 14:04:22  jasonlin
** Import Linos source code
**
** Revision 1.4  2004/11/15 03:43:17  lino
** rename ATM SAR max packet length register
**
** Revision 1.3  2004/09/01 13:15:47  lino
** fixed when pc shutdown, system will reboot
**
** Revision 1.2  2004/08/27 12:16:37  lino
** change SYS_HCLK to 96Mhz
**
** Revision 1.1  2004/07/02 08:03:04  lino
** tc3160 and tc3162 code merge
**
*/

#ifndef _PKGID_H_
#define _PKGID_H_
#include <common/ecnt_chip_id.h>
#include <modules/scu/ecnt_scu.h>

#ifdef TCSUPPORT_CPU_ARMV8
extern uint32_t GET_PACKAGE_ID(void);

/* AN7583 */
#define isAN7583FP		(isAN7583 && (GET_PACKAGE_ID() == (AN7583FP -AN7583_BASE)))
#define isAN7583FD		(isAN7583 && (GET_PACKAGE_ID() == (AN7583FD -AN7583_BASE)))
#define isAN7583FS		(isAN7583 && (GET_PACKAGE_ID() == (AN7583FS -AN7583_BASE)))
#define isAN7583FG		(isAN7583 && (GET_PACKAGE_ID() == (AN7583FG -AN7583_BASE)))
#define isAN7583FF		(isAN7583 && (GET_PACKAGE_ID() == (AN7583FF -AN7583_BASE)))
#define isAN7583GT		(isAN7583 && (GET_PACKAGE_ID() == (AN7583GT -AN7583_BASE)))
#define isAN7583GIT		(isAN7583 && (GET_PACKAGE_ID() == (AN7583GIT-AN7583_BASE)))
#define isAN7583CT		(isAN7583 && (GET_PACKAGE_ID() == (AN7583CT -AN7583_BASE)))
#define isAN7583DT		(isAN7583 && (GET_PACKAGE_ID() == (AN7583DT -AN7583_BASE)))
#define isAN7583ST		(isAN7583 && (GET_PACKAGE_ID() == (AN7583ST -AN7583_BASE)))
#define isAN9510GT		(isAN7583 && (GET_PACKAGE_ID() == (AN9510GT -AN7583_BASE)))
#define isAN7553GT		(isAN7583 && (GET_PACKAGE_ID() == (AN7553GT -AN7583_BASE)))
#define isAN7553CT		(isAN7583 && (GET_PACKAGE_ID() == (AN7553CT -AN7583_BASE)))
#define isAN7567GT		(isAN7583 && (GET_PACKAGE_ID() == (AN7567GT -AN7583_BASE)))
#define isAN7567CT		(isAN7583 && (GET_PACKAGE_ID() == (AN7567CT -AN7583_BASE)))
#define isAN7583ET		(isAN7583 && (GET_PACKAGE_ID() == (AN7583ET -AN7583_BASE)))
#define isAN7583EIT		(isAN7583 && (GET_PACKAGE_ID() == (AN7583EIT -AN7583_BASE)))

/* AN7552 */
#define isAN7552CT		(isAN7552 && (GET_PACKAGE_ID() == (AN7552CT-AN7552_BASE)))
#define isAN7552ST		(isAN7552 && (GET_PACKAGE_ID() == (AN7552ST-AN7552_BASE)))
#define isAN7552FT		(isAN7552 && (GET_PACKAGE_ID() == (AN7552FT-AN7552_BASE))) 
#define isAN7563CT		(isAN7552 && (GET_PACKAGE_ID() == (AN7563CT-AN7552_BASE)))
#define isAN7563PT		(isAN7552 && (GET_PACKAGE_ID() == (AN7563PT-AN7552_BASE)))


/* AN7581 */
#define isAN7581GT		(isEN7581 && (GET_PACKAGE_ID() == (AN7581GT-AN7581_BASE)))
#define isAN7566GT		(isEN7581 && (GET_PACKAGE_ID() == (AN7566GT-AN7581_BASE)))
#define isAN7581PT		(isEN7581 && (GET_PACKAGE_ID() == (AN7581PT-AN7581_BASE)))
#define isAN7581ST		(isEN7581 && (GET_PACKAGE_ID() == (AN7581ST-AN7581_BASE))) | isAN7581SIT
#define isAN7551PT		(isEN7581 && (GET_PACKAGE_ID() == (AN7551PT-AN7581_BASE)))      /* 2'b 0_0100 */
#define isAN7581CT		(isEN7581 && (GET_PACKAGE_ID() == (AN7581CT-AN7581_BASE)))
#define isAN7581DT		(isEN7581 && (GET_PACKAGE_ID() == (AN7581DT-AN7581_BASE)))
#define isAN7581FG		(isEN7581 && (GET_PACKAGE_ID() == (AN7581FG-AN7581_BASE)))
#define isAN7581FP		(isEN7581 && (GET_PACKAGE_ID() == (AN7581FP-AN7581_BASE)))      /* 2'b 0_1000 */
#define isAN7581FD		(isEN7581 && (GET_PACKAGE_ID() == (AN7581FD-AN7581_BASE)))
#define isAN7551GT		(isEN7581 && (GET_PACKAGE_ID() == (AN7551GT-AN7581_BASE)))
#define isAN7566PT		(isEN7581 && (GET_PACKAGE_ID() == (AN7566PT-AN7581_BASE)))
#define isAN7581IT		(isEN7581 && (GET_PACKAGE_ID() == (AN7581IT-AN7581_BASE)))
#define isAN7581SIT		(isEN7581 && (GET_PACKAGE_ID() == (AN7581SIT-AN7581_BASE)))


/* EN7523 */
#define isEN7529DU		(isEN7523 && (GET_PACKAGE_ID() == (EN7529DU-EN7523_BASE)))
#define isEN7529DT		(isEN7523 && (GET_PACKAGE_ID() == (EN7529DT-EN7523_BASE)))
#define isEN7529CU		(isEN7523 && (GET_PACKAGE_ID() == (EN7529CU-EN7523_BASE)))
#define isEN7562DU		(isEN7523 && (GET_PACKAGE_ID() == (EN7562DU-EN7523_BASE)))
#define isEN7562DT		(isEN7523 && (GET_PACKAGE_ID() == (EN7562DT-EN7523_BASE)))
#define isEN7562CU		(isEN7523 && (GET_PACKAGE_ID() == (EN7562CU-EN7523_BASE)))
#define isEN7523GU		(isEN7523 && (GET_PACKAGE_ID() == (EN7523GU-EN7523_BASE)))
#define isEN7523DU		(isEN7523 && (GET_PACKAGE_ID() == (EN7523DU-EN7523_BASE)))
#define isEN7529GTH		(isEN7523 && (GET_PACKAGE_ID() == (EN7529GTH-EN7523_BASE)))	/* 2'b 0_1000 */
#define isEN7562GTH		(isEN7523 && (GET_PACKAGE_ID() == (EN7562GTH-EN7523_BASE)))
#define isEN7523SU		(isEN7523 && (GET_PACKAGE_ID() == (EN7523SU-EN7523_BASE)))
#define isEN7529GTS		(isEN7523 && (GET_PACKAGE_ID() == (EN7529GTS-EN7523_BASE)))
#define isEN7562GTS		(isEN7523 && (GET_PACKAGE_ID() == (EN7562GTS-EN7523_BASE)))
#define isEN7529IT		(isEN7523 && (GET_PACKAGE_ID() == (EN7529IT-EN7523_BASE)))
#define isEN7529CT		(isEN7523 && (GET_PACKAGE_ID() == (EN7529CT-EN7523_BASE)))
#define isEN7562CT		(isEN7523 && (GET_PACKAGE_ID() == (EN7562CT-EN7523_BASE)))
#define isEN7523DT		(isEN7523 && (GET_PACKAGE_ID() == (EN7523DT-EN7523_BASE)))	/* 2'b 1_0000 */
#define isEN7529DTM		(isEN7523 && (GET_PACKAGE_ID() == (EN7529DTM-EN7523_BASE)))
#define isEN7562DTM		(isEN7523 && (GET_PACKAGE_ID() == (EN7562DTM-EN7523_BASE)))
#define isEN7529ITM		(isEN7523 && (GET_PACKAGE_ID() == (EN7529ITM-EN7523_BASE)))
#define isEN7529CTM		(isEN7523 && (GET_PACKAGE_ID() == (EN7529CTM-EN7523_BASE)))
#define isEN7562CTM		(isEN7523 && (GET_PACKAGE_ID() == (EN7562CTM-EN7523_BASE)))
#define isEN7523DTM		(isEN7523 && (GET_PACKAGE_ID() == (EN7523DTM-EN7523_BASE)))

#else
	
#define isAN7583GT		(0)
#define isAN7583GIT		(0)
#define isAN7583CT		(0)
#define isAN7583DT		(0)
#define isAN7583ST		(0)
#define isAN9510GT		(0)
#define isAN7553GT		(0)
#define isAN7553CT		(0)
#define isAN7567GT		(0)
#define isAN7567CT		(0)
#define isAN7583ET		(0)
#define isAN7583EIT		(0)
#define isAN7583FG		(0)
#define isAN7583FP		(0)
#define isAN7583FD		(0)
#define isAN7583FS		(0)
#define isAN7583FF		(0)


/* AN7552 */
#define isAN7552CT		(0)
#define isAN7552ST		(0)
#define isAN7563CT		(0)
#define isAN7563PT		(0)
#define isAN7552FT		(0)

/* AN7581 */
#define isAN7581GT		(0)
#define isAN7566GT		(0)
#define isAN7581PT		(0)
#define isAN7581ST		(0)
#define isAN7551PT		(0)
#define isAN7581CT		(0)
#define isAN7581DT		(0)
#define isAN7581FG		(0)
#define isAN7581FP		(0)
#define isAN7581FD		(0)
#define isAN7551GT		(0)
#define isAN7566PT		(0)
#define isAN7581IT		(0)
#define isAN7581SIT		(0)


/* EN7523 */
#define isEN7529DU		(0)
#define isEN7529DT		(0)
#define isEN7529CU		(0)
#define isEN7562DU		(0)
#define isEN7562DT		(0)
#define isEN7562CU		(0)
#define isEN7523GU		(0)
#define isEN7523DU		(0)
#define isEN7529GTH		(0)
#define isEN7562GTH		(0)
#define isEN7523SU		(0)
#define isEN7529GTS		(0)
#define isEN7562GTS		(0)
#define isEN7529IT		(0)
#define isEN7529CT		(0)
#define isEN7562CT		(0)
#define isEN7523DT		(0)
#define isEN7529DTM		(0)
#define isEN7562DTM		(0)
#define isEN7529ITM		(0)
#define isEN7529CTM		(0)
#define isEN7562CTM		(0)
#define isEN7523DTM		(0)

#endif

#endif /* _PKGID_H_ */
