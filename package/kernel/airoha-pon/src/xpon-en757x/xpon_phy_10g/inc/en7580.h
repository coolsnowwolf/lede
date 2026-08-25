#ifndef _EN7580_H_
#define _EN7580_H_

// -- includes -----------------------------------------------------------------

#include "phy_def.h"
#include "phy_types.h"


// -- local forward declarations ---------------------------------------------------

extern ponPhyFuncTbl en7580_gepon_func[];
extern ponPhyFuncTbl en7580_xgpon_func[];
extern ponPhyFuncTbl en7580_xepon_func[];

int en7580_get_wan_sel(void);
int en7580_func_init(void);
int en7580_set_trans_setting(void);
int en7580_phy_cnt_enable(int errcnt_en, int bipcnt_en, int fmcnt_en);
int en7580_phy_reset_counter(void);
int en7580_phy_clear_int(void);

unchar en7580_phy_gpon_mode(void) ;
int en7580_phy_set_epon_ts_continue_mode(uint mode);
int en7580_fw_ready(int fwrdy_en);
int en7580_disable_pcs_tdc(void);
#if ASIC_SERDES
#ifndef TCSUPPORT_CPU_EN7581
void phy_eyescan_test(PHY_EYESCAN_MODE_LIST mode, int Ovr_sel, int start_p, int sweep_r);
#endif
#endif

void XPON_eyescan_setting(PHY_EYESCAN_MODE_LIST mode);
void XFI_eyescan_setting(PHY_EYESCAN_MODE_LIST mode);
void XPON_eyescan_Cal(void);
void XFI_eyescan_Cal(void);
int eyescan_countPoint(uint EYE_X_FW, uint EYE_Y_FW, PHY_EYESCAN_MODE_LIST mode);
uint eyescan_moveX(uint EYE_X_HW, uint EYE_Y_HW, int Ovr_sel, PHY_EYESCAN_MODE_LIST mode);

void pon_phy_clear_rogueonu(void);
void pon_phy_rogueonu_int_en(unchar rogue_int_en);

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
} REG_PHY_FEC_INDICATION;

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
} REG_PHY_PSBU_INFO;

#endif /* _EN7580_H_ */

