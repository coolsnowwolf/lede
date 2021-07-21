#include "code_pattern.h"

#if LINKSYS_MODEL == WRT54GSV4
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v1.05.6"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #else	// ETSI
    #define CYBERTAN_VERSION	"v1.05.8"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #endif
#elif LINKSYS_MODEL == WRT54GS	// WRT54GSV1-3
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v4.70.8"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #else	// ETSI
    #define CYBERTAN_VERSION	"v4.71.0"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #endif
#elif LINKSYS_MODEL == WRT54GL
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v4.30.2"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #else	// ETSI
    #define CYBERTAN_VERSION	"v4.30.4"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #endif
#elif LINKSYS_MODEL == WRT54G	// WRT54GV1-4
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v4.20.9"
    #define SERIAL_NUMBER	"001"
    #define MINOR_VERSION	""
  #else	// ETSI
    #define CYBERTAN_VERSION	"v4.21.0"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #endif
#elif LINKSYS_MODEL == T_MOBILE
    #define CYBERTAN_VERSION	"v5.00.0"
    #define SERIAL_NUMBER	"002"
    #define MINOR_VERSION	""
#elif LINKSYS_MODEL == WRT54G3G
    #define CYBERTAN_VERSION	"v1.90.0"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
#elif LINKSYS_MODEL == WRT300N
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v0.92.1"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #else	// ETSI
    #define CYBERTAN_VERSION	"v0.92.1"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	""
  #endif
#elif LINKSYS_MODEL == WRT350N
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v1.03.9"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	"-beta3"
  #else	// ETSI
    #define CYBERTAN_VERSION	"v1.03.9"
    #define SERIAL_NUMBER	"000"
    #define MINOR_VERSION	"-beta3"
  #endif
#elif LINKSYS_MODEL == WRT610N
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v2.00.00"
    #define SERIAL_NUMBER	"02 betasvn0526"
    #define MINOR_VERSION	""
//add by michael at 200804014
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
//end by michael
  #else	// ETSI
    #define CYBERTAN_VERSION	"v0.00.01"
    #define SERIAL_NUMBER	"09"
    #define MINOR_VERSION	""
//add by michael at 20080414
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #endif
//end by michael
#elif LINKSYS_MODEL == E200
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v0.0.01"
    #define SERIAL_NUMBER	"001"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #else	// ETSI
    #define CYBERTAN_VERSION	"v0.00.01"
    #define SERIAL_NUMBER	"001"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #endif
#elif LINKSYS_MODEL == E300
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v3.0.00"
    #define SERIAL_NUMBER	"006"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #else	// ETSI
    #define CYBERTAN_VERSION	"v0.00.01"
    #define SERIAL_NUMBER	"09"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #endif
#elif LINKSYS_MODEL == E3200
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v1.0.00"
    #define SERIAL_NUMBER	"1"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #else	// ETSI
    #define CYBERTAN_VERSION	"v0.00.01"
    #define SERIAL_NUMBER	"01"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #endif
#elif LINKSYS_MODEL == E4200
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v1.0.00"
    #define SERIAL_NUMBER	"9"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #else	// ETSI
    #define CYBERTAN_VERSION	"v0.00.01"
    #define SERIAL_NUMBER	"09"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #endif
#elif LINKSYS_MODEL == E2500
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v3.0.00"
    #define SERIAL_NUMBER	"6"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #else	// ETSI
    #define CYBERTAN_VERSION	"v1.0.00"
    #define SERIAL_NUMBER	"01"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
  #endif
#elif LINKSYS_MODEL == AC2350
  #if LOCALE == USA
    #define CYBERTAN_VERSION	"v1.0.02"
    #define SERIAL_NUMBER	"009"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
    #define CYBERTAN_UBOOT_VERSION	"v1.0"
  #else	// ETSI
    #define CYBERTAN_VERSION	"v1.0.00"
    #define SERIAL_NUMBER	"01"
    #define MINOR_VERSION	""
    #define BUILD_KEYWORD   " B"
    #define BUILD_NUMBER    SERIAL_NUMBER
    #define BETA_VERSION    " "
    #define CYBERTAN_UBOOT_VERSION	"v1.0"
  #endif

#endif

#if LINKSYS_MODEL == T_MOBILE
    #define CFES_BOOT_VERSION	"v5.3"
#elif LINKSYS_MODEL == E2500
    //#define CFES_BOOT_VERSION	"v4.13"
    //#define CFES_BOOT_VERSION	"v5.00"
    #define CFES_BOOT_VERSION	"v6.07"
#else
    #define CFES_BOOT_VERSION	"v5.2"
#endif

#define LANG_VERSION		"v1.00.00"    //for lang.bin version control(setupwizard)
#define PMON_BOOT_VERSION	"v1.8"
#define QCA_WL_VERSION		"v10.2.138.3" /*add for QCA wireless version by Jemmy 2014.5.21*/


#if LINKSYS_MODEL == T_MOBILE
#define BOOT_IPADDR "192.168.0.1"
#define BOOT_NETMASK "255.255.255.0"
#else
#define BOOT_IPADDR "192.168.1.1"
#define BOOT_NETMASK "255.255.255.0"
#endif

#define SUPPORT_4712_CHIP	0x0001
#define SUPPORT_INTEL_FLASH	0x0002
#define SUPPORT_5325E_SWITCH	0x0004
#define SUPPORT_4704_CHIP	0x0008
#define SUPPORT_5352E_CHIP	0x0010

struct code_header {
	char magic[4];	// Code Pattern
	char res1[4];	// for extra magic
	char fwdate[3];	// Firmware build date
	char fwvern[3];	// Firmware version
	char id[4];	// U2ND
	char hw_ver;    // 0) for 4702, 1) for 4712, 2) for 4712L, 3) for 4704, 4) for 5352E
	unsigned char  sn;	// Serial Number
	unsigned short flags;
	unsigned char  stable[2];	// The image is stable (for dual image)
	unsigned char  try1[2];		// Try to boot image first time (for dual image)
	unsigned char  try2[2];		// Try to boot image second time (for dual image)
	unsigned char  try3[2];		// Try to boot image third time (for dual_image)
	unsigned char  res3[2];
} ;

//add for AC2350 F/W header
#define FWHDR_MAGIC_STR "CHDR"
#define FWHDR_MAGIC 0X52444843
struct cbt_fw_header
{
	unsigned int magic;		/* "CHDR" */
	unsigned int len;		/* Length of file including header */
	unsigned int crc32;		/* 32-bit CRC */
	unsigned int res;	
};

//#ifdef MULTILANG_SUPPORT
struct lang_header {
        char magic[4];
        char res1[4];   // for extra magic
        char fwdate[3];
        char fwvern[3];
        char id[4];     // U2ND
        char hw_ver;    // 0: for 4702, 1: for 4712
	char res2;
        unsigned long len;
        unsigned char res3[8];
} ;
//#endif

struct boot_header {
	char magic[3];
	char res[29];
};

/***************************************
 * define upnp misc                    *
 ***************************************/
#if OEM == LINKSYS
  #if LANGUAGE == ENGLISH
    #define URL			"http://www.linksys.com/"
  #else
    #define URL			"http://www.linksys.co.jp/"
  #endif
  #define DEV_FRIENDLY_NAME	"Linksys"MODEL_NAME
  #define DEV_MFR		"Linksys"
  //#define DEV_MFR_URL		URL
  #define DEV_MFR_URL		"http://www.linksys.com"
  #define DEV_MODEL_DESCRIPTION	"Internet Access Server"
  #define DEV_MODEL		MODEL_NAME
  #define DEV_MODEL_NO		CYBERTAN_VERSION
  //#define DEV_MODEL_URL		URL
  #define DEV_MODEL_URL		"http://www.linksys.com/international"
#elif OEM == PCI
  #if LANGUAGE == ENGLISH
    #define URL			"http://www.planex.com/"
  #else
    #define URL			"http://www.planex.co.jp/"
  #endif
  #define DEV_FRIENDLY_NAME	"BLW-04G Wireless Broadband Router"
  #define DEV_MFR		"Planex Communciations Inc."
  #define DEV_MFR_URL		URL
  #define DEV_MODEL_DESCRIPTION	"Internet Gateway Device with UPnP support"
  #define DEV_MODEL		"BLW-04G"
  #define DEV_MODEL_NO 		CYBERTAN_VERSION
  #define DEV_MODEL_URL		URL
#else
  #define URL			""
  #define DEV_FRIENDLY_NAME	MODEL_NAME
  #define DEV_MFR		""
  #define DEV_MFR_URL		URL
  #define DEV_MODEL_DESCRIPTION	"Internet Access Server"
  #define DEV_MODEL		MODEL_NAME
  #define DEV_MODEL_NO		CYBERTAN_VERSION
  #define DEV_MODEL_URL		URL
#endif

/***************************************
 * define Parental Control link        *
 ***************************************/
#if LOCALE == EUROPE
  #define	SIGN_UP_URL	"http://pcsvc.ourlinksys.com/eu/language.jsp"
  #define	MORE_INFO_URL	"http://www.linksys.com/pcsvc/eu/info_eu.asp"
  #define	ADMIN_URL	"http://pcsvc.ourlinksys.com/en"
#elif LOCALE == GERMANY
  #define	SIGN_UP_URL	"http://pcsvc.ourlinksys.com/de/trial.asp"
  #define	MORE_INFO_URL	"http://www.linksys.com/pcsvc/de/info_de.asp"
  #define	ADMIN_URL	"http://pcsvc.ourlinksys.com/de/admin.asp"
#elif LOCALE == FRANCE
  #define	SIGN_UP_URL	"http://pcsvc.ourlinksys.com/fr/trial.asp"
  #define	MORE_INFO_URL	"http://www.linksys.com/pcsvc/fr/info_fr.asp"
  #define	ADMIN_URL	"http://pcsvc.ourlinksys.com/fr/admin.asp"
#else
  #define	SIGN_UP_URL	"http://pcsvc.ourlinksys.com/us/trial.asp"
  #define	MORE_INFO_URL	"http://www.linksys.com/pcsvc/info.asp"
  #define	ADMIN_URL	"http://pcsvc.ourlinksys.com/us/admin.asp"
#endif

/***************************************
 * define PPTP info		       * 
 ***************************************/
#if OEM == LINKSYS
#define	PPTP_VENDOR	"Linksys"
#else
#define	PPTP_VENDOR	CT_VENDOR
#endif
#define PPTP_HOSTNAME	""

/***************************************
 * define L2TP info		       *
 ***************************************/
#if OEM == LINKSYS
#define	L2TP_VENDOR	"Linksys"
#else
#define	L2TP_VENDOR	CT_VENDOR
#endif
#define L2TP_HOSTNAME	MODEL_NAME //2005-03-04 by kanki
