

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "epon_oam.h"
#include "epon_sys_api.h"
#include "ctc/oam_ctc_node.h"
#include "api/mgr_api.h"
#include "libcompileoption.h"

extern eponOamCfg_t eponOamCfg;
extern int startloidAuth;
int current_state[8] = {-1, -1, -1, -1, -1, -1, -1, -1};

#if defined(TCSUPPORT_XPON_LED)
int xpon_led(char *led_st)
{
  FILE *xpon_led_proc;

  xpon_led_proc = fopen("/proc/tc3162/led_xpon", "w+");
    
  if (!xpon_led_proc) {
  	tcdbg_printf("\nopen led_xpon proc fail.");
    return -1;
  }

  //tcdbg_printf("\n>>>OAM XPON_LED=%s<<<\n", led_st);
  fwrite(led_st, 1, strlen(led_st), xpon_led_proc);
  fclose(xpon_led_proc);

  return 0;
}
#endif

/* 
 * Get ONU TYPE : 
 *  return: HGU / SFU(default)
 */
int epon_sys_get_onu_type(void){
	MACRO_CMD_STRUCT(SysOnuType) sysOnuType;
	
	if(PonApi_SysOnuType_Get(&sysOnuType) == EXEC_OK) {
		if (sysOnuType.OnuType == ENUM_SYSTEM_ONU_TYPE_HGU)
			return SYS_ONU_TYPE_HGU;
	}
		
	return SYS_ONU_TYPE_SFU;
}

/*
 * Set EPON WAN Driver Tx&Rx Mode 
 * 	rx_mode: ENUM_EPON_RX_FORWARDING = 0,
 * 			 ENUM_EPON_RX_DISCARD,
 * 			 ENUM_EPON_RX_LOOPBACK
 * 	tx_mode: ENUM_EPON_TX_FORWARDING = 0,
 * 			 ENUM_EPON_TX_DISCARD
 * Return: FAIL(-1) or SUCCESS(0)
 */
int epon_sys_set_trx_mode(u_char llid, u_char rx_mode, u_char tx_mode){
	MACRO_CMD_STRUCT(EponRxConfig) rx_cfg;
	MACRO_CMD_STRUCT(EponTxConfig) tx_cfg;
	
	rx_cfg.LlidIdx = llid;
	rx_cfg.RxMode = rx_mode;
	if (EXEC_OK != PonApi_EponRxConfig_Set(MASK_ALL, &rx_cfg)){
		return FAIL;
	}
	
	tx_cfg.LlidIdx = llid;
	tx_cfg.TxMode = tx_mode;
	if (EXEC_OK != PonApi_EponTxConfig_Set(MASK_ALL, &tx_cfg)){
		return FAIL;
	}
	return SUCCESS;
}

/*
 * Set LLID's Link State:
 * 	state: 0=down, 1=up
 * Return: FAIL(-1) or SUCCESS(0)
 * */

int epon_sys_set_link_state(u_char llid, u_char state){
	int sta[EPON_LLID_MAX_NUM] = {0};
	char buf[128] = {0};
	FILE *fp = NULL;
	static int protect = 1;

	if (current_state[llid] == state)
		return SUCCESS;
	current_state[llid] = state;

	if (protect != 1)
		return FAIL;
	protect = 0;
	
	/* mac auth, close device register function */
	if (startloidAuth == 0 && state == LINK_UP) {
		tcapi_set("deviceAccount_Entry", "isNOLandingPage", "1");
		tcapi_commit("deviceAccount_Entry");
	}
	
	fp = fopen(OAM_LINK_STAT_PATH, "w+");
	if (fp ==  -1){
		return FAIL;
	}
	
	fscanf(fp, "%d %d %d %d %d %d %d %d", &sta[0], &sta[1], &sta[2], 
		&sta[3], &sta[4], &sta[5], &sta[6], &sta[7]);
		
	sta[llid] = state;
	
	fprintf(fp, "%d %d %d %d %d %d %d %d", sta[0], sta[1], sta[2], 
		sta[3], sta[4], sta[5], sta[6], sta[7]);
	fclose(fp);

#if defined(TCSUPPORT_XPON_LED)
	if (state == LINK_UP){
		xpon_led("2");
	}
#endif		
	protect = 1;
	return SUCCESS;
}

/*
 * Get Transceiver Info:
 * Return: FAIL(-1) or SUCCESS(0)
 * */
int epon_sys_get_transceiver_info(OamOpticalTransceiverDiagnosis_Ptr info_p){
	MACRO_CMD_STRUCT(PhyTransParameters) getEntry;

	if (EXEC_OK != PonApi_PhyTransParameters_Get(&getEntry)){
		return FAIL;
	}
	
	info_p->transceiverTemperature = (u_short)getEntry.Temperature;
	info_p->supplyVoltage = (u_short)getEntry.Voltage;
	info_p->txBiasCurrent = (u_short)getEntry.TxCurrent;
	info_p->txPower = (u_short)getEntry.TxPower;
	info_p->rxPower = (u_short)getEntry.RxPower;
	return SUCCESS;
}

