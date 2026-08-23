/***************************************************************
Copyright Statement:

This software/firmware and related documentation (EcoNet Software) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (EcoNet) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (ECONET SOFTWARE) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN AS IS 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVERS SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVERS SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

#ifndef _UAPI_PROLINECMD_H
#define _UAPI_PROLINECMD_H

/**
* \file  prolinecmd.h 
* \brief This file is prolinecmd header file that will be exported for other use.
* \author ECONET
* \date     2020-12-02
* \version  A001 
* \copyright EcoNet Inc                                                              
*/


#ifdef TCSUPPORT_PRODUCTIONLINE
#ifndef __KERNEL__
#include    <stddef.h>
#endif

//#include    "flash_layout/tc_partition.h"

#define SUCCESS 0
#define FAIL -1

/*common struct*/
typedef	struct _operPara{
	int 	flashsize;		//cover size of flash size
	int 	flashoffset;		//offset of base address of current zone
	char zonename[16];	//name of zone
	char readfile[64];		//mtd cmd used
	char writefile[64];		//mtd cmd used
}operPara;

#define PL_CP 0	
typedef struct _proLineFlag{
	int type;

	int flag;	//according to types
	struct _operPara para;
} proLineFlag;

/*
PL:product line
CP:cwmp parameter
*/

#define WEBPWDLEN 16
#define WEBACCOUNTLEN 16
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CHS)
#define HWSNLEN 64
#else/*TCSUPPORT_COMPILE*/
#define HWSNLEN 0
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_RESERVEAREA_EXTEND)
enum PL_CP_Flags {
    PL_CP_PRODUCTCLASS=0,
    PL_CP_MANUFACUREROUI,
    PL_CP_SERIALNUM,
    PL_CP_SSID,
    PL_CP_WPAKEY,
    PL_CP_WEBPWD,
    PL_CP_PPPUSRNAME,
    PL_CP_PPPPWD,
    PL_CP_CFEUSRNAME,
    PL_CP_CFEPWD,
    PL_CP_XPONSN,
    PL_CP_XPONPWD,
    PL_CP_MACADDR,
    PL_CP_TELNET,
    PL_FON_KEYWORD,
    PL_FON_MAC,
    PL_CP_BARCODE,
    PL_CP_XPONMODE,
    PL_CP_SSID2nd,
    PL_CP_WPAKEY2nd,
    PL_CP_WEB_ACCOUNT,
    PL_CP_HW_VER,
    PL_CP_ROMFILE_SELECT,
    PL_CP_MT7570BOB,
    PL_CP_SSIDAC,
    PL_CP_WPAKEYAC,
    PL_CP_SSID2ndAC,
    PL_CP_WPAKEY2ndAC,
    PL_GPON_REGID,
#if defined(TCSUPPORT_CT_UBUS)
	PL_CP_CTEI,
	PL_CP_PROVINCE,
#endif
#if defined(TCSUPPORT_ECNT_MAP)
    PL_CP_MAPDEVROLE,
#endif
    PL_CP_END
};

#define FLAG_TELNET_DISABLE "DIS"
#define FLAG_TELNET_ENABLE "ENB"

#define DEFAULT_CFE_USERNAME "telecomadmin"
#define DEFAULT_CFE_PWD "nE7jA%5m"

#if defined(TCSUPPORT_ECNT_MAP)
#define   PL_CP_MAPBHSSID2G      PL_CP_MACADDR   
#define   PL_CP_MAPBHWPAKEY2G    PL_FON_KEYWORD
#define   PL_CP_MAPBHSSID5G      PL_FON_MAC
#define   PL_CP_MAPBHWPAKEY5G    PL_CP_BARCODE
#endif
#define PL_CP_PRODUCTCLASS_FLAG (1<<PL_CP_PRODUCTCLASS)
#define PL_CP_MANUFACUREROUI_FLAG (1<<PL_CP_MANUFACUREROUI)
#define PL_CP_SERIALNUM_FLAG (1<<PL_CP_SERIALNUM)
#define PL_CP_SSID_FLAG (1<<PL_CP_SSID)
#define PL_CP_WPAKEY_FLAG (1<<PL_CP_WPAKEY)
#define PL_CP_WEBPWD_FLAG (1<<PL_CP_WEBPWD)
#define PL_CP_PPPUSRNAME_FLAG (1<<PL_CP_PPPUSRNAME)
#define PL_CP_PPPPWD_FLAG (1<<PL_CP_PPPPWD)
#define PL_CP_CFEUSRNAME_FLAG (1<<PL_CP_CFEUSRNAME)
#define PL_CP_CFEPWD_FLAG (1<<PL_CP_CFEPWD)
#define PL_CP_XPONSN_FLAG (1<<PL_CP_XPONSN)
#define PL_CP_XPONPWD_FLAG (1<<PL_CP_XPONPWD)
#if !defined(TCSUPPORT_ECNT_MAP)
#define PL_CP_MACADDR_FLAG (1<<PL_CP_MACADDR)
#define PL_FON_KEYWORD_FLAG (1<<PL_FON_KEYWORD)
#define PL_FON_MAC_FLAG (1<<PL_FON_MAC)
#define PL_CP_BARCODE_FLAG (1<<PL_CP_BARCODE)
#else
#define PL_CP_MAPDEVROLE_FLAG (1<<PL_CP_MAPDEVROLE)
#define PL_CP_MAPBHSSID2G_FLAG (1<<PL_CP_MAPBHSSID2G)
#define PL_CP_MAPBHWPAKEY2G_FLAG (1<<PL_CP_MAPBHWPAKEY2G)
#define PL_CP_MAPBHSSID5G_FLAG (1<<PL_CP_MAPBHSSID5G)
#define PL_CP_MAPBHWPAKEY5G_FLAG (1<<PL_CP_MAPBHWPAKEY5G)
#endif
#define PL_CP_XPONMODE_FLAG (1<<PL_CP_XPONMODE)
#define PL_CP_TELNET_FLAG (1<<PL_CP_TELNET)
#define PL_CP_SSID2nd_FLAG (1<<PL_CP_SSID2nd)
#define PL_CP_WPAKEY2nd_FLAG (1<<PL_CP_WPAKEY2nd)
#define PL_CP_WEB_ACCOUNT_FLAG (1<<PL_CP_WEB_ACCOUNT)
#define PL_CP_HW_VER_FLAG	(1<<PL_CP_HW_VER)
#define PL_CP_ROMFILE_SELECT_FLAG	(1<<PL_CP_ROMFILE_SELECT)
#define PL_CP_MT7570BOB_FLAG	(1<<PL_CP_MT7570BOB)
#define PL_CP_SSIDAC_FLAG (1<<PL_CP_SSIDAC)
#define PL_CP_WPAKEYAC_FLAG (1<<PL_CP_WPAKEYAC)
#define PL_CP_SSID2ndAC_FLAG (1<<PL_CP_SSID2ndAC)
#define PL_CP_WPAKEY2ndAC_FLAG (1<<PL_CP_WPAKEY2ndAC)
#define PL_GPON_REGID_FLAG (1<<PL_GPON_REGID)
#if defined(TCSUPPORT_CT_UBUS)
#define PL_CP_CTEI_FLAG (1<<PL_CP_CTEI)
#define PL_CP_PROVINCE_FLAG (1<<PL_CP_PROVINCE)
#endif

#define PRDDUCTCLASSLEN 64
#define MANUFACUREROUILEN 64
#define SERIALNUMLEN 128
#define SSIDLEN 32
#define WPAKEYLEN 64
#define PPPUSRNAMELEN 64
#define PPPPWDLEN 64
#define CFEUSRNAMELEN 64
#define CFEPWDLEN 64
#define XPONSNLEN 32
#define XPONPWDLEN 80
#define MACADDRLEN 32
#define XPONMODELEN	8
#define FONKEYWORDLEN 65
#define FONMACLEN 18
#define BARCODELEN	32
#define TELNETLEN 4
#define HWVERLEN 64
#define ROMFILESELECTLEN 64
#define MT7570BOBLEN 225
#define GPONREGIDLEN 36
#if defined(TCSUPPORT_CT_UBUS)
#define CTEILEN 16
#define PROVINCELEN 64
#endif
#if defined(TCSUPPORT_ECNT_MAP)
#define MAPDEVROLELEN 4
#endif

typedef struct _proline_Para{
	int flag;
	int magic;
	char telnet[TELNETLEN];
	char barcode[BARCODELEN];
	char productclass[PRDDUCTCLASSLEN];
	char manufacturerOUI[MANUFACUREROUILEN];
	char serialnum[SERIALNUMLEN];
	char ssid[SSIDLEN];
	char wpakey[WPAKEYLEN];
	char webpwd[WEBPWDLEN];
	char pppusrname[PPPUSRNAMELEN];
	char ppppwd[PPPPWDLEN];
	char cfeusrname[CFEUSRNAMELEN];
	char cfepwd[CFEPWDLEN];
	char xponsn[XPONSNLEN];
	char xponpwd[XPONPWDLEN];
	char macaddr[MACADDRLEN];
	char xponmode[XPONMODELEN];
	char fonKeyword[FONKEYWORDLEN];
	char fonMac[FONMACLEN];
	char ssid2nd[SSIDLEN];
	char wpakey2nd[WPAKEYLEN];
	char webAcct[WEBACCOUNTLEN];
	char hwver[HWVERLEN];
	char romfileselect[ROMFILESELECTLEN];
	char mt7570bob[MT7570BOBLEN];
	char ssidac[SSIDLEN];
	char wpakeyac[WPAKEYLEN];
	char ssid2ndac[SSIDLEN];
	char wpakey2ndac[WPAKEYLEN];
	char gponregid[XPONPWDLEN];
#if defined(TCSUPPORT_ECNT_MAP)
	char mapdevrole[MAPDEVROLELEN];
	char mapbhssid2g[SSIDLEN];
	char mapbhwpakey2g[WPAKEYLEN];
	char mapbhssid5g[SSIDLEN];
	char mapbhwpakey5g[WPAKEYLEN];	
#endif
#if defined(TCSUPPORT_CT_UBUS)
	char ctei[CTEILEN];
	char province[PROVINCELEN];
#if defined(TCSUPPORT_ECNT_MAP)
	char reserve[203]; 
#else
	char reserve[399]; 
#endif
#else
#if defined(TCSUPPORT_ECNT_MAP)
	char reserve[283]; 
#else
	char reserve[479]; 
#endif
#endif
} proline_Para;

#undef BOB_RA_OFFSET
#define BOB_RA_OFFSET (PROLINE_CWMPPARA_RA_OFFSET + ((size_t) &((proline_Para *)0)->mt7570bob))

#elif defined(TCSUPPORT_CY)
#define PL_CP_PRODUCTCLASS 0
#define PL_CP_MANUFACUREROUI 1
#define PL_CP_SERIALNUM 2
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_DATE)
#define PL_PROG_DATE 3
#define PL_MFT_DATE 4
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_CONTENT)
#define PL_PRODUCT_CONTENT 5
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_FON)
#define PL_FON_KEYWORD 6
#define PL_FON_MAC 7
#endif
#define	PL_CP_BARCODE	8

#define PL_CP_PRODUCTCLASS_FLAG (1<<0)
#define PL_CP_MANUFACUREROUI_FLAG (1<<1)
#define PL_CP_SERIALNUM_FLAG (1<<2)
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_DATE)
#define PL_PROG_DATE_FLAG (1<<3)
#define PL_MFT_DATE_FLAG (1<<4)
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_CONTENT)
#define PL_PRODUCT_CONTENT_FLAG (1<<5)
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_FON)
#define PL_FON_KEYWORD_FLAG (1<<6)
#define PL_FON_MAC_FLAG (1<<7)
#endif
#define PL_CP_BARCODE_FLAG	(1<<8)

#define PRDDUCTCLASSLEN 64
#define MANUFACUREROUILEN 64
#define SERIALNUMLEN 128

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_CONTENT)
#define PRODUCTCONTENTLEN 129
#else/*TCSUPPORT_COMPILE*/
#define PRODUCTCONTENTLEN 0
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_DATE)
#define PROGDATELEN 32
#define MFTDATELEN 32
#else/*TCSUPPORT_COMPILE*/
#define PROGDATELEN 0
#define MFTDATELEN 0
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_FON)
#define FONKEYWORDLEN 65
#define FONMACLEN 18
#else
#define FONKEYWORDLEN 0
#define FONMACLEN 0
#endif
#define BARCODELEN	32
#define RESERVELEN (PROLINE_CWMPPARA_RA_SIZE-PRDDUCTCLASSLEN-MANUFACUREROUILEN-SERIALNUMLEN-PROGDATELEN-MFTDATELEN-PRODUCTCONTENTLEN-FONKEYWORDLEN-FONMACLEN-BARCODELEN-2*sizeof(int))

typedef struct _proline_cwmpPara{
	int flag;
	char productclass[PRDDUCTCLASSLEN];
	char manufacturerOUI[MANUFACUREROUILEN];
	char serialnum[SERIALNUMLEN];
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_DATE)
	char progDate[PROGDATELEN];
	char mftDate[MFTDATELEN];
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_CONTENT)
	char productContent[PRODUCTCONTENTLEN];
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_FON)
	char fonKeyword[FONKEYWORDLEN];
	char fonMac[FONMACLEN];
#endif
	char barcode[BARCODELEN];
	char reserve[RESERVELEN];
	int magic;
} proline_cwmpPara;
typedef struct _proline_cwmpPara proline_Para;

#else // #ifdef TCSUPPORT_RESERVEAREA_EXTEND
#define PL_CP_PRODUCTCLASS 0
#define PL_CP_MANUFACUREROUI 1
#define PL_CP_SERIALNUM 2
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_DATE)
#define PL_PROG_DATE 3
#define PL_MFT_DATE 4
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_CONTENT)
#define PL_PRODUCT_CONTENT 5
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_FON) || defined(TCSUPPORT_FON_V2)
#define PL_FON_KEYWORD 6
#define PL_FON_MAC 7
#endif
#if defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON)
#define PL_PON_MACNUM  8
#define PL_PON_USERADMIN_PASSWD 9
#if defined(TCSUPPORT_WAN_GPON)
#define PL_GPON_SN  10
#define PL_GPON_PASSWD  11
#endif
#endif
#define PL_CP_WEBPWD 12
#define PL_CP_WEB_ACCOUNT 13
#define PL_CP_TELNET 14
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CHS)
#define PL_HW_SN 15
#endif/*TCSUPPORT_COMPILE*/
#define PL_CP_BOOTINFO 17
#define PL_CP_END 18


#define PL_CP_PRODUCTCLASS_FLAG (1<<0)
#define PL_CP_MANUFACUREROUI_FLAG (1<<1)
#define PL_CP_SERIALNUM_FLAG (1<<2)
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_DATE)
#define PL_PROG_DATE_FLAG (1<<3)
#define PL_MFT_DATE_FLAG (1<<4)
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_CONTENT)
#define PL_PRODUCT_CONTENT_FLAG (1<<5)
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_FON) || defined(TCSUPPORT_FON_V2)
#define PL_FON_KEYWORD_FLAG (1<<6)
#define PL_FON_MAC_FLAG (1<<7)
#endif
#if defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON)
#define PL_PON_MACNUM_FLAG  (1<<8)
#define PL_PON_USERADMIN_PASSWD_FLAG (1<<9)
#if defined(TCSUPPORT_WAN_GPON)
#define PL_GPON_SN_FLAG     (1<<10)
#define PL_GPON_PASSWD_FLAG (1<<11)
#endif
#endif

#define PL_CP_WEBPWD_FLAG      (1<<12)
#define PL_CP_WEB_ACCOUNT_FLAG (1<<13)
#define PL_CP_TELNET_FLAG (1<<14)
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CHS)
#define PL_HW_SN_FLAG (1<<15)
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_WAN_GPON)
#define PL_GPON_EXTPWD_FLAG (1<<16)
#endif

#define PL_CP_BOOTINFO_FLAG (1<<17)

/****************************************
 * NEED TO PAY ATTENTION:
 * For upgrade on using network device, the prolinecmd
 * area need be reserved for feature. 
 * So the General Feature must not use compile.
 * When area is not enough, extend it.
 ***************************************/
#define PRDDUCTCLASSLEN 64
#define MANUFACUREROUILEN 64
#define SERIALNUMLEN 128

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_CONTENT)
#define PRODUCTCONTENTLEN 129
#else/*TCSUPPORT_COMPILE*/
#define PRODUCTCONTENTLEN 0
#endif/*TCSUPPORT_COMPILE*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_DATE)
#define PROGDATELEN 32
#define MFTDATELEN 32
#else/*TCSUPPORT_COMPILE*/
#define PROGDATELEN 0
#define MFTDATELEN 0
#endif/*TCSUPPORT_COMPILE*/

#if defined(TCSUPPORT_FON) || defined(TCSUPPORT_FON_V2)
#define FONKEYWORDLEN 65
#define FONMACLEN 18
#else
#define FONKEYWORDLEN 0
#define FONMACLEN 0
#endif
#if defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON)
#define PONMACNUMLEN  3
#define PONUSERADMINPASSWDLEN 32
#if defined(TCSUPPORT_WAN_GPON)
#define GPONSNLEN  13
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_C1_ZY)
#define GPONPASSWDLEN 11
#define GPONEXTENDPWDLEN 15
#else/*TCSUPPORT_COMPILE*/
#define GPONPASSWDLEN 9
#define GPONEXTENDPWDLEN 15
#endif/*TCSUPPORT_COMPILE*/
#else
#define GPONSNLEN  0
#define GPONPASSWDLEN 0
#define GPONEXTENDPWDLEN 0
#endif
#else
#define PONMACNUMLEN  0
#define PONUSERADMINPASSWDLEN 0
#define GPONSNLEN  0
#define GPONPASSWDLEN 0
#define GPONEXTENDPWDLEN 0
#endif

#define BOOTINFOLEN	5


#if 0// defined(TCSUPPORT_CY)
#define BARCODELEN	32
#else
#define BARCODELEN	0
#endif
#define RESERVELEN (PROLINE_CWMPPARA_RA_SIZE     \
                    - 2*sizeof(int)              \
                    - PRDDUCTCLASSLEN            \
                    - MANUFACUREROUILEN          \
                    - SERIALNUMLEN               \
                    - PROGDATELEN                \
                    - MFTDATELEN                 \
                    - PRODUCTCONTENTLEN          \
                    - FONKEYWORDLEN              \
                    - FONMACLEN                  \
                    - PONMACNUMLEN               \
                    - PONUSERADMINPASSWDLEN      \
                    - GPONSNLEN                  \
                    - GPONPASSWDLEN              \
                    - WEBPWDLEN                  \
                    - WEBACCOUNTLEN              \
                    - HWSNLEN                    \
                    - GPONEXTENDPWDLEN           \
                    - BOOTINFOLEN  				\
                    )
                    
typedef struct _proline_cwmpPara{
	int flag;
	int magic;	
	char productclass[PRDDUCTCLASSLEN];
	char manufacturerOUI[MANUFACUREROUILEN];
	char serialnum[SERIALNUMLEN];
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_DATE)
	char progDate[PROGDATELEN];
	char mftDate[MFTDATELEN];
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PRODUCTIONLINE_CONTENT)
	char productContent[PRODUCTCONTENTLEN];
#endif/*TCSUPPORT_COMPILE*/

/**************************************************
 * Attention: 
 * remove compile for reserved area when upgrade on using device 
 *************************************************/
#if defined(TCSUPPORT_FON) || defined(TCSUPPORT_FON_V2)
	char fonKeyword[FONKEYWORDLEN];
	char fonMac[FONMACLEN];
#endif
#if defined(TCSUPPORT_WAN_GPON) || defined(TCSUPPORT_WAN_EPON)
    char ponMacNum[PONMACNUMLEN];
	char ponUseradminPasswd[PONUSERADMINPASSWDLEN];
#if defined(TCSUPPORT_WAN_GPON)
	char gponSn[GPONSNLEN];
	char gponPasswd[GPONPASSWDLEN];
#endif
#endif
    char webpwd[WEBPWDLEN];
    char webAcct[WEBACCOUNTLEN];
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CHS)
    char hwSn[HWSNLEN];
#endif/*TCSUPPORT_COMPILE*/
#if defined(TCSUPPORT_WAN_GPON)
	char gponExtPwd[GPONEXTENDPWDLEN];
#endif
	char bootinfo[BOOTINFOLEN];
	char reserve[RESERVELEN];
} proline_cwmpPara;

typedef struct _proline_cwmpPara proline_Para;

#endif


#define PROLINECMD_SOCK_PATH 	"/tmp/prolinecmd_sock"
#define PROLINECMD_CWMPPARA_FILE  "/tmp/prolinecwmppara"
#define PROLINECMD_CWMPPARAREAD_FILE  "/tmp/prolinecwmppararead"
#define PROLINECMD_CWMPPARAWRITE_FILE  "/tmp/prolinecwmpparwrite"

#define CWMPPARAMAGIC 0x12344321

#endif
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_PROLINE_CMD_ACTION)
#define SERIALNUMLENTH 18
#define SERIALNUMLOFFSET 40
#define MODELENTH 12
#define MODEOFFSET 58
#define SYSMACLENTH 12
#define SYSMACOFFSET 70
#define SYSMAC 0
#define SERIVALNUM 1
#define ALL 9
#endif/*TCSUPPORT_COMPILE*/ 
#endif
