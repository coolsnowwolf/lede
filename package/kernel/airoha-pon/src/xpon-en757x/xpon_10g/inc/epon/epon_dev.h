#ifndef EPON_DEV_H
#define EPON_DEV_H

#define EPON_REGS_READ			(0)
#define EPON_REGS_WRITE			(1)
#define EPON_ADDR_REG_LOW		(0)
#define EPON_ADDR_REG_HIGH		(1)

#define EPON_MAC_ADDR_CFG_OPERATE(reg,addr_id,llid,opt)		{\
															    reg.Bits.mac_addr_dw_idx = addr_id;\
																reg.Bits.mac_addr_llid_indx = llid;\
																reg.Bits.mac_addr_rwcmd = opt;\
																WRITE_REG_WORD(e_mac_addr_cfg , reg.Raw);\
															}

int epon_check_mac_cfg(void);


#define EPON_CHECK_MAC_ADDR_CFG_DONE   	if(epon_check_mac_cfg() < 0){\
											xpon_debug_print("ERROR: %s do epon_check_mac_cfg",__FUNCTION__);\
											return EPON_NORMAL_ERROR;\
										}

int get_onu_mac_address(__u8 *macAddr);
int epon_get_mac_address(EPON_LLID_INFO_T *llidEntry_p);
int epon_set_mac_address(EPON_LLID_INFO_T *llidEntry_p);
void epon_dev_set_tx_burst_mode(__u8 mode);
int epon_dev_tx_rx_disable(void);
void epon_dev_set_hw_discv_states(__u8 llid_index,__u8 state);
void epon_dev_send_mpcp_msg(MPCP_SEND_INFO_T *send_msg_info);
void epon_dev_interrupt_mask_enable(void);
int epon_get_llid_regs_mac_address(__u8 addr_index,__u8 llid_index, __u32 *addr_val);
int epon_set_llid_regs_mac_address(__u8 addr_index,__u8 llid_index, __u32 addr_val);

#endif
