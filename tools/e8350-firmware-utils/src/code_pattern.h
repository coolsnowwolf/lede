#ifndef _CODE_PATTERN_H_
#define _CODE_PATTERN_H_

#define CODE_ID		"U2ND"
#define BOOT_PATTERN	"EST"
#define UP_PMON		1
#define UP_MAC		2
#define UP_CODE		3
#define UP_PIGGY	4
#define UP_EOU_KEY	5
#define UP_SN		6
#define UP_LANG		7
#define UP_T_CERT	8
#define CMP_T_CERT	9	//for factory test.
#define UP_CODE2       10
#define UP_FULL_WL     11
#define UP_WSC_PIN     12
#define UP_COUNTRY     13
#define UP_QUAN_BOOT   14
#define UP_QUAN_IMAGE   15

#define LINKSYS		7
#define PCI_OEM		9	// <remove the line>
#define ELSA		10	// <remove the line>
#define ALLNET		11	// <remove the line>
#define REENET		12	// 2003/06/21	<remove the line>
#define CISCO		90
#define	OTHER_OEM	99	// <remove the line>

#define ENGLISH 	1
#define JAPANESE	2
#define GERMAN		3
#define FRENCH		4
#define KOREAN		5

#define	USA		1
#define	JAPAN		2
#define	EUROPE		3
#define WW		4
#define GERMANY		5
#define	KOREA		6
#define	FRANCE		7

#define	WRT54G		1
#define	WRT54GS		2
#define	WRT54V5		3
#define	DPN		4
#define	RTA41		5
#define	WRT54G3G	6
#define	T_MOBILE 	7
#define WRT300N		8
#define WRT54GL         9
#define WRT54GSV4       10
#define WRT350N		11
#define WRT610N		12
#define WRT61XN		13
#define E200		14
#define E300		15
//John
#define WRT54GV5        16

#define E3200		17
#define E4200		18
#define E2500		19
#define AC2350		20


#define CES_M_SERIAL	1
#define CES_E_SERIAL	2

/***************************************
 * define OEM                          *
 * Don't edit, auto edit by select.sh  *
 ***************************************/
#define OEM LINKSYS //LINKSYS
//#define OEM PCI_OEM //PCI_OEM
//#define OEM ELSA //ELSA
//#define OEM ALLNET //ALLNET
//#define OEM REENET //REENET
//#define OEM OTHER_OEM //OTHER_OEM

/***************************************
 * define MODEL NAME                          *
 * Don't edit, auto edit by select.sh  *
 ***************************************/

//#define LINKSYS_MODEL E3200 //E3200
//#define LINKSYS_MODEL E300 //E300
//#define LINKSYS_MODEL E200 //E200
//#define LINKSYS_MODEL E4200 //E4200
//#define LINKSYS_MODEL E2500 //E2500
#define LINKSYS_MODEL AC2350 //AC2350

/***************************************
 * define MODEL SERIAL                          *
 * Don't edit, auto edit by select.sh  *
 ***************************************/
//#define LINKSYS_MODEL_SERIAL CES_M_SERIAL //CES_M_SERIAL
#define LINKSYS_MODEL_SERIAL CES_E_SERIAL //CES_E_SERIAL

/***************************************
 * define country                      *
 * LOCALE=COUNTRY =                    *
 ***************************************/
#define COUNTRY		LOCALE
#define LOCALE USA
//#define LOCALE JAPAN
//#define LOCALE EUROPE
//#define LOCALE WW
//#define LOCALE GERMANY	
//#define LOCALE FRANCE
//#define LOCALE KOREA
//#define LOCALE UK

/***************************************
 * define model name and code pattern  *
 * MODEL_NAME =                        *
 * CODE_PATTERN =                      *
 ***************************************/
#if OEM == LINKSYS
	#define	CT_VENDOR		"LINKSYS"
	#define UI_STYLE	CISCO

	#if LINKSYS_MODEL == WRT54GS
		#define CODE_PATTERN   "W54S"
		#if LOCALE == GERMANY
			#define MODEL_NAME	"WRT54GS-DE"
			#define MODEL_VERSION	""
		#elif LOCALE == FRANCE
			#define MODEL_NAME	"WRT54GS-FR"
			#define MODEL_VERSION	""
		#elif LOCALE == JAPAN
			#define MODEL_NAME	"WRT54GS-JP"
			#define MODEL_VERSION	""
		#else
			#define MODEL_NAME	"WRT54GS"
			#define MODEL_VERSION	"V3.0"
		#endif
	#elif LINKSYS_MODEL == WRT54GSV4
		#define CODE_PATTERN   "W54s"
		#define MODEL_NAME     "WRT54GSV4"
		#define MODEL_VERSION  ""
	#elif LINKSYS_MODEL == DPN  //DPN
		#define CODE_PATTERN   "WDPN"
		#define MODEL_NAME     "DPN"
		#define MODEL_VERSION	""
	#elif LINKSYS_MODEL == WRT54GV5
		#define CODE_PATTERN   "W54V"
		#define MODEL_NAME     "WRTA54G"
		#define MODEL_VERSION	""
	#elif LINKSYS_MODEL == RTA41
		#define CODE_PATTERN   "W54N"
		#define MODEL_NAME     "RTA41"
		#define MODEL_VERSION	""
	#elif LINKSYS_MODEL == WRT54G3G
		#define CODE_PATTERN   "W54F"
		#if LOCALE == EUROPE
			#define MODEL_NAME     "WRT54G3G-EU"
			#define MODEL_VERSION	""
		#elif LOCALE == UK
			#define MODEL_NAME     "WRT54G3G-UK"
			#define MODEL_VERSION	""
		#else
			#define MODEL_NAME     "WRT54G3G"
			#define MODEL_VERSION	"V1"
		#endif
	#elif LINKSYS_MODEL == T_MOBILE
		#define CODE_PATTERN   "W54T"
		#define MODEL_NAME     "WRT54G-TM"
		#define MODEL_VERSION	"V3.0"
	#elif LINKSYS_MODEL == WRT300N
		#define CODE_PATTERN   "EWCB"
		#define MODEL_NAME     "WRT300N"
		#define MODEL_VERSION	"V1.0"
	#elif LINKSYS_MODEL == WRT350N
		#define CODE_PATTERN   "EWCG"
		#define MODEL_NAME     "WRT350N"
		#define MODEL_VERSION	"V1.0"
	#elif LINKSYS_MODEL == WRT54G
		#define CODE_PATTERN   "W54G"
		#if LOCALE == GERMANY
			#define MODEL_NAME	"WRT54G-DE"
			#define MODEL_VERSION	""
		#elif LOCALE == FRANCE
			#define MODEL_NAME	"WRT54G-FR"
			#define MODEL_VERSION	""
		#elif LOCALE == JAPAN
			#define MODEL_NAME	"WRT54G-JP"
			#define MODEL_VERSION	""
		#else
			#define MODEL_NAME	"WRT54G"
			#define MODEL_VERSION	"V4.0"
		#endif
	#elif LINKSYS_MODEL == WRT54GL
		#define CODE_PATTERN   "W54G"
		#define MODEL_NAME	"WRT54GL"
		#define MODEL_VERSION	"V1"
	#elif LINKSYS_MODEL == WRT610N
		#define CODE_PATTERN   "610N"
		#define MODEL_NAME	"WRT610N"
		#define MODEL_VERSION	"V2"              //V1 or V2
	#elif LINKSYS_MODEL == E200
		#define CODE_PATTERN   "32XN"
		#define MODEL_NUMBER	"E2000"
		#define MODEL_NAME	"Linksys E2000"
		#define MODEL_VERSION	"V1.0"
	#elif LINKSYS_MODEL == E300
		#define CODE_PATTERN   "61XN"
		#define MODEL_NUMBER	"E3000"	    //model number added by Jemmy 2009.11.17
		#define MODEL_NAME	"Linksys E3000"
		#define MODEL_VERSION	"V1"              //V1 or V2
	#elif LINKSYS_MODEL == E3200
		#define CODE_PATTERN   "3200"
		#define MODEL_NUMBER	"E3200"	    //model number added by zhaoguang 2010.10.25
		#define MODEL_NAME	"Linksys E3200"
		#define MODEL_VERSION	"V1" 
	#elif LINKSYS_MODEL == E4200
		#define CODE_PATTERN   "4200"
		#define MODEL_NUMBER	"E4200"	    //model number added by Jemmy 2009.11.17
		#define MODEL_NAME	"Linksys E4200"
		#define MODEL_VERSION	"V1"              //V1 or V2
	#elif (LINKSYS_MODEL == E2500)
		#define CODE_PATTERN   "25V3"
		#define MODEL_NUMBER	"E2500"	    //model number
		#define MODEL_NAME	"Linksys E2500"
		#define MODEL_VERSION	"V3"              //V1 or V2	
	#elif (LINKSYS_MODEL == AC2350)
		#define CODE_PATTERN   "8350"
		#define MODEL_NUMBER	"E8350"	    //model number
		#define MODEL_NAME	"Linksys E8350"
		#define MODEL_VERSION	"V1"              //V1 or V2

	#else
		#error "You must select a LINKSYS_MODEL!!"
	#endif

	#if LINKSYS_MODEL == WRT54G || LINKSYS_MODEL == WRT54GL
		#define INTEL_FLASH_SUPPORT_VERSION_FROM "v1.41.8"
		#define BCM4712_CHIP_SUPPORT_VERSION_FROM "v1.50.0"
		#define INTEL_FLASH_SUPPORT_BOOT_VERSION_FROM "v1.3"
		#define BCM4712_CHIP_SUPPORT_BOOT_VERSION_FROM "v2.0"
	#else
		#define INTEL_FLASH_SUPPORT_VERSION_FROM "v0.00.0"
		#define BCM4712_CHIP_SUPPORT_VERSION_FROM "v0.00.0"
		#define INTEL_FLASH_SUPPORT_BOOT_VERSION_FROM "v0.0"
		#define BCM4712_CHIP_SUPPORT_BOOT_VERSION_FROM "v0.0"
	#endif
	#if	LINKSYS_MODEL == WRT350N
		#define WRT350N_SUPPORT_VERSION_FROM "v0.85.3"		
	#endif

#elif OEM == PCI_OEM
	#define CODE_PATTERN    "W54P"
	#if LOCALE == JAPAN
		#define MODEL_NAME	"Planex BLW-04G Wireless Router"
	#else
		#define MODEL_NAME	"Advanced Wireless 11g Router"
	#endif
	#define	CT_VENDOR		"PCI"
#elif OEM == ELSA
	#define CODE_PATTERN    "W54E"
	#define MODEL_NAME	"ELSA WLAN DSL-Router 4P G"
	#define	CT_VENDOR	"ELSA"
#elif OEM == ALLNET
	#define CODE_PATTERN    "W54A"
	#define MODEL_NAME	"Advanced Wireless 11g Router"
	#define	CT_VENDOR	"ALLNET"
#elif OEM == REENET
	#define CODE_PATTERN    "W54R"
	#define MODEL_NAME	"RW-IPG500"
	#define	CT_VENDOR	"REENET"
#elif OEM == OTHER_OEM
	#define CODE_PATTERN    "W54O"
	#define MODEL_NAME	"Advanced Wireless 11g Router"
	#define	CT_VENDOR	"OTHER_OEM"
#else
	#error "Your must select a OEM name!!"
#endif

/***************************************
 * define language                     *
 * LANGUAGE =                          *
 * LANG_SEL=EN                         *
 * HTTP_CHARSET =		       *
 ***************************************/
#if LOCALE == JAPAN
	#define	LANGUAGE	JAPANESE
	#define	HTTP_CHARSET	"shift-jis"
#elif LOCALE == GERMANY
	#define LANGUAGE	GERMAN
	#define	HTTP_CHARSET	"iso-8859-1"
#elif LOCALE == FRANCE
	#define LANGUAGE	FRENCH
	#define	HTTP_CHARSET	"iso-8859-1"
#elif LOCALE == KOREA
	#define LANGUAGE 	KOREAN
	#define	HTTP_CHARSET	"euc-kr"
#else
	#define LANGUAGE 	ENGLISH
	#define	HTTP_CHARSET	"iso-8859-1"
#endif

/***************************************
 * define wireless max channel         *
 * WL_MAX_CHANNEL =                    *
 ***************************************/
#if LOCALE == JAPAN || LOCALE == EUROPE || LOCALE == GERMANY || LOCALE == FRANCE
	#define	WL_MAX_CHANNEL	"13"
#elif OEM == KOREA			// <remove the line>
	#define	WL_MAX_CHANNEL	"13"	// <remove the line>
#else
	#define WL_MAX_CHANNEL	"11"
#endif

/***************************************
 * define web file path                *
 * WEB_PAGE =                          *
 ***************************************/
#if OEM == LINKSYS
#if UI_STYLE ==  CISCO
	#if LOCALE == JAPAN
		#define	WEB_PAGE	"cisco_wrt54g_jp"
	#elif LOCALE == GERMANY
		#define	WEB_PAGE	"cisco_wrt54g_de"
	#elif LOCALE == FRANCE
		#define	WEB_PAGE	"cisco_wrt54g_fr"
	#else
		#if LINKSYS_MODEL == E200
			#define WEB_PAGE        "ces_m"
		#elif LINKSYS_MODEL == E300
			#define WEB_PAGE        "ces_m"
		#elif LINKSYS_MODEL == E3200
			#define WEB_PAGE        "e3200_m"
		#elif LINKSYS_MODEL == E4200
			#define WEB_PAGE        "e4200_m"
		#elif (LINKSYS_MODEL == E2500)
			#define WEB_PAGE        "e2500_m"	
		#elif (LINKSYS_MODEL == AC2350)
			#define WEB_PAGE        "ac2350_m"
		#else
			#define WEB_PAGE	"cisco_wrt54g_m"
		#endif
	#endif
#else
	#if LOCALE == JAPAN
		#define	WEB_PAGE	"linksys_wrt54g_jp"
	#else
		#define WEB_PAGE	"linksys_wrt54g_en"
	#endif
#endif
#elif OEM == PCI_OEM
	#if LOCALE == JAPAN
		#define	WEB_PAGE	"pci_wrt54g_jp"
	#else
		#define WEB_PAGE	"pci_wrt54g_en"
	#endif
#elif OEM == ELSA
	#define WEB_PAGE	"elsa_wrt54g_en"
#elif OEM == ALLNET
	#define WEB_PAGE	"nonbrand"
#elif OEM == REENET
	#define WEB_PAGE	"reenet_wrt54g_kr"
#elif	OEM == OTHER_OEM
	#define WEB_PAGE	"nonbrand"
#endif

/***************************************
 * check LOCALE
 ***************************************/
#if OEM == LINKSYS
 #if LOCALE != JAPAN && LOCALE != USA && LOCALE != EUROPE && LOCALE != GERMANY && LOCALE != FRANCE
	#error	"The LOCALE for LINKSYS is error, must be USA, EUROPE, JAPAN, GERMANY or FRANCE"
 #endif
#elif OEM == PCI_OEM
 #if LOCALE != JAPAN && LOCALE != USA && LOCALE != EUROPE
	#error	"The LOCALE for PCI is error, must be USA or JAPAN"
 #endif
#elif OEM == ELSA
 #if LOCALE != EUROPE
	#error	"The LOCALE for ELSA is error, must be EUROPE"
 #endif
#elif OEM == ALLNET
 #if LOCALE != EUROPE
	#error	"The LOCALE for ALLNET is error, must be EUROPE"
 #endif
#elif OEM == REENET
 #if LOCALE != KOREA
	#error	"The LOCALE for REENET is error, must be KOREA"
 #endif
#elif OEM == OTHER_OEM
 #if LOCALE != USA && LOCALE != EUROPE
	#error	"The LOCALE for NONBRAND is error, must be USA or EUROPE"
 #endif
#endif

#endif
