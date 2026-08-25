#ifndef _I2C_H_
#define _I2C_H_

// ----- Include ----- 

#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include <ecnt_hook/ecnt_hook_pon_phy.h>
#include <asm/tc3162/tc3162.h>

#include "phy.h"
#include "phy_def.h"
#include "phy_debug.h"
#include "phy_init.h"
#include "phy_tx.h"
//#include "en7580.h"
#include "phy_reg.h"

// ----- Define for XFP----- 

/*** INF-8077i <XFP Rev 4.5>***/

//________Two-wire interface ID: Data Fields ¨C Address A0h________

#define	XFP_TRANS_REG_TABLE		    0x50

// XFP low memory map
#define	XFP_TRANS_IDENTIFIER_LOW_MEM           				0
#define	XFP_TRANS_SINGNAL_CONDITIONER_CONTROL  				1
#define	XFP_TRANS_TEMP_HIGH_ALARM_MSB          				2
#define	XFP_TRANS_TEMP_HIGH_ALARM_LSB          				3
#define	XFP_TRANS_TEMP_LOW_ALARM_MSB           				4
#define	XFP_TRANS_TEMP_LOW_ALARM_LSB           				5
#define	XFP_TRANS_TEMP_HIGH_WARNING_MSB        				6
#define	XFP_TRANS_TEMP_HIGH_WARNING_LSB        				7
#define	XFP_TRANS_TEMP_LOW_WARNING_MSB         				8
#define	XFP_TRANS_TEMP_LOW_WARNING_LSB         				9
#define	XFP_TRANS_VOLTAGE_HIGH_ALARM_MSB       				10
#define	XFP_TRANS_VOLTAGE_HIGH_ALARM_LSB       				11
#define	XFP_TRANS_VOLTAGE_LOW_ALARM_MSB        				12
#define	XFP_TRANS_VOLTAGE_LOW_ALARM_LSB        				13
#define	XFP_TRANS_VOLTAGE_HIGH_WARNING_MSB     				14
#define	XFP_TRANS_VOLTAGE_HIGH_WARNING_LSB     				15
#define	XFP_TRANS_VOLTAGE_LOW_WARNING_MSB      				16
#define	XFP_TRANS_VOLTAGE_LOW_WARNING_LSB      				17
#define	XFP_TRANS_BIAS_HIGH_ALARM_MSB          				18
#define	XFP_TRANS_BIAS_HIGH_ALARM_LSB          				19
#define	XFP_TRANS_BIAS_LOW_ALARM_MSB           				20
#define	XFP_TRANS_BIAS_LOW_ALARM_LSB           				21
#define	XFP_TRANS_BIAS_HIGH_WARNING_MSB        				22
#define	XFP_TRANS_BIAS_HIGH_WARNING_LSB        				23
#define	XFP_TRANS_BIAS_LOW_WARNING_MSB         				24
#define	XFP_TRANS_BIAS_LOW_WARNING_LSB         				25
#define	XFP_TRANS_TX_POWER_HIGH_ALARM_MSB      				26
#define	XFP_TRANS_TX_POWER_HIGH_ALARM_LSB      				27
#define	XFP_TRANS_TX_POWER_LOW_ALARM_MSB       				28
#define	XFP_TRANS_TX_POWER_LOW_ALARM_LSB       				29
#define	XFP_TRANS_TX_POWER_HIGH_WARNING_MSB    				30
#define	XFP_TRANS_TX_POWER_HIGH_WARNING_LSB    				31
#define	XFP_TRANS_TX_POWER_LOW_WARNING_MSB     				32
#define	XFP_TRANS_TX_POWER_LOW_WARNING_LSB     				33
#define	XFP_TRANS_RX_POWER_HIGH_ALARM_MSB      				34
#define	XFP_TRANS_RX_POWER_HIGH_ALARM_LSB      				35
#define	XFP_TRANS_RX_POWER_LOW_ALARM_MSB       				36
#define	XFP_TRANS_RX_POWER_LOW_ALARM_LSB       				37
#define	XFP_TRANS_RX_POWER_HIGH_WARNING_MSB    				38
#define	XFP_TRANS_RX_POWER_HIGH_WARNING_LSB    				39
#define	XFP_TRANS_RX_POWER_LOW_WARNING_MSB     				40
#define	XFP_TRANS_RX_POWER_LOW_WARNING_LSB     				41
#define	XFP_TRANS_AUX1_HIGH_ALARM_MSB          				42   	
#define	XFP_TRANS_AUX1_HIGH_ALARM_LSB          				43   	
#define	XFP_TRANS_AUX1_LOW_ALARM_MSB           				44   	
#define	XFP_TRANS_AUX1_LOW_ALARM_LSB           				45   	
#define	XFP_TRANS_AUX1_HIGH_WARNING_MSB        				46   	
#define	XFP_TRANS_AUX1_HIGH_WARNING_LSB        				47   	
#define	XFP_TRANS_AUX1_LOW_WARNING_MSB         				48   	
#define	XFP_TRANS_AUX1_LOW_WARNING_LSB         				49   	
#define	XFP_TRANS_AUX2_HIGH_ALARM_MSB          				50   	
#define	XFP_TRANS_AUX2_HIGH_ALARM_LSB          				51   	
#define	XFP_TRANS_AUX2_LOW_ALARM_MSB           				52   	
#define	XFP_TRANS_AUX2_LOW_ALARM_LSB           				53   	
#define	XFP_TRANS_AUX2_HIGH_WARNING_MSB        				54   	
#define	XFP_TRANS_AUX2_HIGH_WARNING_LSB        				55   	
#define	XFP_TRANS_AUX2_LOW_WARNING_MSB         				56   	
#define	XFP_TRANS_AUX2_LOW_WARNING_LSB         				57
#define	XFP_TRANS_OPTIONAL_VPS_CTRL_REG_0      				58      	
#define	XFP_TRANS_OPTIONAL_VPS_CTRL_REG_1      				59
#define	XFP_TRANS_RESERVED_0                   				60
#define	XFP_TRANS_RESERVED_1                   				61
#define	XFP_TRANS_RESERVED_2                   				62
#define	XFP_TRANS_RESERVED_3                   				63
#define	XFP_TRANS_RESERVED_4                   				64
#define	XFP_TRANS_RESERVED_5                   				65
#define	XFP_TRANS_RESERVED_6                   				66
#define	XFP_TRANS_RESERVED_7                   				67
#define	XFP_TRANS_RESERVED_8                   				68
#define	XFP_TRANS_RESERVED_9                   				69
#define	XFP_TRANS_BER_REPORTING_0              				70
#define	XFP_TRANS_BER_REPORTING_1              				71
#define	XFP_TRANS_WAVELENGTH_CTRL_REG_0        				72
#define	XFP_TRANS_WAVELENGTH_CTRL_REG_1        				73
#define	XFP_TRANS_WAVELENGTH_CTRL_REG_2        				74
#define	XFP_TRANS_WAVELENGTH_CTRL_REG_3        				75
#define	XFP_TRANS_FEC_CTRL_REG_0               				76
#define	XFP_TRANS_FEC_CTRL_REG_1               				77
#define	XFP_TRANS_FEC_CTRL_REG_2               				78
#define	XFP_TRANS_FEC_CTRL_REG_3               				79
#define	XFP_TRANS_FLAG_AND_INT_CTRL_REG_0      				80
#define	XFP_TRANS_FLAG_AND_INT_CTRL_REG_1      				81
#define	XFP_TRANS_FLAG_AND_INT_CTRL_REG_2      				82
#define	XFP_TRANS_FLAG_AND_INT_CTRL_REG_3      				83
#define	XFP_TRANS_FLAG_AND_INT_CTRL_REG_4      				84
#define	XFP_TRANS_FLAG_AND_INT_CTRL_REG_5      				85
#define	XFP_TRANS_FLAG_AND_INT_CTRL_REG_6      				86
#define	XFP_TRANS_FLAG_AND_INT_CTRL_REG_7      				87
#define	XFP_TRANS_INT_MASKING_BITS_0           				88
#define	XFP_TRANS_INT_MASKING_BITS_1           				89
#define	XFP_TRANS_INT_MASKING_BITS_2           				90
#define	XFP_TRANS_INT_MASKING_BITS_3           				91
#define	XFP_TRANS_INT_MASKING_BITS_4           				92
#define	XFP_TRANS_INT_MASKING_BITS_5           				93
#define	XFP_TRANS_INT_MASKING_BITS_6           				94
#define	XFP_TRANS_INT_MASKING_BITS_7           				95
#define XFP_TRANS_DIAGNOSTICS_TEMP_MSB         				96
#define XFP_TRANS_DIAGNOSTICS_TEMP_LSB         				97
#define XFP_TRANS_DIAGNOSTICS_VCC_MSB          				98
#define XFP_TRANS_DIAGNOSTICS_VCC_LSB          				99
#define XFP_TRANS_TX_BIAS_MSB                  				100
#define XFP_TRANS_TX_BIAS_LSB                  				101
#define XFP_TRANS_TX_POWER_MSB                 				102
#define XFP_TRANS_TX_POWER_LSB                 				103
#define XFP_TRANS_RX_POWER_MSB                 				104
#define XFP_TRANS_RX_POWER_LSB                 				105
#define XFP_TRANS_AUX1_MSB              					106 
#define XFP_TRANS_AUX1_LSB              					107
#define XFP_TRANS_AUX2_MSB                     				108
#define XFP_TRANS_AUX2_LSB                     				109
#define XFP_TRANS_GENERAL_CTRL_STA_BITS_0      				110
#define XFP_TRANS_GENERAL_CTRL_STA_BITS_1      				111
#define	XFP_TRANS_NGPON2_TX_CHAN_SEL           				112           	
#define	XFP_TRANS_NGPON2_RX_CHAN_SEL           				113           	
#define	XFP_TRANS_RESERVED_10                  				114
#define	XFP_TRANS_RESERVED_11                  				115
#define	XFP_TRANS_RESERVED_12                  				116
#define	XFP_TRANS_RESERVED_13                  				117
#define	XFP_TRANS_RESERVED_14                  				118
#define	XFP_TRANS_PASSWORDS_CHANGE_AREA_0      				119      
#define	XFP_TRANS_PASSWORDS_CHANGE_AREA_1      				120     
#define	XFP_TRANS_PASSWORDS_CHANGE_AREA_2      				121     
#define	XFP_TRANS_PASSWORDS_CHANGE_AREA_3      				122     
#define	XFP_TRANS_PASSWORDS_ENTRY_AREA_0       				123      
#define	XFP_TRANS_PASSWORDS_ENTRY_AREA_1       				124      
#define	XFP_TRANS_PASSWORDS_ENTRY_AREA_2       				125      
#define	XFP_TRANS_PASSWORDS_ENTRY_AREA_3       				126
#define	XFP_TRANS_PAGE_SEL_BYTE                				127


// XFP high memory map                      
#define	XFP_TRANS_IDENTIFIER						128                	
#define	XFP_TRANS_EXT_IDENTIFIER					129            	
#define	XFP_TRANS_CONNECTOR             			130    	
#define	XFP_TRANS_TRANSCEIVER_0         			131      	
#define	XFP_TRANS_TRANSCEIVER_1         			132    	
#define	XFP_TRANS_TRANSCEIVER_2         			133    	
#define	XFP_TRANS_TRANSCEIVER_3 					134
#define	XFP_TRANS_TRANSCEIVER_4         			135    	
#define	XFP_TRANS_TRANSCEIVER_5         			136    	
#define	XFP_TRANS_TRANSCEIVER_6						137
#define	XFP_TRANS_TRANSCEIVER_7						138
#define	XFP_TRANS_ENCODING              			139    	
#define	XFP_TRANS_BR_MIN  							140
#define	XFP_TRANS_BR_MAX          					141
#define	XFP_TRANS_LENGTH_SMF_KM         			142    	
#define	XFP_TRANS_LENGTH_EBW_50UM       			143    	
#define	XFP_TRANS_LENGTH_50UM          				144
#define	XFP_TRANS_LENGTH_62P5UM         			145	
#define	XFP_TRANS_LENGTH_CABLE          			146    	
#define	XFP_TRANS_DEVICE_TEC						147
#define	XFP_TRANS_VENDOR_NAME						148
#define	XFP_TRANS_CDR        						164    	
#define	XFP_TRANS_VENDOR_OUI						165
#define	XFP_TRANS_VENDOR_PN							168                 	
#define	XFP_TRANS_VENDOR_REV						184                	
#define	XFP_TRANS_TX_WAVELENGTH_MSB					186
#define	XFP_TRANS_TX_WAVELENGTH_LSB					187
#define	XFP_TRANS_WAVELENGTH_TOLERANCE_MSB			188
#define	XFP_TRANS_WAVELENGTH_TOLERANCE_LSB			189
#define	XFP_TRANS_MAX_TEMPERATURE           		190
#define	XFP_TRANS_CC_BASE							191
#define	XFP_TRANS_POWER_SUPPLY_0         			192      	
#define	XFP_TRANS_POWER_SUPPLY_1            		193 	
#define	XFP_TRANS_POWER_SUPPLY_2            		194 	
#define	XFP_TRANS_POWER_SUPPLY_3					195                 	
#define	XFP_TRANS_VENDOR_SN                 		196	
#define	XFP_TRANS_DATE_CODE                 		212	
#define	XFP_TRANS_DIAG_MONITOR_TYPE         		220	
#define	XFP_TRANS_ENHANCED_OPTIONS          		221	
#define	XFP_TRANS_AUX_MONITORING       				222
#define	XFP_TRANS_CC_EXT                    		223	
#define	XFP_TRANS_RESERVED                  		224	


#define XFP_TRANS_BYTE_SIZE                     	8


#define XFP_TRANS_TX_DISABLE_STA                    (1<<7)
#define XFP_TRANS_SOFT_TX_DISABLE_SEL               (1<<6)
#define XFP_TRANS_MOD_NR_STA                    	(1<<5)
#define XFP_TRANS_POWER_DOWN_STA                    (1<<4)
#define XFP_TRANS_SOFT_POWER_DOWN_SEL               (1<<3)
#define XFP_TRANS_INT_STA                     		(1<<2)
#define XFP_TRANS_RX_LOS_STA                    	(1<<1)
#define XFP_TRANS_DATA_NR_STA                    	(1)

#define XFP_TRANS_TX_NR_STA							(1<<7)
#define XFP_TRANS_TX_FAULT_STA               		(1<<6)
#define XFP_TRANS_TX_CDR_NOTLOCK_STA                (1<<5)
#define XFP_TRANS_RX_NR_STA                    		(1<<4)
#define XFP_TRANS_RX_CDR_NOTLOCK_STA                (1<<3)




// ----- Define for SFP----- 

/*** SFF 8472 < Diagnostic Monitoring Interface for Optical Transceivers > Rev 11.0 ***/

//________Table 3.1 Two-wire interface ID: Data Fields ¨C Address A0h________

#define	PHY_TRANS_REG_TABLE1		    0x50

//BASE ID FIELDS                        
#define PHY_TRANS_IDENTIFIER            0
#define PHY_TRANS_EXT_IDENTIFIER        1
#define PHY_TRANS_CONNECTOR             2
#define PHY_TRANS_TRANSCEIVER           3
#define PHY_TRANS_TRANSCEIVER_6         6
#define PHY_TRANS_TRANSCEIVER_8         8
#define PHY_TRANS_TRANSCEIVER_9         9
#define PHY_TRANS_ENCODING              11
#define PHY_TRANS_BR                    12
#define PHY_TRANS_RATE_IDENTIFIER       13
#define PHY_TRANS_LENGTH_SMF_KM         14
#define PHY_TRANS_LENGTH_SMF_100M       15
#define PHY_TRANS_LENGTH_50UM_10M       16
#define PHY_TRANS_LENGTH_62P5UM_10M     17
#define PHY_TRANS_LENGTH_CABLE          18
#define PHY_TRANS_LENGTH_OM3            19
#define PHY_TRANS_VENDOR_NAME           20
#define PHY_TRANS_TRANSCEIVER_36        36
#define PHY_TRANS_VENDOR_OUI            37
#define PHY_TRANS_VENDOR_PN             40
#define PHY_TRANS_VENDOR_REV            56
#define PHY_TRANS_WAVELENGTH            60
#define PHY_TRANS_UNALLOCATED           62
#define PHY_TRANS_CC_BASE               63
//EXTENDED ID FIELDS                    
#define PHY_TRANS_OPTIONS               64
#define PHY_TRANS_OPTIONS_65            65
#define PHY_TRANS_BR_MAX                66
#define PHY_TRANS_BR_MIN                67
#define PHY_TRANS_VENDOR_SN             68
#define PHY_TRANS_DATE_CODE             84
#define PHY_TRANS_DIAG_MONITOR_TYPE     92
#define PHY_TRANS_ENHANCED_OPTIONS      93
#define PHY_TRANS_SFF_8472_COMPLIANCE   94
#define PHY_TRANS_CC_EXT                95
//VENDOR SPECIFIC ID FIELDS
#define PHY_TRANS_VENDOR_SPECIFIC_96    96
#define PHY_TRANS_RESERVED              128

//TABLE 3.2: Identifier values
#define PHY_TRANS_IDENTIFIER_UNKNOWN          0x00
#define PHY_TRANS_IDENTIFIER_GBIC			  0x01
#define PHY_TRANS_IDENTIFIER_SFF              0x02
#define PHY_TRANS_IDENTIFIER_SFP_OR_SFP_PLUS  0x03
#define PHY_TRANS_IDENTIFIER_XBI			  0x04
#define PHY_TRANS_IDENTIFIER_XENPAK			  0x05
#define PHY_TRANS_IDENTIFIER_XFP              0x06
#define PHY_TRANS_IDENTIFIER_XFF              0x07
#define PHY_TRANS_IDENTIFIER_XFPE             0x08
#define PHY_TRANS_IDENTIFIER_XPAK             0x09
#define PHY_TRANS_IDENTIFIER_X2               0x0A

//TABLE 3.4: Connector values
#define PHY_TRANS_CONNECTOR_SC          0x1
#define PHY_TRANS_CONNECTOR_LC          0x7
#define PHY_TRANS_CONNECTOR_RJ45        0x22

//Table 3.5: Transceiver codes(Address A0h)
//10G Ethernet Compliance Codes
#define PHY_TRANS_TRANSCEIVER_3_10GBASE_ER           (1<<7)
#define PHY_TRANS_TRANSCEIVER_3_10GBASE_LRM          (1<<6)
#define PHY_TRANS_TRANSCEIVER_3_10GBASE_LR           (1<<5)
#define PHY_TRANS_TRANSCEIVER_3_10GBASE_SR           (1<<4)
#define PHY_TRANS_TRANSCEIVER_3_1X_SX                (1<<3)
#define PHY_TRANS_TRANSCEIVER_3_1X_LX                (1<<2)
#define PHY_TRANS_TRANSCEIVER_3_1X_COPPER_ACTIVE     (1<<1)
#define PHY_TRANS_TRANSCEIVER_3_1X_COPPER_PASSIVE    (1)

//Ethernet Compliance Codes
#define PHY_TRANS_TRANSCEIVER_6_BASE_PX              (1<<7)
#define PHY_TRANS_TRANSCEIVER_6_BASE_BX10            (1<<6)
#define PHY_TRANS_TRANSCEIVER_6_100BASE_FX           (1<<5)
#define PHY_TRANS_TRANSCEIVER_6_100BASE_LX_LX10      (1<<4)
#define PHY_TRANS_TRANSCEIVER_6_1000BASE_T           (1<<3)
#define PHY_TRANS_TRANSCEIVER_6_1000BASE_CX          (1<<2)
#define PHY_TRANS_TRANSCEIVER_6_1000BASE_LX          (1<<1)
#define PHY_TRANS_TRANSCEIVER_6_1000BASE_SX          (1)

//SFP+ Cable Technology
#define PHY_TRANS_TRANSCEIVER_8_SFP_CABLE_TECH_MASK  0x0C

//Fibre Channel Transmission Media
#define PHY_TRANS_TRANSCEIVER_9_TW                   (1<<7)
#define PHY_TRANS_TRANSCEIVER_9_TP                   (1<<6)
#define PHY_TRANS_TRANSCEIVER_9_MI                   (1<<5)
#define PHY_TRANS_TRANSCEIVER_9_TV                   (1<<4)
#define PHY_TRANS_TRANSCEIVER_9_M6                   (1<<3)
#define PHY_TRANS_TRANSCEIVER_9_M5_M5E               (1<<2)
#define PHY_TRANS_TRANSCEIVER_9_SM                   (1)

//Table 3.6: Encoding codes
#define PHY_TRANS_ENCODING_8B_10B       0x1
#define PHY_TRANS_ENCODING_NRZ          0x3
#define PHY_TRANS_ENCODING_64B_66B      0x6

//Vendor name
#define PHY_TRANS_VENDOR_NAME_SIZE      16

//Vendor OUI
#define PHY_TRANS_VENDOR_OUI_SIZE       3

//Vendor PN
#define PHY_TRANS_VENDOR_PN_SIZE        16

//Vendor PN
#define PHY_TRANS_VENDOR_REV_SIZE       4

//Table 3.7: Option values
#define PHY_TRANS_OPTIONS_65_RATE_SELECT     (1<<5)
#define PHY_TRANS_OPTIONS_65_TX_DISABLE      (1<<4)
#define PHY_TRANS_OPTIONS_65_TX_FAULT        (1<<3)
#define PHY_TRANS_OPTIONS_65_LOS_INV         (1<<2)
#define PHY_TRANS_OPTIONS_65_LOS             (1<<1)

//Vendor SN
#define PHY_TRANS_VENDOR_SN_SIZE        16  

//Date code
#define PHY_TRANS_DATE_CODE_SIZE        8

//Table 3.9: Diagnostic Monitoring Type
#define PHY_TRANS_DIAG_MONITOR_TYPE_DIGITAL_DIAG_MONITOR  (1<<6)
#define PHY_TRANS_DIAG_MONITOR_TYPE_INTERNAL_CALIBRATED   (1<<5)
#define PHY_TRANS_DIAG_MONITOR_TYPE_EXTERNAL_CALIBRATED   (1<<4)
#define PHY_TRANS_DIAG_MONITOR_TYPE_AVERAGE_POWER         (1<<3)
#define PHY_TRANS_DIAG_MONITOR_TYPE_ADDRESSING_MODES      (1<<2)

//Table 3.10: Enhanced Options
#define PHY_TRANS_ENHANCED_OPTIONS_A_W               (1<<7)
#define PHY_TRANS_ENHANCED_OPTIONS_SOFT_TX_DISABLE   (1<<6)
#define PHY_TRANS_ENHANCED_OPTIONS_SOFT_TX_FAULT     (1<<5)
#define PHY_TRANS_ENHANCED_OPTIONS_SOFT_RX_LOS       (1<<4)
#define PHY_TRANS_ENHANCED_OPTIONS_SOFT_RATE_SELECT  (1<<3)



//________Table 3.1a Diagnostics: Data Fields ¨C Address A2h________

#define	PHY_TRANS_REG_TABLE2			0x51

//DIAGNOSTIC AND CONTROL/STATUS FIELDS
#define PHY_TRANS_A_W_THRESHOLDS        0
#define PHY_TRANS_UNALLOCATED_2         40
#define PHY_TRANS_EXT_CAL_CONSTANTS     56
#define PHY_TRANS_UNALLOCATED_3         92
#define PHY_TRANS_CC_DMI                95
#define PHY_TRANS_DIAGNOSTICS           96
#define PHY_TRANS_UNALLOCATED_4         106
#define PHY_TRANS_STATUS_CONTROL        110
#define PHY_TRANS_RESERVED_2            111
#define PHY_TRANS_ALARM_FLAGS           112
#define PHY_TRANS_ALARM_FLAGS_2         113
#define PHY_TRANS_UNALLOCATED_5         114
#define PHY_TRANS_WARNING_FLAGS         116
#define PHY_TRANS_WARNING_FLAGS_2       117
#define PHY_TRANS_EXT_STATUS_CONTROL    118
//GENERAL USE FIELDS
#define PHY_TRANS_VENDOR_SPECIFIC_120   120
#define PHY_TRANS_USER_EEPROM           128
#define PHY_TRANS_VENDOR_CONTROL        248


//Table 3.15: Alarm and Warning Thresholds (2-Wire Address A2h)
#define PHY_TRANS_A_W_THRESHOLDS_TEMP_HIGH_ALARM_MSB           0  
#define PHY_TRANS_A_W_THRESHOLDS_TEMP_HIGH_ALARM_LSB           1  
#define PHY_TRANS_A_W_THRESHOLDS_TEMP_LOW_ALARM_MSB            2  
#define PHY_TRANS_A_W_THRESHOLDS_TEMP_LOW_ALARM_LSB            3  
#define PHY_TRANS_A_W_THRESHOLDS_TEMP_HIGH_WARNING_MSB         4  
#define PHY_TRANS_A_W_THRESHOLDS_TEMP_HIGH_WARNING_LSB         5  
#define PHY_TRANS_A_W_THRESHOLDS_TEMP_LOW_WARNING_MSB          6  
#define PHY_TRANS_A_W_THRESHOLDS_TEMP_LOW_WARNING_LSB          7  
#define PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_ALARM_MSB        8  
#define PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_ALARM_LSB        9  
#define PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_ALARM_MSB         10 
#define PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_ALARM_LSB         11 
#define PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_WARNING_MSB      12 
#define PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_HIGH_WARNING_LSB      13 
#define PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_WARNING_MSB       14 
#define PHY_TRANS_A_W_THRESHOLDS_VOLTAGE_LOW_WARNING_LSB       15 
#define PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_ALARM_MSB           16 
#define PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_ALARM_LSB           17 
#define PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_ALARM_MSB            18 
#define PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_ALARM_LSB            19 
#define PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_WARNING_MSB         20 
#define PHY_TRANS_A_W_THRESHOLDS_BIAS_HIGH_WARNING_LSB         21 
#define PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_WARNING_MSB          22 
#define PHY_TRANS_A_W_THRESHOLDS_BIAS_LOW_WARNING_LSB          23 
#define PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_MSB       24 
#define PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_ALARM_LSB       25 
#define PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_MSB        26 
#define PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_ALARM_LSB        27 
#define PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_MSB     28 
#define PHY_TRANS_A_W_THRESHOLDS_TX_POWER_HIGH_WARNING_LSB     29 
#define PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_MSB      30 
#define PHY_TRANS_A_W_THRESHOLDS_TX_POWER_LOW_WARNING_LSB      31 
#define PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_MSB       32 
#define PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_ALARM_LSB       33 
#define PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_MSB        34 
#define PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_ALARM_LSB        35 
#define PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_MSB     36 
#define PHY_TRANS_A_W_THRESHOLDS_RX_POWER_HIGH_WARNING_LSB     37 
#define PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_MSB      38 
#define PHY_TRANS_A_W_THRESHOLDS_RX_POWER_LOW_WARNING_LSB      39 

//TABLE 3.16: Calibration constants for External Calibration Option (2 Wire Address A2h)


//TABLE 3.17: A/D Values and Status Bits (2 Wire Address A2h)
#define PHY_TRANS_DIAGNOSTICS_TEMP_MSB  96
#define PHY_TRANS_DIAGNOSTICS_TEMP_LSB  97
#define PHY_TRANS_DIAGNOSTICS_VCC_MSB   98
#define PHY_TRANS_DIAGNOSTICS_VCC_LSB   99
#define PHY_TRANS_TX_BIAS_MSB           100
#define PHY_TRANS_TX_BIAS_LSB           101
#define PHY_TRANS_TX_POWER_MSB          102
#define PHY_TRANS_TX_POWER_LSB          103
#define PHY_TRANS_RX_POWER_MSB          104
#define PHY_TRANS_RX_POWER_LSB          105

#define PHY_TRANS_STATUS_CONTROL_TX_DISABLE_STATE        (1<<7)
#define PHY_TRANS_STATUS_CONTROL_SOFT_TX_DISABLE_SELECT  (1<<6)
#define PHY_TRANS_STATUS_CONTROL_RS1_STATE               (1<<5)
#define PHY_TRANS_STATUS_CONTROL_RATE_SELECT_STATE       (1<<4)
#define PHY_TRANS_STATUS_CONTROL_SOFT_RATE_SELECT_STATE  (1<<3)
#define PHY_TRANS_STATUS_CONTROL_TX_FAULT_STATE          (1<<2)
#define PHY_TRANS_STATUS_CONTROL_RX_LOS_STATE            (1<<1)
#define PHY_TRANS_STATUS_CONTROL_DATA_READY_BAR_STATE    (1)


//Table 3.18: Alarm and Warning Flag Bits (2-Wire Address A2h)
#define	PHY_TRANS_ALARM_FLAGS_TEMP_HIGH_ALARM         (1<<7)
#define	PHY_TRANS_ALARM_FLAGS_TEMP_LOW_ALARM          (1<<6)
#define	PHY_TRANS_ALARM_FLAGS_VCC_HIGH_ALARM          (1<<5)
#define	PHY_TRANS_ALARM_FLAGS_VCC_LOW_ALARM           (1<<4)
#define	PHY_TRANS_ALARM_FLAGS_TX_BIAS_HIGH_ALARM      (1<<3)
#define	PHY_TRANS_ALARM_FLAGS_TX_BIAS_LOW_ALARM       (1<<2)
#define	PHY_TRANS_ALARM_FLAGS_TX_POWER_HIGH_ALARM     (1<<1)
#define	PHY_TRANS_ALARM_FLAGS_TX_POWER_LOW_ALARM      (1)

#define	PHY_TRANS_ALARM_FLAGS_2_RX_POWER_HIGH_ALARM   (1<<7)
#define	PHY_TRANS_ALARM_FLAGS_2_RX_POWER_LOW_ALARM    (1<<6)

#define	PHY_TRANS_WARNING_FLAGS_TEMP_HIGH_WARNING         (1<<7)
#define	PHY_TRANS_WARNING_FLAGS_TEMP_LOW_WARNING          (1<<6)
#define	PHY_TRANS_WARNING_FLAGS_VCC_HIGH_WARNING          (1<<5)
#define	PHY_TRANS_WARNING_FLAGS_VCC_LOW_WARNING           (1<<4)
#define	PHY_TRANS_WARNING_FLAGS_TX_BIAS_HIGH_WARNING      (1<<3)
#define	PHY_TRANS_WARNING_FLAGS_TX_BIAS_LOW_WARNING       (1<<2)
#define	PHY_TRANS_WARNING_FLAGS_TX_POWER_HIGH_WARNING     (1<<1)
#define	PHY_TRANS_WARNING_FLAGS_TX_POWER_LOW_WARNING      (1)

#define	PHY_TRANS_WARNING_FLAGS_2_RX_POWER_HIGH_WARNING   (1<<7)
#define	PHY_TRANS_WARNING_FLAGS_2_RX_POWER_LOW_WARNING    (1<<6)

//Trans REG 1 byte = 8 bits
#define PHY_TRANS_BYTE_SIZE                     8

#define PHY_TRANS_NOT_FOUND_IN_IOT_LIST         0xffffffff

// ----- Structure ----- 

typedef struct
{
	UINT8  id;
	char   name[48];
	UINT8  addr;	
	UINT8  def;
	UINT8  cur;
} I2C_REG_ENTRY, *PI2C_REG_ENTRY; 

//#if defined(TCSUPPORT_CPU_AN7583) //julia_7583
typedef struct{
	UINT32 index;
	char vendor_id[PHY_TRANS_VENDOR_NAME_SIZE+1];
	char vendor_pn[PHY_TRANS_VENDOR_NAME_SIZE+1];
	UINT32 tx_setting;
	UINT32 rx_setting;
}PHY_TRANS_INFO_7583,*PPHY_TRANS_INFO_7583;
extern PHY_TRANS_INFO_7583 phy_trans_iot_list_7583[];

//#else
typedef struct{
	UINT32 index;
	char vendor_id[PHY_TRANS_VENDOR_NAME_SIZE+1];
	char vendor_pn[PHY_TRANS_VENDOR_NAME_SIZE+1];
	UINT32 gepon_csr_xpon_setting;
	UINT32 xgpon_phy_sfp_vld_level;
	UINT32 xepon_pcs_sfp_status;
	UINT32 pma_xpon_setting_0;
	UINT32 pma_xpon_setting_1;	
}PHY_TRANS_INFO,*PPHY_TRANS_INFO;
//#endif
extern PHY_TRANS_INFO phy_trans_iot_list[];

typedef struct{
	UINT32 index;
	char vendor_id[PHY_TRANS_VENDOR_NAME_SIZE+1];
	char vendor_pn[PHY_TRANS_VENDOR_NAME_SIZE+1];
	UINT32 gepon_csr_xpon_setting;
	UINT32 xgpon_phy_sfp_vld_level;
	UINT32 xepon_pcs_sfp_status;
	UINT32 pma_xpon_setting_0;
	UINT32 pma_xpon_setting_1;
	UINT32 mod_pw;	//xfp module pass word
}XFP_TRANS_INFO,*XXFP_TRANS_INFO;

extern XFP_TRANS_INFO xfp_trans_iot_list[];

// ----- Function ----- 
ushort phy_I2C_read(unchar u1CHannelID, ushort u2ClkDiv, unchar u1DevAddr, 
				 unchar u1WordAddrNum, uint u4WordAddr, unchar *pu1Buf, 
				 ushort u2ByteCnt);
ushort phy_I2C_write(unchar u1CHannelID, ushort u2ClkDiv, unchar u1DevAddr, 
						  unchar u1WordAddrNum, uint u4WordAddr, unchar *pu1Buf, 
						  ushort u2ByteCnt);
#if SIF_DEBUG_LEVEL_CONTROL
int sifm_proc_read(char *output);
int sifm_proc_write(char *input);
#endif
char* rtrim(char *name);
int sff_8472_power_to_dBm(UINT16 input); //float sff_8472_power_to_dBm(float input); //chunhua_20230320
void phy_dBm_print(int input); //void phy_dBm_print(float input); //chunhua_20230320

int is_en7572_7573(void);

int phy_trans_model_setting(void);
int phy_trans_task_wait(void);

void phy_trans_model_found(char *vendor_id,char *vendor_pn); //julia_7583
void phy_trans_model_found_7581(char *vendor_id,char *vendor_pn); //julia_7583
void phy_trans_model_found_7583(char *vendor_id,char *vendor_pn); //julia_7583


void phy_trans_param_status_real(PPHY_TransParam_T transceiver_param);
void phy_trans_params_show(void);
void phy_trans_params_dump(void);
void phy_trans_params_parser(void);


int xfp_trans_model_setting(void);

void xfp_trans_pw_entry(void);
int xfp_trans_set_ngpon2_rx_chan(PPHY_Ngpon2_Chan_Sel_T ngpon2_chan);
int xfp_trans_set_ngpon2_tx_chan(PPHY_Ngpon2_Chan_Sel_T ngpon2_chan);
int xfp_trans_set_ngpon2_chan(PPHY_Ngpon2_Chan_Sel_T ngpon2_chan); //julia_20221021 ngpon2
int xfp_trans_get_ngpon2_chan(PPHY_Ngpon2_Chan_Sel_T ngpon2_chan);


#endif /* _I2C_H_ */




