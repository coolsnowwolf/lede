#ifndef EPON_SYS_API_H_
#define EPON_SYS_API_H_

#define SYS_ONU_TYPE_SFU 0
#define SYS_ONU_TYPE_HGU 1


#define MASK_TRTCM_SCALE 		(1<<0) 
#define MASK_TRTCM_PARAMETERS			(1<<1) 
#define MASK_CONGEST_TRTCM_MODE 		(1<<0) 
int epon_sys_get_onu_type(void);

int epon_sys_set_trx_mode(u_char llid, u_char rx_mode, u_char tx_mode);
int epon_sys_set_link_state(u_char llid, u_char state);

#endif // EPON_SYS_API_H_