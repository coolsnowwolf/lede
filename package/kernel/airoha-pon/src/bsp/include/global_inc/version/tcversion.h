/***************************************************************************************
 
 * Please check the moduleversion and releasetype when you release    
 * please modify the TRUNK_TIME define when you create a new Branch
****************************************************************************************/

#define TRUNK_TIME ""  
#define BUILD_NUM __DATE__"-"__TIME__          

/***************************************************************************************
 
 * #define  MODULE_VERSION_NAME          modulename    moduleversion releasetype trunktime buildnum      
 * releasetype: "#" for test code ,"$" for verification, " " for normal release 
 * Ex:
	release test tccicmd
	#define MODULE_VERSION_TCCICMD           "tccicmd"       " V1.1.0.0"     "#"     TRUNK_TIME "("BUILD_NUM")"
	release verification tccicmd
	#define MODULE_VERSION_TCCICMD           "tccicmd"       " V1.1.0.0"     "$"     TRUNK_TIME "("BUILD_NUM")"
	release normal tccicmd
	#define MODULE_VERSION_TCCICMD           "tccicmd"       " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"

****************************************************************************************/


/************************************************************
 *for modules version control
 *it will be printed when module init
*************************************************************/
#define MODULE_VERSION_TCCICMD           "tccicmd"       " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_IGMPSNOOP         "igmpsnoop"     " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_TCLEDCTRL         "tcledctrl"     " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_TCFULLCONE        "tcfullcone"    " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_TCSMUX            "tcsmux"        " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_MLDSNOOPING       "mldsnooping"   " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_TC3162MAC         "tc3162mac"     " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_TC3162SAR         "tc3162sar"     " V1.2.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_TCPORTBIND        "tcportbind"    " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_GMAC              "tc3262gmac"    " V1.0.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_GMAC1             "tc3262gmac1"   " V1.0.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_PTM               "tc3262ptm"     " V1.0.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_HWNAT             "hwnat"         " V1.0.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_ACCESSLIMIT       "accesslimit"   " V1.2.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_XPONMAC           "xPONMAC"       " V1.0.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_10GXPONMAC        "10GxPONMAC"    " V1.0.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_ADSLCMD           "adslphxcmd"       " V1.2.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_TCWDOG            "tcwdog"        " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"	/*redefine width apps/private/version/tcversion.h*/
#define MODULE_VERSION_TCCI              "tcci"          " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")" /*redefine width apps/private/version/tcversion.h*/
#define MODULE_VERSION_SMUXCTL           "smuxctl"       " V1.1.0.0"     " "     TRUNK_TIME "("BUILD_NUM")"
#define MODULE_VERSION_ETHCMD            "ethphxcmd"        " V1.2.0.0"     " "     TRUNK_TIME "("BUILD_NUM")" /*redefine width apps/private/version/tcversion.h*/
#define BOOLOADER_VERSION	             "1.1"
