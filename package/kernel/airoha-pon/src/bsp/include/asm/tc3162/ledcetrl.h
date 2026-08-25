/*
** $Id: ledcetrl.h,v 1.4 2011/01/07 04:05:20 pork Exp $
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
** $Log: ledcetrl.h,v $
** Revision 1.4  2011/01/07 04:05:20  pork
** commit TDI layer with Zarlink 2S1O
**
** Revision 1.3  2010/10/15 05:02:10  xmdai_nj
** #7017:add function for wlan button(main trunk)
**
** Revision 1.2  2010/06/11 00:31:06  xhshi
** #5964 support dare's new led and key requests
**
** Revision 1.1.1.1  2010/04/09 09:39:21  feiyan
** New TC Linux Make Flow Trunk
**
** Revision 1.3  2010/03/22 14:15:16  yzwang_nj
** [Bug#5128] Implement TR068 LED
**
** Revision 1.2  2010/03/19 07:16:59  xyyou
** # 5213 Add  RT3390 11n Webpage
** Support RT3390 AutoChannel feature
** Support RT3390 WMM feature
** # 5214 Support RT3390 wps feature
**      Add wps led spec&not spec
**
** Revision 1.1.1.1  2009/12/17 01:42:47  josephxu
** 20091217, from Hinchu ,with VoIP
**
** Revision 1.1.1.1  2007/04/12 09:42:03  ian
** TCLinuxTurnkey2007
**
** Revision 1.2  2006/07/06 07:24:57  lino
** update copyright year
**
** Revision 1.1.1.1  2005/11/02 05:45:38  lino
** no message
**
** Revision 1.3  2004/10/12 08:33:45  lino
** add 10M/100M ethernet led define
**
*/

#ifndef _LEDCETRL_H
#define _LEDCETRL_H

#include <asm/memory.h>

/**** from arch/mips/include/asm/tc3162/tc3162.h *********************
 * GPIO Module Registers *
 *************************/
extern unsigned long        g_gpio_base;
#define CR_GPIO_BASE       	(g_gpio_base)
#define CR_GPIO_CTRL	    (CR_GPIO_BASE + UL(0x00))
#define CR_GPIO_DATA	    (CR_GPIO_BASE + UL(0x04))
#define CR_GPIO_INTS      	(CR_GPIO_BASE + UL(0x08))
#define CR_GPIO_EDET	    (CR_GPIO_BASE + UL(0x0C))
#define CR_GPIO_LDET       	(CR_GPIO_BASE + UL(0x10))
#define CR_GPIO_ODRAIN      (CR_GPIO_BASE + UL(0x14))
#define CR_GPIO_CTRL1	    (CR_GPIO_BASE + UL(0x20))
#define GPIO_REG_CTRL		(CR_GPIO_BASE + 0x00)
#define GPIO_REG_DATA		(CR_GPIO_BASE + 0x04)
#define GPIO_REG_OEN		(CR_GPIO_BASE + 0x14)
#define GPIO_REG_CTRL1		(CR_GPIO_BASE + 0x20)
#ifdef TCSUPPORT_GPIO_ECM
#define CR_SGPIO_DATA        (CR_GPIO_BASE + UL(0x24))
#define CR_SGPIO_CDIV        (CR_GPIO_BASE + UL(0x28))
#define CR_SGPIO_CDLY        (CR_GPIO_BASE + UL(0x2C))
#define CR_SGPIO_MODE	    (CR_GPIO_BASE + UL(0x30))
#define CR_GPIO_FLAMOD	    (CR_GPIO_BASE + UL(0x34))
#define CR_GPIO_IMME	    (CR_GPIO_BASE + UL(0x38))
#define CR_GPIO_FLAP0	    (CR_GPIO_BASE + UL(0x3C))
#define CR_GPIO_FLAP1	    (CR_GPIO_BASE + UL(0x40))
#define CR_GPIO_FLAP2 	    (CR_GPIO_BASE + UL(0x44))
#define CR_GPIO_FLAP3	    (CR_GPIO_BASE + UL(0x48))
#define CR_GPIO_FMAP0	    (CR_GPIO_BASE + UL(0x4C))
#define CR_GPIO_FMAP1 	    (CR_GPIO_BASE + UL(0x50))
#define CR_SGPIO_FMAP0	    (CR_GPIO_BASE + UL(0x54))
#define CR_SGPIO_FMAP1	    (CR_GPIO_BASE + UL(0x58))
#define CR_SGPIO_FMAP2	    (CR_GPIO_BASE + UL(0x5C))
#endif
/* MT7510 */
#define CR_GPIO_CTRL2	    (CR_GPIO_BASE + UL(0x60))
#define CR_GPIO_CTRL3	    (CR_GPIO_BASE + UL(0x64))
#define CR_GPIO_FLAMOD_EXT	(CR_GPIO_BASE + UL(0x68))
#define CR_GPIO_DATA1		(CR_GPIO_BASE + UL(0x70))
#define CR_GPIO_ODRAIN1     (CR_GPIO_BASE + UL(0x78))
#define GPIO_REG_CTRL2		(CR_GPIO_BASE + 0x60)
#define GPIO_REG_CTRL3		(CR_GPIO_BASE + 0x64)
#define GPIO_REG_DATA1		(CR_GPIO_BASE + 0x70)
#define GPIO_REG_OEN1		(CR_GPIO_BASE + 0x78)

/*__________________________________________________________________________
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%      constant definition
%%________________________________________________________________________*/

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_BUTTONDETECT)
#include<linux/ioctl.h>
#define LEDBUTTON_MAJOR_NUM 	248
#define LEDBUTTON_IOCTL_MAGIC 0x99
#define MODULE_NAME	"led_button"

typedef enum e_ledbutton_ioctl
{
	LEDBUTTON_IOCTL_SELECT					= _IOW(LEDBUTTON_IOCTL_MAGIC, 0, int[2]),
	LEDBUTTON_MAX_IOCTL		/* please define I/O controls above this item */	
	
} E_LEDBUTTON_IOCTL;

#endif/*TCSUPPORT_COMPILE*/


#ifdef TCSUPPORT_GPIO_ECM
#define LED_TYPE_MAX_NO		16
	//define led flash peroid parameter
#define MAX_LED_FLASH_PERIOD		128
#define LED_FLASH_PERIOD_LOW		1
#define LED_FLASH_PERIOD_HIGH		0
#define MAX_LED_FLASH_MAP			8
	//define serial gpio control status
#define SLED_STATUS_REDAY			0x1<<31
	// define led interface type
#define LED_SERIAL					1
#define LED_PARALLEL				0
#endif

#ifdef TCSUPPORT_USB_HOST_LED
#define USBPHYPORT1 1
#define USBPHYPORT2 2

#define USB_DISCONNECT 1
#define USB_CONNECT 2
#define USB_BLINK 3
#define USB_DEFAULT 4
#endif

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_CT_SIMCARD_SEPARATION)
#define SIMCARD_LED_INTERFACE	1
#define SIMCARD_LED_CONFIG		2

#define SIMCARD_INTERFACE_UNUSED 	0
#define SIMCARD_INTERFACE_CONNECT 	1
#define SIMCARD_INTERFACE_ERROR		2
#define SIMCARD_INTERFACE_AUTH_FAIL	3

#define SIMCARD_CONFIG_FAIL	0
#define SIMCARD_CONFIG_RUNNING	1
#define SIMCARD_CONFIG_SUCCESS	2
#endif/*TCSUPPORT_COMPILE*/
#ifdef TCSUPPORT_WLAN_LED_BY_SW
#define WLAN_DISCONNECT 0
#define WLAN_CONNECT 1
#define WLAN_BLINK 2
#define WLAN_DEFAULT 3
#endif
// define led mode
#define LED_MODE_NOT_USED			0
#define LED_MODE_ONOFF				1
#define LED_MODE_BLINK				2
#define LED_MODE_NOACT				3
#define LED_MODE_PULSE				4
/*20090811pork modified: add for sys reset gpio*/
#define LED_MODE_INPUT				5
#define LED_MODE_MASK				0xf
#define LED_MODE_BICOLOR			0x10

// define led blink speed
#define LED_SPEED_FAST				0
#define LED_SPEED_MED				1
#define LED_SPEED_SLOW				2
#define LED_SPEED_VERY_SLOW			3
#ifdef TCSUPPORT_WLAN_LED_BY_SW
#define LED_SPEED_FASTEST			4
#endif
// define software led no
#define LED_BASE 					0

#define LED_DSL_STATUS				(LED_BASE + 0)
#define LED_DSL_ACT_STATUS			(LED_BASE + 1)
#define LED_DSL_NOACT_STATUS		(LED_BASE + 2)
#define LED_DSL_IDLE_STATUS			(LED_BASE + 3)
#define LED_DSL_HANDSHAKE_STATUS	(LED_BASE + 4)
#define LED_DSL_TRAINING_STATUS		(LED_BASE + 5)
#define LED_DSL_SHOWTIME_STATUS		(LED_BASE + 6)

#define LED_PPP_STATUS				(LED_BASE + 7)
#define LED_PPP_ACT_STATUS			(LED_BASE + 8)
#define LED_PPP_NOACT_STATUS		(LED_BASE + 9)

#define LED_USB_STATUS				(LED_BASE + 10)
#define LED_USB_ACT_STATUS			(LED_BASE + 11)
#define LED_USB_NOACT_STATUS		(LED_BASE + 12)

#define LED_WLAN_STATUS				(LED_BASE + 13)
#define LED_WLAN_ACT_STATUS			(LED_BASE + 14)
#define LED_WLAN_NOACT_STATUS		(LED_BASE + 15)

#define LED_ETHER_STATUS			(LED_BASE + 16)
#define LED_ETHER_ACT_STATUS		(LED_BASE + 17)
#define LED_ETHER_NOACT_STATUS		(LED_BASE + 18)

#define LED_FLASH_STATUS			(LED_BASE + 19)
#define LED_SYS_BOOT_STATUS			(LED_BASE + 20)
#define LED_SYS_STATUS				(LED_BASE + 21)
#define LED_SYS_INIT_STATUS			(LED_BASE + 22)

#define LED_ETHER_10M_STATUS		(LED_BASE + 23)
#define LED_ETHER_10M_ACT_STATUS	(LED_BASE + 24)
#define LED_ETHER_10M_NOACT_STATUS	(LED_BASE + 25)

#define LED_ETHER_100M_STATUS		(LED_BASE + 26)
#define LED_ETHER_100M_ACT_STATUS	(LED_BASE + 27)
#define LED_ETHER_100M_NOACT_STATUS	(LED_BASE + 28)

#define LED_SIM_CFG_STATUS	(LED_BASE + 29)
#define LED_SIM_CFG_ACT_STATUS	(LED_BASE + 30)

#define LED_LAN_RESET				(LED_BASE + 31)
//add for TR068 compliance
#define LED_INTERNET_STATUS		(LED_BASE + 33)
#define LED_INTERNET_ACT_STATUS		(LED_BASE + 34)
#define LED_INTERNET_NOACT_STATUS	(LED_BASE + 35)
#define LED_INTERNET_TRYING_STATUS	(LED_BASE + 38)
#define LED_TR68_PWR_BOOTING		(LED_BASE + 40)
#define LED_TR68_PWR_BOOTED		(LED_BASE + 41)
#define LED_PHY_TX_POWER_DISABLE		(LED_BASE + 42)
#define LED_WLAN_WPS_STATUS		(LED_BASE + 43)
#define LED_WLAN_WPS_ACT_STATUS	(LED_BASE + 44)
#define LED_WLAN_WPS_NOACT_STATUS	(LED_BASE + 45)

#define LED_VOIP_SLIC1_RESET (LED_BASE + 46)
#define LED_VOIP_SLIC2_RESET (LED_BASE + 47)
#define LED_DSL_ALARM (LED_BASE + 48)
#define LED_VOIP_HOOK1_STATUS (LED_BASE + 49)
#define LED_VOIP_HOOK2_STATUS (LED_BASE + 50)

#define LED_SIM_STATUS	(LED_BASE + 51)
#define LED_SIM_ACT_STATUS	(LED_BASE + 52)
#define LED_SIM_NOACT_STATUS	(LED_BASE + 53)


#ifdef TCSUPPORT_USB_HOST_LED
#if defined(TCSUPPORT_CPU_MT7510) || defined(TCSUPPORT_CPU_MT7520) || defined(TCSUPPORT_CPU_MT7505)
#define LED_USB2_STATUS				(LED_BASE + 54)
#define LED_USB2_ACT_STATUS			(LED_BASE + 55)
#endif
#endif

#define GPIO_SYS_RESET				(LED_BASE + 56)

#define LED_WLAN_RADIO                 	(LED_BASE + 57) 
#define GPIO_WLAN_WPS				(LED_BASE + 58)

//add by brian for gpio additional request
#define LED_PWR_FLASH				(LED_BASE + 59)

#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_C2_TRUE)
#define LED_PWR_GREEN_FLASH				(LED_BASE + 60)
#endif/*TCSUPPORT_COMPILE*/

#define LED_PWR_USB				(LED_BASE + 61)

#define LED_LED_SWITCH                          (LED_BASE + 62)

/*20090811 pork added: fit for linos LED setting*/
#define LED_LAN1_RESET				(LED_BASE + 63)

#ifdef TCSUPPORT_XPON_LED
#define LED_XPON_STATUS				(LED_BASE + 64)
#define LED_XPON_TRYING_STATUS		(LED_BASE + 65)

#define LED_XPON_LOS_ON_STATUS		(LED_BASE + 66)
#define LED_XPON_LOS_STATUS			(LED_BASE + 67)

#define LED_ETHER_PORT1_STATUS			(LED_BASE + 68)
#define LED_ETHER_PORT1_ACT_STATUS	(LED_BASE + 69)
#define LED_ETHER_PORT2_STATUS			(LED_BASE + 70)
#define LED_ETHER_PORT2_ACT_STATUS	(LED_BASE + 71)
#define LED_ETHER_PORT3_STATUS			(LED_BASE + 72)
#define LED_ETHER_PORT3_ACT_STATUS	(LED_BASE + 73)
#define LED_ETHER_PORT4_STATUS			(LED_BASE + 74)
#define LED_ETHER_PORT4_ACT_STATUS	(LED_BASE + 75)
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_C9_ROST_LED)
#define LED_XPON_UNREG_STATUS (LED_BASE + 76)
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_XPON_HAL_API_EXT)				
#define LED_ETHER_PORT5_STATUS			(LED_BASE + 76)
#define LED_ETHER_PORT5_ACT_STATUS	(LED_BASE + 77)
#define LED_ETHER_PORT6_STATUS			(LED_BASE + 78)
#define LED_ETHER_PORT6_ACT_STATUS	(LED_BASE + 79)
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_C5_XPON_AUTH_LED)
#define LED_XPON_AUTH_STATUS			(LED_BASE + 76) // DARE XPON Auth LED
#define LED_XPON_AUTH_TRYING_STATUS		(LED_BASE + 77)
#endif/*TCSUPPORT_COMPILE*/
#if/*TCSUPPORT_COMPILE*/ defined(TCSUPPORT_TW_BOARD_CDS)
#define LED_ETHER_PORT1_1000M_STATUS			(LED_BASE + 76)
#define LED_ETHER_PORT1_1000M_ACT_STATUS	(LED_BASE + 77)
#define LED_ETHER_PORT2_1000M_STATUS			(LED_BASE + 78)
#define LED_ETHER_PORT2_1000M_ACT_STATUS	(LED_BASE + 79)
#define LED_ETHER_PORT3_1000M_STATUS			(LED_BASE + 80)
#define LED_ETHER_PORT3_1000M_ACT_STATUS	(LED_BASE + 81)
#define LED_ETHER_PORT4_1000M_STATUS			(LED_BASE + 82)
#define LED_ETHER_PORT4_1000M_ACT_STATUS	(LED_BASE + 83)

#define LED_ETHER_PORT1_10M_STATUS			(LED_BASE + 84)
#define LED_ETHER_PORT1_10M_ACT_STATUS	(LED_BASE + 85)
#define LED_ETHER_PORT2_10M_STATUS			(LED_BASE + 86)
#define LED_ETHER_PORT2_10M_ACT_STATUS	(LED_BASE + 87)
#define LED_ETHER_PORT3_10M_STATUS			(LED_BASE + 88)
#define LED_ETHER_PORT3_10M_ACT_STATUS	(LED_BASE + 89)
#define LED_ETHER_PORT4_10M_STATUS			(LED_BASE + 90)
#define LED_ETHER_PORT4_10M_ACT_STATUS	(LED_BASE + 91)

#define LED_XPON_REG_STATUS (LED_BASE + 92)

#define LED_DUX1_FD	(LED_BASE + 93)
#define LED_DUX2_FD	(LED_BASE + 94)
#define LED_DUX3_FD	(LED_BASE + 95)
#define LED_DUX4_FD	(LED_BASE + 96)
#define LED_DUX1_HD	(LED_BASE + 97)
#define LED_DUX2_HD	(LED_BASE + 98)
#define LED_DUX3_HD	(LED_BASE + 99)
#define LED_DUX4_HD	(LED_BASE + 100)


#endif/*TCSUPPORT_COMPILE*/
#endif

#if defined(TCSUPPORT_COMBO_PON)
#define GPIO_GPON_LOS	(LED_BASE + 96)
#define GPIO_XGSPON_LOS	(LED_BASE + 97)
#define GPIO_GPON_DIS	(LED_BASE + 98)
#define GPIO_XGSPON_DIS	(LED_BASE + 99)
#define GPIO_PON_S_SEL	(LED_BASE + 100)
#endif

#define LED_VOIP_REG_STATUS	(LED_BASE + 101)
#define LED_PHY_VCC_DISABLE		(LED_BASE + 102)

#if defined(TCSUPPORT_CPU_EN7516)||defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7580)
#define GPIO_WLAN_WPS_5G	(LED_BASE + 104)
#define LED_WLAN_RADIO_5G	(LED_BASE + 105) 
#endif

#define LED_PON_TX_BEN		(LED_BASE + 106)
#define LED_ARQ113C_RSTN	(LED_BASE + 107)
#define LED_ARQ113C_INT		(LED_BASE + 108)
#define LED_EN8811_RESET	(LED_BASE + 109)

#define mtkled_read_reg_word(reg) 			regRead32(reg)
#define mtkled_write_reg_word(reg, wdata) 	regWrite32(reg, wdata)
/*__________________________________________________________________________
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%      function declaration
%%________________________________________________________________________*/

void ledInit(void);
void ledSysInitOn(void);
void ledSysInitOff(void);
void ledTurnOn(uint8 led_no);
void ledTurnOff(uint8 led_no);
#ifdef TCSUPPORT_KERNEL_API
void ledBlinkOn(uint8 led_no, int speed);
void ledBlinkOn2(uint8 led_no, unsigned long delay_on, unsigned long delay_off);
void ledTurnOnRT(uint8 led_no);
void ledTurnOffRT(uint8 led_no);
#endif
uint8 ledGetMode(uint8 led_no);
void ledSetMode(uint8 led_no, uint8 mode);
uint8 ledGetGpio(uint8 led_no);
void led_oen(uint8 led_no);
void led_ien(uint8 led_no);
//#define ledTurnOn(led_no)
//#define ledTurnOff(led_no)
int gpioSetOnoff(uint8 led_no, uint8 onoff);
int gpioGetOnoff(uint8 led_no);
int exModeMDIOGpioConf(uint8 mdc_gpio_num,uint8  mdio_gpio_num);
void exModeMDIOGpioQuery(uint8 * mdc_gpio_num,uint8 * mdio_gpio_num);
uint32 exModeMDIORead(uint32 reg);
void exModeMDIOWrite(uint32 reg, uint32 data);

void ledMtnInit(void);

int	ledTaskInit(void);
void ledTask(void);

/* For GPIO control */
void gpioctl_init(void);
void gpioctl_exit(void);

void gpioctlSetOutput(char* gpio_name, int highLow);
void gpioctlSetInput(char* gpio_name);
void gpioctlSetVal(char* gpio_name, int highLow);
int  gpioctlGetVal(char* gpio_name);

#ifdef TCSUPPORT_XPON_HAL_API_EXT

#define WLAN_LED_OFF 0
#define WLAN_LED_ON  1
#define WLAN_LED_OEN  2
#define WLAN_LED_IEN  3
#define WLAN_LED_RECOVER 4

int led_wififh(int action, int num, int b);
int get_led_mode_data(int led_gpio);
int get_led_datafh(int num);

/* get the ctrl value of gpio num x */
#define MTK_LED_GET_GPIO_CTRL(x) get_led_mode_data(x)

/* get the data value of gpio num x */
#define MTK_LED_GET_GPIO_DATA(x) get_led_datafh(x)

/* gpio num x to write value 1 */
#define MTK_DO_LED_OFF(x) do {								\
							if(!(led_wififh(WLAN_LED_OFF, x, 0))){				\
								if(x > 31){					\
									regWrite32(CR_GPIO_DATA1,regRead32(CR_GPIO_DATA1)|(1<<(x-32)));	\
								}else{						\
									regWrite32(CR_GPIO_DATA,regRead32(CR_GPIO_DATA)|(1<<x));	\
								}						\
							}								\
							} while (0)
/* gpio num x to write value 0 */
#define MTK_DO_LED_ON(x) do {								\
							if(!(led_wififh(WLAN_LED_ON, x, 0))){					\
								if(x > 31){ 				\
									regWrite32(CR_GPIO_DATA1,regRead32(CR_GPIO_DATA1)& ~(1<<(x-32)));	\
								}else{						\
									regWrite32(CR_GPIO_DATA,regRead32(CR_GPIO_DATA)& ~(1<<x));	\
								}						\
								} 							\
							} while (0)
/* gpio num x set to ouput enable */
#define MTK_LED_OEN(x)		do { 	if(!(led_wififh(WLAN_LED_OEN, x, 0))){	\
							if(x > 31){ 						\
								if(x > 47){ 					\
									regWrite32(CR_GPIO_CTRL3,regRead32(CR_GPIO_CTRL3)|(1<<((x-48)*2))); \
								}else{							\
									regWrite32(CR_GPIO_CTRL2,regRead32(CR_GPIO_CTRL2)|(1<<((x-32)*2))); \
								}								\
								regWrite32(CR_GPIO_ODRAIN1,regRead32(CR_GPIO_ODRAIN1)|(1<<(x-32))); \
									}									\
									else{								\
								if(x > 15){ 					\
									regWrite32(CR_GPIO_CTRL1,regRead32(CR_GPIO_CTRL1)|(1<<((x-16)*2))); \
								}else{							\
									regWrite32(CR_GPIO_CTRL,regRead32(CR_GPIO_CTRL)|(1<<(x*2)));	\
								}								\
								regWrite32(CR_GPIO_ODRAIN,regRead32(CR_GPIO_ODRAIN)|(1<<(x)));	\
							}\
								}\
						} while(0)
/* gpio num x set to input enable */
#define MTK_LED_IEN(x)		do { 	if(!(led_wififh(WLAN_LED_IEN, x, 0))){	\
							if(x > 31){ 						\
								if(x > 47)						\
									regWrite32(CR_GPIO_CTRL3,regRead32(CR_GPIO_CTRL3)&~(0x00000003 << ((x-48)* 2)));	\
								else							\
									regWrite32(CR_GPIO_CTRL2,regRead32(CR_GPIO_CTRL2)&~(0x00000003 << ((x-32)* 2)));	\
								regWrite32(CR_GPIO_ODRAIN1,regRead32(CR_GPIO_ODRAIN1)&~(0x00000001 << (x-32))); \
									}									\
									else{								\
								if(x > 15)						\
									regWrite32(CR_GPIO_CTRL1,regRead32(CR_GPIO_CTRL1)&~(0x00000003 << ((x-16)* 2)));	\
								else							\
									regWrite32(CR_GPIO_CTRL,regRead32(CR_GPIO_CTRL)&~(0x00000003 << (x* 2)));	\
								regWrite32(CR_GPIO_ODRAIN,regRead32(CR_GPIO_ODRAIN)&~(0x00000001 << x));	\
							}									\
								}\
					} while(0)

#endif
#endif
