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

#include <asm/tc3162/tc3162.h>
#include <asm/tc3162/ledcetrl.h>
#include "common/drv_global.h"
#include "common/xpon_led.h"

#ifndef TIMER_FUN_PAAM
#if LINUX_VERSION_CODE < KERNEL_VERSION(4,15,0) 
#define TIMER_FUN_PAAM unsigned long
#else
#define TIMER_FUN_PAAM struct timer_list *
#endif
#endif

void change_customer_led_status(XPON_ALARM_LED_Status_t customer_led_status)
{
#ifdef TCSUPPORT_XPON_LED
	switch(customer_led_status)
    {
    	case ALARM_LED_ON:
#ifdef TCSUPPORT_C9_ROST_LED
    		ledTurnOn(LED_XPON_UNREG_STATUS);
#endif
#ifdef TCSUPPORT_TW_BOARD_CDS
			ledTurnOn(LED_XPON_REG_STATUS);
#endif
#ifdef TCSUPPORT_C5_XPON_AUTH_LED
			ledTurnOff(LED_XPON_AUTH_TRYING_STATUS);
    		ledTurnOn(LED_XPON_AUTH_STATUS);
#endif

    		PON_MSG(MSG_DBG, "turn on LED success. \n");
    		break;

    	case ALARM_LED_OFF:
#ifdef TCSUPPORT_C9_ROST_LED
    		ledTurnOff(LED_XPON_UNREG_STATUS);
#endif
#ifdef TCSUPPORT_TW_BOARD_CDS
			ledTurnOff(LED_XPON_REG_STATUS);
#endif
#ifdef TCSUPPORT_C5_XPON_AUTH_LED
			ledTurnOff(LED_XPON_AUTH_STATUS);
    		ledTurnOff(LED_XPON_AUTH_TRYING_STATUS);
#endif
    		PON_MSG(MSG_DBG, "turn off LED success. \n");
    		break;

		case  ALARM_LED_FLICKER:
#ifdef TCSUPPORT_C5_XPON_AUTH_LED
    		ledTurnOff(LED_XPON_AUTH_STATUS);
    		ledTurnOn(LED_XPON_AUTH_TRYING_STATUS);
#endif
    		PON_MSG(MSG_DBG, "LED flicker success. \n");
    		break;
			
    	default:
    		PON_MSG(MSG_ERR, "CUSTOMER LED fail. \n");
    		break;
    }
#endif
}


void change_internet_led_status(XPON_ALARM_LED_Status_t internet_led_status)
{
#ifdef TCSUPPORT_XPON_LED
	switch(internet_led_status)
    {
    	case ALARM_LED_OFF:
#ifdef TCSUPPORT_KERNEL_API
			ledTurnOff(LED_INTERNET_STATUS);

#else
    		ledTurnOff(LED_INTERNET_ACT_STATUS);
    		ledTurnOff(LED_INTERNET_STATUS);
    		ledTurnOff(LED_INTERNET_TRYING_STATUS);
#endif
    		PON_MSG(MSG_DBG, "turn off LED success. \n");
    		break;

    	case  ALARM_LED_FLICKER:
			ledTurnOff(LED_INTERNET_STATUS);
#ifdef TCSUPPORT_KERNEL_API
			ledBlinkOn(LED_INTERNET_STATUS, 1);

#else
    		ledTurnOn(LED_INTERNET_ACT_STATUS);
#endif
    		PON_MSG(MSG_DBG, "LED flicker success. \n");
    		break;

    	default:
    		PON_MSG(MSG_ERR, "INTERNET LED fail. \n");
    		break;
    }
#endif
}

void change_pon_led_status(XPON_ALARM_LED_Status_t pon_led_status)
{
#ifdef TCSUPPORT_XPON_LED

    switch(pon_led_status)
    {
    	case ALARM_LED_ON:
#ifdef TCSUPPORT_KERNEL_API
    		ledTurnOff(LED_XPON_STATUS);   

#else
			ledTurnOff(LED_XPON_TRYING_STATUS);
#endif
    		ledTurnOn(LED_XPON_STATUS);
    		PON_MSG(MSG_DBG, "turn on LED success. \n");
    		break;

    	case ALARM_LED_OFF:
#ifdef TCSUPPORT_KERNEL_API
    		ledTurnOff(LED_XPON_STATUS);
#else
			ledTurnOff(LED_XPON_TRYING_STATUS);
			ledTurnOff(LED_XPON_STATUS);
#endif
    		PON_MSG(MSG_DBG, "turn off LED success. \n");
    		break;

    	case  ALARM_LED_FLICKER:
			ledTurnOff(LED_XPON_STATUS);
#ifdef TCSUPPORT_KERNEL_API
    		ledBlinkOn(LED_XPON_STATUS, 1);
#else	
    		ledTurnOn(LED_XPON_TRYING_STATUS);
#endif
    		PON_MSG(MSG_DBG, "LED flicker success. \n");
    		break;

    	default:
    		PON_MSG(MSG_ERR, "PON LED fail. \n");
    		break;
    }
#endif
}


/*****************************************************************************
//function :
//		change_alarm_led_status
//description : 
//		this function is used to control the LOS LED lighting
//input :	
//		los_led_status : ALARM_LED_OFF; ALARM_LED_ON; ALARM_LED_FLICKER 
//output :
//		setting success or failure
******************************************************************************/

void change_alarm_led_status(XPON_ALARM_LED_Status_t los_led_status)
{
#ifdef TCSUPPORT_XPON_LED
    switch(los_led_status)
    {
    	case ALARM_LED_ON:
#ifdef TCSUPPORT_KERNEL_API
    		ledTurnOff(LED_XPON_LOS_ON_STATUS);
#else	
    		ledTurnOff(LED_XPON_LOS_STATUS);
#endif
    		ledTurnOn(LED_XPON_LOS_ON_STATUS);
    		PON_MSG(MSG_DBG, "turn on LED success. \n");
    		break;

    	case ALARM_LED_OFF:
#ifdef TCSUPPORT_KERNEL_API
    		ledTurnOff(LED_XPON_LOS_ON_STATUS);
#else	
    		ledTurnOff(LED_XPON_LOS_ON_STATUS);
			ledTurnOff(LED_XPON_LOS_STATUS);
#endif
    		PON_MSG(MSG_DBG, "turn off LED success. \n");
    		break;

    	case  ALARM_LED_FLICKER:
			ledTurnOff(LED_XPON_LOS_ON_STATUS);
#ifdef TCSUPPORT_KERNEL_API
			ledBlinkOn(LED_XPON_LOS_ON_STATUS, 1);
#else	
			ledTurnOn(LED_XPON_LOS_STATUS);
#endif
    		PON_MSG(MSG_DBG, "LED flicker success. \n");
    		break;

    	default:
    		PON_MSG(MSG_ERR, "LOS LED fail. \n");
    		break;
    }
#endif /* TCSUPPORT_XPON_LED */
}



#if LINUX_VERSION_CODE > KERNEL_VERSION(6,6,0)
ssize_t xpon_los_led_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	int len = 0;
	/*
	len = sprintf(page, "%d\n", gpPonSysData->ponPhyStaus);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;
	*/
	return len;
}

ssize_t xpon_los_led_write_proc(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	char val_string[8] = {0};
	unsigned int val  = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buf, count))
		return -EFAULT;

	val_string[count] = '\0';

	//val = simple_strtoul(val_string, NULL, 10);
	sscanf(val_string, "%u", &val);

	change_alarm_led_status(val);
	
	return count;
}
#else
int xpon_los_led_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len = 0;
	/*
	len = sprintf(page, "%d\n", gpPonSysData->ponPhyStaus);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;
	*/
	return len;
}

int xpon_los_led_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[8] = {0};
	unsigned int val  = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	//val = simple_strtoul(val_string, NULL, 10);
	sscanf(val_string, "%u", &val);

	change_alarm_led_status(val);
	
	return count;
}
#endif


