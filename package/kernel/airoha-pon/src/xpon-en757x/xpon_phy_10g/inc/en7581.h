#ifndef _EN7581_H_
#define _EN7581_H_

// -- includes -----------------------------------------------------------------

#include "phy_def.h"
#include "phy_types.h"


// -- local forward declarations ---------------------------------------------------

extern ponPhyFuncTbl en7581_gepon_func[];
extern ponPhyFuncTbl en7581_xgpon_func[];
extern ponPhyFuncTbl en7581_xepon_func[];

int en7581_get_wan_sel(void);
int en7581_func_init(void);
int en7581_set_trans_setting(void);
int en7581_phy_cnt_enable(int errcnt_en, int bipcnt_en, int fmcnt_en);
int en7581_phy_reset_counter(void);
int en7581_phy_clear_int(void);

unchar en7581_phy_gpon_mode(void) ;
int en7581_phy_set_epon_ts_continue_mode(uint mode);
int en7581_fw_ready(int fwrdy_en);
int en7581_disable_pcs_tdc(void);

void en7581_pon_phy_clear_rogueonu(void);
void en7581_pon_phy_rogueonu_int_en(unchar rogue_int_en);

#if ASIC_SERDES
void freq_check (void);	//add by david 20180904
void set_pma_fir(void);	
void get_pma_fir(void);	
void tdc_bw(uint bandwidth_level);	//add by david 20190306
void xpon_pma_param_opt (void);	//add by david 20190315
void pma_fifo_check (uint ncpo_check_cnt);	//add by ang_20190130
void pma_reg_dump (void);
void pma_reg_reset(uint start, uint end);
void pma_no_los_no_ready_reset(void);
#endif

typedef  union
{
     struct
    {
#ifdef __BIG_ENDIAN
        UINT32 rsv_25                    : 7;
        UINT32 xg_tx_fec_en4             : 1;
        UINT32 rsv_17                    : 7;
        UINT32 xg_tx_fec_en3             : 1;
        UINT32 rsv_9                     : 7;
        UINT32 xg_tx_fec_en2             : 1;
        UINT32 rsv_1                     : 7;
        UINT32 xg_tx_fec_en1             : 1;
#else
        UINT32 xg_tx_fec_en1             : 1;
        UINT32 rsv_1                     : 7;
        UINT32 xg_tx_fec_en2             : 1;
        UINT32 rsv_9                     : 7;
        UINT32 xg_tx_fec_en3             : 1;
        UINT32 rsv_17                    : 7;
        UINT32 xg_tx_fec_en4             : 1;
        UINT32 rsv_25                    : 7;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} EN7581_REG_PHY_FEC_INDICATION;

typedef  union
{
     struct
    {
#ifdef __BIG_ENDIAN        
        UINT32 rsv_24                   : 8;
        UINT32 prmb_rpt_num             : 8;
        UINT32 rsv_12                   : 4;
        UINT32 ptmb_len                 : 4;
        UINT32 rsv_4                    : 4;
        UINT32 dlmt_len                 : 4;
#else
        UINT32 dlmt_len                 : 4;
        UINT32 rsv_4                    : 4;
        UINT32 ptmb_len                 : 4;
        UINT32 rsv_12                   : 4;
        UINT32 prmb_rpt_num             : 8;
        UINT32 rsv_24                   : 8;
#endif /* __BIG_ENDIAN */
    } Bits;
    UINT32 Raw;
} EN7581_REG_PHY_PSBU_INFO;


#endif /* _EN7581_H_ */

