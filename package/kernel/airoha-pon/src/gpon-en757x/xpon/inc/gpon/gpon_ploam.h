#ifndef _GPON_PLOAM_H_
#define _GPON_PLOAM_H_

#include "gpon/gpon_ploam_raw.h"
#include "xmcs/xmcs_gpon.h"

#define BYTE_DELAY_MASK	~(0x7)
#define BIT_DELAY_MASK	 (0x7)

#define PLOAM_DISABLE_DENIED			(0xFF)
#define PLOAM_DISABLE_PARTICIPATE_ALL	(0x0F)
#define PLOAM_DISABLE_PARTICIPATE		(0x00)

/* ALU OLT Extension, all onu in state other than O1 should goto O7 */
#define PLOAM_DISABLE_DENIED_ALL        (0xF0)

typedef int (*ploam_recv_handler_t)(PLOAM_RAW_General_T *pPloamMsg) ;

int ploam_init(void) ;
int ploam_parser_down_message(PLOAM_RAW_General_T *pGenPloamMsg) ;
void ploam_eqd_adjustment(uint newEqd) ;

int ploam_send_dying_gasp(void) ;
int ploam_send_pee_msg(void) ;
int ploam_send_pst_msg(unchar line_num, unchar k1_ctl, unchar k2_ctl) ;
int ploam_send_rei_msg(uint err_count, unchar *seq_num_p) ;
int ploam_send_sleep_request_msg(GPON_PLOAMu_SLEEP_MODE_t sleepMode);

#endif /* _GPON_PLOAM_H_ */

