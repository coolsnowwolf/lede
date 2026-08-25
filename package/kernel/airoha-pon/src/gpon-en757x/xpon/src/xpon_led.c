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
#include "../inc/common/drv_global.h"
#include "../inc/common/xpon_led.h"


/*****************************************************************************
//function :
//		xpon_los_led_flicker
//description : 
//		this function is used to let LED flicker
//input :	
//		N/A
//output :
//		N/A
******************************************************************************/
#ifndef TCSUPPORT_XPON_LED
#define GPIO_GPIO17_OUTPUT_ENABLE				(1<<17)
void xpon_los_led_flicker(ulong data)
{
	uint read_data = 0;
	uint write_data = 0;
	uint inverse_data = 0;
	read_data = IO_GREG(GPIO_CSR_DATA);
	if((read_data & GPIO_GPIO17_OUTPUT_ENABLE) == 0)
		inverse_data = GPIO_GPIO17_OUTPUT_ENABLE;
	else
		inverse_data = 0;
	write_data = (read_data & ~(GPIO_GPIO17_OUTPUT_ENABLE)) | inverse_data;
	IO_SREG(GPIO_CSR_DATA, write_data);
    if( !gpPonSysData->flicker_stop ){
    	XPON_START_TIMER(gpPonSysData->led_timer) ;
    }
}
#endif


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
#if defined(CONFIG_USE_MT7520_ASIC) || defined(TCSUPPORT_LED_SWITCH_BUTTON)

#ifdef TCSUPPORT_XPON_LED

void change_alarm_led_status(XPON_ALARM_LED_Status_t los_led_status)
{
    switch(los_led_status)
    {
    	case ALARM_LED_ON:
    		ledTurnOff(LED_XPON_LOS_STATUS);
    		ledTurnOn(LED_XPON_LOS_ON_STATUS);
    		PON_MSG(MSG_DBG, "turn on LED success. \n");
    		break;

    	case ALARM_LED_OFF:
    		ledTurnOff(LED_XPON_LOS_ON_STATUS);
    		ledTurnOff(LED_XPON_LOS_STATUS);
    		PON_MSG(MSG_DBG, "turn off LED success. \n");
    		break;

    	case  ALARM_LED_FLICKER:
    		ledTurnOff(LED_XPON_LOS_ON_STATUS);
    		ledTurnOn(LED_XPON_LOS_STATUS);
    		PON_MSG(MSG_DBG, "LED flicker success. \n");
    		break;

    	default:
    		PON_MSG(MSG_ERR, "LOS LED fail. \n");
    		break;
    }
}

#else /* TCSUPPORT_XPON_LED */

#define LOS_LED_GPIO_MASK						(1<<17)

void change_alarm_led_status(XPON_ALARM_LED_Status_t los_led_status)
{
	uint read_data  = 0;
	uint write_data = 0;

    switch(los_led_status)
    {
    	case ALARM_LED_ON:
            gpPonSysData->flicker_stop = 1;
    		XPON_STOP_TIMER(gpPonSysData->led_timer) ;
    		read_data = IO_GREG(GPIO_CSR_DATA);
    		write_data = read_data | (LOS_LED_GPIO_MASK);
    		IO_SREG(GPIO_CSR_DATA, write_data);
    		PON_MSG(MSG_DBG, "turn on LOS LED success. \n");
    		break;

    	case ALARM_LED_OFF:
            gpPonSysData->flicker_stop = 1;
    		XPON_STOP_TIMER(gpPonSysData->led_timer) ;
    		read_data = IO_GREG(GPIO_CSR_DATA);
    		write_data = read_data & ~(LOS_LED_GPIO_MASK);
    		IO_SREG(GPIO_CSR_DATA, write_data);
    		PON_MSG(MSG_DBG, "turn off LOS LED success. \n");
    		break;

    	case  ALARM_LED_FLICKER:
            gpPonSysData->flicker_stop = 0;
    		XPON_START_TIMER(gpPonSysData->led_timer) ;
    		PON_MSG(MSG_DBG, "LOS LED flicker success. \n");
    		break;

    	default:
            gpPonSysData->flicker_stop = 1;
    		XPON_STOP_TIMER(gpPonSysData->led_timer) ;
    		PON_MSG(MSG_ERR, "LOS LED fail. \n");
    		break;
    }
}
#endif /* TCSUPPORT_XPON_LED */

#else  /* defined(CONFIG_USE_MT7520_ASIC) || defined(TCSUPPORT_LED_SWITCH_BUTTON) */

void change_alarm_led_status(XPON_ALARM_LED_Status_t los_led_status)
{
    return ;
}

#endif /* defined(CONFIG_USE_MT7520_ASIC) || defined(TCSUPPORT_LED_SWITCH_BUTTON) */

extern int xpon_los_status;              // indicate if there is optical signal

int xpon_los_led_read_proc(char *page, char **start, off_t off,
	int count, int *eof, void *data)
{
	int len = 0;

	len = sprintf(page, "%d\n", xpon_los_status);

	len -= off;
	*start = page + off;

	if (len > count)
		len = count;
	else
		*eof = 1;

	if (len < 0)
		len = 0;

	return len;
}

int xpon_los_led_write_proc(struct file *file, const char *buffer,
	unsigned long count, void *data)
{
	char val_string[8] = {0};
	unsigned long val  = 0;

	if (count > sizeof(val_string) - 1)
		return -EINVAL;

	if (copy_from_user(val_string, buffer, count))
		return -EFAULT;

	val_string[count] = '\0';

	val = simple_strtoul(val_string, NULL, 10);

	change_alarm_led_status(val);
	
	return count;
}



