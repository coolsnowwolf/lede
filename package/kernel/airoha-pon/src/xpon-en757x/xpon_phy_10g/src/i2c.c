
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/timer.h>
#include <linux/proc_fs.h>
#include <linux/module.h>
#include <linux/delay.h>

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36) 
#include <linux/time64.h>
#endif


#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#ifdef TCSUPPORT_MT7570
#include <ecnt_hook/ecnt_hook_lddla.h>
#endif
#include <asm/tc3162/tc3162.h>

#include "phy.h"
#include "phy_def.h"
#include "phy_debug.h"
#include "phy_init.h"
#include "phy_tx.h"
//#include "en7580.h"
#include "phy_reg.h"
#include "i2c.h"

#ifdef TCSUPPORT_CPU_EN7581
#include "en7581.h"
#include "en7581_pma.h"
#include "en7581_reg.h"

#include "en7583.h"
//#include "en7583_pma.h"
#include "en7583_reg.h"

#else
#include "en7580.h"
#include "en7580_pma.h"
#include "en7580_reg.h"
#endif


#if SIF_DEBUG_LEVEL_CONTROL
#define PROC_BUF_SIZE          1
#define PROC_READ_WRITE_TIMES  1
#define SIF_NO_LOG_PRINT       "0"

unsigned char *proc_buffer;
unsigned char sifm_proc_filename[]="/proc/tc3162/sifm_log_lever";
#endif
//#ifdef TCSUPPORT_CPU_AN7583
PHY_TRANS_INFO_7583 phy_trans_iot_list_7583[]={  
//   index,    vendor name,     vendor pn,             tx,         rx
    { 0 ,  "cmd",               "manual",              0x0,      0x0}, 
    { 1 ,  "WTD",               "RTXM167-407-C10",     0x1,      0x0},
    { 2 ,  "WTD",               "RTXM169-504",         0x1,      0x0},
    { 3 ,  "WTD",               "RTXM169-514",         0x1,      0x0},
    { 4 ,  "SOURCEPHOTONICS",   "SFA3424THPCDFJ",      0x0,      0x0},
    { 5 ,  "SOURCEPHOTONICS",   "SFU34GBP2CDFA",       0x1,      0x0},
    { 6 ,  "Hisense",           "LTB34D2-SC+",         0x1,      0x0},
    { 7 ,  "Hisense",           "LTB34D3-SC+",         0x1,      0x0},
    { 8 ,  "Hisense",           "LTE3468L-BHG+",       0x0,      0x1},
    { 9 ,  "PHYHOME",           "FHM-U2814-S3A20C",    0x0,      0x0},
    { 10,  "PHYHOME",           "FHM-U1213-S3A20C",    0x1,      0x0},
    { 11,  "PHYHOME",           "FHM-U2814-U3A20C",    0x0,      0x0},
    { 12,  "PHYHOME",           "FHM-U1213-U3A20C",    0x1,      0x0},
    { 13,  "PHYHOME",           "FHM-U2814-A3A20C",    0x0,      0x0},
    { 14,  "PHYHOME",           "FHM-U1213-A3A20C",    0x1,      0x0},
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_C7) 
    { 15,  "Hisense",           "LTB3468-BHG+",        0x0,      0x1},
#else/*TCSUPPORT_COMPILE*/ 
    { 15,  "Hisense",           "LTB3468-BHG+",        0x0,      0x0},
#endif/*TCSUPPORT_COMPILE*/ 
    { 16,  "DARE TECH OPT",     "E31LA21B+",           0x1,      0x0},
    { 17,  "APAC Opto",         "LSF2C3STCN322",       0x0,      0x0},
    { 18,  "APAC Opto",         "LSF2C3MTCN3G4",       0x0,      0x0},
    { 19,  "MENTECHOPTO",       "NOG22-D6C-ND",        0x0,      0x0},
    { 20,  "DELTA",             "OPEP-33-A4G1RC",      0x1,      0x0},
    { 21,  "MENTECHOPTO",       "NOE22-LD6C-NT",       0x1,      0x0}, /* epon */
    { 22,  "SUPERXON LTD.",     "SOEB3466-FSGF",       0x1,      0x0}, /* epon */
    { 23,  "EOPTOLINK INC",     "EOLF-GUA-25-DZTE",    0x0,      0x0}, /* gpon */
    { 24,  "SUPERXON LTD.",     "SOGP3412-FSGA",       0x0,      0x0}, /* gpon */
    { 25,  "SUPERXON LTD.",     "SOEB3466-FUGF",       0x1,      0x0},
    { 26,  "WTD",               "RTXM167-407",         0x1,      0x0},
    { 27,  "XUGUANG LTD.",      "XTEC3411F-C",         0x1,      0x0}, /* epon */
    { 28,  "XUGUANG LTD.",      "XTGP3434FRS-A",       0x0,      0x0}, /* gpon */
    { 29,  "T&W",               "TW2362G-CDEH",        0x0,      0x1},
    { 30,  "HUAWEI",            "HPSP2120",            0x1,      0x1},
    { 31,  "Hisense",           "LTE3468-BCG+",        0x0,      0x0},
#if 0  //this case is for LTE34D3-SH+I which is a special customized SFP+ module
    { 32,  "Hisense",           "LTE34D3-SH+",         0x1,      0x1}, //LTE34D3-SH+I    use the same PN as LTE34D3-SH+, while its LOS signal is inverted 
#else
    { 32,  "Hisense",           "LTE34D3-SH+",         0x1,      0x0},
#endif
    { 33,  "HONLUS",            "HOLSFP3420330C",      0x1,      0x0},
    { 34,  "HONLUS",            "HOLSP3412452DC0C",    0x1,      0x0},
    { 35,  "T&W",               "TW2362G-CDEH",        0x0,      0x1},
    { 36,  "EOPTOLINK INC",     "EOLF-GEUA-25-D1A",    0x1,      0x0}, /* epon */
    { 37,  "Q-STAR",            "BYFS-462A4-HB-12",    0x0,      0x0}, /* gpon */
    { 38,  "Q-STAR",            "BYFS-44234-LB-12",    0x1,      0x0}, /* epon */
    { 39,  "MENTECHOPTO",       "NOG22-D6C-SD",        0x0,      0x0}, /* gpon */
    { 40,  "MENTECHOPTO",       "NOE22-LD6C-SD",       0x1,      0x0}, /* epon */
    { 41,  "MENTECHOPTO",       "NOG22-LD6C-ST",       0x0,      0x0},
    { 42,  "MENTECHOPTO",       "NOE22-LD6C-ST",       0x1,      0x0},
    { 43,  "FIBERTOWER",        "FEU3411S-2DC-0C",     0x1,      0x0}, /* epon */
    { 44,  "FIBERTOWER",        "FGU3412S-2DC-1C",     0x0,      0x0}, 
    { 45,  "ZKTEL",             "ZP4342034-KCST",      0x0,      0x0}, 
//EN7580 new add
    { 46,  "Ligent Photonics",  "LTF7219-BH",          0x1,      0x1}, //XEPON symm may support XGSPON
    { 47,  "Hisense",           "LTF7215-BC+",         0x0,      0x0}, //XEPON asym
    { 48,  "Hisense",           "LTF7215-BH+",         0x1,      0x1}, //XEPON asym
    { 49,  "SOURCEPHOTONICS",   "SPPS27XER3CDFB",      0x1,      0x1}, //XEPON symm may support XGSPON
    { 50,  "NEOPHOTONICS",      "PTNGN1-42NP+",        0x1,      0x0}, //XGPON
    { 51,  "Ligent Photonics",  "LTF7221-BC",          0x1,      0x1}, //XGPON
    { 52,  "Hisense",           "LTF7221-BH+",         0x1,      0x1}, //XGPON support 
    { 53,  "WTD",               "RTXM166-502",         0x0,      0x1}, //XEPON symm  
    { 54,  "SOURCEPHOTONICS",   "SPPS2748FN2IDFB",     0x1,      0x1}, //XGPON OK
    { 55,  "SOURCEPHOTONICS",   "SPPS27XER3CDFD",      0x1,      0x1}, //XGSPON OK  support XEPON symm 
    { 56,  "WTD",               "RTXM169-531",         0x1,      0x1}, //EPON
    { 57,  "WTD",               "RTXM167-431",         0x1,      0x1}, //GPON
    { 58,  "SOURCEPHOTONICS",   "SPPS37GBOX3CDFD",     0x1,      0x1}, //XEPON asym
    { 59,  "Hisense",           "LTE3468-BHG+",        0x0,      0x0}, //GPON
    { 60,  "Hisense",           "LTF7219-BH+",         0x1,      0x1}, //XEPON symm
    { 61,  "DIPPEROPTICSLTD.",  "D22799-SSCA",         0x1,      0x1}, //XEPON asym
    { 62,  "T&W",               "TW91A6H-CGAL-G95",    0x1,      0x1}, //XGPON
    { 63,  "T&W",               "TW91A6H-CGAL",        0x1,      0x1}, //XGPON
    { 64,  "T&W",               "TW91A9H-HCAL",        0x1,      0x1}, //XGSPON
    { 65,  "PRX30",             "GN28L95",             0x1,      0x1}, //XEPON / XGPON BoB
    { 66,  "Hisense",           "LTF7222-BC+",         0x1,      0x0}, //XGPON OLT  
    { 67,  "Hisense",           "LTF7226-BC+",         0x1,      0x0}, //XGSPON OLT 
    { 68,  "Hisense",           "LTF4301C+",           0x1,      0x1}, //XEPON OLT 
    { 69,  "Z-QUICK",           "ZN62720AF-QCSL",      0x1,      0x1}, //XGPON ONU 
    { 70,  "Hisense",           "LTF7225-BH+",         0x1,      0x1}, //XGSPON ONU 
    { 71,  "TSUHAN LTD.",       "THMPRS-2647-GXBI",    0x1,      0x1}, //XGPON ONU 
    { 72,  "Potron",            "SPP424C-GBA0",        0x1,      0x1}, //XGPON ONU 
    { 73,  "TSUHAN LTD.",       "THMPRS-3618-EXBI",    0x1,      0x1}, //XEPON asym ONU 
    { 74,  "Potron",            "SPP424C-EBA4",        0x1,      0x1}, //XEPON asym ONU 
    { 75,  "MENTECHOPTO",       "MPOT22-LD6C-GTT1",    0x1,      0x1}, //XEPON asym ONU 
    { 76,  "MENTECHOPTO",       "MPOX22-LD6C-GTT1",    0x1,      0x1}, //XGPON ONU 
    { 77,  "MENTECHOPTO",       "MSOG22-LD6C-DCT1",    0x1,      0x0}, //GPON ONU 
    { 78,  "MENTECHOPTO",       "MSOE22-LD6C-ACT1",    0x1,      0x0}, //EPON ONU 
    { 79,  "MENTECHOPTO",       "MSOT22-LD6C-GTT1",    0x1,      0x1}, //XEPON asym ONU 
    { 80,  "PR30",              "GN28L96",             0x1,      0x1}, //XGSPON BoB, BEN LOW active
    { 81,  "Hisense",           "LTF4301-BC+",         0x1,	   	 0x1}, //XEPON OLT 
    { 82,  "ECONET",		    "EN7572",              0x0, 	 0x1}, //XEPON 7572 ONU , BEN HIGH active
    { 83,  "ACMETEK",           "123456789",           0x1,      0x1}, //XEPON_asy & XGPON ONU
    { 84,  "Hisense",           "LTF7225-BC+",         0x1,      0x1}, //XGSPON ONU 
    { 85,  "Hisense",           "LTE3468-BC+",         0x0,      0x0}, //GPON?
    { 86,  "JZ",           		"28L96",           	   0x1,      0x1}, //xgpon
    { 87,  "JZ",           		"28L95",         	   0x1,      0x1}, //xgpon
    { 88,  "MENTECHOPTO",       "POS22-LD6I-DS",       0x1,      0x1}, //XGSPON ONU 
    { 89,  "TSUHAN LTD.",       "THMPRS-3434-GBHI",    0x1,      0x0}, //GPON  
	{ 90,  "MENTECHOPTO",       "MSOG22-LD6I-ACNK",    0x1,      0x0}, //GPON 
    { 91,  "TSUHAN LTD.",       "THMPRS-2677-GXAI",    0x1,      0x1}, //XGPON ONU 
    { 92,  "Potron",       		"SFP235E-GBC0",  	   0x1,      0x1}, //GPON  SYM ONU 
    { 93,  "AIROHA",			"AN8900",			   0x1, 	 0x1},
//add new optical module info above                                                        
    { PHY_TRANS_NOT_FOUND_IN_IOT_LIST,  "",  "",       0x0,        0x0} //end of the list

};

//#else
#if ASIC_SERDES
PHY_TRANS_INFO phy_trans_iot_list[]={  
//   index,    vendor name,                vendor pn,                        gepon,           xgpon,          xepon,             pma0,           pma1
    { 0 ,  "cmd",               "manual",              0x14f,      0x0,        0xdf000000,  0x10001,    0x1010100}, 
    { 1 ,  "WTD",               "RTXM167-407-C10",     0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 2 ,  "WTD",               "RTXM169-504",         0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 3 ,  "WTD",               "RTXM169-514",         0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 4 ,  "SOURCEPHOTONICS",   "SFA3424THPCDFJ",      0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 5 ,  "SOURCEPHOTONICS",   "SFU34GBP2CDFA",       0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 6 ,  "Hisense",           "LTB34D2-SC+",         0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 7 ,  "Hisense",           "LTB34D3-SC+",         0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 8 ,  "Hisense",           "LTE3468L-BHG+",       0x10f,      0x0,        0x0,         0x10001,    0x1010100},
    { 9 ,  "PHYHOME",           "FHM-U2814-S3A20C",    0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 10,  "PHYHOME",           "FHM-U1213-S3A20C",    0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 11,  "PHYHOME",           "FHM-U2814-U3A20C",    0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 12,  "PHYHOME",           "FHM-U1213-U3A20C",    0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 13,  "PHYHOME",           "FHM-U2814-A3A20C",    0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 14,  "PHYHOME",           "FHM-U1213-A3A20C",    0x14f,      0x0,        0x0,         0x10101,    0x1010100},
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_C7) 
    { 15,  "Hisense",           "LTB3468-BHG+",        0x10f,      0x0,        0x0,         0x10001,    0x1010100},
#else/*TCSUPPORT_COMPILE*/ 
    { 15,  "Hisense",           "LTB3468-BHG+",        0x14f,      0x0,        0x0,         0x10001,    0x1010100},
#endif/*TCSUPPORT_COMPILE*/ 
    { 16,  "DARE TECH OPT",     "E31LA21B+",           0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 17,  "APAC Opto",         "LSF2C3STCN322",       0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 18,  "APAC Opto",         "LSF2C3MTCN3G4",       0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 19,  "MENTECHOPTO",       "NOG22-D6C-ND",        0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 20,  "DELTA",             "OPEP-33-A4G1RC",      0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 21,  "MENTECHOPTO",       "NOE22-LD6C-NT",       0x14f,      0x0,        0x0,         0x10101,    0x1010100}, /* epon */
    { 22,  "SUPERXON LTD.",     "SOEB3466-FSGF",       0x14f,      0x0,        0x0,         0x10101,    0x1010100}, /* epon */
    { 23,  "EOPTOLINK INC",     "EOLF-GUA-25-DZTE",    0x14f,      0x0,        0x0,         0x10001,    0x1010100}, /* gpon */
    { 24,  "SUPERXON LTD.",     "SOGP3412-FSGA",       0x14f,      0x0,        0x0,         0x10001,    0x1010100}, /* gpon */
    { 25,  "SUPERXON LTD.",     "SOEB3466-FUGF",       0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 26,  "WTD",               "RTXM167-407",         0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 27,  "XUGUANG LTD.",      "XTEC3411F-C",         0x14f,      0x0,        0x0,         0x10101,    0x1010100}, /* epon */
    { 28,  "XUGUANG LTD.",      "XTGP3434FRS-A",       0x14f,      0x0,        0x0,         0x10001,    0x1010100}, /* gpon */
    { 29,  "T&W",               "TW2362G-CDEH",        0x10f,      0x0,        0x0,         0x10001,    0x1010100},
    { 30,  "HUAWEI",            "HPSP2120",            0x10f,      0x0,        0x0,         0x10101,    0x1010100},
    { 31,  "Hisense",           "LTE3468-BCG+",        0x14f,      0x0,        0x0,         0x10001,    0x1010100},
#if 0  //this case is for LTE34D3-SH+I which is a special customized SFP+ module
    { 32,  "Hisense",           "LTE34D3-SH+",         0x10f,      0x0,        0x0,         0x10101,    0x1010100}, //LTE34D3-SH+I    use the same PN as LTE34D3-SH+, while its LOS signal is inverted 
#else
    { 32,  "Hisense",           "LTE34D3-SH+",         0x14f,      0x0,        0x0,         0x10101,    0x1010100},
#endif
    { 33,  "HONLUS",            "HOLSFP3420330C",      0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 34,  "HONLUS",            "HOLSP3412452DC0C",    0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 35,  "T&W",               "TW2362G-CDEH",        0x10f,      0x0,        0x0,         0x10001,    0x1010100},
    { 36,  "EOPTOLINK INC",     "EOLF-GEUA-25-D1A",    0x14f,      0x0,        0x0,         0x10101,    0x1010100}, /* epon */
    { 37,  "Q-STAR",            "BYFS-462A4-HB-12",    0x14f,      0x0,        0x0,         0x10001,    0x1010100}, /* gpon */
    { 38,  "Q-STAR",            "BYFS-44234-LB-12",    0x14f,      0x0,        0x0,         0x10101,    0x1010100}, /* epon */
    { 39,  "MENTECHOPTO",       "NOG22-D6C-SD",        0x14f,      0x0,        0x0,         0x10001,    0x1010100}, /* gpon */
    { 40,  "MENTECHOPTO",       "NOE22-LD6C-SD",       0x14f,      0x0,        0x0,         0x10101,    0x1010100}, /* epon */
    { 41,  "MENTECHOPTO",       "NOG22-LD6C-ST",       0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 42,  "MENTECHOPTO",       "NOE22-LD6C-ST",       0x14f,      0x0,        0x0,         0x10101,    0x1010100},
    { 43,  "FIBERTOWER",        "FEU3411S-2DC-0C",     0x14f,      0x0,        0x0,         0x10101,    0x1010100}, /* epon */
    { 44,  "FIBERTOWER",        "FGU3412S-2DC-1C",     0x14f,      0x0,        0x0,         0x10001,    0x1010100}, 
    { 45,  "ZKTEL",             "ZP4342034-KCST",      0x14f,      0x0,        0x0,         0x10001,    0x1010100}, 
//EN7580 new add
    { 46,  "Ligent Photonics",  "LTF7219-BH",          0x0,        0x9,        0x9f000000,  0x10101,    0x1010101}, //XEPON symm may support XGSPON
    { 47,  "Hisense",           "LTF7215-BC+",         0x0,        0x0,        0xff800000,  0x0,        0x1010100}, //XEPON asym
    { 48,  "Hisense",           "LTF7215-BH+",         0x0,        0x0,        0x9f800000,  0x10101,    0x1010100}, //XEPON asym
    { 49,  "SOURCEPHOTONICS",   "SPPS27XER3CDFB",      0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XEPON symm may support XGSPON
    { 50,  "NEOPHOTONICS",      "PTNGN1-42NP+",        0x0,        0xa,        0x0,         0x10101,    0x1010100}, //XGPON
    { 51,  "Ligent Photonics",  "LTF7221-BC",          0x0,        0x9,        0x0,         0x10101,    0x1010100}, //XGPON
    { 52,  "Hisense",           "LTF7221-BH+",         0x0,        0x9,        0x0,         0x10101,    0x1010100}, //XGPON support 
    { 53,  "WTD",               "RTXM166-502",         0x0,        0x0,        0xdf000000,  0x0,        0x1010100}, //XEPON symm  
    { 54,  "SOURCEPHOTONICS",   "SPPS2748FN2IDFB",     0x0,        0x1,        0x0,         0x10101,    0x1010100}, //XGPON OK
    { 55,  "SOURCEPHOTONICS",   "SPPS27XER3CDFD",      0x0,        0x1,        0x9b000000,  0x10101,    0x1010100}, //XGSPON OK  support XEPON symm 
    { 56,  "WTD",               "RTXM169-531",         0x10f,      0x0,        0x0,         0x10101,    0x1010100}, //EPON
    { 57,  "WTD",               "RTXM167-431",         0x10f,      0x0,        0x0,         0x10101,    0x1010100}, //GPON
    { 58,  "SOURCEPHOTONICS",   "SPPS37GBOX3CDFD",     0x0,        0x0,        0x9f000000,  0x10101,    0x1010100}, //XEPON asym
    { 59,  "Hisense",           "LTE3468-BHG+",        0x14f,      0x0,        0x0,         0x10001,    0x1010100}, //GPON
    { 60,  "Hisense",           "LTF7219-BH+",         0x0,        0x9,        0x9f000000,  0x10101,    0x1010101}, //XEPON symm
    { 61,  "DIPPEROPTICSLTD.",  "D22799-SSCA",         0x0,        0xa,        0x0,         0x10101,    0x1010100}, //XEPON asym
    { 62,  "T&W",               "TW91A6H-CGAL-G95",    0x0,        0x9,        0x0,         0x10101,    0x1010100}, //XGPON
    { 63,  "T&W",               "TW91A6H-CGAL",        0x0,        0x9,        0x0,         0x10101,    0x1010100}, //XGPON
    { 64,  "T&W",               "TW91A9H-HCAL",        0x0,        0x9,        0x0,         0x10101,    0x1010100}, //XGSPON
    { 65,  "PRX30",             "GN28L95",             0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XEPON / XGPON BoB
    { 66,  "Hisense",           "LTF7222-BC+",         0x14f,      0x0,        0xdf000000,  0x10301,    0x1010100}, //XGPON OLT  
    { 67,  "Hisense",           "LTF7226-BC+",         0x10f,      0x0,        0xaf000000,  0x10301,    0x1010100}, //XGSPON OLT 
    { 68,  "Hisense",           "LTF4301C+",           0x14f,      0x0,        0xdf000000,  0x10301,    0x1010100}, //XEPON OLT 
    { 69,  "Z-QUICK",           "ZN62720AF-QCSL",      0x14f,      0x1,        0xdf000000,  0x10101,    0x1010100}, //XGPON ONU 
    { 70,  "Hisense",           "LTF7225-BH+",         0x0,        0x9,        0x9f000000,  0x10101,    0x1010101}, //XGSPON ONU 
    { 71,  "TSUHAN LTD.",       "THMPRS-2647-GXBI",    0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XGPON ONU 
    { 72,  "Potron",            "SPP424C-GBA0",        0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XGPON ONU 
    { 73,  "TSUHAN LTD.",       "THMPRS-3618-EXBI",    0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XEPON asym ONU 
    { 74,  "Potron",            "SPP424C-EBA4",        0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XEPON asym ONU 
    { 75,  "MENTECHOPTO",       "MPOT22-LD6C-GTT1",    0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XEPON asym ONU 
    { 76,  "MENTECHOPTO",       "MPOX22-LD6C-GTT1",    0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XGPON ONU 
    { 77,  "MENTECHOPTO",       "MSOG22-LD6C-DCT1",    0x14f,      0x0,        0xdf000000,  0x10101,    0x1010100}, //GPON ONU 
    { 78,  "MENTECHOPTO",       "MSOE22-LD6C-ACT1",    0x14f,      0x0,        0xdf000000,  0x10101,    0x1010100}, //EPON ONU 
    { 79,  "MENTECHOPTO",       "MSOT22-LD6C-GTT1",    0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XEPON asym ONU 
    { 80,  "PR30",              "GN28L96",             0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XGSPON BoB, BEN LOW active
    { 81,  "Hisense",           "LTF4301-BC+",         0x14f,	   0x0,        0xdf000000,  0x10301,    0x1010100}, //XEPON OLT 
    { 82,  "ECONET",		    "EN7572",              0x14f, 	   0x9, 	   0xdf800000,	0x10001,	0x1010100}, //XEPON 7572 ONU , BEN HIGH active
    { 83,  "ACMETEK",           "123456789",           0x14f,      0x1,        0xdf000000,  0x10101,    0x1010100}, //XEPON_asy & XGPON ONU
    { 84,  "Hisense",           "LTF7225-BC+",         0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XGSPON ONU 
    { 85,  "Hisense",           "LTE3468-BC+",         0x14f,      0x0,        0x0,         0x10001,    0x1010100},
    { 86,  "JZ",           		"28L96",         	   0x0,        0x9,        0x9f000000,  0x10101,    0x1010100},
    { 87,  "JZ",           		"28L95",         	   0x0,        0x9,        0x9f000000,  0x10101,    0x1010100},
    { 88,  "MENTECHOPTO",       "POS22-LD6I-DS",       0x0,        0x9,        0x9f000000,  0x10101,    0x1010100}, //XGSPON ONU 
    { 89,  "TSUHAN LTD.",       "THMPRS-3434-GBHI",    0x14f,      0x9,        0x9f000000,  0x10101,    0x1010101}, //GPON  
	{ 90,  "MENTECHOPTO",       "MSOG22-LD6I-ACNK",    0x14f,      0x9,        0x9f000000,  0x10101,    0x1010101}, //GPON 
	{ 91,  "MENTECHOPTO",			"MSOG22-LD6I-ACNK",			   0x14f,	   0x9, 	   0xdf800000,	0x10101,	0x1010101},
    { 92,  "TSUHAN LTD.",       "THMPRS-2677-GXAI",    0x0,        0x1,        0x0, 		0x10101,    0x1010100}, //XGPON ONU 
	{ 93,  "AIROHA",			"AN8900",			   0x14f,	   0x9, 	   0xdf800000,	0x10001,	0x1010100},
//add new optical module info above                                                        
    { PHY_TRANS_NOT_FOUND_IN_IOT_LIST,  "",  "",       0x0,        0x0,        0x0,         0x0,        0x0} //end of the list

};
#endif

#if A60972_SERDES

PHY_TRANS_INFO phy_trans_iot_list[]={  
//   index,   vendor_id,                      vendor_pn,   phy_csr_xpon_val, xgpon_phy_sfp_vld_level, xepon_pcs_sfp_status
    { 0 ,  "WTD",               "RTXM167-407-C10",   0x1cf,       0x0,         0x0         },
    { 1 ,  "WTD",               "RTXM169-504",       0x1cf,       0x0,         0x0         },
    { 2 ,  "WTD",               "RTXM169-514",       0x1cf,       0x0,         0x0         },
    { 3 ,  "SOURCEPHOTONICS",   "SFA3424THPCDFJ",    0x14f,       0x0,         0x0         },
    { 4 ,  "SOURCEPHOTONICS",   "SFU34GBP2CDFA",     0x1cf,       0x0,         0x0         },
    { 5 ,  "Hisense",           "LTB34D2-SC+",       0x1cf,       0x0,         0x0         },
    { 6 ,  "Hisense",           "LTB34D3-SC+",       0x1cf,       0x0,         0x0         },
    { 7 ,  "Hisense",           "LTE3468L-BHG+",     0x1cf,       0x0,         0x0         },
    { 8 ,  "PHYHOME",           "FHM-U2814-S3A20C",  0x14f,       0x0,         0x0         },
    { 9 ,  "PHYHOME",           "FHM-U1213-S3A20C",  0x1cf,       0x0,         0x0         },
    { 10,  "PHYHOME",           "FHM-U2814-U3A20C",  0x14f,       0x0,         0x0         },
    { 11,  "PHYHOME",           "FHM-U1213-U3A20C",  0x1cf,       0x0,         0x0         },
    { 12,  "PHYHOME",           "FHM-U2814-A3A20C",  0x14f,       0x0,         0x0         },
    { 13,  "PHYHOME",           "FHM-U1213-A3A20C",  0x1cf,       0x0,         0x0         },
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_C7)                                      
    { 14,  "Hisense",           "LTB3468-BHG+",      0x10f,       0x0,         0x0         },
#else/*TCSUPPORT_COMPILE*/                                                                 
    { 14,  "Hisense",           "LTB3468-BHG+",      0x14f,       0x0,         0x0         },
#endif/*TCSUPPORT_COMPILE*/                                                                
    { 15,  "DARE TECH OPT",     "E31LA21B+",         0x1cf,       0x0,         0x0         },
    { 16,  "APAC Opto",         "LSF2C3STCN322",     0x14f,       0x0,         0x0         },
    { 17,  "APAC Opto",         "LSF2C3MTCN3G4",     0x14f,       0x0,         0x0         },
    { 18,  "MENTECHOPTO",       "NOG22-D6C-ND",      0x14f,       0x0,         0x0         },
    { 19,  "DELTA",             "OPEP-33-A4G1RC",    0x1cf,       0x0,         0x0         },
    { 20,  "MENTECHOPTO",       "NOE22-LD6C-NT",     0x1cf,       0x0,         0x0         }, /* epon */
    { 21,  "SUPERXON LTD.",     "SOEB3466-FSGF",     0x1cf,       0x0,         0x0         }, /* epon */
    { 22,  "EOPTOLINK INC",     "EOLF-GUA-25-DZTE",  0x14f,       0x0,         0x0         }, /* gpon */
    { 23,  "MENTECHOPTO",       "NOG22-D6C-ND",      0x14f,       0x0,         0x0         }, /* gpon */
    { 24,  "SUPERXON LTD.",     "SOGP3412-FSGA",     0x14f,       0x0,         0x0         }, /* gpon */
    { 25,  "SUPERXON LTD.",     "SOEB3466-FUGF",     0x1cf,       0x0,         0x0         },
    { 26,  "WTD",               "RTXM167-407",       0x1cf,       0x0,         0x0         },
    { 27,  "XUGUANG LTD.",      "XTEC3411F-C",       0x1cf,       0x0,         0x0         },/* jiuzhou epon */
    { 28,  "XUGUANG LTD.",      "XTGP3434FRS-A",     0x14f,       0x0,         0x0         },/* jiuzhou gpon */
    { 29,  "T&W",               "TW2362G-CDEH",      0x10f,       0x0,         0x0         },
    { 30,  "HUAWEI",            "HPSP2120",          0x1cf,       0x0,         0x0         },
    { 31,  "Hisense",           "LTE3468-BCG+",      0x10f,       0x0,         0x0         },
    { 32,  "Hisense",           "LTE34D3-SH+",       0x18f,       0x0,         0x0         },
    { 33,  "HONLUS",            "HOLSFP3420330C",    0x1cf,       0x0,         0x0         },
    { 34,  "HONLUS",            "HOLSP3412452DC0C",  0x1cf,       0x0,         0x0         },
    { 35,  "T&W",               "TW2362G-CDEH",      0x10f,       0x0,         0x0         },
    { 36,  "EOPTOLINK INC",     "EOLF-GEUA-25-D1A",  0x1cf,       0x0,         0x0         }, /* epon */
    { 37,  "Q-STAR",            "BYFS-462A4-HB-12",  0x14f,       0x0,         0x0         }, /* gpon */
    { 38,  "Q-STAR",            "BYFS-44234-LB-12",  0x1cf,       0x0,         0x0         }, /* epon */
    { 39,  "MENTECHOPTO",       "NOG22-D6C-SD",      0x14f,       0x0,         0x0         }, /* gpon */
    { 40,  "MENTECHOPTO",       "NOE22-LD6C-SD",     0x1cf,       0x0,         0x0         }, /* epon */
    { 41,  "MENTECHOPTO",       "NOG22-LD6C-ST",     0x14f,       0x0,         0x0         },
    { 42,  "MENTECHOPTO",       "NOE22-LD6C-ST",     0x1cf,       0x0,         0x0         },
    { 43,  "FIBERTOWER",        "FEU3411S-2DC-0C",   0x1cf,       0x0,         0x0         }, /* epon */
    { 44,  "FIBERTOWER",        "FGU3412S-2DC-1C",   0x14f,       0x0,         0x0         }, 
    { 45,  "ZKTEL",             "ZP4342034-KCST",    0x14f,       0x0,         0x0         }, 
//EN7580 new add
    { 46,  "Ligent Photonics",  "LTF7219-BH",        0x0,         0x0,         0xff000000  }, //XEPON symm may support XGSPON
    { 47,  "Hisense",           "LTF7215-BC+",       0x0,         0x0,         0xff800000  }, 
    { 48,  "Hisense",           "LTF7215-BH+",       0x0,         0x0,         0xff800000  }, 
    { 49,  "SOURCEPHOTONICS",   "SPPS27XER3CDFB",    0x0,         0x0,         0xff000000  }, //XEPON symm may support XGSPON
    { 50,  "NEOPHOTONICS",      "PTNGN1-42NP+",      0x0,         0xf,         0x0         },  
    { 51,  "Ligent Photonics",  "LTF7221-BC",        0x0,         0x0,         0x0         },  
    { 52,  "Hisense",           "LTF7221-BH+",       0x0,         0x0,         0x0         },  
    { 53,  "WTD",               "RTXM166-502",       0x0,         0x0,         0xdf000000  }, //XEPON symm  
    { 54,  "SOURCEPHOTONICS",   "SPPS2748FN2IDFB",   0x0,         0x0,         0x0         }, //XGPON OK
    { 55,  "SOURCEPHOTONICS",   "SPPS27XER3CDFD",    0x0,         0x0,         0xff000000  }, //XGSPON OK  support XEPON symm 
    { 56,  "WTD",               "RTXM169-531",       0x1cf,       0x0,         0x0         },
    { 57,  "WTD",		        "RTXM167-431",	     0x1cf, 	  0x0,	       0x0         },
    { 58,  "SOURCEPHOTONICS",   "SPPS37GBOX3CDFD",   0x0,         0x0,         0xff000000  }, //support XEPON asym
    { 59,  "Hisense",           "LTE3468-BHG+",      0x10f,       0x0,         0x0         },
    { 60,  "Hisense",  			"LTF7219-BH+",       0x0,         0x0,         0xff000000  }, 
    { 61,  " ",                 " ",                 0x0,         0x0,         0x0         }, 
    { 62,  "ECONET",		    "EN7572",            0x14f, 	  0x9, 	       0xdf800000  }, //XEPON 7572 ONU 
    { 63,  "Hisense",           "LTE3468-BC+",       0x10f,       0x0,         0x0         },
    { 64,  "TSUHAN LTD.",       "THMPRS-3434-GBHI",  0x10f,       0x9,         0x9f000000  }, //GPON  
	{ 65,  "MENTECHOPTO",       "MSOG22-LD6I-ACNK",  0x14f,       0x9,         0x9f000000  }, //GPON 
	{ 66,  "Hisense",           "LTF7225-BH+",       0x0,         0x0,         0x9f000000  }, //XGSPON ONU 
	{ 67,  "AIROHA",			"AN8900",			 0x14f, 	  0x9,		   0xdf800000  },
	//add new optical module info above                                                        
    { PHY_TRANS_NOT_FOUND_IN_IOT_LIST,  "",  "",     0x0,         0x0,         0x0         }, //end of the list
};
#endif



#if XILINX_SERDES

PHY_TRANS_INFO phy_trans_iot_list[]={  
//   index,   vendor_id,         vendor_pn,   phy_csr_xpon_val, xgpon_phy_sfp_vld_level, xepon_pcs_sfp_status, en7581_fpga_xilinx_serdes_ctl  // en7581_fpga_xilinx_serdes_ctl only for 7581 xilinx serdes, contorl BEN(1fa84030[31])
    { 0 ,  "WTD",               "RTXM167-407-C10",   0x1cf,       0x0,         0x0         },
    { 1 ,  "WTD",               "RTXM169-504",       0x1cf,       0x0,         0x0         },
    { 2 ,  "WTD",               "RTXM169-514",       0x1cf,       0x0,         0x0         },
    { 3 ,  "SOURCEPHOTONICS",   "SFA3424THPCDFJ",    0x14f,       0x0,         0x0         },
    { 4 ,  "SOURCEPHOTONICS",   "SFU34GBP2CDFA",     0x1cf,       0x0,         0x0         },
    { 5 ,  "Hisense",           "LTB34D2-SC+",       0x1cf,       0x0,         0x0         },
    { 6 ,  "Hisense",           "LTB34D3-SC+",       0x1cf,       0x0,         0x0         },
    { 7 ,  "Hisense",           "LTE3468L-BHG+",     0x1cf,       0x0,         0x0,			0x80000000},
    { 8 ,  "PHYHOME",           "FHM-U2814-S3A20C",  0x14f,       0x0,         0x0         },
    { 9 ,  "PHYHOME",           "FHM-U1213-S3A20C",  0x1cf,       0x0,         0x0         },
    { 10,  "PHYHOME",           "FHM-U2814-U3A20C",  0x14f,       0x0,         0x0         },
    { 11,  "PHYHOME",           "FHM-U1213-U3A20C",  0x1cf,       0x0,         0x0         },
    { 12,  "PHYHOME",           "FHM-U2814-A3A20C",  0x14f,       0x0,         0x0         },
    { 13,  "PHYHOME",           "FHM-U1213-A3A20C",  0x1cf,       0x0,         0x0         },
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_PON_C7)                                      
    { 14,  "Hisense",           "LTB3468-BHG+",      0x10f,       0x0,         0x0         },
#else/*TCSUPPORT_COMPILE*/                                                                 
    { 14,  "Hisense",           "LTB3468-BHG+",      0x14f,       0x0,         0x0         },
#endif/*TCSUPPORT_COMPILE*/                                                                
    { 15,  "DARE TECH OPT",     "E31LA21B+",         0x1cf,       0x0,         0x0         },
    { 16,  "APAC Opto",         "LSF2C3STCN322",     0x14f,       0x0,         0x0         },
    { 17,  "APAC Opto",         "LSF2C3MTCN3G4",     0x14f,       0x0,         0x0         },
    { 18,  "MENTECHOPTO",       "NOG22-D6C-ND",      0x14f,       0x0,         0x0         },
    { 19,  "DELTA",             "OPEP-33-A4G1RC",    0x1cf,       0x0,         0x0         },
    { 20,  "MENTECHOPTO",       "NOE22-LD6C-NT",     0x1cf,       0x0,         0x0         }, /* epon */
    { 21,  "SUPERXON LTD.",     "SOEB3466-FSGF",     0x1cf,       0x0,         0x0         }, /* epon */
    { 22,  "EOPTOLINK INC",     "EOLF-GUA-25-DZTE",  0x14f,       0x0,         0x0         }, /* gpon */
    { 23,  "MENTECHOPTO",       "NOG22-D6C-ND",      0x14f,       0x0,         0x0         }, /* gpon */
    { 24,  "SUPERXON LTD.",     "SOGP3412-FSGA",     0x14f,       0x0,         0x0         }, /* gpon */
    { 25,  "SUPERXON LTD.",     "SOEB3466-FUGF",     0x1cf,       0x0,         0x0         },
    { 26,  "WTD",               "RTXM167-407",       0x1cf,       0x0,         0x0         },
    { 27,  "XUGUANG LTD.",      "XTEC3411F-C",       0x1cf,       0x0,         0x0         },/* jiuzhou epon */
    { 28,  "XUGUANG LTD.",      "XTGP3434FRS-A",     0x14f,       0x0,         0x0         },/* jiuzhou gpon */
    { 29,  "T&W",               "TW2362G-CDEH",      0x10f,       0x0,         0x0         },
    { 30,  "HUAWEI",            "HPSP2120",          0x1cf,       0x0,         0x0         },
    { 31,  "Hisense",           "LTE3468-BCG+",      0x10f,       0x0,         0x0         },
    { 32,  "Hisense",           "LTE34D3-SH+",       0x18f,       0x0,         0x0         },
    { 33,  "HONLUS",            "HOLSFP3420330C",    0x1cf,       0x0,         0x0         },
    { 34,  "HONLUS",            "HOLSP3412452DC0C",  0x1cf,       0x0,         0x0         },
    { 35,  "T&W",               "TW2362G-CDEH",      0x10f,       0x0,         0x0         },
    { 36,  "EOPTOLINK INC",     "EOLF-GEUA-25-D1A",  0x1cf,       0x0,         0x0         }, /* epon */
    { 37,  "Q-STAR",            "BYFS-462A4-HB-12",  0x14f,       0x0,         0x0         }, /* gpon */
    { 38,  "Q-STAR",            "BYFS-44234-LB-12",  0x1cf,       0x0,         0x0         }, /* epon */
    { 39,  "MENTECHOPTO",       "NOG22-D6C-SD",      0x14f,       0x0,         0x0         }, /* gpon */
    { 40,  "MENTECHOPTO",       "NOE22-LD6C-SD",     0x1cf,       0x0,         0x0         }, /* epon */
    { 41,  "MENTECHOPTO",       "NOG22-LD6C-ST",     0x14f,       0x0,         0x0         },
    { 42,  "MENTECHOPTO",       "NOE22-LD6C-ST",     0x1cf,       0x0,         0x0         },
    { 43,  "FIBERTOWER",        "FEU3411S-2DC-0C",   0x1cf,       0x0,         0x0         }, /* epon */
    { 44,  "FIBERTOWER",        "FGU3412S-2DC-1C",   0x14f,       0x0,         0x0         }, 
    { 45,  "ZKTEL",             "ZP4342034-KCST",    0x14f,       0x0,         0x0         }, 
//EN7580 new add
    { 46,  "Ligent Photonics",  "LTF7219-BH",        0x0,         0x0,         0xff000000  }, //XEPON symm may support XGSPON
    { 47,  "Hisense",           "LTF7215-BC+",       0x0,         0x0,         0xff800000  }, 
    { 48,  "Hisense",           "LTF7215-BH+",       0x0,         0x0,         0xff800000  }, 
    { 49,  "SOURCEPHOTONICS",   "SPPS27XER3CDFB",    0x0,         0x0,         0xff000000  }, //XEPON symm may support XGSPON
    { 50,  "NEOPHOTONICS",      "PTNGN1-42NP+",      0x0,         0xf,         0x0         },  
    { 51,  "Ligent Photonics",  "LTF7221-BC",        0x0,         0x0,         0x0         },  
    { 52,  "Hisense",           "LTF7221-BH+",       0x0,         0x0,         0x0         },  
    { 53,  "WTD",               "RTXM166-502",       0x0,         0x0,         0xdf000000  }, //XEPON symm  
    { 54,  "SOURCEPHOTONICS",   "SPPS2748FN2IDFB",   0x0,         0x0,         0x0         }, //XGPON OK
    { 55,  "SOURCEPHOTONICS",   "SPPS27XER3CDFD",    0x0,         0x0,         0xff000000  }, //XGSPON OK  support XEPON symm 
    { 56,  "WTD",               "RTXM169-531",       0x1cf,       0x0,         0x0         },
    { 57,  "WTD",		        "RTXM167-431",	     0x1cf, 	  0x0,	       0x0         },
    { 58,  "SOURCEPHOTONICS",   "SPPS37GBOX3CDFD",   0x0,         0x0,         0xff000000  }, //support XEPON asym
    { 59,  "Hisense",           "LTE3468-BHG+",      0x10f,       0x0,         0x0,					0x0}, //GPON  
    { 60,  "Hisense",  			"LTF7219-BH+",       0x0,         0x0,         0xff000000  }, 
    { 61,  " ",                 " ",                 0x0,         0x0,         0x0         }, 
    { 62,  "ECONET",		    "EN7572",            0x14f, 	  0x9, 	       0xdf800000  }, //XEPON 7572 ONU 
    { 63,  "Hisense",           "LTE3468-BC+",       0x10f,       0x0,         0x0         },
    { 64,  "TSUHAN LTD.",       "THMPRS-3434-GBHI",  0x10f,       0x9,         0x9f000000,			0x80000000}, //GPON  
	{ 65,  "MENTECHOPTO",       "MSOG22-LD6I-ACNK",  0x14f,       0x9,         0x9f000000  }, //GPON 
	{ 66,  "Hisense",           "LTF7225-BH+",       0x0,         0x0,         0x9f000000  }, //XGSPON ONU 
    { 67,  "Hisense",           "LTF7221-BH+",       0x0,         0x0,         0x0, 	   },
    { 68,  "TSUHAN LTD.",       "THMPRS-2677-GXAI",  0x0,        0x0,        	0x0, 	   }, //XGPON ONU 
    { 69,  "Potron",       		"SFP235E-GBC0",  	 0x18f,        0x0,        	0x0, 	   }, //GPON  SYM ONU 
	{ 70,  "AIROHA",			"AN8900",			 0x14f, 	  0x9,		   0xdf800000  },
    { 71,  "Hisense",           "LTF7225-BC+",         0x0,        0x0,        0x0}, //XGSPON ONU 

//add new optical module info above    
    { PHY_TRANS_NOT_FOUND_IN_IOT_LIST,  "",  "",     0x0,         0x0,         0x0         }, //end of the list
};
#endif
//#endif

static I2C_REG_ENTRY phy_trans_table_1[]={
//   id  ,  name                             ,  addr                        ,default ,current
   { 0   , "PHY_TRANS_IDENTIFIER"            , PHY_TRANS_IDENTIFIER              , 0 , 0 },
   { 1   , "PHY_TRANS_EXT_IDENTIFIER"        , PHY_TRANS_EXT_IDENTIFIER          , 0 , 0 },
   { 2   , "PHY_TRANS_CONNECTOR"             , PHY_TRANS_CONNECTOR               , 0 , 0 },
   { 3   , "PHY_TRANS_TRANSCEIVER"           , PHY_TRANS_TRANSCEIVER             , 0 , 0 },
   { 4   , "PHY_TRANS_TRANSCEIVER"           , 4                                 , 0 , 0 },
   { 5   , "PHY_TRANS_TRANSCEIVER"           , 5                                 , 0 , 0 },
   { 6   , "PHY_TRANS_TRANSCEIVER"           , 6                                 , 0 , 0 },
   { 7   , "PHY_TRANS_TRANSCEIVER"           , 7                                 , 0 , 0 },
   { 8   , "PHY_TRANS_TRANSCEIVER"           , 8                                 , 0 , 0 },
   { 9   , "PHY_TRANS_TRANSCEIVER"           , 9                                 , 0 , 0 },
   { 10  , "PHY_TRANS_TRANSCEIVER"           , 10                                , 0 , 0 },
   { 11  , "PHY_TRANS_ENCODING"              , PHY_TRANS_ENCODING                , 0 , 0 },
   { 12  , "PHY_TRANS_BR"                    , PHY_TRANS_BR                      , 0 , 0 },
   { 13  , "PHY_TRANS_RATE_IDENTIFIER"       , PHY_TRANS_RATE_IDENTIFIER         , 0 , 0 },
   { 14  , "PHY_TRANS_LENGTH_SMF_KM"         , PHY_TRANS_LENGTH_SMF_KM           , 0 , 0 },
   { 15  , "PHY_TRANS_LENGTH_SMF_100M"       , PHY_TRANS_LENGTH_SMF_100M         , 0 , 0 },
   { 16  , "PHY_TRANS_LENGTH_50UM_10M"       , PHY_TRANS_LENGTH_50UM_10M         , 0 , 0 },
   { 17  , "PHY_TRANS_LENGTH_62P5UM_10M"     , PHY_TRANS_LENGTH_62P5UM_10M       , 0 , 0 },
   { 18  , "PHY_TRANS_LENGTH_CABLE"          , PHY_TRANS_LENGTH_CABLE            , 0 , 0 },
   { 19  , "PHY_TRANS_LENGTH_OM3"            , PHY_TRANS_LENGTH_OM3              , 0 , 0 },
   { 20  , "PHY_TRANS_VENDOR_NAME"           , PHY_TRANS_VENDOR_NAME             , 0 , 0 },
   { 21  , "PHY_TRANS_VENDOR_NAME"           , 21                                , 0 , 0 },
   { 22  , "PHY_TRANS_VENDOR_NAME"           , 22                                , 0 , 0 },
   { 23  , "PHY_TRANS_VENDOR_NAME"           , 23                                , 0 , 0 },
   { 24  , "PHY_TRANS_VENDOR_NAME"           , 24                                , 0 , 0 },
   { 25  , "PHY_TRANS_VENDOR_NAME"           , 25                                , 0 , 0 },
   { 26  , "PHY_TRANS_VENDOR_NAME"           , 26                                , 0 , 0 },
   { 27  , "PHY_TRANS_VENDOR_NAME"           , 27                                , 0 , 0 },
   { 28  , "PHY_TRANS_VENDOR_NAME"           , 28                                , 0 , 0 },
   { 29  , "PHY_TRANS_VENDOR_NAME"           , 29                                , 0 , 0 },
   { 30  , "PHY_TRANS_VENDOR_NAME"           , 30                                , 0 , 0 },
   { 31  , "PHY_TRANS_VENDOR_NAME"           , 31                                , 0 , 0 },
   { 32  , "PHY_TRANS_VENDOR_NAME"           , 32                                , 0 , 0 },
   { 33  , "PHY_TRANS_VENDOR_NAME"           , 33                                , 0 , 0 },
   { 34  , "PHY_TRANS_VENDOR_NAME"           , 34                                , 0 , 0 },
   { 35  , "PHY_TRANS_VENDOR_NAME"           , 35                                , 0 , 0 },
   { 36  , "PHY_TRANS_TRANSCEIVER_36"        , PHY_TRANS_TRANSCEIVER_36          , 0 , 0 },
   { 37  , "PHY_TRANS_VENDOR_OUI"            , PHY_TRANS_VENDOR_OUI              , 0 , 0 },
   { 38  , "PHY_TRANS_VENDOR_OUI"            , 38                                , 0 , 0 },
   { 39  , "PHY_TRANS_VENDOR_OUI"            , 39                                , 0 , 0 },
   { 40  , "PHY_TRANS_VENDOR_PN"             , PHY_TRANS_VENDOR_PN               , 0 , 0 },
   { 41  , "PHY_TRANS_VENDOR_PN"             , 41                                , 0 , 0 },
   { 42  , "PHY_TRANS_VENDOR_PN"             , 42                                , 0 , 0 },
   { 43  , "PHY_TRANS_VENDOR_PN"             , 43                                , 0 , 0 },
   { 44  , "PHY_TRANS_VENDOR_PN"             , 44                                , 0 , 0 },
   { 45  , "PHY_TRANS_VENDOR_PN"             , 45                                , 0 , 0 },
   { 46  , "PHY_TRANS_VENDOR_PN"             , 46                                , 0 , 0 },
   { 47  , "PHY_TRANS_VENDOR_PN"             , 47                                , 0 , 0 },
   { 48  , "PHY_TRANS_VENDOR_PN"             , 48                                , 0 , 0 },
   { 49  , "PHY_TRANS_VENDOR_PN"             , 49                                , 0 , 0 },
   { 50  , "PHY_TRANS_VENDOR_PN"             , 50                                , 0 , 0 },
   { 51  , "PHY_TRANS_VENDOR_PN"             , 51                                , 0 , 0 },
   { 52  , "PHY_TRANS_VENDOR_PN"             , 52                                , 0 , 0 },
   { 53  , "PHY_TRANS_VENDOR_PN"             , 53                                , 0 , 0 },
   { 54  , "PHY_TRANS_VENDOR_PN"             , 54                                , 0 , 0 },
   { 55  , "PHY_TRANS_VENDOR_PN"             , 55                                , 0 , 0 },
   { 56  , "PHY_TRANS_VENDOR_REV"            , PHY_TRANS_VENDOR_REV              , 0 , 0 },
   { 57  , "PHY_TRANS_VENDOR_REV"            , 57                                , 0 , 0 },
   { 58  , "PHY_TRANS_VENDOR_REV"            , 58                                , 0 , 0 },
   { 59  , "PHY_TRANS_VENDOR_REV"            , 59                                , 0 , 0 },
   { 60  , "PHY_TRANS_WAVELENGTH"            , PHY_TRANS_WAVELENGTH              , 0 , 0 },
   { 61  , "PHY_TRANS_WAVELENGTH"            , 61                                , 0 , 0 },
   { 62  , "PHY_TRANS_UNALLOCATED"           , PHY_TRANS_UNALLOCATED             , 0 , 0 },
   { 63  , "PHY_TRANS_CC_BASE"               , PHY_TRANS_CC_BASE                 , 0 , 0 },
   { 64  , "PHY_TRANS_OPTIONS"               , PHY_TRANS_OPTIONS                 , 0 , 0 },
   { 65  , "PHY_TRANS_OPTIONS"               , 65                                , 0 , 0 },
   { 66  , "PHY_TRANS_BR_MAX"                , PHY_TRANS_BR_MAX                  , 0 , 0 },
   { 67  , "PHY_TRANS_BR_MIN"                , PHY_TRANS_BR_MIN                  , 0 , 0 },
   { 68  , "PHY_TRANS_VENDOR_SN"             , PHY_TRANS_VENDOR_SN               , 0 , 0 },
   { 69  , "PHY_TRANS_VENDOR_SN"             , 69                                , 0 , 0 },
   { 70  , "PHY_TRANS_VENDOR_SN"             , 70                                , 0 , 0 },
   { 71  , "PHY_TRANS_VENDOR_SN"             , 71                                , 0 , 0 },
   { 72  , "PHY_TRANS_VENDOR_SN"             , 72                                , 0 , 0 },
   { 73  , "PHY_TRANS_VENDOR_SN"             , 73                                , 0 , 0 },
   { 74  , "PHY_TRANS_VENDOR_SN"             , 74                                , 0 , 0 },
   { 75  , "PHY_TRANS_VENDOR_SN"             , 75                                , 0 , 0 },
   { 76  , "PHY_TRANS_VENDOR_SN"             , 76                                , 0 , 0 },
   { 77  , "PHY_TRANS_VENDOR_SN"             , 77                                , 0 , 0 },
   { 78  , "PHY_TRANS_VENDOR_SN"             , 78                                , 0 , 0 },
   { 79  , "PHY_TRANS_VENDOR_SN"             , 79                                , 0 , 0 },
   { 80  , "PHY_TRANS_VENDOR_SN"             , 80                                , 0 , 0 },
   { 81  , "PHY_TRANS_VENDOR_SN"             , 81                                , 0 , 0 },
   { 82  , "PHY_TRANS_VENDOR_SN"             , 82                                , 0 , 0 },
   { 83  , "PHY_TRANS_VENDOR_SN"             , 83                                , 0 , 0 },
   { 84  , "PHY_TRANS_DATE_CODE"             , PHY_TRANS_DATE_CODE               , 0 , 0 },
   { 85  , "PHY_TRANS_DATE_CODE"             , 85                                , 0 , 0 },
   { 86  , "PHY_TRANS_DATE_CODE"             , 86                                , 0 , 0 },
   { 87  , "PHY_TRANS_DATE_CODE"             , 87                                , 0 , 0 },
   { 88  , "PHY_TRANS_DATE_CODE"             , 88                                , 0 , 0 },
   { 89  , "PHY_TRANS_DATE_CODE"             , 89                                , 0 , 0 },
   { 90  , "PHY_TRANS_DATE_CODE"             , 90                                , 0 , 0 },
   { 91  , "PHY_TRANS_DATE_CODE"             , 91                                , 0 , 0 },
   { 92  , "PHY_TRANS_DIAG_MONITOR_TYPE"     , PHY_TRANS_DIAG_MONITOR_TYPE       , 0 , 0 },
   { 93  , "PHY_TRANS_ENHANCED_OPTIONS"      , PHY_TRANS_ENHANCED_OPTIONS        , 0 , 0 },
   { 94  , "PHY_TRANS_SFF_8472_COMPLIANCE"   , PHY_TRANS_SFF_8472_COMPLIANCE     , 0 , 0 },
   { 95  , "PHY_TRANS_CC_EXT"                , PHY_TRANS_CC_EXT                  , 0 , 0 },
   { 96  , "PHY_TRANS_VENDOR_SPECIFIC"       , PHY_TRANS_VENDOR_SPECIFIC_96      , 0 , 0 },
   { 97  , "PHY_TRANS_VENDOR_SPECIFIC"       , 97                                , 0 , 0 },
   { 98  , "PHY_TRANS_VENDOR_SPECIFIC"       , 98                                , 0 , 0 },
   { 99  , "PHY_TRANS_VENDOR_SPECIFIC"       , 99                                , 0 , 0 },
   { 100 , "PHY_TRANS_VENDOR_SPECIFIC"       , 100                               , 0 , 0 },
   { 101 , "PHY_TRANS_VENDOR_SPECIFIC"       , 101                               , 0 , 0 },
   { 102 , "PHY_TRANS_VENDOR_SPECIFIC"       , 102                               , 0 , 0 },
   { 103 , "PHY_TRANS_VENDOR_SPECIFIC"       , 103                               , 0 , 0 },
   { 104 , "PHY_TRANS_VENDOR_SPECIFIC"       , 104                               , 0 , 0 },
   { 105 , "PHY_TRANS_VENDOR_SPECIFIC"       , 105                               , 0 , 0 },
   { 106 , "PHY_TRANS_VENDOR_SPECIFIC"       , 106                               , 0 , 0 },
   { 107 , "PHY_TRANS_VENDOR_SPECIFIC"       , 107                               , 0 , 0 },
   { 108 , "PHY_TRANS_VENDOR_SPECIFIC"       , 108                               , 0 , 0 },
   { 109 , "PHY_TRANS_VENDOR_SPECIFIC"       , 109                               , 0 , 0 },
   { 110 , "PHY_TRANS_VENDOR_SPECIFIC"       , 110                               , 0 , 0 },
   { 111 , "PHY_TRANS_VENDOR_SPECIFIC"       , 111                               , 0 , 0 },
   { 112 , "PHY_TRANS_VENDOR_SPECIFIC"       , 112                               , 0 , 0 },
   { 113 , "PHY_TRANS_VENDOR_SPECIFIC"       , 113                               , 0 , 0 },
   { 114 , "PHY_TRANS_VENDOR_SPECIFIC"       , 114                               , 0 , 0 },
   { 115 , "PHY_TRANS_VENDOR_SPECIFIC"       , 115                               , 0 , 0 },
   { 116 , "PHY_TRANS_VENDOR_SPECIFIC"       , 116                               , 0 , 0 },
   { 117 , "PHY_TRANS_VENDOR_SPECIFIC"       , 117                               , 0 , 0 },
   { 118 , "PHY_TRANS_VENDOR_SPECIFIC"       , 118                               , 0 , 0 },
   { 119 , "PHY_TRANS_VENDOR_SPECIFIC"       , 119                               , 0 , 0 },
   { 120 , "PHY_TRANS_VENDOR_SPECIFIC"       , 120                               , 0 , 0 },
   { 121 , "PHY_TRANS_VENDOR_SPECIFIC"       , 121                               , 0 , 0 },
   { 122 , "PHY_TRANS_VENDOR_SPECIFIC"       , 122                               , 0 , 0 },
   { 123 , "PHY_TRANS_VENDOR_SPECIFIC"       , 123                               , 0 , 0 },
   { 124 , "PHY_TRANS_VENDOR_SPECIFIC"       , 124                               , 0 , 0 },
   { 125 , "PHY_TRANS_VENDOR_SPECIFIC"       , 125                               , 0 , 0 },
   { 126 , "PHY_TRANS_VENDOR_SPECIFIC"       , 126                               , 0 , 0 },
   { 127 , "PHY_TRANS_VENDOR_SPECIFIC"       , 127                               , 0 , 0 },
   { 128 , "PHY_TRANS_RESERVED"              , PHY_TRANS_RESERVED                , 0 , 0 },
   { 129 , "PHY_TRANS_RESERVED"              , 129                               , 0 , 0 },
   { 130 , "PHY_TRANS_RESERVED"              , 130                               , 0 , 0 },
   { 131 , "PHY_TRANS_RESERVED"              , 131                               , 0 , 0 },
   { 132 , "PHY_TRANS_RESERVED"              , 132                               , 0 , 0 },
   { 133 , "PHY_TRANS_RESERVED"              , 133                               , 0 , 0 },
   { 134 , "PHY_TRANS_RESERVED"              , 134                               , 0 , 0 },
   { 135 , "PHY_TRANS_RESERVED"              , 135                               , 0 , 0 },
   { 136 , "PHY_TRANS_RESERVED"              , 136                               , 0 , 0 },
   { 137 , "PHY_TRANS_RESERVED"              , 137                               , 0 , 0 },
   { 138 , "PHY_TRANS_RESERVED"              , 138                               , 0 , 0 },
   { 139 , "PHY_TRANS_RESERVED"              , 139                               , 0 , 0 },
   { 140 , "PHY_TRANS_RESERVED"              , 140                               , 0 , 0 },
   { 141 , "PHY_TRANS_RESERVED"              , 141                               , 0 , 0 },
   { 142 , "PHY_TRANS_RESERVED"              , 142                               , 0 , 0 },
   { 143 , "PHY_TRANS_RESERVED"              , 143                               , 0 , 0 },
   { 144 , "PHY_TRANS_RESERVED"              , 144                               , 0 , 0 },
   { 145 , "PHY_TRANS_RESERVED"              , 145                               , 0 , 0 },
   { 146 , "PHY_TRANS_RESERVED"              , 146                               , 0 , 0 },
   { 147 , "PHY_TRANS_RESERVED"              , 147                               , 0 , 0 },
   { 148 , "PHY_TRANS_RESERVED"              , 148                               , 0 , 0 },
   { 149 , "PHY_TRANS_RESERVED"              , 149                               , 0 , 0 },
   { 150 , "PHY_TRANS_RESERVED"              , 150                               , 0 , 0 },
   { 151 , "PHY_TRANS_RESERVED"              , 151                               , 0 , 0 },
   { 152 , "PHY_TRANS_RESERVED"              , 152                               , 0 , 0 },
   { 153 , "PHY_TRANS_RESERVED"              , 153                               , 0 , 0 },
   { 154 , "PHY_TRANS_RESERVED"              , 154                               , 0 , 0 },
   { 155 , "PHY_TRANS_RESERVED"              , 155                               , 0 , 0 },
   { 156 , "PHY_TRANS_RESERVED"              , 156                               , 0 , 0 },
   { 157 , "PHY_TRANS_RESERVED"              , 157                               , 0 , 0 },
   { 158 , "PHY_TRANS_RESERVED"              , 158                               , 0 , 0 },
   { 159 , "PHY_TRANS_RESERVED"              , 159                               , 0 , 0 },
   { 160 , "PHY_TRANS_RESERVED"              , 160                               , 0 , 0 },
   { 161 , "PHY_TRANS_RESERVED"              , 161                               , 0 , 0 },
   { 162 , "PHY_TRANS_RESERVED"              , 162                               , 0 , 0 },
   { 163 , "PHY_TRANS_RESERVED"              , 163                               , 0 , 0 },
   { 164 , "PHY_TRANS_RESERVED"              , 164                               , 0 , 0 },
   { 165 , "PHY_TRANS_RESERVED"              , 165                               , 0 , 0 },
   { 166 , "PHY_TRANS_RESERVED"              , 166                               , 0 , 0 },
   { 167 , "PHY_TRANS_RESERVED"              , 167                               , 0 , 0 },
   { 168 , "PHY_TRANS_RESERVED"              , 168                               , 0 , 0 },
   { 169 , "PHY_TRANS_RESERVED"              , 169                               , 0 , 0 },
   { 170 , "PHY_TRANS_RESERVED"              , 170                               , 0 , 0 },
   { 171 , "PHY_TRANS_RESERVED"              , 171                               , 0 , 0 },
   { 172 , "PHY_TRANS_RESERVED"              , 172                               , 0 , 0 },
   { 173 , "PHY_TRANS_RESERVED"              , 173                               , 0 , 0 },
   { 174 , "PHY_TRANS_RESERVED"              , 174                               , 0 , 0 },
   { 175 , "PHY_TRANS_RESERVED"              , 175                               , 0 , 0 },
   { 176 , "PHY_TRANS_RESERVED"              , 176                               , 0 , 0 },
   { 177 , "PHY_TRANS_RESERVED"              , 177                               , 0 , 0 },
   { 178 , "PHY_TRANS_RESERVED"              , 178                               , 0 , 0 },
   { 179 , "PHY_TRANS_RESERVED"              , 179                               , 0 , 0 },
   { 180 , "PHY_TRANS_RESERVED"              , 180                               , 0 , 0 },
   { 181 , "PHY_TRANS_RESERVED"              , 181                               , 0 , 0 },
   { 182 , "PHY_TRANS_RESERVED"              , 182                               , 0 , 0 },
   { 183 , "PHY_TRANS_RESERVED"              , 183                               , 0 , 0 },
   { 184 , "PHY_TRANS_RESERVED"              , 184                               , 0 , 0 },
   { 185 , "PHY_TRANS_RESERVED"              , 185                               , 0 , 0 },
   { 186 , "PHY_TRANS_RESERVED"              , 186                               , 0 , 0 },
   { 187 , "PHY_TRANS_RESERVED"              , 187                               , 0 , 0 },
   { 188 , "PHY_TRANS_RESERVED"              , 188                               , 0 , 0 },
   { 189 , "PHY_TRANS_RESERVED"              , 189                               , 0 , 0 },
   { 190 , "PHY_TRANS_RESERVED"              , 190                               , 0 , 0 },
   { 191 , "PHY_TRANS_RESERVED"              , 191                               , 0 , 0 },
   { 192 , "PHY_TRANS_RESERVED"              , 192                               , 0 , 0 },
   { 193 , "PHY_TRANS_RESERVED"              , 193                               , 0 , 0 },
   { 194 , "PHY_TRANS_RESERVED"              , 194                               , 0 , 0 },
   { 195 , "PHY_TRANS_RESERVED"              , 195                               , 0 , 0 },
   { 196 , "PHY_TRANS_RESERVED"              , 196                               , 0 , 0 },
   { 197 , "PHY_TRANS_RESERVED"              , 197                               , 0 , 0 },
   { 198 , "PHY_TRANS_RESERVED"              , 198                               , 0 , 0 },
   { 199 , "PHY_TRANS_RESERVED"              , 199                               , 0 , 0 },
   { 200 , "PHY_TRANS_RESERVED"              , 200                               , 0 , 0 },
   { 201 , "PHY_TRANS_RESERVED"              , 201                               , 0 , 0 },
   { 202 , "PHY_TRANS_RESERVED"              , 202                               , 0 , 0 },
   { 203 , "PHY_TRANS_RESERVED"              , 203                               , 0 , 0 },
   { 204 , "PHY_TRANS_RESERVED"              , 204                               , 0 , 0 },
   { 205 , "PHY_TRANS_RESERVED"              , 205                               , 0 , 0 },
   { 206 , "PHY_TRANS_RESERVED"              , 206                               , 0 , 0 },
   { 207 , "PHY_TRANS_RESERVED"              , 207                               , 0 , 0 },
   { 208 , "PHY_TRANS_RESERVED"              , 208                               , 0 , 0 },
   { 209 , "PHY_TRANS_RESERVED"              , 209                               , 0 , 0 },
   { 210 , "PHY_TRANS_RESERVED"              , 210                               , 0 , 0 },
   { 211 , "PHY_TRANS_RESERVED"              , 211                               , 0 , 0 },
   { 212 , "PHY_TRANS_RESERVED"              , 212                               , 0 , 0 },
   { 213 , "PHY_TRANS_RESERVED"              , 213                               , 0 , 0 },
   { 214 , "PHY_TRANS_RESERVED"              , 214                               , 0 , 0 },
   { 215 , "PHY_TRANS_RESERVED"              , 215                               , 0 , 0 },
   { 216 , "PHY_TRANS_RESERVED"              , 216                               , 0 , 0 },
   { 217 , "PHY_TRANS_RESERVED"              , 217                               , 0 , 0 },
   { 218 , "PHY_TRANS_RESERVED"              , 218                               , 0 , 0 },
   { 219 , "PHY_TRANS_RESERVED"              , 219                               , 0 , 0 },
   { 220 , "PHY_TRANS_RESERVED"              , 220                               , 0 , 0 },
   { 221 , "PHY_TRANS_RESERVED"              , 221                               , 0 , 0 },
   { 222 , "PHY_TRANS_RESERVED"              , 222                               , 0 , 0 },
   { 223 , "PHY_TRANS_RESERVED"              , 223                               , 0 , 0 },
   { 224 , "PHY_TRANS_RESERVED"              , 224                               , 0 , 0 },
   { 225 , "PHY_TRANS_RESERVED"              , 225                               , 0 , 0 },
   { 226 , "PHY_TRANS_RESERVED"              , 226                               , 0 , 0 },
   { 227 , "PHY_TRANS_RESERVED"              , 227                               , 0 , 0 },
   { 228 , "PHY_TRANS_RESERVED"              , 228                               , 0 , 0 },
   { 229 , "PHY_TRANS_RESERVED"              , 229                               , 0 , 0 },
   { 230 , "PHY_TRANS_RESERVED"              , 230                               , 0 , 0 },
   { 231 , "PHY_TRANS_RESERVED"              , 231                               , 0 , 0 },
   { 232 , "PHY_TRANS_RESERVED"              , 232                               , 0 , 0 },
   { 233 , "PHY_TRANS_RESERVED"              , 233                               , 0 , 0 },
   { 234 , "PHY_TRANS_RESERVED"              , 234                               , 0 , 0 },
   { 235 , "PHY_TRANS_RESERVED"              , 235                               , 0 , 0 },
   { 236 , "PHY_TRANS_RESERVED"              , 236                               , 0 , 0 },
   { 237 , "PHY_TRANS_RESERVED"              , 237                               , 0 , 0 },
   { 238 , "PHY_TRANS_RESERVED"              , 238                               , 0 , 0 },
   { 239 , "PHY_TRANS_RESERVED"              , 239                               , 0 , 0 },
   { 240 , "PHY_TRANS_RESERVED"              , 240                               , 0 , 0 },
   { 241 , "PHY_TRANS_RESERVED"              , 241                               , 0 , 0 },
   { 242 , "PHY_TRANS_RESERVED"              , 242                               , 0 , 0 },
   { 243 , "PHY_TRANS_RESERVED"              , 243                               , 0 , 0 },
   { 244 , "PHY_TRANS_RESERVED"              , 244                               , 0 , 0 },
   { 245 , "PHY_TRANS_RESERVED"              , 245                               , 0 , 0 },
   { 246 , "PHY_TRANS_RESERVED"              , 246                               , 0 , 0 },
   { 247 , "PHY_TRANS_RESERVED"              , 247                               , 0 , 0 },
   { 248 , "PHY_TRANS_RESERVED"              , 248                               , 0 , 0 },
   { 249 , "PHY_TRANS_RESERVED"              , 249                               , 0 , 0 },
   { 250 , "PHY_TRANS_RESERVED"              , 250                               , 0 , 0 },
   { 251 , "PHY_TRANS_RESERVED"              , 251                               , 0 , 0 },
   { 252 , "PHY_TRANS_RESERVED"              , 252                               , 0 , 0 },
   { 253 , "PHY_TRANS_RESERVED"              , 253                               , 0 , 0 },
   { 254 , "PHY_TRANS_RESERVED"              , 254                               , 0 , 0 },
   { 255 , "PHY_TRANS_RESERVED"              , 255                               , 0 , 0 }
};   

static I2C_REG_ENTRY phy_trans_table_2[]={
//   id  ,  name                             ,  addr                        ,default ,current
   { 0   , "PHY_TRANS_A_W_THRESHOLDS"      , PHY_TRANS_A_W_THRESHOLDS       , 0 , 0 },
   { 1   , "PHY_TRANS_A_W_THRESHOLDS"      , 1                              , 0 , 0 },
   { 2   , "PHY_TRANS_A_W_THRESHOLDS"      , 2                              , 0 , 0 },
   { 3   , "PHY_TRANS_A_W_THRESHOLDS"      , 3                              , 0 , 0 },
   { 4   , "PHY_TRANS_A_W_THRESHOLDS"      , 4                              , 0 , 0 },
   { 5   , "PHY_TRANS_A_W_THRESHOLDS"      , 5                              , 0 , 0 },
   { 6   , "PHY_TRANS_A_W_THRESHOLDS"      , 6                              , 0 , 0 },
   { 7   , "PHY_TRANS_A_W_THRESHOLDS"      , 7                              , 0 , 0 },
   { 8   , "PHY_TRANS_A_W_THRESHOLDS"      , 8                              , 0 , 0 },
   { 9   , "PHY_TRANS_A_W_THRESHOLDS"      , 9                              , 0 , 0 },
   { 10  , "PHY_TRANS_A_W_THRESHOLDS"      , 10                             , 0 , 0 },
   { 11  , "PHY_TRANS_A_W_THRESHOLDS"      , 11                             , 0 , 0 },
   { 12  , "PHY_TRANS_A_W_THRESHOLDS"      , 12                             , 0 , 0 },
   { 13  , "PHY_TRANS_A_W_THRESHOLDS"      , 13                             , 0 , 0 },
   { 14  , "PHY_TRANS_A_W_THRESHOLDS"      , 14                             , 0 , 0 },
   { 15  , "PHY_TRANS_A_W_THRESHOLDS"      , 15                             , 0 , 0 },
   { 16  , "PHY_TRANS_A_W_THRESHOLDS"      , 16                             , 0 , 0 },
   { 17  , "PHY_TRANS_A_W_THRESHOLDS"      , 17                             , 0 , 0 },
   { 18  , "PHY_TRANS_A_W_THRESHOLDS"      , 18                             , 0 , 0 },
   { 19  , "PHY_TRANS_A_W_THRESHOLDS"      , 19                             , 0 , 0 },
   { 20  , "PHY_TRANS_A_W_THRESHOLDS"      , 20                             , 0 , 0 },
   { 21  , "PHY_TRANS_A_W_THRESHOLDS"      , 21                             , 0 , 0 },
   { 22  , "PHY_TRANS_A_W_THRESHOLDS"      , 22                             , 0 , 0 },
   { 23  , "PHY_TRANS_A_W_THRESHOLDS"      , 23                             , 0 , 0 },
   { 24  , "PHY_TRANS_A_W_THRESHOLDS"      , 24                             , 0 , 0 },
   { 25  , "PHY_TRANS_A_W_THRESHOLDS"      , 25                             , 0 , 0 },
   { 26  , "PHY_TRANS_A_W_THRESHOLDS"      , 26                             , 0 , 0 },
   { 27  , "PHY_TRANS_A_W_THRESHOLDS"      , 27                             , 0 , 0 },
   { 28  , "PHY_TRANS_A_W_THRESHOLDS"      , 28                             , 0 , 0 },
   { 29  , "PHY_TRANS_A_W_THRESHOLDS"      , 29                             , 0 , 0 },
   { 30  , "PHY_TRANS_A_W_THRESHOLDS"      , 30                             , 0 , 0 },
   { 31  , "PHY_TRANS_A_W_THRESHOLDS"      , 31                             , 0 , 0 },
   { 32  , "PHY_TRANS_A_W_THRESHOLDS"      , 32                             , 0 , 0 },
   { 33  , "PHY_TRANS_A_W_THRESHOLDS"      , 33                             , 0 , 0 },
   { 34  , "PHY_TRANS_A_W_THRESHOLDS"      , 34                             , 0 , 0 },
   { 35  , "PHY_TRANS_A_W_THRESHOLDS"      , 35                             , 0 , 0 },
   { 36  , "PHY_TRANS_A_W_THRESHOLDS"      , 36                             , 0 , 0 },
   { 37  , "PHY_TRANS_A_W_THRESHOLDS"      , 37                             , 0 , 0 },
   { 38  , "PHY_TRANS_A_W_THRESHOLDS"      , 38                             , 0 , 0 },
   { 39  , "PHY_TRANS_A_W_THRESHOLDS"      , 39                             , 0 , 0 },
   { 40  , "PHY_TRANS_UNALLOCATED_2"       , PHY_TRANS_UNALLOCATED_2        , 0 , 0 },
   { 41  , "PHY_TRANS_UNALLOCATED_2"       , 41                             , 0 , 0 },
   { 42  , "PHY_TRANS_UNALLOCATED_2"       , 42                             , 0 , 0 },
   { 43  , "PHY_TRANS_UNALLOCATED_2"       , 43                             , 0 , 0 },
   { 44  , "PHY_TRANS_UNALLOCATED_2"       , 44                             , 0 , 0 },
   { 45  , "PHY_TRANS_UNALLOCATED_2"       , 45                             , 0 , 0 },
   { 46  , "PHY_TRANS_UNALLOCATED_2"       , 46                             , 0 , 0 },
   { 47  , "PHY_TRANS_UNALLOCATED_2"       , 47                             , 0 , 0 },
   { 48  , "PHY_TRANS_UNALLOCATED_2"       , 48                             , 0 , 0 },
   { 49  , "PHY_TRANS_UNALLOCATED_2"       , 49                             , 0 , 0 },
   { 50  , "PHY_TRANS_UNALLOCATED_2"       , 50                             , 0 , 0 },
   { 51  , "PHY_TRANS_UNALLOCATED_2"       , 51                             , 0 , 0 },
   { 52  , "PHY_TRANS_UNALLOCATED_2"       , 52                             , 0 , 0 },
   { 53  , "PHY_TRANS_UNALLOCATED_2"       , 53                             , 0 , 0 },
   { 54  , "PHY_TRANS_UNALLOCATED_2"       , 54                             , 0 , 0 },
   { 55  , "PHY_TRANS_UNALLOCATED_2"       , 55                             , 0 , 0 },
   { 56  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , PHY_TRANS_EXT_CAL_CONSTANTS    , 0 , 0 },
   { 57  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 57                             , 0 , 0 },
   { 58  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 58                             , 0 , 0 },
   { 59  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 59                             , 0 , 0 },
   { 60  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 60                             , 0 , 0 },
   { 61  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 61                             , 0 , 0 },
   { 62  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 62                             , 0 , 0 },
   { 63  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 63                             , 0 , 0 },
   { 64  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 64                             , 0 , 0 },
   { 65  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 65                             , 0 , 0 },
   { 66  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 66                             , 0 , 0 },
   { 67  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 67                             , 0 , 0 },
   { 68  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 68                             , 0 , 0 },
   { 69  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 69                             , 0 , 0 },
   { 70  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 70                             , 0 , 0 },
   { 71  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 71                             , 0 , 0 },
   { 72  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 72                             , 0 , 0 },
   { 73  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 73                             , 0 , 0 },
   { 74  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 74                             , 0 , 0 },
   { 75  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 75                             , 0 , 0 },
   { 76  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 76                             , 0 , 0 },
   { 77  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 77                             , 0 , 0 },
   { 78  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 78                             , 0 , 0 },
   { 79  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 79                             , 0 , 0 },
   { 80  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 80                             , 0 , 0 },
   { 81  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 81                             , 0 , 0 },
   { 82  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 82                             , 0 , 0 },
   { 83  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 83                             , 0 , 0 },
   { 84  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 84                             , 0 , 0 },
   { 85  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 85                             , 0 , 0 },
   { 86  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 86                             , 0 , 0 },
   { 87  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 87                             , 0 , 0 },
   { 88  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 88                             , 0 , 0 },
   { 89  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 89                             , 0 , 0 },
   { 90  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 90                             , 0 , 0 },
   { 91  , "PHY_TRANS_EXT_CAL_CONSTANTS"   , 91                             , 0 , 0 },
   { 92  , "PHY_TRANS_UNALLOCATED_3"       , PHY_TRANS_UNALLOCATED_3        , 0 , 0 },
   { 93  , "PHY_TRANS_UNALLOCATED_3"       , 93                             , 0 , 0 },
   { 94  , "PHY_TRANS_UNALLOCATED_3"       , 94                             , 0 , 0 },
   { 95  , "PHY_TRANS_CC_DMI"              , PHY_TRANS_CC_DMI               , 0 , 0 },
   { 96  , "PHY_TRANS_DIAGNOSTICS"         , PHY_TRANS_DIAGNOSTICS          , 0 , 0 },
   { 97  , "PHY_TRANS_DIAGNOSTICS"         , 97                             , 0 , 0 },
   { 98  , "PHY_TRANS_DIAGNOSTICS"         , 98                             , 0 , 0 },
   { 99  , "PHY_TRANS_DIAGNOSTICS"         , 99                             , 0 , 0 },
   { 100 , "PHY_TRANS_DIAGNOSTICS"         , 100                            , 0 , 0 },
   { 101 , "PHY_TRANS_DIAGNOSTICS"         , 101                            , 0 , 0 },
   { 102 , "PHY_TRANS_DIAGNOSTICS"         , 102                            , 0 , 0 },
   { 103 , "PHY_TRANS_DIAGNOSTICS"         , 103                            , 0 , 0 },
   { 104 , "PHY_TRANS_DIAGNOSTICS"         , 104                            , 0 , 0 },
   { 105 , "PHY_TRANS_DIAGNOSTICS"         , 105                            , 0 , 0 },
   { 106 , "PHY_TRANS_UNALLOCATED_4"       , PHY_TRANS_UNALLOCATED_4        , 0 , 0 },
   { 107 , "PHY_TRANS_UNALLOCATED_4"       , 107                            , 0 , 0 },
   { 108 , "PHY_TRANS_UNALLOCATED_4"       , 108                            , 0 , 0 },
   { 109 , "PHY_TRANS_UNALLOCATED_4"       , 109                            , 0 , 0 },
   { 110 , "PHY_TRANS_STATUS_CONTROL"      , PHY_TRANS_STATUS_CONTROL       , 0 , 0 },
   { 111 , "PHY_TRANS_RESERVED_2"          , PHY_TRANS_RESERVED_2           , 0 , 0 },
   { 112 , "PHY_TRANS_ALARM_FLAGS"         , PHY_TRANS_ALARM_FLAGS          , 0 , 0 },
   { 113 , "PHY_TRANS_ALARM_FLAGS_2"       , PHY_TRANS_ALARM_FLAGS_2        , 0 , 0 },
   { 114 , "PHY_TRANS_UNALLOCATED_5"       , PHY_TRANS_UNALLOCATED_5        , 0 , 0 },
   { 115 , "PHY_TRANS_UNALLOCATED_5"       , 115                            , 0 , 0 },
   { 116 , "PHY_TRANS_WARNING_FLAGS"       , PHY_TRANS_WARNING_FLAGS        , 0 , 0 },
   { 117 , "PHY_TRANS_WARNING_FLAGS_2"     , PHY_TRANS_WARNING_FLAGS_2      , 0 , 0 },
   { 118 , "PHY_TRANS_EXT_STATUS_CONTROL"  , PHY_TRANS_EXT_STATUS_CONTROL   , 0 , 0 },
   { 119 , "PHY_TRANS_EXT_STATUS_CONTROL"  , 119                            , 0 , 0 },
   { 120 , "PHY_TRANS_VENDOR_SPECIFIC"     , PHY_TRANS_VENDOR_SPECIFIC_120  , 0 , 0 },
   { 121 , "PHY_TRANS_VENDOR_SPECIFIC"     , 121                            , 0 , 0 },
   { 122 , "PHY_TRANS_VENDOR_SPECIFIC"     , 122                            , 0 , 0 },
   { 123 , "PHY_TRANS_VENDOR_SPECIFIC"     , 123                            , 0 , 0 },
   { 124 , "PHY_TRANS_VENDOR_SPECIFIC"     , 124                            , 0 , 0 },
   { 125 , "PHY_TRANS_VENDOR_SPECIFIC"     , 125                            , 0 , 0 },
   { 126 , "PHY_TRANS_VENDOR_SPECIFIC"     , 126                            , 0 , 0 },
   { 127 , "PHY_TRANS_VENDOR_SPECIFIC"     , 127                            , 0 , 0 },
   { 128 , "PHY_TRANS_USER_EEPROM"         , PHY_TRANS_USER_EEPROM          , 0 , 0 },
   { 129 , "PHY_TRANS_USER_EEPROM"         , 129                            , 0 , 0 },
   { 130 , "PHY_TRANS_USER_EEPROM"         , 130                            , 0 , 0 },
   { 131 , "PHY_TRANS_USER_EEPROM"         , 131                            , 0 , 0 },
   { 132 , "PHY_TRANS_USER_EEPROM"         , 132                            , 0 , 0 },
   { 133 , "PHY_TRANS_USER_EEPROM"         , 133                            , 0 , 0 },
   { 134 , "PHY_TRANS_USER_EEPROM"         , 134                            , 0 , 0 },
   { 135 , "PHY_TRANS_USER_EEPROM"         , 135                            , 0 , 0 },
   { 136 , "PHY_TRANS_USER_EEPROM"         , 136                            , 0 , 0 },
   { 137 , "PHY_TRANS_USER_EEPROM"         , 137                            , 0 , 0 },
   { 138 , "PHY_TRANS_USER_EEPROM"         , 138                            , 0 , 0 },
   { 139 , "PHY_TRANS_USER_EEPROM"         , 139                            , 0 , 0 },
   { 140 , "PHY_TRANS_USER_EEPROM"         , 140                            , 0 , 0 },
   { 141 , "PHY_TRANS_USER_EEPROM"         , 141                            , 0 , 0 },
   { 142 , "PHY_TRANS_USER_EEPROM"         , 142                            , 0 , 0 },
   { 143 , "PHY_TRANS_USER_EEPROM"         , 143                            , 0 , 0 },
   { 144 , "PHY_TRANS_USER_EEPROM"         , 144                            , 0 , 0 },
   { 145 , "PHY_TRANS_USER_EEPROM"         , 145                            , 0 , 0 },
   { 146 , "PHY_TRANS_USER_EEPROM"         , 146                            , 0 , 0 },
   { 147 , "PHY_TRANS_USER_EEPROM"         , 147                            , 0 , 0 },
   { 148 , "PHY_TRANS_USER_EEPROM"         , 148                            , 0 , 0 },
   { 149 , "PHY_TRANS_USER_EEPROM"         , 149                            , 0 , 0 },
   { 150 , "PHY_TRANS_USER_EEPROM"         , 150                            , 0 , 0 },
   { 151 , "PHY_TRANS_USER_EEPROM"         , 151                            , 0 , 0 },
   { 152 , "PHY_TRANS_USER_EEPROM"         , 152                            , 0 , 0 },
   { 153 , "PHY_TRANS_USER_EEPROM"         , 153                            , 0 , 0 },
   { 154 , "PHY_TRANS_USER_EEPROM"         , 154                            , 0 , 0 },
   { 155 , "PHY_TRANS_USER_EEPROM"         , 155                            , 0 , 0 },
   { 156 , "PHY_TRANS_USER_EEPROM"         , 156                            , 0 , 0 },
   { 157 , "PHY_TRANS_USER_EEPROM"         , 157                            , 0 , 0 },
   { 158 , "PHY_TRANS_USER_EEPROM"         , 158                            , 0 , 0 },
   { 159 , "PHY_TRANS_USER_EEPROM"         , 159                            , 0 , 0 },
   { 160 , "PHY_TRANS_USER_EEPROM"         , 160                            , 0 , 0 },
   { 161 , "PHY_TRANS_USER_EEPROM"         , 161                            , 0 , 0 },
   { 162 , "PHY_TRANS_USER_EEPROM"         , 162                            , 0 , 0 },
   { 163 , "PHY_TRANS_USER_EEPROM"         , 163                            , 0 , 0 },
   { 164 , "PHY_TRANS_USER_EEPROM"         , 164                            , 0 , 0 },
   { 165 , "PHY_TRANS_USER_EEPROM"         , 165                            , 0 , 0 },
   { 166 , "PHY_TRANS_USER_EEPROM"         , 166                            , 0 , 0 },
   { 167 , "PHY_TRANS_USER_EEPROM"         , 167                            , 0 , 0 },
   { 168 , "PHY_TRANS_USER_EEPROM"         , 168                            , 0 , 0 },
   { 169 , "PHY_TRANS_USER_EEPROM"         , 169                            , 0 , 0 },
   { 170 , "PHY_TRANS_USER_EEPROM"         , 170                            , 0 , 0 },
   { 171 , "PHY_TRANS_USER_EEPROM"         , 171                            , 0 , 0 },
   { 172 , "PHY_TRANS_USER_EEPROM"         , 172                            , 0 , 0 },
   { 173 , "PHY_TRANS_USER_EEPROM"         , 173                            , 0 , 0 },
   { 174 , "PHY_TRANS_USER_EEPROM"         , 174                            , 0 , 0 },
   { 175 , "PHY_TRANS_USER_EEPROM"         , 175                            , 0 , 0 },
   { 176 , "PHY_TRANS_USER_EEPROM"         , 176                            , 0 , 0 },
   { 177 , "PHY_TRANS_USER_EEPROM"         , 177                            , 0 , 0 },
   { 178 , "PHY_TRANS_USER_EEPROM"         , 178                            , 0 , 0 },
   { 179 , "PHY_TRANS_USER_EEPROM"         , 179                            , 0 , 0 },
   { 180 , "PHY_TRANS_USER_EEPROM"         , 180                            , 0 , 0 },
   { 181 , "PHY_TRANS_USER_EEPROM"         , 181                            , 0 , 0 },
   { 182 , "PHY_TRANS_USER_EEPROM"         , 182                            , 0 , 0 },
   { 183 , "PHY_TRANS_USER_EEPROM"         , 183                            , 0 , 0 },
   { 184 , "PHY_TRANS_USER_EEPROM"         , 184                            , 0 , 0 },
   { 185 , "PHY_TRANS_USER_EEPROM"         , 185                            , 0 , 0 },
   { 186 , "PHY_TRANS_USER_EEPROM"         , 186                            , 0 , 0 },
   { 187 , "PHY_TRANS_USER_EEPROM"         , 187                            , 0 , 0 },
   { 188 , "PHY_TRANS_USER_EEPROM"         , 188                            , 0 , 0 },
   { 189 , "PHY_TRANS_USER_EEPROM"         , 189                            , 0 , 0 },
   { 190 , "PHY_TRANS_USER_EEPROM"         , 190                            , 0 , 0 },
   { 191 , "PHY_TRANS_USER_EEPROM"         , 191                            , 0 , 0 },
   { 192 , "PHY_TRANS_USER_EEPROM"         , 192                            , 0 , 0 },
   { 193 , "PHY_TRANS_USER_EEPROM"         , 193                            , 0 , 0 },
   { 194 , "PHY_TRANS_USER_EEPROM"         , 194                            , 0 , 0 },
   { 195 , "PHY_TRANS_USER_EEPROM"         , 195                            , 0 , 0 },
   { 196 , "PHY_TRANS_USER_EEPROM"         , 196                            , 0 , 0 },
   { 197 , "PHY_TRANS_USER_EEPROM"         , 197                            , 0 , 0 },
   { 198 , "PHY_TRANS_USER_EEPROM"         , 198                            , 0 , 0 },
   { 199 , "PHY_TRANS_USER_EEPROM"         , 199                            , 0 , 0 },
   { 200 , "PHY_TRANS_USER_EEPROM"         , 200                            , 0 , 0 },
   { 201 , "PHY_TRANS_USER_EEPROM"         , 201                            , 0 , 0 },
   { 202 , "PHY_TRANS_USER_EEPROM"         , 202                            , 0 , 0 },
   { 203 , "PHY_TRANS_USER_EEPROM"         , 203                            , 0 , 0 },
   { 204 , "PHY_TRANS_USER_EEPROM"         , 204                            , 0 , 0 },
   { 205 , "PHY_TRANS_USER_EEPROM"         , 205                            , 0 , 0 },
   { 206 , "PHY_TRANS_USER_EEPROM"         , 206                            , 0 , 0 },
   { 207 , "PHY_TRANS_USER_EEPROM"         , 207                            , 0 , 0 },
   { 208 , "PHY_TRANS_USER_EEPROM"         , 208                            , 0 , 0 },
   { 209 , "PHY_TRANS_USER_EEPROM"         , 209                            , 0 , 0 },
   { 210 , "PHY_TRANS_USER_EEPROM"         , 210                            , 0 , 0 },
   { 211 , "PHY_TRANS_USER_EEPROM"         , 211                            , 0 , 0 },
   { 212 , "PHY_TRANS_USER_EEPROM"         , 212                            , 0 , 0 },
   { 213 , "PHY_TRANS_USER_EEPROM"         , 213                            , 0 , 0 },
   { 214 , "PHY_TRANS_USER_EEPROM"         , 214                            , 0 , 0 },
   { 215 , "PHY_TRANS_USER_EEPROM"         , 215                            , 0 , 0 },
   { 216 , "PHY_TRANS_USER_EEPROM"         , 216                            , 0 , 0 },
   { 217 , "PHY_TRANS_USER_EEPROM"         , 217                            , 0 , 0 },
   { 218 , "PHY_TRANS_USER_EEPROM"         , 218                            , 0 , 0 },
   { 219 , "PHY_TRANS_USER_EEPROM"         , 219                            , 0 , 0 },
   { 220 , "PHY_TRANS_USER_EEPROM"         , 220                            , 0 , 0 },
   { 221 , "PHY_TRANS_USER_EEPROM"         , 221                            , 0 , 0 },
   { 222 , "PHY_TRANS_USER_EEPROM"         , 222                            , 0 , 0 },
   { 223 , "PHY_TRANS_USER_EEPROM"         , 223                            , 0 , 0 },
   { 224 , "PHY_TRANS_USER_EEPROM"         , 224                            , 0 , 0 },
   { 225 , "PHY_TRANS_USER_EEPROM"         , 225                            , 0 , 0 },
   { 226 , "PHY_TRANS_USER_EEPROM"         , 226                            , 0 , 0 },
   { 227 , "PHY_TRANS_USER_EEPROM"         , 227                            , 0 , 0 },
   { 228 , "PHY_TRANS_USER_EEPROM"         , 228                            , 0 , 0 },
   { 229 , "PHY_TRANS_USER_EEPROM"         , 229                            , 0 , 0 },
   { 230 , "PHY_TRANS_USER_EEPROM"         , 230                            , 0 , 0 },
   { 231 , "PHY_TRANS_USER_EEPROM"         , 231                            , 0 , 0 },
   { 232 , "PHY_TRANS_USER_EEPROM"         , 232                            , 0 , 0 },
   { 233 , "PHY_TRANS_USER_EEPROM"         , 233                            , 0 , 0 },
   { 234 , "PHY_TRANS_USER_EEPROM"         , 234                            , 0 , 0 },
   { 235 , "PHY_TRANS_USER_EEPROM"         , 235                            , 0 , 0 },
   { 236 , "PHY_TRANS_USER_EEPROM"         , 236                            , 0 , 0 },
   { 237 , "PHY_TRANS_USER_EEPROM"         , 237                            , 0 , 0 },
   { 238 , "PHY_TRANS_USER_EEPROM"         , 238                            , 0 , 0 },
   { 239 , "PHY_TRANS_USER_EEPROM"         , 239                            , 0 , 0 },
   { 240 , "PHY_TRANS_USER_EEPROM"         , 240                            , 0 , 0 },
   { 241 , "PHY_TRANS_USER_EEPROM"         , 241                            , 0 , 0 },
   { 242 , "PHY_TRANS_USER_EEPROM"         , 242                            , 0 , 0 },
   { 243 , "PHY_TRANS_USER_EEPROM"         , 243                            , 0 , 0 },
   { 244 , "PHY_TRANS_USER_EEPROM"         , 244                            , 0 , 0 },
   { 245 , "PHY_TRANS_USER_EEPROM"         , 245                            , 0 , 0 },
   { 246 , "PHY_TRANS_USER_EEPROM"         , 246                            , 0 , 0 },
   { 247 , "PHY_TRANS_USER_EEPROM"         , 247                            , 0 , 0 },
   { 248 , "PHY_TRANS_VENDOR_CONTROL"      , PHY_TRANS_VENDOR_CONTROL       , 0 , 0 },
   { 249 , "PHY_TRANS_VENDOR_CONTROL"      , 249                            , 0 , 0 },
   { 250 , "PHY_TRANS_VENDOR_CONTROL"      , 250                            , 0 , 0 },
   { 251 , "PHY_TRANS_VENDOR_CONTROL"      , 251                            , 0 , 0 },
   { 252 , "PHY_TRANS_VENDOR_CONTROL"      , 252                            , 0 , 0 },
   { 253 , "PHY_TRANS_VENDOR_CONTROL"      , 253                            , 0 , 0 },
   { 254 , "PHY_TRANS_VENDOR_CONTROL"      , 254                            , 0 , 0 },
   { 255 , "PHY_TRANS_VENDOR_CONTROL"      , 255                            , 0 , 0 } 
};

//trans_info for XFP module

#if ASIC_SERDES
XFP_TRANS_INFO xfp_trans_iot_list[]={  
//   index,    vendor name,                vendor pn,                        gepon,           xgpon,               xepon,             pma0,           pma1		xfp module pass word
	{ 0,  "Hisense",           "LTW2601D-BC+",        0x0,        0x9,        0x9f000000,  0x10101,    0x1010100, 		0x12345678}, //NGPON2 XFP ONU
    { 1,  "Hisense",  			"LTW2601F-BC+",       0x0,        0x9,        0x9f000000,  0x10101,    0x1010100, 	 		0x12345678  }, //cheng_20220314

//add new optical module info above                                                        
    { PHY_TRANS_NOT_FOUND_IN_IOT_LIST,  "",  "",       0x0,        0x0,        0x0,         0x0,        0x0} //end of the list

};
#endif

#if A60972_SERDES || XILINX_SERDES
XFP_TRANS_INFO xfp_trans_iot_list[]={  
//   index,   vendor_id,                      	      vendor_pn,   phy_csr_xpon_val, xgpon_phy_sfp_vld_level, xepon_pcs_sfp_status		invaild for FPGA	invaild for FPGA	xfp module pass word
    { 0,  "Hisense",  			"LTW2601D-BC+",      0x0,          0x0,                 0xff000000, 			0x0,  			0x0, 	 		0x12345678  }, 
    { 1,  "Hisense",  			"LTW2601F-BC+",      0x0,          0x0,                 0xff000000, 			0x0,  			0x0, 	 		0x12345678  }, //cheng_20220314

//add new optical module info above                                                        
    { PHY_TRANS_NOT_FOUND_IN_IOT_LIST,  "",  "",     0x0,         0x0,         0x0,		0x0,	0x0,	0x0         }, //end of the list
};
#endif

static I2C_REG_ENTRY xfp_trans_table_low_mem[]={
//  	    id    ,  					name                            			 ,  						addr                                    ,  default ,current
	{ 0	  ,	"XFP_TRANS_IDENTIFIER_LOW_MEM"           	,	XFP_TRANS_IDENTIFIER_LOW_MEM          	,	0	,	0	},
	{ 1	  ,	"XFP_TRANS_SINGNAL_CONDITIONER_CONTROL"  	,	XFP_TRANS_SINGNAL_CONDITIONER_CONTROL 	,	0	,	0	},
	{ 2	  ,	"XFP_TRANS_TEMP_HIGH_ALARM_MSB"          	,	XFP_TRANS_TEMP_HIGH_ALARM_MSB         	,	0	,	0	},
	{ 3	  ,	"XFP_TRANS_TEMP_HIGH_ALARM_LSB"          	,	XFP_TRANS_TEMP_HIGH_ALARM_LSB         	,	0	,	0	},
	{ 4	  ,	"XFP_TRANS_TEMP_LOW_ALARM_MSB"           	,	XFP_TRANS_TEMP_LOW_ALARM_MSB          	,	0	,	0	},
	{ 5	  ,	"XFP_TRANS_TEMP_LOW_ALARM_LSB"           	,	XFP_TRANS_TEMP_LOW_ALARM_LSB          	,	0	,	0	},
	{ 6	  ,	"XFP_TRANS_TEMP_HIGH_WARNING_MSB"        	,	XFP_TRANS_TEMP_HIGH_WARNING_MSB       	,	0	,	0	},
	{ 7	  ,	"XFP_TRANS_TEMP_HIGH_WARNING_LSB"        	,	XFP_TRANS_TEMP_HIGH_WARNING_LSB       	,	0	,	0	},
	{ 8	  ,	"XFP_TRANS_TEMP_LOW_WARNING_MSB"         	,	XFP_TRANS_TEMP_LOW_WARNING_MSB        	,	0	,	0	},
	{ 9	  ,	"XFP_TRANS_TEMP_LOW_WARNING_LSB"         	,	XFP_TRANS_TEMP_LOW_WARNING_LSB        	,	0	,	0	},
	{ 10  ,	"XFP_TRANS_VOLTAGE_HIGH_ALARM_MSB"       	,	XFP_TRANS_VOLTAGE_HIGH_ALARM_MSB      	,	0	,	0	},
	{ 11  ,	"XFP_TRANS_VOLTAGE_HIGH_ALARM_LSB"       	,	XFP_TRANS_VOLTAGE_HIGH_ALARM_LSB      	,	0	,	0	},
	{ 12  ,	"XFP_TRANS_VOLTAGE_LOW_ALARM_MSB"        	,	XFP_TRANS_VOLTAGE_LOW_ALARM_MSB       	,	0	,	0	},
	{ 13  ,	"XFP_TRANS_VOLTAGE_LOW_ALARM_LSB"        	,	XFP_TRANS_VOLTAGE_LOW_ALARM_LSB       	,	0	,	0	},
	{ 14  ,	"XFP_TRANS_VOLTAGE_HIGH_WARNING_MSB"     	,	XFP_TRANS_VOLTAGE_HIGH_WARNING_MSB    	,	0	,	0	},
	{ 15  ,	"XFP_TRANS_VOLTAGE_HIGH_WARNING_LSB"     	,	XFP_TRANS_VOLTAGE_HIGH_WARNING_LSB    	,	0	,	0	},
	{ 16  ,	"XFP_TRANS_VOLTAGE_LOW_WARNING_MSB"      	,	XFP_TRANS_VOLTAGE_LOW_WARNING_MSB     	,	0	,	0	},
	{ 17  ,	"XFP_TRANS_VOLTAGE_LOW_WARNING_LSB"      	,	XFP_TRANS_VOLTAGE_LOW_WARNING_LSB     	,	0	,	0	},
	{ 18  ,	"XFP_TRANS_BIAS_HIGH_ALARM_MSB"          	,	XFP_TRANS_BIAS_HIGH_ALARM_MSB         	,	0	,	0	},
	{ 19  ,	"XFP_TRANS_BIAS_HIGH_ALARM_LSB"          	,	XFP_TRANS_BIAS_HIGH_ALARM_LSB         	,	0	,	0	},
	{ 20  ,	"XFP_TRANS_BIAS_LOW_ALARM_MSB"           	,	XFP_TRANS_BIAS_LOW_ALARM_MSB          	,	0	,	0	},
	{ 21  ,	"XFP_TRANS_BIAS_LOW_ALARM_LSB"           	,	XFP_TRANS_BIAS_LOW_ALARM_LSB          	,	0	,	0	},
	{ 22  ,	"XFP_TRANS_BIAS_HIGH_WARNING_MSB"        	,	XFP_TRANS_BIAS_HIGH_WARNING_MSB       	,	0	,	0	},
	{ 23  ,	"XFP_TRANS_BIAS_HIGH_WARNING_LSB"        	,	XFP_TRANS_BIAS_HIGH_WARNING_LSB       	,	0	,	0	},
	{ 24  ,	"XFP_TRANS_BIAS_LOW_WARNING_MSB"         	,	XFP_TRANS_BIAS_LOW_WARNING_MSB        	,	0	,	0	},
	{ 25  ,	"XFP_TRANS_BIAS_LOW_WARNING_LSB"         	,	XFP_TRANS_BIAS_LOW_WARNING_LSB        	,	0	,	0	},
	{ 26  ,	"XFP_TRANS_TX_POWER_HIGH_ALARM_MSB"      	,	XFP_TRANS_TX_POWER_HIGH_ALARM_MSB     	,	0	,	0	},
	{ 27  ,	"XFP_TRANS_TX_POWER_HIGH_ALARM_LSB"      	,	XFP_TRANS_TX_POWER_HIGH_ALARM_LSB     	,	0	,	0	},
	{ 28  ,	"XFP_TRANS_TX_POWER_LOW_ALARM_MSB"       	,	XFP_TRANS_TX_POWER_LOW_ALARM_MSB      	,	0	,	0	},             
	{ 29  ,	"XFP_TRANS_TX_POWER_LOW_ALARM_LSB"       	,	XFP_TRANS_TX_POWER_LOW_ALARM_LSB      	,	0	,	0	},             
	{ 30  ,	"XFP_TRANS_TX_POWER_HIGH_WARNING_MSB"    	,	XFP_TRANS_TX_POWER_HIGH_WARNING_MSB   	,	0	,	0	},             
	{ 31  ,	"XFP_TRANS_TX_POWER_HIGH_WARNING_LSB"    	,	XFP_TRANS_TX_POWER_HIGH_WARNING_LSB   	,	0	,	0	},             
	{ 32  ,	"XFP_TRANS_TX_POWER_LOW_WARNING_MSB"     	,	XFP_TRANS_TX_POWER_LOW_WARNING_MSB    	,	0	,	0	},             
	{ 33  ,	"XFP_TRANS_TX_POWER_LOW_WARNING_LSB"     	,	XFP_TRANS_TX_POWER_LOW_WARNING_LSB    	,	0	,	0	},             
	{ 34  ,	"XFP_TRANS_RX_POWER_HIGH_ALARM_MSB"      	,	XFP_TRANS_RX_POWER_HIGH_ALARM_MSB     	,	0	,	0	},             
	{ 35  ,	"XFP_TRANS_RX_POWER_HIGH_ALARM_LSB"      	,	XFP_TRANS_RX_POWER_HIGH_ALARM_LSB     	,	0	,	0	},             
	{ 36  ,	"XFP_TRANS_RX_POWER_LOW_ALARM_MSB"       	,	XFP_TRANS_RX_POWER_LOW_ALARM_MSB      	,	0	,	0	},             
	{ 37  ,	"XFP_TRANS_RX_POWER_LOW_ALARM_LSB"       	,	XFP_TRANS_RX_POWER_LOW_ALARM_LSB      	,	0	,	0	},             
	{ 38  ,	"XFP_TRANS_RX_POWER_HIGH_WARNING_MSB"    	,	XFP_TRANS_RX_POWER_HIGH_WARNING_MSB   	,	0	,	0	},             
	{ 39  ,	"XFP_TRANS_RX_POWER_HIGH_WARNING_LSB"    	,	XFP_TRANS_RX_POWER_HIGH_WARNING_LSB   	,	0	,	0	},             
	{ 40  ,	"XFP_TRANS_RX_POWER_LOW_WARNING_MSB"     	,	XFP_TRANS_RX_POWER_LOW_WARNING_MSB    	,	0	,	0	},             
	{ 41  ,	"XFP_TRANS_RX_POWER_LOW_WARNING_LSB"     	,	XFP_TRANS_RX_POWER_LOW_WARNING_LSB    	,	0	,	0	},             
	{ 42  ,	"XFP_TRANS_AUX1_HIGH_ALARM_MSB"          	,	XFP_TRANS_AUX1_HIGH_ALARM_MSB         	,	0	,	0	},             
	{ 43  ,	"XFP_TRANS_AUX1_HIGH_ALARM_LSB"          	,	XFP_TRANS_AUX1_HIGH_ALARM_LSB         	,	0	,	0	},             
	{ 44  ,	"XFP_TRANS_AUX1_LOW_ALARM_MSB"           	,	XFP_TRANS_AUX1_LOW_ALARM_MSB          	,	0	,	0	},             
	{ 45  ,	"XFP_TRANS_AUX1_LOW_ALARM_LSB"           	,	XFP_TRANS_AUX1_LOW_ALARM_LSB          	,	0	,	0	},             
	{ 46  ,	"XFP_TRANS_AUX1_HIGH_WARNING_MSB"        	,	XFP_TRANS_AUX1_HIGH_WARNING_MSB       	,	0	,	0	},             
	{ 47  ,	"XFP_TRANS_AUX1_HIGH_WARNING_LSB"        	,	XFP_TRANS_AUX1_HIGH_WARNING_LSB       	,	0	,	0	},             
	{ 48  ,	"XFP_TRANS_AUX1_LOW_WARNING_MSB"         	,	XFP_TRANS_AUX1_LOW_WARNING_MSB        	,	0	,	0	},             
	{ 49  ,	"XFP_TRANS_AUX1_LOW_WARNING_LSB"         	,	XFP_TRANS_AUX1_LOW_WARNING_LSB        	,	0	,	0	},             
	{ 50  ,	"XFP_TRANS_AUX2_HIGH_ALARM_MSB"          	,	XFP_TRANS_AUX2_HIGH_ALARM_MSB         	,	0	,	0	},             
	{ 51  ,	"XFP_TRANS_AUX2_HIGH_ALARM_LSB"          	,	XFP_TRANS_AUX2_HIGH_ALARM_LSB         	,	0	,	0	},             
	{ 52  ,	"XFP_TRANS_AUX2_LOW_ALARM_MSB"           	,	XFP_TRANS_AUX2_LOW_ALARM_MSB          	,	0	,	0	},             
	{ 53  ,	"XFP_TRANS_AUX2_LOW_ALARM_LSB"           	,	XFP_TRANS_AUX2_LOW_ALARM_LSB          	,	0	,	0	},             
	{ 54  ,	"XFP_TRANS_AUX2_HIGH_WARNING_MSB"        	,	XFP_TRANS_AUX2_HIGH_WARNING_MSB       	,	0	,	0	},             
	{ 55  ,	"XFP_TRANS_AUX2_HIGH_WARNING_LSB"        	,	XFP_TRANS_AUX2_HIGH_WARNING_LSB       	,	0	,	0	},             
	{ 56  ,	"XFP_TRANS_AUX2_LOW_WARNING_MSB"         	,	XFP_TRANS_AUX2_LOW_WARNING_MSB        	,	0	,	0	},             
	{ 57  ,	"XFP_TRANS_AUX2_LOW_WARNING_LSB"         	,	XFP_TRANS_AUX2_LOW_WARNING_LSB        	,	0	,	0	},             
	{ 58  ,	"XFP_TRANS_OPTIONAL_VPS_CTRL_REG_0"      	,	XFP_TRANS_OPTIONAL_VPS_CTRL_REG_0     	,	0	,	0	},             
	{ 59  ,	"XFP_TRANS_OPTIONAL_VPS_CTRL_REG_1"      	,	XFP_TRANS_OPTIONAL_VPS_CTRL_REG_1     	,	0	,	0	},             
	{ 60  ,	"XFP_TRANS_RESERVED_0"                   	,	XFP_TRANS_RESERVED_0                  	,	0	,	0	},             
	{ 61  ,	"XFP_TRANS_RESERVED_1"                   	,	XFP_TRANS_RESERVED_1                  	,	0	,	0	},             
	{ 62  ,	"XFP_TRANS_RESERVED_2"                   	,	XFP_TRANS_RESERVED_2                  	,	0	,	0	},             
	{ 63  ,	"XFP_TRANS_RESERVED_3"                   	,	XFP_TRANS_RESERVED_3                  	,	0	,	0	},             
	{ 64  ,	"XFP_TRANS_RESERVED_4"                   	,	XFP_TRANS_RESERVED_4                  	,	0	,	0	},             
	{ 65  ,	"XFP_TRANS_RESERVED_5"                   	,	XFP_TRANS_RESERVED_5                  	,	0	,	0	},             
	{ 66  ,	"XFP_TRANS_RESERVED_6"                   	,	XFP_TRANS_RESERVED_6                  	,	0	,	0	},             
	{ 67  ,	"XFP_TRANS_RESERVED_7"                   	,	XFP_TRANS_RESERVED_7                  	,	0	,	0	},             
	{ 68  ,	"XFP_TRANS_RESERVED_8"                   	,	XFP_TRANS_RESERVED_8                  	,	0	,	0	},             
	{ 69  ,	"XFP_TRANS_RESERVED_9"                   	,	XFP_TRANS_RESERVED_9                  	,	0	,	0	},             
	{ 70  ,	"XFP_TRANS_BER_REPORTING_0"              	,	XFP_TRANS_BER_REPORTING_0             	,	0	,	0	},             
	{ 71  ,	"XFP_TRANS_BER_REPORTING_1"              	,	XFP_TRANS_BER_REPORTING_1             	,	0	,	0	},             
	{ 72  ,	"XFP_TRANS_WAVELENGTH_CTRL_REG_0"        	,	XFP_TRANS_WAVELENGTH_CTRL_REG_0       	,	0	,	0	},             
	{ 73  ,	"XFP_TRANS_WAVELENGTH_CTRL_REG_1"        	,	XFP_TRANS_WAVELENGTH_CTRL_REG_1       	,	0	,	0	},             
	{ 74  ,	"XFP_TRANS_WAVELENGTH_CTRL_REG_2"        	,	XFP_TRANS_WAVELENGTH_CTRL_REG_2       	,	0	,	0	},             
	{ 75  ,	"XFP_TRANS_WAVELENGTH_CTRL_REG_3"        	,	XFP_TRANS_WAVELENGTH_CTRL_REG_3       	,	0	,	0	},             
	{ 76  ,	"XFP_TRANS_FEC_CTRL_REG_0"               	,	XFP_TRANS_FEC_CTRL_REG_0              	,	0	,	0	},             
	{ 77  ,	"XFP_TRANS_FEC_CTRL_REG_1"               	,	XFP_TRANS_FEC_CTRL_REG_1              	,	0	,	0	},             
	{ 78  ,	"XFP_TRANS_FEC_CTRL_REG_2"               	,	XFP_TRANS_FEC_CTRL_REG_2              	,	0	,	0	},             
	{ 79  ,	"XFP_TRANS_FEC_CTRL_REG_3"               	,	XFP_TRANS_FEC_CTRL_REG_3              	,	0	,	0	},             
	{ 80  ,	"XFP_TRANS_FLAG_AND_INT_CTRL_REG_0"      	,	XFP_TRANS_FLAG_AND_INT_CTRL_REG_0     	,	0	,	0	},             
	{ 81  ,	"XFP_TRANS_FLAG_AND_INT_CTRL_REG_1"      	,	XFP_TRANS_FLAG_AND_INT_CTRL_REG_1     	,	0	,	0	},             
	{ 82  ,	"XFP_TRANS_FLAG_AND_INT_CTRL_REG_2"      	,	XFP_TRANS_FLAG_AND_INT_CTRL_REG_2     	,	0	,	0	},             
	{ 83  ,	"XFP_TRANS_FLAG_AND_INT_CTRL_REG_3"      	,	XFP_TRANS_FLAG_AND_INT_CTRL_REG_3     	,	0	,	0	},             
	{ 84  ,	"XFP_TRANS_FLAG_AND_INT_CTRL_REG_4"      	,	XFP_TRANS_FLAG_AND_INT_CTRL_REG_4     	,	0	,	0	},             
	{ 85  ,	"XFP_TRANS_FLAG_AND_INT_CTRL_REG_5"      	,	XFP_TRANS_FLAG_AND_INT_CTRL_REG_5     	,	0	,	0	},             
	{ 86  ,	"XFP_TRANS_FLAG_AND_INT_CTRL_REG_6"      	,	XFP_TRANS_FLAG_AND_INT_CTRL_REG_6     	,	0	,	0	},             
	{ 87  ,	"XFP_TRANS_FLAG_AND_INT_CTRL_REG_7"      	,	XFP_TRANS_FLAG_AND_INT_CTRL_REG_7     	,	0	,	0	},             
	{ 88  ,	"XFP_TRANS_INT_MASKING_BITS_0"           	,	XFP_TRANS_INT_MASKING_BITS_0          	,	0	,	0	},             
	{ 89  ,	"XFP_TRANS_INT_MASKING_BITS_1"           	,	XFP_TRANS_INT_MASKING_BITS_1          	,	0	,	0	},             
	{ 90  ,	"XFP_TRANS_INT_MASKING_BITS_2"           	,	XFP_TRANS_INT_MASKING_BITS_2          	,	0	,	0	},             
	{ 91  ,	"XFP_TRANS_INT_MASKING_BITS_3"           	,	XFP_TRANS_INT_MASKING_BITS_3          	,	0	,	0	},             
	{ 92  ,	"XFP_TRANS_INT_MASKING_BITS_4"           	,	XFP_TRANS_INT_MASKING_BITS_4          	,	0	,	0	},             
	{ 93  ,	"XFP_TRANS_INT_MASKING_BITS_5"           	,	XFP_TRANS_INT_MASKING_BITS_5          	,	0	,	0	},             
	{ 94  ,	"XFP_TRANS_INT_MASKING_BITS_6"           	,	XFP_TRANS_INT_MASKING_BITS_6          	,	0	,	0	},             
	{ 95  ,	"XFP_TRANS_INT_MASKING_BITS_7"           	,	XFP_TRANS_INT_MASKING_BITS_7          	,	0	,	0	},             
	{ 96  ,	"XFP_TRANS_DIAGNOSTICS_TEMP_MSB"         	,	XFP_TRANS_DIAGNOSTICS_TEMP_MSB        	,	0	,	0	},             
	{ 97  ,	"XFP_TRANS_DIAGNOSTICS_TEMP_LSB"         	,	XFP_TRANS_DIAGNOSTICS_TEMP_LSB        	,	0	,	0	},             
	{ 98  ,	"XFP_TRANS_DIAGNOSTICS_VCC_MSB"          	,	XFP_TRANS_DIAGNOSTICS_VCC_MSB         	,	0	,	0	},             
	{ 99  ,	"XFP_TRANS_DIAGNOSTICS_VCC_LSB"          	,	XFP_TRANS_DIAGNOSTICS_VCC_LSB         	,	0	,	0	},             
	{ 100 ,	"XFP_TRANS_TX_BIAS_MSB"                  	,	XFP_TRANS_TX_BIAS_MSB                 	,	0	,	0	},             
	{ 101 ,	"XFP_TRANS_TX_BIAS_LSB"                  	,	XFP_TRANS_TX_BIAS_LSB                 	,	0	,	0	},             
	{ 102 ,	"XFP_TRANS_TX_POWER_MSB"                 	,	XFP_TRANS_TX_POWER_MSB                	,	0	,	0	},             
	{ 103 ,	"XFP_TRANS_TX_POWER_LSB"                 	,	XFP_TRANS_TX_POWER_LSB                	,	0	,	0	},             
	{ 104 ,	"XFP_TRANS_RX_POWER_MSB"                 	,	XFP_TRANS_RX_POWER_MSB                	,	0	,	0	},             
	{ 105 ,	"XFP_TRANS_RX_POWER_LSB"                 	,	XFP_TRANS_RX_POWER_LSB                	,	0	,	0	},             
	{ 106 ,	"XFP_TRANS_AUX1_MSB"              			,	XFP_TRANS_AUX1_MSB             	,	0	,	0	},             
	{ 107 ,	"XFP_TRANS_AUX1_LSB"              			,	XFP_TRANS_AUX1_LSB             	,	0	,	0	},             
	{ 108 ,	"XFP_TRANS_AUX2_MSB"                     	,	XFP_TRANS_AUX2_MSB                    	,	0	,	0	},             
	{ 109 ,	"XFP_TRANS_AUX2_LSB"                     	,	XFP_TRANS_AUX2_LSB                    	,	0	,	0	},             
	{ 110 ,	"XFP_TRANS_GENERAL_CTRL_STA_BITS_0"      	,	XFP_TRANS_GENERAL_CTRL_STA_BITS_0     	,	0	,	0	},             
	{ 111 ,	"XFP_TRANS_GENERAL_CTRL_STA_BITS_1"      	,	XFP_TRANS_GENERAL_CTRL_STA_BITS_1     	,	0	,	0	},             
	{ 112 ,	"XFP_TRANS_NGPON2_TX_CHAN_SEL"           	,	XFP_TRANS_NGPON2_TX_CHAN_SEL          	,	0	,	0	},             
	{ 113 ,	"XFP_TRANS_NGPON2_RX_CHAN_SEL"           	,	XFP_TRANS_NGPON2_RX_CHAN_SEL          	,	0	,	0	},             
	{ 114 ,	"XFP_TRANS_RESERVED_10"                  	,	XFP_TRANS_RESERVED_10                 	,	0	,	0	},             
	{ 115 ,	"XFP_TRANS_RESERVED_11"                  	,	XFP_TRANS_RESERVED_11                 	,	0	,	0	},             
	{ 116 ,	"XFP_TRANS_RESERVED_12"                  	,	XFP_TRANS_RESERVED_12                 	,	0	,	0	},             
	{ 117 ,	"XFP_TRANS_RESERVED_13"                  	,	XFP_TRANS_RESERVED_13                 	,	0	,	0	},             
	{ 118 ,	"XFP_TRANS_RESERVED_14"                  	,	XFP_TRANS_RESERVED_14                 	,	0	,	0	},             
	{ 119 ,	"XFP_TRANS_PASSWORDS_CHANGE_AREA_0"      	,	XFP_TRANS_PASSWORDS_CHANGE_AREA_0     	,	0	,	0	},             
	{ 120 ,	"XFP_TRANS_PASSWORDS_CHANGE_AREA_1"      	,	XFP_TRANS_PASSWORDS_CHANGE_AREA_1     	,	0	,	0	},             
	{ 121 ,	"XFP_TRANS_PASSWORDS_CHANGE_AREA_2"      	,	XFP_TRANS_PASSWORDS_CHANGE_AREA_2     	,	0	,	0	},             
	{ 122 ,	"XFP_TRANS_PASSWORDS_CHANGE_AREA_3"      	,	XFP_TRANS_PASSWORDS_CHANGE_AREA_3     	,	0	,	0	},             
	{ 123 ,	"XFP_TRANS_PASSWORDS_ENTRY_AREA_0"       	,	XFP_TRANS_PASSWORDS_ENTRY_AREA_0      	,	0	,	0	},             
	{ 124 ,	"XFP_TRANS_PASSWORDS_ENTRY_AREA_1"       	,	XFP_TRANS_PASSWORDS_ENTRY_AREA_1      	,	0	,	0	},             
	{ 125 ,	"XFP_TRANS_PASSWORDS_ENTRY_AREA_2"       	,	XFP_TRANS_PASSWORDS_ENTRY_AREA_2      	,	0	,	0	},             
	{ 126 ,	"XFP_TRANS_PASSWORDS_ENTRY_AREA_3"       	,	XFP_TRANS_PASSWORDS_ENTRY_AREA_3      	,	0	,	0	},             
	{ 127 ,	"XFP_TRANS_PAGE_SEL_BYTE"                	,	XFP_TRANS_PAGE_SEL_BYTE               	,	0	,	0	}              
};   

static I2C_REG_ENTRY xfp_trans_table_high_mem[]={
	//   id  ,  name                            							 ,    addr                        							,default    ,current
	{ 128 ,	"XFP_TRANS_IDENTIFIER"                   	,	XFP_TRANS_IDENTIFIER                	,	0	,	0	},
	{ 129 ,	"XFP_TRANS_EXT_IDENTIFIER"               	,	XFP_TRANS_EXT_IDENTIFIER            	,	0	,	0	},
	{ 130 ,	"XFP_TRANS_CONNECTOR "                   	,	XFP_TRANS_CONNECTOR                 	,	0	,	0	},
	{ 131 ,	"XFP_TRANS_TRANSCEIVER_0 "               	,	XFP_TRANS_TRANSCEIVER_0             	,	0	,	0	},
	{ 132 ,	"XFP_TRANS_TRANSCEIVER_1 "               	,	XFP_TRANS_TRANSCEIVER_1             	,	0	,	0	},
	{ 133 ,	"XFP_TRANS_TRANSCEIVER_2 "               	,	XFP_TRANS_TRANSCEIVER_2             	,	0	,	0	},
	{ 134 ,	"XFP_TRANS_TRANSCEIVER_3 "               	,	XFP_TRANS_TRANSCEIVER_3             	,	0	,	0	},
	{ 135 ,	"XFP_TRANS_TRANSCEIVER_4 "               	,	XFP_TRANS_TRANSCEIVER_4             	,	0	,	0	},
	{ 136 ,	"XFP_TRANS_TRANSCEIVER_5 "               	,	XFP_TRANS_TRANSCEIVER_5             	,	0	,	0	},
	{ 137 ,	"XFP_TRANS_TRANSCEIVER_6"                	,	XFP_TRANS_TRANSCEIVER_6             	,	0	,	0	},
	{ 138 ,	"XFP_TRANS_TRANSCEIVER_7"                	,	XFP_TRANS_TRANSCEIVER_7             	,	0	,	0	},
	{ 139 ,	"XFP_TRANS_ENCODING"                     	,	XFP_TRANS_ENCODING                  	,	0	,	0	},
	{ 140 ,	"XFP_TRANS_BR_MIN"                       	,	XFP_TRANS_BR_MIN                    	,	0	,	0	},
	{ 141 ,	"XFP_TRANS_BR_MAX"                       	,	XFP_TRANS_BR_MAX                    	,	0	,	0	},
	{ 142 ,	"XFP_TRANS_LENGTH_SMF_KM "               	,	XFP_TRANS_LENGTH_SMF_KM             	,	0	,	0	},
	{ 143 ,	"XFP_TRANS_LENGTH_EBW_50UM "             	,	XFP_TRANS_LENGTH_EBW_50UM           	,	0	,	0	},
	{ 144 ,	"XFP_TRANS_LENGTH_50UM "                 	,	XFP_TRANS_LENGTH_50UM               	,	0	,	0	},
	{ 145 ,	"XFP_TRANS_LENGTH_62P5UM "               	,	XFP_TRANS_LENGTH_62P5UM             	,	0	,	0	},
	{ 146 ,	"XFP_TRANS_LENGTH_CABLE"                 	,	XFP_TRANS_LENGTH_CABLE              	,	0	,	0	},
	{ 147 ,	"XFP_TRANS_DEVICE_TEC"                   	,	XFP_TRANS_DEVICE_TEC                	,	0	,	0	},
	{ 148 ,	"XFP_TRANS_VENDOR_NAME"                  	,	XFP_TRANS_VENDOR_NAME               	,	0	,	0	},
	{ 149 ,	"XFP_TRANS_VENDOR_NAME"                  	,	149                                     	,	0	,	0	},
	{ 150 ,	"XFP_TRANS_VENDOR_NAME"                  	,	150                                     	,	0	,	0	},
	{ 151 ,	"XFP_TRANS_VENDOR_NAME"                  	,	151                                     	,	0	,	0	},
	{ 152 ,	"XFP_TRANS_VENDOR_NAME"                  	,	152                                     	,	0	,	0	},
	{ 153 ,	"XFP_TRANS_VENDOR_NAME"                  	,	153                                     	,	0	,	0	},
	{ 154 ,	"XFP_TRANS_VENDOR_NAME"                  	,	154                                     	,	0	,	0	},
	{ 155 ,	"XFP_TRANS_VENDOR_NAME"                  	,	155                                     	,	0	,	0	},
	{ 156 ,	"XFP_TRANS_VENDOR_NAME"                  	,	156                                     	,	0	,	0	},
	{ 157 ,	"XFP_TRANS_VENDOR_NAME"                  	,	157                                     	,	0	,	0	},
	{ 158 ,	"XFP_TRANS_VENDOR_NAME"                  	,	158                                     	,	0	,	0	},
	{ 159 ,	"XFP_TRANS_VENDOR_NAME"                  	,	159                                     	,	0	,	0	},
	{ 160 ,	"XFP_TRANS_VENDOR_NAME"                  	,	160                                     	,	0	,	0	},
	{ 161 ,	"XFP_TRANS_VENDOR_NAME"                  	,	161                                     	,	0	,	0	},
	{ 162 ,	"XFP_TRANS_VENDOR_NAME"                  	,	162                                     	,	0	,	0	},
	{ 163 ,	"XFP_TRANS_VENDOR_NAME"                  	,	163                                     	,	0	,	0	},
	{ 164 ,	"XFP_TRANS_CDR "                         	,	XFP_TRANS_CDR                       	,	0	,	0	},
	{ 165 ,	"XFP_TRANS_VENDOR_OUI"                   	,	XFP_TRANS_VENDOR_OUI                	,	0	,	0	},
	{ 166 ,	"XFP_TRANS_VENDOR_OUI"                      ,	166                                     	,	0	,	0	},
	{ 167 ,	"XFP_TRANS_VENDOR_OUI"                      ,	167                                     	,	0	,	0	},
	{ 168 ,	"XFP_TRANS_VENDOR_PN"                    	,	XFP_TRANS_VENDOR_PN                 	,	0	,	0	},
	{ 169 ,	"XFP_TRANS_VENDOR_PN"                    	,	169                                     	,	0	,	0	},
	{ 170 ,	"XFP_TRANS_VENDOR_PN"                    	,	170                                     	,	0	,	0	},
	{ 171 ,	"XFP_TRANS_VENDOR_PN"                    	,	171                                     	,	0	,	0	},
	{ 172 ,	"XFP_TRANS_VENDOR_PN"                    	,	172                                     	,	0	,	0	},
	{ 173 ,	"XFP_TRANS_VENDOR_PN"                    	,	173                                     	,	0	,	0	},
	{ 174 ,	"XFP_TRANS_VENDOR_PN"                    	,	174                                     	,	0	,	0	},
	{ 175 ,	"XFP_TRANS_VENDOR_PN"                    	,	175                                     	,	0	,	0	},
	{ 176 ,	"XFP_TRANS_VENDOR_PN"                    	,	176                                     	,	0	,	0	},
	{ 177 ,	"XFP_TRANS_VENDOR_PN"                    	,	177                                     	,	0	,	0	},
	{ 178 ,	"XFP_TRANS_VENDOR_PN"                    	,	178                                     	,	0	,	0	},
	{ 179 ,	"XFP_TRANS_VENDOR_PN"                    	,	179                                     	,	0	,	0	},
	{ 180 ,	"XFP_TRANS_VENDOR_PN"                    	,	180                                     	,	0	,	0	},
	{ 181 ,	"XFP_TRANS_VENDOR_PN"                    	,	181                                     	,	0	,	0	},
	{ 182 ,	"XFP_TRANS_VENDOR_PN"                    	,	182                                     	,	0	,	0	},
	{ 183 ,	"XFP_TRANS_VENDOR_PN"                    	,	183                                     	,	0	,	0	},
	{ 184 ,	"XFP_TRANS_VENDOR_REV"                   	,	184                                     	,	0	,	0	},
	{ 185 ,	"XFP_TRANS_VENDOR_REV"                   	,	185                                     	,	0	,	0	},
	{ 186 ,	"XFP_TRANS_TX_WAVELENGTH_MSB"            	,	XFP_TRANS_TX_WAVELENGTH_MSB         	,	0	,	0	},
	{ 187 ,	"XFP_TRANS_TX_WAVELENGTH_LSB"            	,	XFP_TRANS_TX_WAVELENGTH_LSB         	,	0	,	0	},
	{ 188 ,	"XFP_TRANS_WAVELENGTH_TOLERANCE_MSB"     	,	XFP_TRANS_WAVELENGTH_TOLERANCE_MSB  	,	0	,	0	},
	{ 189 ,	"XFP_TRANS_WAVELENGTH_TOLERANCE_LSB"     	,	XFP_TRANS_WAVELENGTH_TOLERANCE_LSB  	,	0	,	0	},
	{ 190 ,	"XFP_TRANS_MAX_TEMPERATURE "             	,	XFP_TRANS_MAX_TEMPERATURE           	,	0	,	0	},
	{ 191 ,	"XFP_TRANS_CC_BASE"                      	,	XFP_TRANS_CC_BASE                   	,	0	,	0	},
	{ 192 ,	"XFP_TRANS_POWER_SUPPLY_0"               	,	XFP_TRANS_POWER_SUPPLY_0            	,	0	,	0	},
	{ 193 ,	"XFP_TRANS_POWER_SUPPLY_1"               	,	XFP_TRANS_POWER_SUPPLY_1            	,	0	,	0	},
	{ 194 ,	"XFP_TRANS_POWER_SUPPLY_2"               	,	XFP_TRANS_POWER_SUPPLY_2            	,	0	,	0	},
	{ 195 ,	"XFP_TRANS_POWER_SUPPLY_3"               	,	XFP_TRANS_POWER_SUPPLY_3            	,	0	,	0	},
	{ 196 ,	"XFP_TRANS_VENDOR_SN "                   	,	XFP_TRANS_VENDOR_SN                 	,	0	,	0	},
	{ 197 ,	"XFP_TRANS_VENDOR_SN "                   	,	197                                     	,	0	,	0	},
	{ 198 ,	"XFP_TRANS_VENDOR_SN "                   	,	198                                     	,	0	,	0	},
	{ 199 ,	"XFP_TRANS_VENDOR_SN "                   	,	199                                     	,	0	,	0	},
	{ 200 ,	"XFP_TRANS_VENDOR_SN "                   	,	200                                     	,	0	,	0	},
	{ 201 ,	"XFP_TRANS_VENDOR_SN "                   	,	201                                     	,	0	,	0	},
	{ 202 ,	"XFP_TRANS_VENDOR_SN "                   	,	202                                     	,	0	,	0	},
	{ 203 ,	"XFP_TRANS_VENDOR_SN "                   	,	203                                     	,	0	,	0	},
	{ 204 ,	"XFP_TRANS_VENDOR_SN "                   	,	204                                     	,	0	,	0	},
	{ 205 ,	"XFP_TRANS_VENDOR_SN "                   	,	205                                     	,	0	,	0	},
	{ 206 ,	"XFP_TRANS_VENDOR_SN "                   	,	206                                     	,	0	,	0	},
	{ 207 ,	"XFP_TRANS_VENDOR_SN "                   	,	207                                     	,	0	,	0	},
	{ 208 ,	"XFP_TRANS_VENDOR_SN "                   	,	208                                     	,	0	,	0	},
	{ 209 ,	"XFP_TRANS_VENDOR_SN "                   	,	209                                     	,	0	,	0	},
	{ 210 ,	"XFP_TRANS_VENDOR_SN "                   	,	210                                     	,	0	,	0	},
	{ 211 ,	"XFP_TRANS_VENDOR_SN "                   	,	211                                     	,	0	,	0	},
	{ 212 ,	"XFP_TRANS_DATE_CODE"                    	,	XFP_TRANS_DATE_CODE                 	,	0	,	0	},
	{ 213 ,	"XFP_TRANS_DATE_CODE"                    	,	213                                     	,	0	,	0	},
	{ 214 ,	"XFP_TRANS_DATE_CODE"                    	,	214                                     	,	0	,	0	},
	{ 215 ,	"XFP_TRANS_DATE_CODE"                    	,	215                                     	,	0	,	0	},
	{ 216 ,	"XFP_TRANS_DATE_CODE"                    	,	216                                     	,	0	,	0	},
	{ 217 ,	"XFP_TRANS_DATE_CODE"                    	,	217                                     	,	0	,	0	},
	{ 218 ,	"XFP_TRANS_DATE_CODE"                    	,	218                                     	,	0	,	0	},
	{ 219 ,	"XFP_TRANS_DATE_CODE"                    	,	219                                     	,	0	,	0	},
	{ 220 ,	"XFP_TRANS_DIAG_MONITOR_TYPE"            	,	XFP_TRANS_DIAG_MONITOR_TYPE         	,	0	,	0	},
	{ 221 ,	"XFP_TRANS_ENHANCED_OPTIONS"             	,	XFP_TRANS_ENHANCED_OPTIONS          	,	0	,	0	},
	{ 222 ,	"XFP_TRANS_AUX_MONITORING "              	,	XFP_TRANS_AUX_MONITORING            	,	0	,	0	},
	{ 223 ,	"XFP_TRANS_CC_EXT"                       	,	XFP_TRANS_CC_EXT                    	,	0	,	0	},
	{ 224 ,	"XFP_TRANS_RESERVED"                     	,	XFP_TRANS_RESERVED                  	,	0	,	0	},
	{ 225 ,	"XFP_TRANS_RESERVED"                     	,	225                                     	,	0	,	0	},
	{ 226 ,	"XFP_TRANS_RESERVED"                     	,	226                                     	,	0	,	0	},
	{ 227 ,	"XFP_TRANS_RESERVED"                     	,	227                                     	,	0	,	0	},
	{ 228 ,	"XFP_TRANS_RESERVED"                     	,	228                                     	,	0	,	0	},
	{ 229 ,	"XFP_TRANS_RESERVED"                     	,	229                                     	,	0	,	0	},
	{ 230 ,	"XFP_TRANS_RESERVED"                     	,	230                                     	,	0	,	0	},
	{ 231 ,	"XFP_TRANS_RESERVED"                     	,	231                                     	,	0	,	0	},
	{ 232 ,	"XFP_TRANS_RESERVED"                     	,	232                                     	,	0	,	0	},
	{ 233 ,	"XFP_TRANS_RESERVED"                     	,	233                                     	,	0	,	0	},
	{ 234 ,	"XFP_TRANS_RESERVED"                     	,	234                                     	,	0	,	0	},
	{ 235 ,	"XFP_TRANS_RESERVED"                     	,	235                                     	,	0	,	0	},
	{ 236 ,	"XFP_TRANS_RESERVED"                     	,	236                                     	,	0	,	0	},
	{ 237 ,	"XFP_TRANS_RESERVED"                     	,	237                                     	,	0	,	0	},
	{ 238 ,	"XFP_TRANS_RESERVED"                     	,	238                                     	,	0	,	0	},
	{ 239 ,	"XFP_TRANS_RESERVED"                     	,	239                                     	,	0	,	0	},
	{ 240 ,	"XFP_TRANS_RESERVED"                     	,	240                                     	,	0	,	0	},
	{ 241 ,	"XFP_TRANS_RESERVED"                     	,	241                                     	,	0	,	0	},
	{ 242 ,	"XFP_TRANS_RESERVED"                     	,	242                                     	,	0	,	0	},
	{ 243 ,	"XFP_TRANS_RESERVED"                     	,	243                                     	,	0	,	0	},
	{ 244 ,	"XFP_TRANS_RESERVED"                     	,	244                                     	,	0	,	0	},
	{ 245 ,	"XFP_TRANS_RESERVED"                     	,	245                                     	,	0	,	0	},
	{ 246 ,	"XFP_TRANS_RESERVED"                     	,	246                                     	,	0	,	0	},
	{ 247 ,	"XFP_TRANS_RESERVED"                     	,	247                                     	,	0	,	0	},
	{ 248 ,	"XFP_TRANS_RESERVED"                     	,	248                                     	,	0	,	0	},
	{ 249 ,	"XFP_TRANS_RESERVED"                     	,	249                                     	,	0	,	0	},
	{ 250 ,	"XFP_TRANS_RESERVED"                     	,	250                                     	,	0	,	0	},
	{ 251 ,	"XFP_TRANS_RESERVED"                     	,	251                                     	,	0	,	0	},
	{ 252 ,	"XFP_TRANS_RESERVED"                     	,	252                                     	,	0	,	0	},
	{ 253 ,	"XFP_TRANS_RESERVED"                     	,	253                                     	,	0	,	0	},
	{ 254 ,	"XFP_TRANS_RESERVED"                     	,	254                                     	,	0	,	0	},
	{ 255 ,	"XFP_TRANS_RESERVED"                     	,	255                                     	,	0	,	0	}
    
};   

extern UINT16 SIF_X_Read(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                         UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt);
extern UINT16 SIF_X_Write(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                          UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt);


#if SIF_DEBUG_LEVEL_CONTROL
int phyProcFileCheck(char *filename){
	struct file	*srcf = NULL;
	char *src = NULL;
	mm_segment_t orgfs;
   	src = filename;

	orgfs = get_fs();
	set_fs(KERNEL_DS);

	srcf = filp_open(src, O_RDONLY, 0);
	if(src && *src) { /* src is not null and not empty string */
		if (IS_ERR(srcf))
		{
			printk("--> Error opening in testFileCheck : %s\n", filename);
			set_fs(orgfs);
			return 1;
		}
		else
		{
			filp_close(srcf,NULL);
		}
	}
	
	set_fs(orgfs);
	return 0;
}

int phyProcFileWrite(char *filename){
	struct file	*srcf = NULL;
	char *src = NULL;
	mm_segment_t orgfs;
	int i;
	
   	src = filename;

	orgfs = get_fs();
	set_fs(KERNEL_DS);

	srcf = filp_open(src, O_RDWR|O_CREAT, 0);
	if(src && *src) { /* src is not null and not empty string */
		if (IS_ERR(srcf))
		{
			printk("--> Error opening in testFileWrite %s\n", filename);
			return 1;
		}
		else
		{
			for(i = 0; i < PROC_READ_WRITE_TIMES; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,115) 
				ecnt_kernel_fs_write(srcf, flash_matrix, sizeof(flash_matrix), &srcf->f_pos);
#else
				srcf->f_op->write(srcf, proc_buffer, PROC_BUF_SIZE, &srcf->f_pos);
#endif
			}
			filp_close(srcf,NULL);
		}
	}
	
	set_fs(orgfs);
	return 0;
}

int phyProcFileRead(char *filename){
	struct file *srcf = NULL;
	char *src = NULL;
	mm_segment_t orgfs;
	int i, j;

   	src = filename;

	orgfs = get_fs();
	set_fs(KERNEL_DS);

	srcf = filp_open(src, O_RDONLY, 0);
	if(src && *src) { /* src is not null and not empty string */
		if (IS_ERR(srcf))
		{
			printk("--> Error opening in testFileRead : %s\n", filename);
			return 1;
		}
		else
		{
			memset(proc_buffer, 0x00, PROC_BUF_SIZE);
			for(i = 0; i < PROC_READ_WRITE_TIMES; i++) {
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,115) 				
				ecnt_kernel_fs_read(srcf, flash_matrix, sizeof(flash_matrix), &srcf->f_pos);
#else
				srcf->f_op->read(srcf, proc_buffer, PROC_BUF_SIZE, &srcf->f_pos);
#endif

			}
			filp_close(srcf,NULL);
		}
	}
	
	set_fs(orgfs);
	return 0;
}


int sifm_proc_read(char *output)
{
	proc_buffer=output;

	if( phyProcFileCheck(sifm_proc_filename) != 0 )
	{
	   return PHY_FAILURE; 
	}
	else
	{
		phyProcFileRead(sifm_proc_filename);
		PON_PHY_PRINT(PHY_MSG_TRANS, "sifm_proc_read[0]%d 0x%x %c\n",*output,*output,*output);
		PON_PHY_PRINT(PHY_MSG_TRANS, "sifm_proc_read[0]%d 0x%x %c\n",*proc_buffer,*proc_buffer,*proc_buffer);
	}

	return PHY_SUCCESS;
}

int sifm_proc_write(char *input)
{
	int i=0;
    proc_buffer=input;
	
	if( phyProcFileCheck(sifm_proc_filename) != 0 )
	{
	   return PHY_FAILURE; 
	}
	else
	{
		PON_PHY_PRINT(PHY_MSG_TRANS, "sifm_proc_write[0]%d 0x%x %c\n",*proc_buffer,*proc_buffer,*proc_buffer);
		phyProcFileWrite(sifm_proc_filename);
		phyProcFileRead(sifm_proc_filename);
		PON_PHY_PRINT(PHY_MSG_TRANS, "sifm_proc_write[0]%d 0x%x %c\n",*proc_buffer,*proc_buffer,*proc_buffer);
	}

	return PHY_SUCCESS;

}
#endif

UINT16 phy_I2C_read(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                         UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt)
{
	uint ret;

#if defined(TCSUPPORT_COMBO_PON)	
#ifdef TCSUPPORT_CPU_AN7583 //7583_COMBO-PON
		if(gpPhyPriv->i2c1_used == TRUE)
			u1CHannelID =1;	
#endif
#endif


	ret = SIF_X_Read(u1CHannelID, u2ClkDiv, u1DevAddr, 
					  u1WordAddrNum, u4WordAddr, pu1Buf, 
					  u2ByteCnt);
	if(ret == 0) {
 		PON_PHY_PRINT(PHY_MSG_TRANS,"\nError! SIF_X_Read return value = 0, u1DevAddr(%d):0x%x, u4WordAddr:%d\n",
			u1WordAddrNum,u1DevAddr, u4WordAddr);
		//dump_stack();
	}

	
	return ret;
}

UINT16 phy_I2C_write(UINT8 u1CHannelID, UINT16 u2ClkDiv, UINT8 u1DevAddr, UINT8 u1WordAddrNum,
                          UINT32 u4WordAddr, UINT8 *pu1Buf, UINT16 u2ByteCnt)
{
	uint ret;

#if defined(TCSUPPORT_COMBO_PON)	
#ifdef TCSUPPORT_CPU_AN7583 //7583_COMBO-PON
			if(gpPhyPriv->i2c1_used == TRUE)
				u1CHannelID =1; 
#endif
#endif

	ret = SIF_X_Write(u1CHannelID, u2ClkDiv, u1DevAddr, 
					   u1WordAddrNum, u4WordAddr, pu1Buf, 
					   u2ByteCnt);

	if(ret == 0) {
		PON_PHY_PRINT(PHY_MSG_TRANS,"\nError! SIF_X_Write return value = 0, u1DevAddr(%d):0x%x, u4WordAddr:%d, pu1Buf:0x%x\n",
			u1WordAddrNum,u1DevAddr, u4WordAddr, *(uint *)pu1Buf);
	}
	
	return ret;
}

//#if defined(CONFIG_USE_A60901) || defined(CONFIG_USE_A60928) //Roger_a60972
uint phy_I2C_read_translet(uint sub_addr)
{
	uint buffer;

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, U1_DEV_ADDR, 2, sub_addr, (unchar *)&buffer, 4);

#ifdef __BIG_ENDIAN
	buffer = ((buffer & 0x000000FF) << 24) | 
			 ((buffer & 0x0000FF00) << 8) | 
			 ((buffer & 0x00FF0000) >> 8) | 
			 ((buffer & 0xFF000000) >> 24);
#endif
	return buffer;
}
EXPORT_SYMBOL(phy_I2C_read_translet);

void phy_I2C_write_translet(uint sub_addr, uint val)
{
	uint buffer = val;
	
#ifdef __BIG_ENDIAN
	buffer = ((val & 0x000000FF) << 24) | 
			 ((val & 0x0000FF00) << 8) | 
			 ((val & 0x00FF0000) >> 8) | 
			 ((val & 0xFF000000) >> 24);
#endif
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, U1_DEV_ADDR, 2, sub_addr, (unchar *)&buffer, 4);
}
EXPORT_SYMBOL(phy_I2C_write_translet);
//#endif

char* rtrim(char *name){
	int i = 0, len = strlen(name);

	for (i = len-1; i >= 0 && name[i] ==' '; i--)
		name[i] = '\0';
	return name;		
}


// In SFF 8472 power is represented by 16 bits unsigned integer with LSB equal to 0.1uW, follow function can convert it to dBm

int sff_8472_power_to_dBm(UINT16 input) //float sff_8472_power_to_dBm(float input) //chunhua_20230320
{
	#define INPUT_EXPONENT_VS_mW      4  //this is the exponent difference between input value and mW, 0.1uW = 10^-4 mW, so the difference here = 4
	#define LOG2_10_multi_100      332 //chunhua_20230320 100*log2(10) //#define LN10      2.302585
    UINT8 input_msb = 0; //chunhua_20230320
    int i = 0; 
	int temp    = -4100; //float temp 		= -41; //chunhua_20230320
	int temp_2 	= 0; //float temp_2 	= 0; //chunhua_20230320
	int temp_3 	= 0; //float temp_3 	= 0; //chunhua_20230320

	if(input==0)
	{
		return temp;
	}
    
	#if 0 //chunhua_20230320
	for ( i=0; i<7 ; i++) 		// find resolution of input 0.5 <= x < 5
	{
		if ( input >= 5 )
			input = input/10;	
		else
			break;
	}

	temp = (input-1)/input;		// ( x-1)/x
	temp_2 = temp*temp;
	temp_3 = temp*temp_2;

	temp = temp + temp_2/2 + temp_3/3 + (temp_2*temp_2)/4 + (temp_2*temp_3)/5 + (temp_3*temp_3)/6;	// lnX	
	temp = temp/LN10;				// log10(X) = lnX / ln10
	temp -= INPUT_EXPONENT_VS_mW-i;	// to log (mW) 
	temp *= 10; 					// 10*logX
    #endif
    
    //=== approximation of log2(input)===start //chunhua_20230320
    for ( i=1; i<=16 ; i++) //find MSB
    {
        if ((input>>i) == 0)
        {
            input_msb = i-1;
            break;
        }
    }
        
    temp_2 = (input*1000/(1<<input_msb))%1000;  //similar to look-up table to search two decimal places of log2(input)

    if(temp_2 < 300)
        temp_3 = temp_2/8+1;
    else if(temp_2 < 590)
        temp_3 = temp_2/9+3;
    else if(temp_2 < 750)
        temp_3 = (temp_2+3)/9;  
    else if(temp_2 < 920)
        temp_3 = temp_2/10+3;
    else
    {
       temp_3 = temp_2/10+2;
       if (temp_3 >= 100)
           temp_3 = 99;
    }
    
    temp = (100000*input_msb+ 1000*temp_3)/LOG2_10_multi_100;	//1000*log10(X) = (100000*log2(x))/(100*log2(10))
    //=== approximation of log2(input)===end //chunhua_20230320

    temp -= INPUT_EXPONENT_VS_mW*1000;	// to 1000*log (mW) 

	return temp;
}

void phy_dBm_print(int input) //void phy_dBm_print(float input) //chunhua_20230320
{
    #if 0 //chunhua_20230320
	int a=0, b=0, c=0;

	a = (int)input;
	b = (int)(input*100)%100;
	c = abs(b);
	if((a==0)&&(b<0))
	{
		printk("(-0.%02d dBm)\r\n", c);
	}
	else
	{
		printk("(%d.%02d dBm)\r\n", a, c);
	}
    #endif
    if (input >= 0)
        printk("(%d.%02d dBm)\r\n", input/100, input%100); //chunhua_20230320
    else
        printk("(-%d.%02d dBm)\r\n", (-input)/100, (-input)%100); //chunhua_20230320
                
}

int is_en7572_7573(void) 
{

	UINT8 ptr[2];
	int ret;
	uint16 en7572_id_1,en7572_id_2;

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 2, 0x408, ptr, 2);
	en7572_id_1 = (uint16)(ptr[0] | (ptr[1]<<PHY_TRANS_BYTE_SIZE));

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, 2, 0x40A, ptr, 2);
	en7572_id_2 = (uint16)(ptr[0] | (ptr[1]<<PHY_TRANS_BYTE_SIZE));


	/*--------------------
	Take 0x408 and 0x40A for identifying.
	0x11C8 and 0xBE00 are IC default value.
	--------------------*/
	if((en7572_id_1 == 0x1388) || (en7572_id_2 == 0x007D))
	{
		printk("EN7572/EN7573 is detected!\n");
		ret=1;
	}
	else
	{
		printk("LDDLA is not EN7572/EN7573.\n");
		ret=0; 	
	}

	return ret;
	
}


int phy_set_trans_setting(void)
{
	uint ret = 0;
	UINT32 read_data = 0;
	
#if defined(TCSUPPORT_CPU_AN7583)
	#if XILINX_SERDES
		read_data=IO_GPHYREG(EN7583_PON_TX_NCPO_STS)& 0x10000000; //bit 28
		if((read_data&0x01)==0)			
			ret=en7581_set_trans_setting();			
		else
			ret=an7583_set_trans_setting();
	#else
		ret=an7583_set_trans_setting();	
	#endif

#elif defined(TCSUPPORT_CPU_EN7581)
	ret=en7581_set_trans_setting();
#else
	ret=en7580_set_trans_setting();
#endif
	return ret;
}

void phy_trans_model_found_7581(char *vendor_id,char *vendor_pn)//julia_7583
{		
	
	PPHY_TRANS_INFO pTrans = NULL;
	pTrans = phy_trans_iot_list;
	
	PON_PHY_PRINT(PHY_MSG_ERR,"%s\r\n","7581 old trans list");

//	gpPhyPriv->trans_tx_ben_level=LOW_ACTIVE;  //move A60972_SERDES param init to pon_phy_init 

		if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST==gpPhyPriv->trans_index)
		{
			while (strcmp(pTrans->vendor_id, "") != 0){
				if ((strncmp(pTrans->vendor_id, vendor_id, MAX(strlen(pTrans->vendor_id), strlen(vendor_id))) == 0) &&
					(strncmp(pTrans->vendor_pn, vendor_pn, MAX(strlen(pTrans->vendor_pn), strlen(vendor_pn))) == 0)){

					gpPhyPriv->trans_index=pTrans->index;
					
					PON_PHY_PRINT(PHY_MSG_TRANS,"Vendor ID and PN found in trans IOT list! index = %d\n",gpPhyPriv->trans_index);			

			#if 0 //  defined(TCSUPPORT_CT_C5_HEN_SFU)
					if(strcmp(vendor_id, "FIBERTOWER") == 0 || strcmp(vendor_id, "MENTECHOPTO") == 0)
					{
						isNeedResetTransciver = 1;
					}
			#endif
				
					break;
				}
				pTrans ++;
			}
		}
		
		PON_PHY_PRINT(PHY_MSG_TRANS,"trans vendor ID : %s\n", vendor_id);
		PON_PHY_PRINT(PHY_MSG_TRANS,"trans vendor PN : %s\n", vendor_pn);
}


void phy_trans_model_found_7583(char *vendor_id,char *vendor_pn)//julia_7583
{
	PPHY_TRANS_INFO_7583 pTrans_7583 = NULL;
	pTrans_7583 = phy_trans_iot_list_7583;
	
	PON_PHY_PRINT(PHY_MSG_ERR,"%s\r\n","7583 new trans list");
	
		if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST==gpPhyPriv->trans_index)
		{
			while (strcmp(pTrans_7583->vendor_id, "") != 0){
				if ((strncmp(pTrans_7583->vendor_id, vendor_id, MAX(strlen(pTrans_7583->vendor_id), strlen(vendor_id))) == 0) &&
					(strncmp(pTrans_7583->vendor_pn, vendor_pn, MAX(strlen(pTrans_7583->vendor_pn), strlen(vendor_pn))) == 0)){
		
					gpPhyPriv->trans_index=pTrans_7583->index;
						
					PON_PHY_PRINT(PHY_MSG_TRANS,"Vendor ID and PN found in trans IOT list! index = %d\n",gpPhyPriv->trans_index);			
						
					break;
				}
				pTrans_7583 ++;
			}
		}
		
		PON_PHY_PRINT(PHY_MSG_TRANS,"trans vendor ID : %s\n", vendor_id);
		PON_PHY_PRINT(PHY_MSG_TRANS,"trans vendor PN : %s\n", vendor_pn);
}


void phy_trans_model_found(char *vendor_id,char *vendor_pn) //julia_7583
{
	UINT32 read_data = 0;

#if defined(TCSUPPORT_CPU_AN7583)
	#if XILINX_SERDES
	read_data=IO_GPHYREG(EN7583_PON_TX_NCPO_STS)& 0x10000000; //bit 28
	if((read_data&0x01)==0) 		
			phy_trans_model_found_7581(vendor_id,vendor_pn);	
		else
			phy_trans_model_found_7583(vendor_id,vendor_pn);
	#else
		phy_trans_model_found_7583(vendor_id,vendor_pn);
	#endif

#else
	phy_trans_model_found_7581(vendor_id,vendor_pn);	

#endif

}


void xfp_trans_pw_entry(void)
{
 	uint buffer;
 	uint mod_pw;
 	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST != gpPhyPriv->trans_index)//cheng_20220314
  	{
 	mod_pw=xfp_trans_iot_list[gpPhyPriv->trans_index].mod_pw;

	//pass_word=0x12345678, pass word entry	
	buffer = ((mod_pw) >> 24) & 0xFF ;// 0x12
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_PASSWORDS_ENTRY_AREA_0, (unchar *)&buffer, 1);
	mdelay(5);
	buffer = ((mod_pw) >> 16) & 0xFF ;// 0x34
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_PASSWORDS_ENTRY_AREA_1, (unchar *)&buffer, 1);
	mdelay(5);
	buffer = ((mod_pw) >> 8) & 0xFF ;// 0x56
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_PASSWORDS_ENTRY_AREA_2, (unchar *)&buffer, 1);
	mdelay(5);
	buffer = ((mod_pw)>> 0) & 0xFF ;// 0x78
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_PASSWORDS_ENTRY_AREA_3, (unchar *)&buffer, 1);
	mdelay(5);
  	}

	return 0;

}




int xfp_trans_set_ngpon2_chan(PPHY_Ngpon2_Chan_Sel_T ngpon2_chan) //julia_20221021 ngpon2
{
	uint buffer;
	while(!kthread_should_stop())
	{
		if(gpPhyPriv->Ngpon2_chan.ngpon2_tx_chan != 5)
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_TX_CHAN_SEL, &buffer, 1);
			if(buffer != gpPhyPriv->Ngpon2_chan.ngpon2_tx_chan)
			{
				printk("xfp_trans_set_ngpon2_tx_chan %x\r\n",gpPhyPriv->Ngpon2_chan.ngpon2_tx_chan);
				xfp_trans_pw_entry();	//entry password
				
				buffer=gpPhyPriv->Ngpon2_chan.ngpon2_tx_chan;
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_TX_CHAN_SEL, (unchar *)&buffer, 1);
			}
			gpPhyPriv->Ngpon2_chan.ngpon2_tx_chan =5;
			phy_print_time();
		}
		mdelay(5);
		if(gpPhyPriv->Ngpon2_chan.ngpon2_rx_chan != 5)
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_RX_CHAN_SEL, &buffer, 1);
			if(buffer != gpPhyPriv->Ngpon2_chan.ngpon2_rx_chan)
			{
				printk("xfp_trans_set_ngpon2_rx_chan %x\r\n",gpPhyPriv->Ngpon2_chan.ngpon2_rx_chan);
				xfp_trans_pw_entry();	//entry password
				
				buffer=gpPhyPriv->Ngpon2_chan.ngpon2_rx_chan;
				phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_RX_CHAN_SEL, (unchar *)&buffer, 1);
			}
			gpPhyPriv->Ngpon2_chan.ngpon2_rx_chan =5;
			phy_print_time();
		}
		msleep(1000);
	}
	return PHY_SUCCESS;
}

int xfp_trans_set_ngpon2_tx_chan(PPHY_Ngpon2_Chan_Sel_T ngpon2_chan)
{
	uint buffer;

	xfp_trans_pw_entry();	//entry password
	
	buffer=ngpon2_chan->ngpon2_tx_chan;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_TX_CHAN_SEL, (unchar *)&buffer, 1);
	
	return PHY_SUCCESS;
}

int xfp_trans_set_ngpon2_rx_chan(PPHY_Ngpon2_Chan_Sel_T ngpon2_chan)
{
	uint buffer;

	xfp_trans_pw_entry();	//entry password

	buffer=ngpon2_chan->ngpon2_rx_chan;
	phy_I2C_write(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_RX_CHAN_SEL, (unchar *)&buffer, 1);

	return PHY_SUCCESS;
}

int xfp_trans_get_ngpon2_chan(PPHY_Ngpon2_Chan_Sel_T ngpon2_chan)
{
	
	UINT8 ptr[1];
	
	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_TX_CHAN_SEL, ptr, 1); //get module identifier sfp/xfp

	ngpon2_chan->ngpon2_tx_chan=ptr[0];

	mdelay(5);

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_NGPON2_RX_CHAN_SEL, ptr, 1); //get module identifier sfp/xfp

	ngpon2_chan->ngpon2_rx_chan=ptr[0];

	return PHY_SUCCESS;
}



int xfp_trans_model_setting(void)
{
	XXFP_TRANS_INFO pTrans = NULL;
//	static char trans_msg_print_cnt=95; 
	static char phy_trans_msg_off=FALSE;
	static char sif_print_flag;
		
	char vendor_id[PHY_TRANS_VENDOR_NAME_SIZE+1]={0};
	char vendor_pn[PHY_TRANS_VENDOR_NAME_SIZE+1]={0};
	
	pTrans = xfp_trans_iot_list;

	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		PON_PHY_PRINT(PHY_MSG_TRANS,"XFP get(%d) known trans_index = %d\r\n",gpPhyPriv->trans_msg_print_cnt,gpPhyPriv->trans_index);
		return phy_set_trans_setting();
	}

	//PON_PHY_PRINT(PHY_MSG_TRANS,"\r\n__phy_trans_model_setting__\r\n");
	gpPhyPriv->trans_msg_print_cnt++;
	
	if(!isFPGA) {
	    if(in_interrupt()){
			PON_PHY_PRINT(PHY_MSG_TRANS,"phy_trans_model_setting in interrupt!\r\n\r\n");
	        return PHY_FAILURE;
	    }
    }

	if ( (strlen(vendor_id) == 0) 
		&& (strlen(vendor_pn) == 0))
	{
		//PON_PHY_PRINT(PHY_MSG_TRANS,"i2c_u2_clk_div=0x%x\r\n",gpPhyPriv->i2c_u2_clk_div);
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_VENDOR_NAME , vendor_id, PHY_TRANS_VENDOR_NAME_SIZE);//read 16byte once
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, 1, XFP_TRANS_VENDOR_PN , vendor_pn, PHY_TRANS_VENDOR_NAME_SIZE);
		vendor_pn[PHY_TRANS_VENDOR_NAME_SIZE]='\0';
		vendor_id[PHY_TRANS_VENDOR_NAME_SIZE]='\0';

		rtrim(vendor_id);
		rtrim(vendor_pn);

	}

//	gpPhyPriv->trans_tx_ben_level=LOW_ACTIVE;  //move A60972_SERDES param init to pon_phy_init 

	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST==gpPhyPriv->trans_index)
	{
		while (strcmp(pTrans->vendor_id, "") != 0){
			if ((strncmp(pTrans->vendor_id, vendor_id, MAX(strlen(pTrans->vendor_id), strlen(vendor_id))) == 0) &&
				(strncmp(pTrans->vendor_pn, vendor_pn, MAX(strlen(pTrans->vendor_pn), strlen(vendor_pn))) == 0)){

				gpPhyPriv->trans_index=pTrans->index;

				if(phy_trans_msg_off==TRUE)
				{
					gpPhyPriv->trans_msg_print_cnt=0;
					phy_trans_msg_off=FALSE;
					
					gpPhyPriv->debugLevel|=PHY_MSG_ERR;
					gpPhyPriv->debugLevel|=PHY_MSG_TIME;
					gpPhyPriv->debugLevel|=PHY_MSG_TRANS;
					
					#if SIF_DEBUG_LEVEL_CONTROL
					sifm_proc_write(&sif_print_flag);
					#endif
				}
				PON_PHY_PRINT(PHY_MSG_TRANS,"Vendor ID and PN found in trans IOT list! index = %d\n",gpPhyPriv->trans_index);			

				#if 0 //  defined(TCSUPPORT_CT_C5_HEN_SFU)
				if(strcmp(vendor_id, "FIBERTOWER") == 0 || strcmp(vendor_id, "MENTECHOPTO") == 0)
				{
					isNeedResetTransciver = 1;
				}
				#endif
			
				break;
			}
			pTrans ++;
		}
	}
	
	
	PON_PHY_PRINT(PHY_MSG_TRANS,"trans vendor ID : %s\n", vendor_id);
	PON_PHY_PRINT(PHY_MSG_TRANS,"trans vendor PN : %s\n", vendor_pn);
	

	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		//handle_hw_irq_event(PHY_EVENT_TRANS_PLUG_IN);
		return phy_set_trans_setting();
	}
	else
	{
		if(gpPhyPriv->trans_msg_print_cnt>100)
		{
			gpPhyPriv->trans_msg_print_cnt=0;
			
			PON_PHY_PRINT(PHY_MSG_ALL,"\r\n------------------------------------\r\n| * Please insert optical module * |\r\n------------------------------------\r\n\r\n");
			if((phy_trans_msg_off==FALSE)&&(gpPhyPriv->debugLevel&PHY_MSG_ERR))
			{
				phy_trans_msg_off=TRUE;

				gpPhyPriv->debugLevel&=~PHY_MSG_ERR;
				gpPhyPriv->debugLevel&=~PHY_MSG_TIME;
				gpPhyPriv->debugLevel&=~PHY_MSG_TRANS;
				
				#if SIF_DEBUG_LEVEL_CONTROL
				sifm_proc_read(&sif_print_flag);
				sifm_proc_write(SIF_NO_LOG_PRINT);
				#endif
			}
		}
		PON_PHY_PRINT(PHY_MSG_TRANS,"trans NOT found in IOT list (%d) !!!\r\n",gpPhyPriv->trans_msg_print_cnt);
		//handle_hw_irq_event(PHY_EVENT_TRANS_PLUG_OUT);
		return PHY_FAILURE;
	}

	return PHY_SUCCESS;
}



int  phy_trans_model_setting(void)
{

	UINT8 ptr[4];
	
	if((gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_XEPON_1G_1G)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_GPON)||(gpPhyPriv->wan_sel == SCU_WAN_CONF_REG_WAN_SEL_TURBO_EPON))
	{		
        unchar ptr1[1]={0};
        UINT8 read_data_1 = 0;
        UINT8 read_data_2 = 0;
	    UINT8 read_data_3 = 0;
        phy_I2C_read(0, 0xc7, 0x70, 2, 0x170, ptr1, 1);
        read_data_1 = ptr1[0];	

        phy_I2C_read(0, 0xc7, 0x70, 2, 0x15c, ptr1, 1); 	
        read_data_2 = ptr1[0];

        phy_I2C_read(0, 0xc7, 0x70, 2, 0x2b4, ptr1, 1); 	
        read_data_3 = ptr1[0] & 0x03;
		
        if( (read_data_1 == 0x03) && (read_data_2 >= 0x03) && (read_data_3 == 0x01) )
        {
	        printk("IC detect: 7571\n");
	#ifdef TCSUPPORT_MT7570
	        gpPhyPriv->en7571_init_done=TRUE;
	#endif
	        printk("vendor id	  = ECONET\n");
	        printk("vendor pn	  = EN7571\n");
	        return PHY_SUCCESS;
        }
	}
	
	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		PON_PHY_PRINT(PHY_MSG_TRANS,"Get(%d) known trans_index = %d\r\n",gpPhyPriv->trans_msg_print_cnt,gpPhyPriv->trans_index);
		return phy_set_trans_setting();
	}

	phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, gpPhyPriv->i2c_addr_num, 0, ptr, 1); //get module identifier sfp/xfp
	gpPhyPriv->trans_mod_id=ptr[0];

 	if(gpPhyPriv->trans_mod_id==PHY_TRANS_IDENTIFIER_XFP)
	{
		xfp_trans_model_setting();
		printk("%s\r\n","XFP_MODULE");
		return PHY_SUCCESS;
	}
	//PPHY_TRANS_INFO pTrans = NULL;
	
	//PPHY_TRANS_INFO_7583 pTrans_7583 = NULL;
	//static char trans_msg_print_cnt=95; 
	static char phy_trans_msg_off=FALSE;
	static char sif_print_flag;
		
	char vendor_id[PHY_TRANS_VENDOR_NAME_SIZE+1]={0};
	char vendor_pn[PHY_TRANS_VENDOR_NAME_SIZE+1]={0};

	
	//pTrans = phy_trans_iot_list;
	//pTrans_7583= phy_trans_iot_list_7583;
	

	//PON_PHY_PRINT(PHY_MSG_TRANS,"\r\n__phy_trans_model_setting__\r\n");
	gpPhyPriv->trans_msg_print_cnt++;

	
	if(!isFPGA) {
	    if(in_interrupt()){
			PON_PHY_PRINT(PHY_MSG_TRANS,"phy_trans_model_setting in interrupt!\r\n\r\n");
	        return PHY_FAILURE;
	    }
    }

#ifdef TCSUPPORT_MT7572
	if(gpPhyPriv->trans_msg_print_cnt==96)
	{
		gpPhyPriv->i2c_addr_num=2;
	}
	else
	{
		gpPhyPriv->i2c_addr_num=1;
	}
#endif
/*
#ifdef TCSUPPORT_MT7572 //check lddla_id,set addr_num
	if(is_en7572_7573()) gpPhyPriv->i2c_addr_num=2; //addr_num must be 2 for en7572/7573, ang_20211112
#endif
*/
	if ( (strlen(vendor_id) == 0) 
		&& (strlen(vendor_pn) == 0))
	{
		//PON_PHY_PRINT(PHY_MSG_TRANS,"i2c_u2_clk_div=0x%x\r\n",gpPhyPriv->i2c_u2_clk_div);

		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, gpPhyPriv->i2c_addr_num, PHY_TRANS_VENDOR_NAME , vendor_id, PHY_TRANS_VENDOR_NAME_SIZE);//read 16byte once
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, gpPhyPriv->i2c_addr_num, PHY_TRANS_VENDOR_PN , vendor_pn, PHY_TRANS_VENDOR_NAME_SIZE);
		vendor_pn[PHY_TRANS_VENDOR_NAME_SIZE]='\0';
		vendor_id[PHY_TRANS_VENDOR_NAME_SIZE]='\0';

		rtrim(vendor_id);
		rtrim(vendor_pn);

	}
	
	if(phy_trans_msg_off==TRUE)
	{
		gpPhyPriv->trans_msg_print_cnt=0;
		phy_trans_msg_off=FALSE;
		
		gpPhyPriv->debugLevel|=PHY_MSG_ERR;
		gpPhyPriv->debugLevel|=PHY_MSG_TIME;
		gpPhyPriv->debugLevel|=PHY_MSG_TRANS;
		
		#if SIF_DEBUG_LEVEL_CONTROL
		sifm_proc_write(&sif_print_flag);
		#endif
	}
	
	phy_trans_model_found(vendor_id,vendor_pn);

	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		//handle_hw_irq_event(PHY_EVENT_TRANS_PLUG_IN);
		return phy_set_trans_setting();
	}
	else
	{
		if(gpPhyPriv->trans_msg_print_cnt>100)
		{
			gpPhyPriv->trans_msg_print_cnt=0;
			
			PON_PHY_PRINT(PHY_MSG_ALL,"\r\n------------------------------------\r\n| * Please insert optical module * |\r\n------------------------------------\r\n\r\n");
			if((phy_trans_msg_off==FALSE)&&(gpPhyPriv->debugLevel&PHY_MSG_ERR))
			{
				phy_trans_msg_off=TRUE;

				gpPhyPriv->debugLevel&=~PHY_MSG_ERR;
				gpPhyPriv->debugLevel&=~PHY_MSG_TIME;
				gpPhyPriv->debugLevel&=~PHY_MSG_TRANS;
				
				#if SIF_DEBUG_LEVEL_CONTROL
				sifm_proc_read(&sif_print_flag);
				sifm_proc_write(SIF_NO_LOG_PRINT);
				#endif
			}
		}
		PON_PHY_PRINT(PHY_MSG_TRANS,"trans NOT found in IOT list (%d)!!!\r\n",gpPhyPriv->trans_msg_print_cnt);
		//handle_hw_irq_event(PHY_EVENT_TRANS_PLUG_OUT);
		
			return PHY_FAILURE;
		
	}
}

int phy_trans_task_wait(void)
{
	int ret=0;

	PON_PHY_PRINT(PHY_MSG_TRANS, "\r\n#TASK:[%s] run\r\n",__FUNCTION__);
	
	while(!kthread_should_stop())
	{
		phy_print_time();
		PON_PHY_PRINT(PHY_MSG_TRANS, "#TASK:[%s] mac_set_phy_init_step = %d\r\n",__FUNCTION__,gpPhyPriv->mac_set_phy_init_step);
		if(gpPhyPriv->is_phy_start == TRUE)
		{
			PON_PHY_PRINT(PHY_MSG_TRANS, "\r\n#TASK:[%s] return\r\n\r\n",__FUNCTION__);
			return ret;
		}

		msleep(6000);

		if((gpPhyPriv->mac_set_phy_init_step >= 1)
			&&(gpPhyPriv->phy_init_done == FALSE))
		{
			if((ret = pon_phy_mod_init())!=0)
			{
				PON_PHY_PRINT(PHY_MSG_TRANS,"#TASK:[%s:%d] pon_phy_mod_init fail !\r\n",__FUNCTION__,__LINE__);
				continue;
			}
			if((ret = phy_trans_power_switch(gpPhyPriv->trans_tx_enable))!=0){
				PON_PHY_PRINT(PHY_MSG_TRANS,"#TASK:[%s:%d] phy_trans_power_switch fail !\r\n\r\n",__FUNCTION__,__LINE__);
				continue;
			}
		}
		if((gpPhyPriv->mac_set_phy_init_step >= 2)
#ifdef TCSUPPORT_MT7570
			&&((gpPhyPriv->trans_index != PHY_TRANS_NOT_FOUND_IN_IOT_LIST)||(gpPhyPriv->en7571_init_done==TRUE))
			#else
			&&(gpPhyPriv->trans_index != PHY_TRANS_NOT_FOUND_IN_IOT_LIST)
			#endif
			&&(gpPhyPriv->is_phy_start == FALSE))
		{
			if((ret = xpon_phy_start())!=0)
			{
				PON_PHY_PRINT(PHY_MSG_TRANS,"#TASK:[%s:%d] xpon_phy_start fail !\r\n",__FUNCTION__,__LINE__);
				continue;
			}
		}
	}
	
	return ret;
}


/*****************************************************************************
//function :
//		phy_trans_param_status_real
//description : 
//		this function is used to get transceiver parameter realtime
		Note: before call this function, should check if(i2c_protect == 0)
//input :	
//		PPHY_TransParam_T structure, defined in phy_api.h
//output :
//		N/A
******************************************************************************/
void phy_trans_param_status_real(PPHY_TransParam_T transceiver_param)
{

	UINT8 ptr[2];
#ifdef TCSUPPORT_MT7570
	LDDLA_Trans_Status_t trans_status = {0};
#endif
	//ushort u2ByteCnt;
	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		if(gpPhyPriv->trans_mod_id==PHY_TRANS_IDENTIFIER_XFP)

		{
			//temprature
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, gpPhyPriv->i2c_addr_num, PHY_TRANS_DIAGNOSTICS_TEMP_MSB, ptr, 2);
			transceiver_param->temprature = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
					
			//voltage
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, gpPhyPriv->i2c_addr_num, PHY_TRANS_DIAGNOSTICS_VCC_MSB, ptr, 2);
			transceiver_param->supply_voltage = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
					
			//tx_curretn
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, gpPhyPriv->i2c_addr_num, PHY_TRANS_TX_BIAS_MSB, ptr, 2);
			transceiver_param->tx_current = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
					
			//tx_power
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, gpPhyPriv->i2c_addr_num, PHY_TRANS_TX_POWER_MSB, ptr, 2);
			transceiver_param->tx_power = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
					
			//rx_power
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, XFP_TRANS_REG_TABLE, gpPhyPriv->i2c_addr_num, PHY_TRANS_RX_POWER_MSB, ptr, 2);
			transceiver_param->rx_power = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));

		}
		else
		{
			//temprature
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_DIAGNOSTICS_TEMP_MSB, ptr, 2);
			transceiver_param->temprature = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
					
			//voltage
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_DIAGNOSTICS_VCC_MSB, ptr, 2);
			transceiver_param->supply_voltage = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
					
			//tx_curretn
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_TX_BIAS_MSB, ptr, 2);
			transceiver_param->tx_current = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
					
			//tx_power
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_TX_POWER_MSB, ptr, 2);
			transceiver_param->tx_power = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
					
			//rx_power
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_RX_POWER_MSB, ptr, 2);
			transceiver_param->rx_power = (ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
		}
					
	}
#ifdef TCSUPPORT_MT7570
	else if(gpPhyPriv->en7571_init_done==TRUE)
	{
		LDDLA_GET_TRANS_STATUS(&trans_status);
		
		transceiver_param->temprature = trans_status.params.temprature;
		transceiver_param->supply_voltage = trans_status.params.supply_voltage;
		transceiver_param->tx_current = trans_status.params.tx_current;
		transceiver_param->tx_power = trans_status.params.tx_power;
		transceiver_param->rx_power = trans_status.params.rx_power;
	}
	#endif
		PON_PHY_PRINT(PHY_MSG_API, "temprature Verify: %.8x\n", transceiver_param->temprature);
		PON_PHY_PRINT(PHY_MSG_API, "voltage Verify: %.8x\n", transceiver_param->supply_voltage);
		PON_PHY_PRINT(PHY_MSG_API, "current Verify: %.8x\n", transceiver_param->tx_current);
		PON_PHY_PRINT(PHY_MSG_API, "tx power Verify: %.8x\n", transceiver_param->tx_power);
		PON_PHY_PRINT(PHY_MSG_API, "rx power Verify: %.8x\n", transceiver_param->rx_power);

		gpPhyPriv->temprature = transceiver_param->temprature; //julia_20230614
		
}

void phy_trans_params_show(void)
{
	UINT8 ptr[4];
	//float temp = 0; //chunhua_20230320
	UINT16 temp_ad = 0;
	int a = 0;
	int b = 0;
	int i ;
	if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		//BOSA temperature
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_DIAGNOSTICS_TEMP_MSB, ptr, 2);
		temp_ad = (uint16)(ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
		a = (int)ptr[0];
		b = (int)ptr[1];
		printk("BOSA_temperature = %d.%d C\n", a, b);

		//Bias current
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_TX_BIAS_MSB, ptr, 2);
		temp_ad = (uint16)(ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
		a = (int)temp_ad/500;
		b = (int)temp_ad%500*2;
		printk("I bias = %d.%.2dmA\n",a, b);

		// VCC
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_DIAGNOSTICS_VCC_MSB, ptr, 2);
		temp_ad = (uint16)(ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
		a = (int)temp_ad/10000;
		b = (int)temp_ad%10000;
		printk("VCC = %d.%.4dV\n",a, b);
	
		// Tx power
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_TX_POWER_MSB, ptr, 2);
		temp_ad = (uint16)(ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE)); //chunhua_20230320  //temp = (float)(ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));
		// a=(int)temp;
		printk("Transciver Tx power : %d uW", temp_ad);	//for debug
		phy_dBm_print(sff_8472_power_to_dBm(temp_ad));

		// Rx power by YW_20160517
		phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, PHY_TRANS_RX_POWER_MSB, ptr, 2);
		temp_ad = (uint16)(ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));	//chunhua_20230320 //temp = (float)(ptr[1] | (ptr[0]<<PHY_TRANS_BYTE_SIZE));	
		//a=(int)temp;
		printk("Transciver Rx power : %d uW", temp_ad);	//for debug
		phy_dBm_print(sff_8472_power_to_dBm(temp_ad));        
	}

#ifdef TCSUPPORT_MT7570
	else if(gpPhyPriv->en7571_init_done==TRUE)
	{
		printk("en7571 cmd : echo show_BoB_information > proc/ldd_la/debug\n");
	}
	#endif
	else
	{
		printk("trans not found!\n");
	}
}

void phy_trans_params_dump(void)
{
	UINT32 i=0,phy_reg_total_num=0;
	UINT8 ptr[4];
	PI2C_REG_ENTRY phy_reg_all=NULL;

	//if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		phy_reg_total_num=255;
		phy_reg_all=phy_trans_table_1;
	
		printk("\r\n%-32s =addr=value\r\n",">>> TRANS REG TABLE 1 DUMP <<<");
		for(i=0;i<=phy_reg_total_num;i++)
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, gpPhyPriv->i2c_addr_num, i, ptr, 1);
			(phy_reg_all+i)->def=ptr[0];
			printk("%-32s =%4d=0x%02x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
		}
			
		phy_reg_total_num=255;
		phy_reg_all=phy_trans_table_2;
		
		printk("\r\n%-32s =addr=value\r\n",">>> TRANS REG TABLE 2 DUMP <<<");
		for(i=0;i<=phy_reg_total_num;i++)
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, i, ptr, 1);
			(phy_reg_all+i)->def=ptr[0];
			printk("%-32s =%4d=0x%02x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
		}

	}

}

/*** Parse I2C REG table based on define in SFF 8472 < Diagnostic Monitoring Interface for Optical Transceivers > Rev 11.0 ***/
void phy_trans_params_parser(void)
{
	UINT32 i=0,phy_reg_total_num=0;
	UINT8 ptr[4];
	PI2C_REG_ENTRY phy_reg_all=NULL;

	//if(PHY_TRANS_NOT_FOUND_IN_IOT_LIST!=gpPhyPriv->trans_index)
	{
		//Table 3.1 Two-wire interface ID: Data Fields ¨C Address A0h
		phy_reg_total_num=255;
		phy_reg_all=phy_trans_table_1;
	
		printk("\r\n%-32s\r\n",">>> trans REG table 1 parser <<<");
		for(i=0;i<=phy_reg_total_num;i++)
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE1, gpPhyPriv->i2c_addr_num, i, ptr, 1);
			(phy_reg_all+i)->def=ptr[0];
			//printk("%-32s =%4d=0x%02x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
		}

		//TABLE 3.2: Identifier values
		printk("%-32s=%4d=0x%02x= ",phy_trans_table_1[PHY_TRANS_IDENTIFIER].name,phy_trans_table_1[PHY_TRANS_IDENTIFIER].addr,phy_trans_table_1[PHY_TRANS_IDENTIFIER].def);
		if((phy_trans_table_1[PHY_TRANS_IDENTIFIER].def)==PHY_TRANS_IDENTIFIER_SFF)
		{
			printk("%s\r\n","SFP/SFP+");
		}
		else if((phy_trans_table_1[PHY_TRANS_IDENTIFIER].def)==PHY_TRANS_IDENTIFIER_SFP_OR_SFP_PLUS)
		{
			printk("%s\r\n","SFP/SFP+");
		}
		else if((phy_trans_table_1[PHY_TRANS_IDENTIFIER].def)==PHY_TRANS_IDENTIFIER_XFP)
		{
			printk("%s\r\n","XFP");
		}
		else
		{
			printk("%s\r\n","other");
		}

		//TABLE 3.4: Connector values
		printk("%-32s=%4d=0x%02x= ",phy_trans_table_1[PHY_TRANS_CONNECTOR].name,phy_trans_table_1[PHY_TRANS_CONNECTOR].addr,phy_trans_table_1[PHY_TRANS_CONNECTOR].def);
		if((phy_trans_table_1[PHY_TRANS_CONNECTOR].def)==PHY_TRANS_CONNECTOR_SC)
		{
			printk("%s\r\n","SC");
		}
		else if((phy_trans_table_1[PHY_TRANS_CONNECTOR].def)==PHY_TRANS_CONNECTOR_LC)
		{
			printk("%s\r\n","LC");
		}
		else if((phy_trans_table_1[PHY_TRANS_CONNECTOR].def)==PHY_TRANS_CONNECTOR_RJ45)
		{
			printk("%s\r\n","RJ45");
		}
		else
		{
			printk("%s\r\n","other");
		}

		//TABLE 3.5: Transceiver codes(Address A0h)
		printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,phy_trans_table_1[PHY_TRANS_TRANSCEIVER].addr,phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def);
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def&PHY_TRANS_TRANSCEIVER_3_10GBASE_ER)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,"10GBASE_ER");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def&PHY_TRANS_TRANSCEIVER_3_10GBASE_LRM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,"10GBASE_LRM");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def&PHY_TRANS_TRANSCEIVER_3_10GBASE_LR)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,"10GBASE_LR");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def&PHY_TRANS_TRANSCEIVER_3_10GBASE_SR)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,"10GBASE_SR");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def&PHY_TRANS_TRANSCEIVER_3_1X_SX)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,"1X_SX");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def&PHY_TRANS_TRANSCEIVER_3_1X_LX)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,"1X_LX");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def&PHY_TRANS_TRANSCEIVER_3_1X_COPPER_ACTIVE)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,"1X_COPPER_ACTIVE");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER].def&PHY_TRANS_TRANSCEIVER_3_1X_COPPER_PASSIVE)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER].name,"1X_COPPER_PASSIVE");
		}

		
		printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].addr,phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def);
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def&PHY_TRANS_TRANSCEIVER_6_BASE_PX)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,"BASE_PX");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def&PHY_TRANS_TRANSCEIVER_6_BASE_BX10)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,"BASE_BX10");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def&PHY_TRANS_TRANSCEIVER_6_100BASE_FX)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,"100BASE_FX");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def&PHY_TRANS_TRANSCEIVER_6_100BASE_LX_LX10)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,"100BASE_LX_LX10");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def&PHY_TRANS_TRANSCEIVER_6_1000BASE_T)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,"1000BASE_T");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def&PHY_TRANS_TRANSCEIVER_6_1000BASE_CX)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,"1000BASE_CX");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def&PHY_TRANS_TRANSCEIVER_6_1000BASE_LX)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,"1000BASE_LX");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].def&PHY_TRANS_TRANSCEIVER_6_1000BASE_SX)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_6].name,"1000BASE_LX");
		}

		printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].name,phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].addr,phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].def);
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].def&PHY_TRANS_TRANSCEIVER_9_TW)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].name,"Twin Axial Pair");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].def&PHY_TRANS_TRANSCEIVER_9_TP)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].name,"Twisted Pair");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].def&PHY_TRANS_TRANSCEIVER_9_MI)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].name,"Miniature Coax");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].def&PHY_TRANS_TRANSCEIVER_9_TV)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].name,"Video Coax");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].def&PHY_TRANS_TRANSCEIVER_9_M6)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].name,"Multimode 62.5um");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].def&PHY_TRANS_TRANSCEIVER_9_M5_M5E)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].name,"Multimode 50um");
		}
		if(phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].def&PHY_TRANS_TRANSCEIVER_9_SM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_TRANSCEIVER_9].name,"Single Mode");
		}


		//Table 3.6: Encoding codes
		printk("%-32s=%4d=0x%02x= ",phy_trans_table_1[PHY_TRANS_ENCODING].name,phy_trans_table_1[PHY_TRANS_ENCODING].addr,phy_trans_table_1[PHY_TRANS_ENCODING].def);
		if((phy_trans_table_1[PHY_TRANS_ENCODING].def)==PHY_TRANS_ENCODING_8B_10B)
		{
			printk("%s\r\n","8B/10B");
		}
		else if((phy_trans_table_1[PHY_TRANS_ENCODING].def)==PHY_TRANS_ENCODING_NRZ)
		{
			printk("%s\r\n","NRZ");
		}
		else if((phy_trans_table_1[PHY_TRANS_ENCODING].def)==PHY_TRANS_ENCODING_64B_66B)
		{
			printk("%s\r\n","64B/66B");
		}
		else
		{
			printk("%s\r\n","other");
		}

        //BR
		printk("%-32s= %d M\r\n",phy_trans_table_1[PHY_TRANS_BR].name,phy_trans_table_1[PHY_TRANS_BR].def*100);

		//Length
		printk("%-32s= %d km\r\n",phy_trans_table_1[PHY_TRANS_LENGTH_SMF_KM].name,phy_trans_table_1[PHY_TRANS_LENGTH_SMF_KM].def);
		printk("%-32s= %d.%d km\r\n",phy_trans_table_1[PHY_TRANS_LENGTH_SMF_100M].name,phy_trans_table_1[PHY_TRANS_LENGTH_SMF_100M].def/10,phy_trans_table_1[PHY_TRANS_LENGTH_SMF_100M].def%10);
		printk("%-32s= %d.%d km\r\n",phy_trans_table_1[PHY_TRANS_LENGTH_50UM_10M].name,phy_trans_table_1[PHY_TRANS_LENGTH_50UM_10M].def/100,phy_trans_table_1[PHY_TRANS_LENGTH_50UM_10M].def%100);
		printk("%-32s= %d.%d km\r\n",phy_trans_table_1[PHY_TRANS_LENGTH_62P5UM_10M].name,phy_trans_table_1[PHY_TRANS_LENGTH_62P5UM_10M].def/100,phy_trans_table_1[PHY_TRANS_LENGTH_62P5UM_10M].def%100);
		printk("%-32s= %d.%d km\r\n",phy_trans_table_1[PHY_TRANS_LENGTH_CABLE].name,phy_trans_table_1[PHY_TRANS_LENGTH_CABLE].def/1000,phy_trans_table_1[PHY_TRANS_LENGTH_CABLE].def%1000);
		printk("%-32s= %d.%d km\r\n",phy_trans_table_1[PHY_TRANS_LENGTH_OM3].name,phy_trans_table_1[PHY_TRANS_LENGTH_OM3].def/100,phy_trans_table_1[PHY_TRANS_LENGTH_OM3].def%100);

		//Vendor name
		printk("%-32s= ",phy_trans_table_1[PHY_TRANS_VENDOR_NAME].name);
		for(i=0;i<PHY_TRANS_VENDOR_NAME_SIZE;i++)
		{
			printk("%c",phy_trans_table_1[PHY_TRANS_VENDOR_NAME+i].def);
		}
		printk("\r\n");
		
		//Vendor OUI
		printk("%-32s= ",phy_trans_table_1[PHY_TRANS_VENDOR_OUI].name);
		for(i=0;i<PHY_TRANS_VENDOR_OUI_SIZE;i++)
		{
			printk("%02x ",phy_trans_table_1[PHY_TRANS_VENDOR_OUI+i].def);
		}
		printk("\r\n");

		//Vendor PN
		printk("%-32s= ",phy_trans_table_1[PHY_TRANS_VENDOR_PN].name);
		for(i=0;i<PHY_TRANS_VENDOR_PN_SIZE;i++)
		{
			printk("%c",phy_trans_table_1[PHY_TRANS_VENDOR_PN+i].def);
		}
		printk("\r\n");

		//Vendor REV
		printk("%-32s= ",phy_trans_table_1[PHY_TRANS_VENDOR_REV].name);
		for(i=0;i<PHY_TRANS_VENDOR_REV_SIZE;i++)
		{
			printk("%c",phy_trans_table_1[PHY_TRANS_VENDOR_REV+i].def);
		}
		printk("\r\n");

		//Laser Wavelength
		if((phy_trans_table_1[PHY_TRANS_TRANSCEIVER_8].def&PHY_TRANS_TRANSCEIVER_8_SFP_CABLE_TECH_MASK)==0)
		{
			printk("%-32s= %d nm\r\n",phy_trans_table_1[PHY_TRANS_WAVELENGTH].name,
				((phy_trans_table_1[PHY_TRANS_WAVELENGTH].def<<PHY_TRANS_BYTE_SIZE)|phy_trans_table_1[PHY_TRANS_WAVELENGTH+1].def));
		}
		
		//Table 3.7: Option values
		printk("%-32s= 0x%02x\r\n",phy_trans_table_1[PHY_TRANS_OPTIONS_65].name,phy_trans_table_1[PHY_TRANS_OPTIONS_65].def);
		if(phy_trans_table_1[PHY_TRANS_OPTIONS_65].def&PHY_TRANS_OPTIONS_65_RATE_SELECT)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_OPTIONS_65].name,"rate select");
		}
		if(phy_trans_table_1[PHY_TRANS_OPTIONS_65].def&PHY_TRANS_OPTIONS_65_TX_DISABLE)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_OPTIONS_65].name,"tx disable");
		}
		if(phy_trans_table_1[PHY_TRANS_OPTIONS_65].def&PHY_TRANS_OPTIONS_65_TX_FAULT)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_OPTIONS_65].name,"tx fault");
		}
		if(phy_trans_table_1[PHY_TRANS_OPTIONS_65].def&PHY_TRANS_OPTIONS_65_LOS_INV)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_OPTIONS_65].name,"LOS inverted");
		}
		if(phy_trans_table_1[PHY_TRANS_OPTIONS_65].def&PHY_TRANS_OPTIONS_65_LOS)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_OPTIONS_65].name,"LOS");
		}
		
		//Vendor SN
		printk("%-32s= ",phy_trans_table_1[PHY_TRANS_VENDOR_SN].name);
		for(i=0;i<PHY_TRANS_VENDOR_SN_SIZE;i++)
		{
			printk("%c",phy_trans_table_1[PHY_TRANS_VENDOR_SN+i].def);
		}
		printk("\r\n");

		//Date code
		printk("%-32s= ",phy_trans_table_1[PHY_TRANS_DATE_CODE].name);
		for(i=0;i<PHY_TRANS_DATE_CODE_SIZE;i++)
		{
			printk("%c",phy_trans_table_1[PHY_TRANS_DATE_CODE+i].def);
		}
		printk("\r\n");
		
		//Table 3.9: Diagnostic Monitoring Type
        printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].name,phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].addr,phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].def);
		if(phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].def&PHY_TRANS_DIAG_MONITOR_TYPE_DIGITAL_DIAG_MONITOR)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].name,"digital diagnostic monitor implemented");
		}
		if(phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].def&PHY_TRANS_DIAG_MONITOR_TYPE_INTERNAL_CALIBRATED)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].name,"internal calibrated");
		}
		if(phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].def&PHY_TRANS_DIAG_MONITOR_TYPE_EXTERNAL_CALIBRATED)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].name,"external calibrated");
		}
		printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].name,
			(phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].def&PHY_TRANS_DIAG_MONITOR_TYPE_AVERAGE_POWER)?"average power":"OMA");
		if(phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].def&PHY_TRANS_DIAG_MONITOR_TYPE_ADDRESSING_MODES)
		{
			printk("%-32s= %s\r\n",phy_trans_table_1[PHY_TRANS_DIAG_MONITOR_TYPE].name,"addressing modes");
		}

        //Table 3.10: Enhanced Options
        printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].name,phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].addr,phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].def);
		if(phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].def&PHY_TRANS_ENHANCED_OPTIONS_A_W)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].name,"alarm warning");
		}
		if(phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].def&PHY_TRANS_ENHANCED_OPTIONS_SOFT_TX_DISABLE)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].name,"soft tx disable");
		}
		if(phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].def&PHY_TRANS_ENHANCED_OPTIONS_SOFT_TX_FAULT)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].name,"soft tx fault");
		}
		if(phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].def&PHY_TRANS_ENHANCED_OPTIONS_SOFT_RX_LOS)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].name,"soft rx LOS");
		}
		if(phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].def&PHY_TRANS_ENHANCED_OPTIONS_SOFT_RATE_SELECT)
		{
			printk("%-32s= %s implemented\r\n",phy_trans_table_1[PHY_TRANS_ENHANCED_OPTIONS].name,"soft rate select");
		}

		//Table 3.1a Diagnostics: Data Fields ¨C Address A2h
		phy_reg_total_num=255;
		phy_reg_all=phy_trans_table_2;
		
		printk("\r\n%-32s\r\n",">>> trans REG table 2 parser <<<");
		for(i=0;i<=phy_reg_total_num;i++)
		{
			phy_I2C_read(0, gpPhyPriv->i2c_u2_clk_div, PHY_TRANS_REG_TABLE2, gpPhyPriv->i2c_addr_num, i, ptr, 1);
			(phy_reg_all+i)->def=ptr[0];
			//printk("%-32s =%4d=0x%02x\r\n",(phy_reg_all+i)->name,(phy_reg_all+i)->addr,(phy_reg_all+i)->def);
		}


		//Table 3.15: Alarm and Warning Thresholds (2-Wire Address A2h)
		printk("%-32s= %d.%02u C\r\n","Temp High Alarm Threshold",(char)phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TEMP_HIGH_ALARM_MSB].def, (phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TEMP_HIGH_ALARM_LSB].def*100)>>8);
		printk("%-32s= %d.%02u C\r\n","Temp Low Alarm Threshold",(char)phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TEMP_LOW_ALARM_MSB].def, (phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TEMP_LOW_ALARM_LSB].def*100)>>8);
		printk("%-32s= %d.%02u C\r\n","Temp High Warning Threshold",(char)phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TEMP_HIGH_WARNING_MSB].def, (phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TEMP_HIGH_WARNING_LSB].def*100)>>8);
		printk("%-32s= %d.%02u C\r\n","Temp Low Warning Threshold",(char)phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TEMP_LOW_WARNING_MSB].def, (phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TEMP_LOW_WARNING_LSB].def*100)>>8);
		
		printk("%-32s= %d.%d V\n","Voltage High Alarm Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_ALARM_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_ALARM_LSB].def))%10000);
		printk("%-32s= %d.%d V\n","Voltage Low Alarm Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_ALARM_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_ALARM_LSB].def))%10000);
		printk("%-32s= %d.%d V\n","Voltage High Warning Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_WARNING_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_WARNING_LSB].def))%10000);
		printk("%-32s= %d.%d V\n","Voltage Low Warning Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_WARNING_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_WARNING_LSB].def))%10000);

		printk("%-32s= %d.%d mA\r\n","TX BIAS High Alarm Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_ALARM_LSB].def))/500,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_ALARM_LSB].def))%500);
		printk("%-32s= %d.%d mA\r\n","TX BIAS Low Alarm Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_ALARM_LSB].def))/500,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_ALARM_LSB].def))%500);
		printk("%-32s= %d.%d mA\r\n","TX BIAS High Warning Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_WARNING_LSB].def))/500,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_WARNING_LSB].def))%500);
		printk("%-32s= %d.%d mA\r\n","TX BIAS Low Warning Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_WARNING_LSB].def))/500,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_WARNING_LSB].def))%500);


		printk("%-32s= %d.%04d mW ","TX Power High Alarm Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_LSB].def)))); //chunhua_20230320 delete float

		
		printk("%-32s= %d.%04d mW ","TX Power Low Alarm Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_LSB].def)))); //chunhua_20230320 delete float
		
		printk("%-32s= %d.%04d mW ","TX Power High Warning Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_LSB].def)))); //chunhua_20230320 delete float
		
		printk("%-32s= %d.%04d mW ","TX Power Low Warning Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_LSB].def)))); //chunhua_20230320 delete float

		printk("%-32s= %d.%04d mW ","RX Power High Alarm Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_LSB].def)))); //chunhua_20230320 delete float
 
		printk("%-32s= %d.%04d mW ","RX Power Low Alarm Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_LSB].def)))); //chunhua_20230320 delete float

		printk("%-32s= %d.%04d mW ","RX Power High Warning Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_LSB].def)))); //chunhua_20230320 delete float

		printk("%-32s= %d.%04d mW ","RX Power Low Warning Threshold",
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_LSB].def)))); //chunhua_20230320 delete float


		//TABLE 3.16: Calibration constants for External Calibration Option (2 Wire Address A2h)
		
		
		//TABLE 3.17: A/D Values and Status Bits (2 Wire Address A2h)
		//Table 3.13: Bit weights (¡ãC) for temperature reporting registers
		printk("%-32s= %d.%02u C\r\n","PHY_TRANS_DIAGNOSTICS_TEMP",(char)phy_trans_table_2[PHY_TRANS_DIAGNOSTICS_TEMP_MSB].def, (phy_trans_table_2[PHY_TRANS_DIAGNOSTICS_TEMP_LSB].def*100)>>8);
		
		printk("%-32s= %d.%d V\n","PHY_TRANS_DIAGNOSTICS_VCC",
			((phy_trans_table_2[PHY_TRANS_DIAGNOSTICS_VCC_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_DIAGNOSTICS_VCC_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_DIAGNOSTICS_VCC_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_DIAGNOSTICS_VCC_LSB].def))%10000);

		printk("%-32s= %d.%d mA\r\n","PHY_TRANS_TX_BIAS",
			((phy_trans_table_2[PHY_TRANS_TX_BIAS_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_TX_BIAS_LSB].def))/500,
			((phy_trans_table_2[PHY_TRANS_TX_BIAS_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_TX_BIAS_LSB].def))%500*2);
		
		printk("%-32s= %d.%04d mW ","PHY_TRANS_TX_POWER",
			((phy_trans_table_2[PHY_TRANS_TX_POWER_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_TX_POWER_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_TX_POWER_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_TX_POWER_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)((phy_trans_table_2[PHY_TRANS_TX_POWER_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_TX_POWER_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm(((phy_trans_table_2[PHY_TRANS_TX_POWER_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_TX_POWER_LSB].def)))); //chunhua_20230320 delete float

		printk("%-32s= %d.%04d mW ","PHY_TRANS_RX_POWER",
			((phy_trans_table_2[PHY_TRANS_RX_POWER_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_RX_POWER_LSB].def))/10000,
			((phy_trans_table_2[PHY_TRANS_RX_POWER_MSB].def<<PHY_TRANS_BYTE_SIZE)|(phy_trans_table_2[PHY_TRANS_RX_POWER_LSB].def))%10000);
		//phy_dBm_print(sff_8472_power_to_dBm((float)(((phy_trans_table_2[PHY_TRANS_RX_POWER_MSB].def<<PHY_TRANS_BYTE_SIZE)|phy_trans_table_2[PHY_TRANS_RX_POWER_LSB].def))));
		phy_dBm_print(sff_8472_power_to_dBm((((phy_trans_table_2[PHY_TRANS_RX_POWER_MSB].def<<PHY_TRANS_BYTE_SIZE)|phy_trans_table_2[PHY_TRANS_RX_POWER_LSB].def)))); //chunhua_20230320 delete float

		printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].name,phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].addr,phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].def);
		printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].name,
			(phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].def&PHY_TRANS_STATUS_CONTROL_TX_DISABLE_STATE)?"tx disable state":"tx not disable state");
		
		printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].name,
			(phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].def&PHY_TRANS_STATUS_CONTROL_SOFT_TX_DISABLE_SELECT)?"soft tx disable select":"soft tx disable not select");

		printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].name,
			(phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].def&PHY_TRANS_STATUS_CONTROL_TX_FAULT_STATE)?"tx fault state":"tx no fault state");

		printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].name,
			(phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].def&PHY_TRANS_STATUS_CONTROL_RX_LOS_STATE)?"rx LOS state":"rx no LOS state");

        printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].name,
			(phy_trans_table_2[PHY_TRANS_STATUS_CONTROL].def&PHY_TRANS_STATUS_CONTROL_DATA_READY_BAR_STATE)?"data ready bar state":"data not ready bar state");


		//Table 3.18: Alarm and Warning Flag Bits (2-Wire Address A2h)
		printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].addr,phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def);
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def&PHY_TRANS_ALARM_FLAGS_TEMP_HIGH_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,"Temp High Alarm");
		}
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def&PHY_TRANS_ALARM_FLAGS_TEMP_LOW_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,"Temp Low Alarm");
		}
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def&PHY_TRANS_ALARM_FLAGS_VCC_HIGH_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,"Vcc High Alarm");
		}
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def&PHY_TRANS_ALARM_FLAGS_VCC_LOW_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,"Vcc Low Alarm");
		}
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def&PHY_TRANS_ALARM_FLAGS_TX_BIAS_HIGH_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,"TX Bias High Alarm");
		}
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def&PHY_TRANS_ALARM_FLAGS_TX_BIAS_LOW_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,"TX Bias Low Alarm");
		}
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def&PHY_TRANS_ALARM_FLAGS_TX_POWER_HIGH_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,"TX Power High Alarm");
		}
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].def&PHY_TRANS_ALARM_FLAGS_TX_POWER_LOW_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS].name,"TX Power Low Alarm");
		}
		
		printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS_2].name,phy_trans_table_2[PHY_TRANS_ALARM_FLAGS_2].addr,phy_trans_table_2[PHY_TRANS_ALARM_FLAGS_2].def);
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS_2].def&PHY_TRANS_ALARM_FLAGS_2_RX_POWER_HIGH_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS_2].name,"RX Power High Alarm");
		}
		if(phy_trans_table_2[PHY_TRANS_ALARM_FLAGS_2].def&PHY_TRANS_ALARM_FLAGS_2_RX_POWER_LOW_ALARM)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_ALARM_FLAGS_2].name,"RX Power Low Alarm");
		}
		
		printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].addr,phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def);
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def&PHY_TRANS_WARNING_FLAGS_TEMP_HIGH_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,"Temp High Warning");
		}
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def&PHY_TRANS_WARNING_FLAGS_TEMP_LOW_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,"Temp Low Warning");
		}
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def&PHY_TRANS_WARNING_FLAGS_VCC_HIGH_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,"Vcc High Warning");
		}
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def&PHY_TRANS_WARNING_FLAGS_VCC_LOW_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,"Vcc Low Warning");
		}
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def&PHY_TRANS_WARNING_FLAGS_TX_BIAS_HIGH_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,"TX Bias High Warning");
		}
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def&PHY_TRANS_WARNING_FLAGS_TX_BIAS_LOW_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,"TX Bias Low Warning");
		}
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def&PHY_TRANS_WARNING_FLAGS_TX_POWER_HIGH_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,"TX Power High Warning");
		}
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].def&PHY_TRANS_WARNING_FLAGS_TX_POWER_LOW_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS].name,"TX Power Low Warning");
		}

		printk("%-32s=%4d=0x%02x\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS_2].name,phy_trans_table_2[PHY_TRANS_WARNING_FLAGS_2].addr,phy_trans_table_2[PHY_TRANS_WARNING_FLAGS_2].def);
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS_2].def&PHY_TRANS_WARNING_FLAGS_2_RX_POWER_HIGH_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS_2].name,"RX Power High Warning");
		}
		if(phy_trans_table_2[PHY_TRANS_WARNING_FLAGS_2].def&PHY_TRANS_WARNING_FLAGS_2_RX_POWER_LOW_WARNING)
		{
			printk("%-32s= %s\r\n",phy_trans_table_2[PHY_TRANS_WARNING_FLAGS_2].name,"RX Power Low Warning");
		}

	}

}


