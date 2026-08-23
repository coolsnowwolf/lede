#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>


#include "epon/epon.h"
#include "epon/epon_hwtest.h"
#include "epon/epon_cmd.h"

/************************************************************************
*                      E X T E R N A L   F U N C T I O N
*************************************************************************
*/

extern long eponMacIoctl(struct file *file, unsigned int cmd, unsigned long arg);
extern void eponUpdateStackMpcpErrCnt(void);

/************************************************************************
*                      E X T E R N A L   D A T A
*************************************************************************
*/
extern __u8 eponTimeDrftHandleFlag ;
extern __u8 mpcpcnt_llid;
#ifdef EPON_MAC_HW_TEST
extern int logRstTestFlag;
#endif

/************************************************************************
*                         D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                          C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                            M A C R O S
*************************************************************************
*/

/************************************************************************
*                        P U B L I C   D A T A
*************************************************************************
*/
__u8 eponStaticRptEnable = 0;
__u32 staticRptData = 0x01018000;
__u32 eponDefLlidMask = 1;

int sniffer_flag = 0;
unsigned int sniffer_config = 0;
unsigned int special_tag = 0;

struct list_head fix_reg_list;
eponMacHwtestReg_t *eponMacRegTable = NULL;

#ifdef EPON_MAC_HW_TEST
__u32 eponSetProbe = 0 ;
__u32 ignoreIntMask = 0xffffffff;
#endif

#ifdef TCSUPPORT_AUTOBENCH
__u32 eponDebugLevel = EPON_DEBUG_LEVEL_NONE;
#else
__u32 eponDebugLevel = EPON_DEBUG_LEVEL_ERR;
#endif

/************************************************************************
*                       P R I V A T E   D A T A
*************************************************************************
*/
static const cmds_t eponMpcpCmds[] = {
        {"deregister",      doEponDeregister,       0x02,   1,  "<deregister>"},
        {"tmdrftflag",      doEponSetTmdrftflag,    0x02,   1,  "<tmdrftflag>"},
#ifdef EPON_MAC_HW_TEST
        {"setprobe",        doEponSetProbe,     0x02,   1,  "<probe>"},
        {"wanreset",        doEponResetWan,     0x02,   0,  "<wanreset>"},
        {"macreset",        doEponResetMac,     0x02,   0,  "<macreset>"},
        {"rgstack",     doEponSetRgstAckType,   0x02,   0,  "<llid,ack>"},
        {"ignoreIntMask",       doEponSetIgnoreIntMask,     0x02,   0,  "<mask(32bit)>"},
        {"setDefLlidMask",      doEponSetDefLlidMask,   0x02,   1,  "<mask(16bit)>"},
#endif
        {NULL,          NULL,                   0x10,   0,  NULL},
    };
    
static const cmds_t eponSetCmds[] = {
        {"llidThrshldNum",      doEponSetLlidThrshldNum,        0x02,   1,  "<llidIndex(0~7)> <llidThrshldNum(1~3)>"},
        {"llidQueThrshld",      doEponSetLlidQueThrshld,    0x02,   1,  "<llidIndex(0~7)> <thrshldIndex(0~2)> <q0 value> ... <q7 value>"},
        {NULL,          NULL,                   0,  0,  NULL},
    };
    
    
#ifdef EPON_MAC_HW_TEST
static const cmds_t eponHwtestCmds[] = {
        {"regtest",     doEponRegtest,      0x02,   1,  "<times>"},
        {"regDefCheck", doEponRegDefCheck,      0x02,   0,  "<regDefCheck>"},
        {"pureLogRstNonCfgReg", doEponRegRstTest, 0x02, 0,  "<pure logic reset with non-configuration registers>"},
        {"pureLogRstCfgReg",	doEponRegNotRstTest,	0x02,	1,	"<pure logic reset with cfg registers>"},
        {"logRstNonCfgReg",		doEponLogRstNonCfgReg , 0x02,    0, "<logic reset non-cfg registers>"},
        {"logRstCfgReg",         doEponLogRstCfgReg,     0x02,    1, "<logic reset cfg registers and power saving registers>"},
        {"ponMacRstTest",       doEponMacRstTest,       0x02,   0,  "<EPON MAC Reset function test>"},
        {NULL,          NULL,                   0x10,   0,  NULL},
    };
#endif
    
static const cmds_t eponCmds[] = {
        {"debuglevel",      doEponDebuglevel,   0x02,   0,  "<debuglevel>"},
        {"dumpallreg",      doEponDumpAllReg,   0x02,   0,  "<dumpallreg>"},
        {"mpcp",            doEponMpcp,         0x12,   0,  NULL},
        {"set",             doEponSet,          0x12,   0,  NULL},
		{"statistic",       doEponStatistic,    0x12,   0,  "<NULL/clean>"},
        {"staticrpt",       doEponStaticRpt,    0x12,   0,  "<enable/disable> <Value>"},
        {"state",           doEponState,        0x12,   0,  NULL},
        {"register",        doEponReg,          0x12,   0,  "<set/show/del> <Addr> <Value>"},
        {"lasertime",       doEponLsrTm,        0x12,   0,  "<off-on(0~0x20)>"},
#if defined(TCSUPPORT_CPU_EN7527) || defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
        {"sniffer",         doEponSniffer,      0x12,   0,  "<enable/disable> <NULL or othmpcp/otheth/othoam/all>"},
#endif
#ifdef EPON_MAC_HW_TEST
        {"hwtest",          doEponHwtest,       0x12,   0,  NULL},
#endif
        {NULL,          NULL,                   0x10,   0,  NULL},
    };


/************************************************************************
*              F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

int eponMacGetRegTblSize(void){
    int i = 0;
    if(NULL == eponMacRegTable)
        return 0;
    while(eponMacRegTable[i].addr != 0)
        i++;
    return i;
}

static int doEpon(int argc, char *argv[], void *p)
{
	
	return subcmd(eponCmds, argc, argv, p);
}

static int doEponMpcp(int argc, char *argv[], void *p)
{
	
	return subcmd(eponMpcpCmds, argc, argv, p);
}

static int doEponSet(int argc, char *argv[], void *p)
{
	
	return subcmd(eponSetCmds, argc, argv, p);
}

/*______________________________________________________________________________
**	function name
**		get_register_count
**	description:
**		get count 
**	parameters:
**		tmpReg
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		v1.0
**____________________________________________________________________________*/

void get_register_count(XPON_DEBUG_STATISTIC *tmpReg)
{
    REG_e_mpcp_stat mpcp_stat;
    REG_e_mpcp_rgst_stat mpcp_rgst_stat;
    REG_e_oam_stat oam_stat;
	REG_e_llid0_gnt_stat mpcp_llid0_normal_gate_stat;
	REG_e_llid1_gnt_stat mpcp_llid1_normal_gate_stat;
	REG_e_llid2_gnt_stat mpcp_llid2_normal_gate_stat;
	REG_e_llid3_gnt_stat mpcp_llid3_normal_gate_stat;
	REG_e_llid4_gnt_stat mpcp_llid4_normal_gate_stat;
	REG_e_llid5_gnt_stat mpcp_llid5_normal_gate_stat;
	REG_e_llid6_gnt_stat mpcp_llid6_normal_gate_stat;
	REG_e_llid7_gnt_stat mpcp_llid7_normal_gate_stat;
	REG_e_rpt_cnt mpcp_report_cnt;
   
#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
    REG_e_rxerr_cnt rx_err_cnt;
    rx_err_cnt.Raw = 0;
#endif

    mpcp_stat.Raw = 0;
    mpcp_rgst_stat.Raw = 0;
    oam_stat.Raw = 0;
	mpcp_llid0_normal_gate_stat.Raw= 0;
	mpcp_llid1_normal_gate_stat.Raw= 0;
	mpcp_llid2_normal_gate_stat.Raw= 0;
	mpcp_llid3_normal_gate_stat.Raw= 0;
	mpcp_llid4_normal_gate_stat.Raw= 0;
	mpcp_llid5_normal_gate_stat.Raw= 0;
	mpcp_llid6_normal_gate_stat.Raw= 0;
	mpcp_llid7_normal_gate_stat.Raw= 0;
	mpcp_report_cnt.Raw = 0;

    if(tmpReg == NULL){
        printk("Error: tmpReg is a NULL pointer!\n");
        return;
    }

    /* Reg: mpcp stat*/
    mpcp_stat.Raw = READ_REG_WORD(e_mpcp_stat);
    tmpReg->mpcpErrCnt = mpcp_stat.Bits.mpcp_err_cnt;
    tmpReg->mpcpRgstCnt = mpcp_stat.Bits.mpcp_rgst_cnt;
    tmpReg->mpcpDscvGateCnt = mpcp_stat.Bits.mpcp_dscv_gate_cnt;

	switch(mpcpcnt_llid)
	{
		case 0:
			mpcp_llid0_normal_gate_stat.Raw = READ_REG_WORD(e_llid0_gnt_stat);
			tmpReg->mpcpNormalGateCnt = mpcp_llid0_normal_gate_stat.Bits.llid0_gnt_cnt;
			break;
		case 1:
			mpcp_llid1_normal_gate_stat.Raw = READ_REG_WORD(e_llid1_gnt_stat);
			tmpReg->mpcpNormalGateCnt = mpcp_llid1_normal_gate_stat.Bits.llid1_gnt_cnt;
			break;
		case 2:
			mpcp_llid2_normal_gate_stat.Raw = READ_REG_WORD(e_llid2_gnt_stat);
			tmpReg->mpcpNormalGateCnt = mpcp_llid2_normal_gate_stat.Bits.llid2_gnt_cnt;
			break;
		case 3:
			mpcp_llid3_normal_gate_stat.Raw = READ_REG_WORD(e_llid3_gnt_stat);
			tmpReg->mpcpNormalGateCnt = mpcp_llid3_normal_gate_stat.Bits.llid3_gnt_cnt;
			break;
		case 4:
			mpcp_llid4_normal_gate_stat.Raw = READ_REG_WORD(e_llid4_gnt_stat);
			tmpReg->mpcpNormalGateCnt = mpcp_llid4_normal_gate_stat.Bits.llid4_gnt_cnt;
			break;
		case 5:
			mpcp_llid5_normal_gate_stat.Raw = READ_REG_WORD(e_llid5_gnt_stat);
			tmpReg->mpcpNormalGateCnt = mpcp_llid5_normal_gate_stat.Bits.llid5_gnt_cnt;
			break;
		case 6:
			mpcp_llid6_normal_gate_stat.Raw = READ_REG_WORD(e_llid6_gnt_stat);
			tmpReg->mpcpNormalGateCnt = mpcp_llid6_normal_gate_stat.Bits.llid6_gnt_cnt;
			break;
		case 7:
			mpcp_llid7_normal_gate_stat.Raw = READ_REG_WORD(e_llid7_gnt_stat);
			tmpReg->mpcpNormalGateCnt = mpcp_llid7_normal_gate_stat.Bits.llid7_gnt_cnt;
			break;
		default:
			break;
	}

	mpcp_report_cnt.Raw = READ_REG_WORD(e_rpt_cnt);
	tmpReg->mpcpTxReportCnt = mpcp_report_cnt.Bits.rpt_cnt;

    /*Reg: mpcp_rgst_stat*/
    mpcp_rgst_stat.Raw = READ_REG_WORD(e_mpcp_rgst_stat);
    tmpReg->mpcpRgstReqCnt = mpcp_rgst_stat.Bits.mpcp_rgst_req_cnt;
    tmpReg->mpcpRgstAckCnt = mpcp_rgst_stat.Bits.mpcp_rgst_ack_cnt;

    /*Reg: rxmpi_eth*/
    tmpReg->rxMpiEth = READ_REG_WORD(e_rxmpi_eth_cnt);

    /*Reg: rxmbi_eth*/
    tmpReg->rxMbiEth = READ_REG_WORD(e_rxmbi_eth_cnt);

    /*Reg: txmpi_eth*/
    tmpReg->txMpiEth = READ_REG_WORD(e_txmpi_eth_cnt);

    /*Reg: txmbi_eth*/
    tmpReg->txMbiEth = READ_REG_WORD(e_txmbi_eth_cnt);
    
    /*Reg: oam_stat*/
    oam_stat.Raw = READ_REG_WORD(e_oam_stat);
    tmpReg->rxOamCnt = oam_stat.Bits.rx_oam_cnt;
    tmpReg->txOamCnt = oam_stat.Bits.tx_oam_cnt;

#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
    /*crc err cnt*/
    rx_err_cnt.Raw = READ_REG_WORD(e_rxerr_cnt);
    tmpReg->rxPrmbCrc8ErrCnt = rx_err_cnt.Bits.rxprmb_crc8_err_cnt;
    tmpReg->rxCrc32ErrCnt = READ_REG_WORD(e_rx_crc32_cnt);
#endif

    return;
}

/*______________________________________________________________________________
**	function name
**		get_register_state
**	description:
**		get state 
**	parameters:
**		tmpReg
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		v1.0
**____________________________________________________________________________*/
static void get_register_state(XPON_DEBUG_STATE *tmpReg)
{
    if(tmpReg == NULL){
        printk("Error: tmpReg is a NULL pointer!\n");
        return;
    }
    
    tmpReg->glbCfg = READ_REG_WORD(e_glb_cfg);
    tmpReg->intSts = READ_REG_WORD(e_int_status);
    tmpReg->intEn = READ_REG_WORD(e_int_en);
    tmpReg->llidCfg0_3 = READ_REG_WORD(e_llid0_3_cfg);
    tmpReg->rptCfg = READ_REG_WORD(e_rpt_cfg);
    tmpReg->laserTm = READ_REG_WORD(e_laser_onoff_time);
    tmpReg->syncTm = READ_REG_WORD(e_sync_time);
    tmpReg->txCnst = READ_REG_WORD(e_tx_cal_cnst);
    tmpReg->pwrSvCfg = READ_REG_WORD(e_pwr_sv_cfg);
    tmpReg->dygspCfg = READ_REG_WORD(e_dyinggsp_cfg);

    return;
}

/*______________________________________________________________________________
**  function name
**      add_fix_reg_list
**  description:
**      add fix register 
**  parameters:
**      addr
**      value
**  global:
**      fix_reg_list
**  return:
**      
**  call:
**      
**  revision:
**      v1.0
**____________________________________________________________________________*/
static void add_fix_reg_list(__u32 addr, __u32 value)
{
    FIX_REG_LIST_ENTRY_T *new_entry = NULL;
	FIX_REG_LIST_ENTRY_T *entry = NULL;
    
    list_for_each_entry_rcu(entry, &fix_reg_list, list)
    {
        if(entry->regInfo.addr == addr){
            entry->regInfo.value = value;
            printk("modify 0x%x: 0x%x\n", entry->regInfo.addr, entry->regInfo.value);
            return;
        }
    }

    new_entry = kmalloc(1 * sizeof(FIX_REG_LIST_ENTRY_T), GFP_ATOMIC);
    if(new_entry == NULL)
        return;
        
    new_entry->regInfo.addr = addr;
    new_entry->regInfo.value = value;
    printk("add 0x%x: 0x%x\n", new_entry->regInfo.addr, new_entry->regInfo.value);    
    INIT_LIST_HEAD(&new_entry->list);
    list_add_tail_rcu(&new_entry->list, &fix_reg_list);
    
    return;
}

/*______________________________________________________________________________
**  function name
**      del_fix_reg_list
**  description:
**      delete fix register 
**  parameters:
**      
**  global:
**      fix_reg_list
**  return:
**      
**  call:
**      
**  revision:
**      v1.0
**____________________________________________________________________________*/
static void del_fix_reg_list(void)
{
    FIX_REG_LIST_ENTRY_T *entry = NULL;
    FIX_REG_LIST_ENTRY_T *tmp = NULL;

    rcu_read_lock();
    list_for_each_entry_rcu(entry, &fix_reg_list, list)
    {
        tmp = list_entry(&entry->rcu, FIX_REG_LIST_ENTRY_T, rcu);
        printk("del 0x%x: 0x%x\n", entry->regInfo.addr, entry->regInfo.value);
        list_del_rcu(&entry->list);
        kfree(tmp);
    }
    rcu_read_unlock();
    
    return;
}

/*______________________________________________________________________________
**  function name
**      show_fix_reg_list
**  description:
**      show fix register 
**  parameters:
**      
**  global:
**      fix_reg_list
**  return:
**      
**  call:
**      
**  revision:
**      v1.0
**____________________________________________________________________________*/
static void show_fix_reg_list(void)
{
    FIX_REG_LIST_ENTRY_T *entry = NULL;
    
    printk("Register List:\n");
    list_for_each_entry_rcu(entry, &fix_reg_list, list){
        printk("0x%x: 0x%x\n", entry->regInfo.addr, entry->regInfo.value);
    }

    return;
}

/*______________________________________________________________________________
**  function name
**      set_fix_reg_list
**  description:
**      set fix register 
**  parameters:
**      
**  global:
**      fix_reg_list
**  return:
**      
**  call:
**      
**  revision:
**      v1.0
**____________________________________________________________________________*/
void set_fix_reg_list(void)
{
    FIX_REG_LIST_ENTRY_T *entry = NULL;
    list_for_each_entry_rcu(entry, &fix_reg_list, list){
        printk("set 0x%x: 0x%x\n", entry->regInfo.addr, entry->regInfo.value);
        IO_SREG(entry->regInfo.addr, entry->regInfo.value);
    }

    return;
}

static int doEponStatistic(int argc, char *argv[], void *p){
	XPON_DEBUG_STATISTIC tmpReg;
    memset(&tmpReg,0,sizeof(XPON_DEBUG_STATISTIC));
  
    if(argv[1]==NULL)
    {
        get_register_count(&tmpReg);

        /*mpcp cnt*/
        printk("\n\033[1;34m-> Mpcp Counter \033[0m\n");
        printk("mpcpDiscGateCnt:\033[1;33m 0x%-8x \033[0m\t", tmpReg.mpcpDscvGateCnt);
        printk("mpcpRgstReqCnt:\033[1;33m 0x%-8x \033[0m\n", tmpReg.mpcpRgstReqCnt);
        printk("mpcpRgstCnt:\033[1;33m 0x%-8x \033[0m\t", tmpReg.mpcpRgstCnt);      
        printk("mpcpRgstAckCnt:\033[1;33m 0x%-8x \033[0m\n", tmpReg.mpcpRgstAckCnt);

        /*oam cnt*/
        printk("\n\033[1;34m-> Oam Counter \033[0m\n");
        printk("rxOamCnt:\033[1;33m 0x%-8x \033[0m\t\t", tmpReg.rxOamCnt);
        printk("txOamCnt:\033[1;33m 0x%-8x \033[0m\n", tmpReg.txOamCnt);

        /*ethernet cnt*/
        printk("\n\033[1;34m-> Ethernet Counter \033[0m\n");
        printk("rxMpiEth:\033[1;33m 0x%-8x \033[0m\t\t", tmpReg.rxMpiEth);
        printk("txMpiEth:\033[1;33m 0x%-8x \033[0m\n", tmpReg.txMpiEth);
        printk("rxMbiEth:\033[1;33m 0x%-8x \033[0m\t\t", tmpReg.rxMbiEth);
        printk("txMbiEth:\033[1;33m 0x%-8x \033[0m\n", tmpReg.txMbiEth);

        #if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
        /*error/drop cnt*/
        printk("\n\033[1;34m-> Error/Drop Counter \033[0m\n");
        printk("rxPrmbCrc8ErrCnt:\033[1;33m 0x%-8x \033[0m\t", tmpReg.rxPrmbCrc8ErrCnt);
        printk("rxCrc32ErrCnt:\033[1;33m 0x%-8x \033[0m\n", tmpReg.rxCrc32ErrCnt);
        #endif
    }else if(!strcmp(argv[1],"clean")){
        doEponClearStatistic();
    }else{
        printk("Cmd error.\n");
    }

	return 0;
}

static int doEponClearStatistic(void){
	eponUpdateStackMpcpErrCnt();
    WRITE_REG_WORD(e_mpcp_stat,0);
    WRITE_REG_WORD(e_mpcp_rgst_stat,0);
	WRITE_REG_WORD(e_rxmpi_eth_cnt,0);
	WRITE_REG_WORD(e_rxmbi_eth_cnt,0);
	WRITE_REG_WORD(e_txmpi_eth_cnt,0);
	WRITE_REG_WORD(e_txmbi_eth_cnt,0);
	WRITE_REG_WORD(e_oam_stat,0);
#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	WRITE_REG_WORD(e_rx_crc32_cnt,0);
#endif
	printk("clear done\n");

	return 0;
}

/*______________________________________________________________________________
**	function name
**		doEponStaticRpt
**	description:
**		EponStaticRpt
**	parameters:
**		
**	global:
**		None
**	return:
**
**	call:
**		v1.0
**____________________________________________________________________________*/
static int doEponStaticRpt(int argc, char *argv[], void *p)
{
    __u32 tmp = 0;
    
    if (argc < 2){
		printk("Usage: <enable/disable> <Value>\n");
		return -1;
	}
	
    if(!strcmp(argv[1],"enable"))
    {
        if(argc < 3){
            printk("Usage: <Value>\n");
            return -1;
        }

        tmp = simple_strtoul(argv[2], NULL, 16);
        tmp = tmp & 0xffff;
        staticRptData = tmp | 0x01010000; //0x0101 means number of queueset is 1 and report bitmap is 1
        eponStaticRptEnable = 1;
        
        e_rpt_len_SET_df_rpt_fifo_clr(e_rpt_len, 1); // clear rpt buffer
		e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
		e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
		e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
		e_rpt_len_GET_df_rpt_fifo_clr(e_rpt_len);
		e_rpt_len_SET_df_rpt_fifo_clr(e_rpt_len, 0); // return normal
		e_rpt_len_SET_df_rpt_dt_len(e_rpt_len, 1);// set def rpt data length
		e_rpt_data_SET_df_rpt_data(e_rpt_data, staticRptData); // static report value
		e_rpt_data_SET_df_rpt_data(e_rpt_data, 0);					
		e_glb_cfg_SET_tx_default_rpt(e_glb_cfg, 1);
        
        printk("queueSet[31:24] bitmap[23:16] q0 report size[15:0]:0x%x\n", staticRptData);
    }else if(!strcmp(argv[1],"disable")){
        e_glb_cfg_SET_tx_default_rpt(e_glb_cfg, 0);
        eponStaticRptEnable = 0;
        staticRptData = 0x01018000;
        printk("Static Report disable.\n");
    }else{
        printk("Cmd error.\n");
    }

    return 0;
}

/*______________________________________________________________________________
**	function name
**		doEponState
**	description:
**		print state 
**	parameters:
**		
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		v1.0
**____________________________________________________________________________*/
static int doEponState(int argc, char *argv[], void *p)
{
    XPON_DEBUG_STATE tmpReg;
    memset(&tmpReg,0,sizeof(XPON_DEBUG_STATE));
    get_register_state(&tmpReg);

    /*globle config*/
    printk("---globle config ---\n");
    printk("glbCfg[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\t\t", (__u32)e_glb_cfg,tmpReg.glbCfg);
    printk("txCnst[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\n", (__u32)e_tx_cal_cnst,tmpReg.txCnst);
    printk("pwrSvCfg[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\t\t", (__u32)e_pwr_sv_cfg,tmpReg.pwrSvCfg);
    printk("dygspCfg[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\n", (__u32)e_dyinggsp_cfg,tmpReg.dygspCfg);

    /*interrupt status*/
    printk("--- interrupt status ---\n");
    printk("intSts[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\t\t", (__u32)e_int_status,tmpReg.intSts);
    printk("intEn[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\n", (__u32)e_int_en,tmpReg.intEn);

    /*llid0 status*/
    printk("--- llid0 status ---\n");
    printk("llid0_3_cfg[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\t\t", (__u32)e_llid0_3_cfg,tmpReg.llidCfg0_3);
    printk("rptCfg[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\n", (__u32)e_rpt_cfg,tmpReg.rptCfg);
    printk("laserTm[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\t\t", (__u32)e_laser_onoff_time,tmpReg.laserTm);
    printk("syncTm[Reg:%08x]:\033[1;33m 0x%-8x \033[0m\n", (__u32)e_sync_time,tmpReg.syncTm);

    return 0;
}

/*______________________________________________________________________________
**	function name
**		doEponReg
**	description:
**		register operation
**	parameters:
**		
**	global:
**		None
**	return:
**
**	call:
**		v1.0
**____________________________________________________________________________*/
static int doEponReg(int argc, char *argv[], void *p)
{
    __u32 addr=0, val=0;

    if (argc < 2){
		printk("Usage: <set/show/del> <addr> <value>\n");
		return -1;
	}
	
    if(!strcmp(argv[1],"set"))
    {
        if(argc < 3){
            printk("Usage: <addr> <value>\n");
            return -1;
        }
        addr = simple_strtoul(argv[2], NULL, 16);
        if(argc < 4){
            printk("Usage: <value>\n");
            return -1;
        }
        val = simple_strtoul(argv[3], NULL, 16);
        add_fix_reg_list(addr, val);
    }else if(!strcmp(argv[1],"show")){
        show_fix_reg_list();
    }else if(!strcmp(argv[1],"del")){
        del_fix_reg_list();
    }else{
        printk("Cmd error.\n");
    }

    return 0;
}

/*______________________________________________________________________________
**	function name
**		doEponLsrTm
**	description:
**		laser time
**	parameters:
**		
**	global:
**		None
**	return:
**
**	call:
**		v1.0
**____________________________________________________________________________*/
static int doEponLsrTm(int argc, char *argv[], void *p)
{
    __u32 tmp = 0;

    if (argc < 2){
		printk("Usage: <set/get> <laser off-on(0~0x20)>\n");
		return -1;
	}
	
	
    if(!strcmp(argv[1],"set"))
    {
        if(argc < 3){
            printk("Usage: <laser off-on(0~0x20)>\n");
            return -1;
        }
        tmp = simple_strtoul(argv[2], NULL, 16);
        WRITE_REG_WORD(e_laser_onoff_time, tmp);
        add_fix_reg_list((__u32)e_laser_onoff_time, tmp);
        printk("laser on 0x%x, laser off 0x%x\n",tmp&0xff, (tmp&0xff00)>>8);
    }else if(!strcmp(argv[1],"get")){
        tmp = READ_REG_WORD(e_laser_onoff_time);
        printk("laser on 0x%x, laser off 0x%x\n",tmp&0xff, (tmp&0xff00)>>8);
    }else{
        printk("Cmd error.\n");
    }

    return 0;
}

#if defined TCSUPPORT_CPU_EN7527 || defined TCSUPPORT_CPU_EN7528 || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
/*______________________________________________________________________________
**	function name
**		doEponSniffer
**	description:
**		sniffer econet onu mpcp
**	parameters:
**		
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		v1.0
**____________________________________________________________________________*/
static int doEponSniffer(int argc, char *argv[], void *p)
{
    REG_e_glb_cfg regRaw;

    regRaw.Raw = 0;
    
    if (argc < 2){
		printk("Usage: <enable/disable> <NULL/othmpcp/otheth/othoam/all>\n");
		return -1;
	}
	
    if(!strcmp(argv[1],"enable"))
    {
        regRaw.Raw = READ_REG_WORD(e_glb_cfg);
        regRaw.Bits.rx_nml_gate_fwd = 1;
        regRaw.Bits.sniffer_mode = 1;
        regRaw.Bits.mpcp_fwd = 1;
        regRaw.Bits.all_unicast_llid_pkt_fwd = 1;
        
        if(!argv[2]){
            regRaw.Bits.other_mpcp_fwd = 0;
            regRaw.Bits.other_data_fwd = 0;
            regRaw.Bits.other_oam_fwd = 0;
            regRaw.Bits.snf_mpcp_fwd = 1;
            printk("Sniffer ECNT ONU MPCP packets.\n");
        }else if(!strcmp(argv[2],"othmpcp")){
            regRaw.Bits.other_mpcp_fwd = 1;
            regRaw.Bits.other_data_fwd = 0;
            regRaw.Bits.other_oam_fwd = 0;
            regRaw.Bits.snf_mpcp_fwd = 0;
            printk("Sniffer other ONU MPCP packets.\n");
        }else if(!strcmp(argv[2],"otheth")){
            regRaw.Bits.other_mpcp_fwd = 0;
            regRaw.Bits.other_data_fwd = 1;
            regRaw.Bits.other_oam_fwd = 0;
            regRaw.Bits.snf_mpcp_fwd = 0;
            printk("Sniffer other ONU data packets.\n");
        }else if(!strcmp(argv[2],"othoam")){
            regRaw.Bits.other_mpcp_fwd = 0;
            regRaw.Bits.other_data_fwd = 0;
            regRaw.Bits.other_oam_fwd = 1;
            regRaw.Bits.snf_mpcp_fwd = 0;
            printk("Sniffer other ONU OAM packets.\n");
        }else if(!strcmp(argv[2],"all")){
            regRaw.Bits.other_mpcp_fwd = 1;
            regRaw.Bits.other_data_fwd = 1;
            regRaw.Bits.other_oam_fwd = 1;
            regRaw.Bits.snf_mpcp_fwd = 1;
            printk("Sniffer all packets.\n");
        }
        else{
            printk("Sniffer subcmd error.\n");
            return -1;
        } 
        sniffer_flag = 1;
        sniffer_config = regRaw.Raw;
        special_tag = 0x20000;
        
        WRITE_REG_WORD(e_glb_cfg,sniffer_config);
        WRITE_REG_WORD(e_sniff_sp_tag,special_tag);
    }
    else if(!strcmp(argv[1],"disable"))
    {
        regRaw.Raw = READ_REG_WORD(e_glb_cfg);
        regRaw.Bits.rx_nml_gate_fwd = 0;
        regRaw.Bits.sniffer_mode = 0;
        regRaw.Bits.mpcp_fwd = 0;
        regRaw.Bits.all_unicast_llid_pkt_fwd = 0;
        regRaw.Bits.other_mpcp_fwd = 0;
        regRaw.Bits.other_data_fwd = 0;
        regRaw.Bits.other_oam_fwd = 0;
        regRaw.Bits.snf_mpcp_fwd = 0;
        sniffer_flag = 0;
        sniffer_config = regRaw.Raw;
        
        WRITE_REG_WORD(e_glb_cfg,sniffer_config);
        printk("Epon sniffer disable.\n");
    }
    else{
        printk("Cmd error.\n");
    } 

    return 0;
}
#endif

static int doEponDebuglevel(int argc, char *argv[], void *p){
	if(argc < 2){
		printk("\r\ndebugLevel = %u\n" , eponDebugLevel);
		return 0;
	}
		
	eponDebugLevel = simple_strtoul(argv[1], NULL, 16);
	return 0;
}

static int doEponDumpAllReg(int argc, char *argv[], void *p){
	
	eponMacDumpAllReg();
	return 0;
}

static int doEponDeregister(int argc, char *argv[], void *p){
	__u32 llidIndex;
	if(argc < 2){
		printk("\r\nderegister <LLID Index | 0-7>\n" );
		return 0;
	}
		
	llidIndex = simple_strtoul(argv[1], NULL, 16);


	eponMpcpLocalDergstr((__u8)llidIndex);
	return 0;
}

static int doEponSetTmdrftflag(int argc, char *argv[], void *p){


	if(argc < 2){
		printk("\r\ntmdrftflag <0 | 1>\n" );
		return 0;
	}

	if(simple_strtoul(argv[1], NULL, 16) == 1){
		eponTimeDrftHandleFlag = XPON_ENABLE;
	}else{
		eponTimeDrftHandleFlag = XPON_DISABLE;
	}
	

	return 0;
}

#ifdef EPON_MAC_HW_TEST
static int doEponSetProbe(int argc, char *argv[], void *p){
	if(argc < 2){
		printk("\r\n set epon mac probe after reset\n" );
		printk("\r\n current defalt probe :0x%x\n",eponSetProbe);
		return 0;
	}
	eponSetProbe = simple_strtoul(argv[1], NULL, 16);
	printk("set probe bfb66100 :0x%x",eponSetProbe);

	return 0;
}

static int doEponSetDefLlidMask(int argc, char *argv[], void *p){
	if(argc < 2){
		
		printk("\r\n set default llid mask after reset\n" );
		printk("\r\n current defalt probe :0x%x\n",eponDefLlidMask);
		return 0;
	}
	eponDefLlidMask = simple_strtoul(argv[1], NULL, 16);
	printk("set default llid mask :0x%x",eponDefLlidMask);

	return 0;
}

static int doEponResetMac(int argc, char *argv[], void *p){
	eponMacReinit(FE_CHANNEL_RETIRE_DISABLE);
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "epon mac reset");
	return 0;

}
static int doEponResetWan(int argc, char *argv[], void *p){
	
	eponMacReinit(FE_CHANNEL_RETIRE_ENABLE);
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "epon wan reset");
	return 0;

}

static int doEponSetRgstAckType(int argc, char *argv[], void *p){
	__u8 llidIndex=0,ack=1;
	if(argc < 3){
		printk("\r\n rgstack <llid:0~7> <flag:0(nack)/1(ack)> \n" );
		printk(" set status(ack/nack) of Register_ack. \n");
		return 0;
	}
	llidIndex = simple_strtoul(argv[1], NULL, 16);
	ack = simple_strtoul(argv[2], NULL, 16);
	if(ack !=0)
		ack =1;
	eponMpcpSetDiscvRgstAck(llidIndex,ack);
	printk( "\r\n rgstack llid=%x,ack=%x\n",llidIndex,ack);
	return 0;

}

static int doEponSetIgnoreIntMask(int argc, char *argv[], void *p){
	if(argc < 2){
		printk("\r\n ignoreIntMask <mask>,current mask %x \n",ignoreIntMask);
		return 0;
	}
	ignoreIntMask = simple_strtoul(argv[1], NULL, 16);
	printk("\r\n ignoreIntMask =%x\n",ignoreIntMask);
	return 0;

}

static int doEponHwtest(int argc, char *argv[], void *p)
{
	return subcmd(eponHwtestCmds, argc, argv, p);
}

static int doEponRegtest(int argc, char *argv[], void *p){
	__u32 times =0;
	if(argc < 2){
		printk("\r\nregtest <times>" );
		return 0;
	}


	times = simple_strtoul(argv[1], NULL, 16);
	eponMacRegTest(times);
	return 0;
}

static int doEponRegDefCheck(int argc, char *argv[], void *p){
	eponMacReinit(FE_CHANNEL_RETIRE_DISABLE);
	
	if (eponRegDefCheck(eponMacRegTable , eponMacGetRegTblSize()) == -1)
	{
		printk("EPON Register default value verifycation is failure!!\r\n");
	}
	else
	{
		printk("EPON Register default value verifycation is ok!!\r\n");
	}
	
	return 0;
}


static int doEponRegRstTest(int argc, char *argv[], void *p){	
	if(!eponRegRstTest()){
		printk("Non-Configuration registers reset successfully!\n");
	}

	return 0;
}

static int doEponRegNotRstTest(int argc, char *argv[], void *p){	
	__u32 pattern = 0;
	if(argc < 2){
		printk("\r\nPureLogRst <pattern>" );
		return 0;
	}

	pattern = simple_strtoul(argv[1], NULL, 16);
	
	if(!eponRegNotRstTest(pattern)){
		printk("Configure registers pure logic reset successfuly!\n");
	}

	return 0;
}

static int doEponLogRstNonCfgReg(int argc, char *argv[], void *p){
	logRstTestFlag = 1;
	if(!eponRegRstTest()){
		printk("Non-Configuration registers Logic reset successfully!\n");
	}
	logRstTestFlag = 0;
	return 0;
}

static int doEponLogRstCfgReg(int argc, char *argv[], void *p){
	__u32 pattern = 0;
	logRstTestFlag = 1;
	if(argc < 2){
		printk("\r\nLogRst <pattern>" );
		return 0;
	}

	pattern = simple_strtoul(argv[1], NULL, 16);
	
	if(!eponRegNotRstTest(pattern)){
		printk("Configure registers logic reset successfuly!\n");
	}

	logRstTestFlag = 0;
	return 0;
}

/********************************************************************
After EPON MAC reset(reg:bfb00834) , check register's value is the same as the hardware default value.
*********************************************************************/
static int doEponMacRstTest(int argc, char *argv[], void *p){
    if(eponMacRstTest() == -1){
        printk("EPON MAC Reset Test was Failed!!!\n");
    }else{
        printk("EPON MAC Reset Test was Successful!!!\n");
    }
	
    return 0;
}

#endif/* EPON_MAC_HW_TEST */

static int doEponSetLlidThrshldNum(int argc, char *argv[], void *p)
{
	__u8 llidIndex = 0;
	__u8 num = 0;
	
	if (argc < 3)
	{
		printk("Usage: llidThrshldNum <llidIndex(0~7)> <llidThrshldNum(1~3)>\n");
		return -1;
	}
	
	llidIndex = (__u8)simple_strtoul(argv[1], NULL, 10);
	num = (__u8)simple_strtoul(argv[2], NULL, 10);

	eponSetLlidThrshldNum(llidIndex, num);

	return 0;
}

static int doEponSetLlidQueThrshld(int argc, char *argv[], void *p)
{
	__u8 queueIndex = 0;
	__u8 llidIndex = 0;
	__u8 thrshldIndex = 0;
	__u16 queueThreshold[8] = {0};
	__u8 i = 0;
	int ret = 0;
	eponQueueThreshold_t EponQThr;
	
	if (argc < 11)
	{
		printk("Usage: llidQueThrshld <llidIndex(0~7)> <thrshldIndex(0~2)> <q0 value> ... <q7 value>\n");	
		return -1;
	}

	llidIndex = (__u8)simple_strtoul(argv[1], NULL, 10);
	thrshldIndex = (__u8)simple_strtoul(argv[2], NULL, 10);
	
	for (i =0; i<8; i++)
	{
		queueThreshold[i] = (__u16)simple_strtoul(argv[3+i], NULL, 10);
	}

	for (queueIndex = 0; queueIndex< 8;queueIndex++)
	{
		if (queueThreshold[queueIndex] != 0)
		{
			EponQThr.channel = llidIndex;
			EponQThr.queue = queueIndex;
			EponQThr.thrIdx = thrshldIndex;
			EponQThr.value = queueThreshold[queueIndex];
			ret = epon_set_queue_threshold(&EponQThr) ;
			if(ret < 0 ){
				printk("doEponSetLlidQueThrshld error\n");
				return -1;
			}		
		}
	}
	
	return 0;
}

void eponMacTableInit(void)
{
	int i = 0;
	eponMacRegTable = (eponMacHwtestReg_t *)kmalloc(sizeof(eponMacHwtestReg_t)*120, GFP_KERNEL) ;

	eponMacRegTable[i].addr = (__u32)e_glb_cfg ;//reset bit can not be set
	eponMacRegTable[i].def_value = 0x02800042;
	eponMacRegTable[i].rwmask = 0x1ffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_int_status;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_int_en;//must disable all INT
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_rpt_mpcp_timeout_llid_idx;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_cfg;
	eponMacRegTable[i].def_value = 0x00000100;
	eponMacRegTable[i].rwmask = 0x8ff1037f;
	i++;
	eponMacRegTable[i].addr = (__u32)e_pending_gnt_num;
	eponMacRegTable[i].def_value = 0x00000040;
	eponMacRegTable[i].rwmask = 0x0000007f;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid0_3_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0f0f0f0f;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid4_7_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0f0f0f0f;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid_dscvry_ctrl;
	eponMacRegTable[i].def_value = 0x00010000;
	eponMacRegTable[i].rwmask = 0xc0001107;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid0_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid1_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid2_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid3_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid4_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid5_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid6_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid7_dscvry_sts;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xc0000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_mac_addr_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x8000000f;
	i++;
	eponMacRegTable[i].addr = (__u32)e_mac_addr_value;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;   /*can't write pattern*/
	i++;
	eponMacRegTable[i].addr = (__u32)e_security_key_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x8000003f;
	i++;
	eponMacRegTable[i].addr = (__u32)e_key_value;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;   /*can't write pattern*/
	i++;
	eponMacRegTable[i].addr = (__u32)e_rpt_data;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_rpt_len;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0000013f;
	i++;
    
	eponMacRegTable[i].addr = (__u32)e_rpt_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_rpt_qthld_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;   /*can't write pattern*/
	i++;

	eponMacRegTable[i].addr = (__u32)e_local_time;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tod_sync_x;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tod_ltncy;
	eponMacRegTable[i].def_value = 0x0000000d;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)p2p_tx_tag1;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)p2p_tx_tag2;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_new_tod_p2p_offset_sec_l32;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_new_tod_p2p_tod_offset_nsec;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tod_p2p_tod_sec_l32;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tod_p2p_tod_nsec;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tod_period;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xff;
	i++;
    
	eponMacRegTable[i].addr = (__u32)e_pwr_sv_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x3700ffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_slp_durt_max;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_slp_duration;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_act_duration;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_pwron_dly;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_slp_duration_i;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;

	eponMacRegTable[i].addr = (__u32)e_txfetch_cfg;
	eponMacRegTable[i].def_value = 0x242a03e8;   /*driver init*/
	eponMacRegTable[i].rwmask = 0xffff0fff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_sync_time;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tx_cal_cnst;
	eponMacRegTable[i].def_value = 0x2612010c;
	eponMacRegTable[i].rwmask = 0xffffff3f;
	i++;
	eponMacRegTable[i].addr = (__u32)e_laser_onoff_time;
	eponMacRegTable[i].def_value = 0x00002020;
	eponMacRegTable[i].rwmask = 0x0000ffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_grd_thrshld;
	eponMacRegTable[i].def_value = 0x00000008;
	eponMacRegTable[i].rwmask = 0x000000ff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_mpcp_timeout_intvl;
	eponMacRegTable[i].def_value = 0x03b9aca0;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_rpt_timeout_intvl;
	eponMacRegTable[i].def_value = 0x002faf08;
	eponMacRegTable[i].rwmask = 0x00ffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_max_future_gnt_time;
	eponMacRegTable[i].def_value = 0x03b9aca0;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_min_proc_time;
	eponMacRegTable[i].def_value = 0x00000400;
	eponMacRegTable[i].rwmask = 0x0000ffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_trx_adjust_time1;
	eponMacRegTable[i].def_value = 0x004ffff1;   /*driver init*/
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_trx_adjust_time2;
	eponMacRegTable[i].def_value = 0x00000006;   /*driver init*/
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dbg_prb_sel;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000f1f;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dbg_prb_h32;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask =0x00000000 ;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dbg_prb_l32;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_rxmbi_eth_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_rxmpi_eth_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_txmbi_eth_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_txmpi_eth_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_mpcp_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_mpcp_rgst_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_gnt_pending_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_gnt_length_stat;
	eponMacRegTable[i].def_value = 0x0000ffff;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_gnt_type_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_time_drft_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid0_gnt_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid1_gnt_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid2_gnt_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid3_gnt_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid4_gnt_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid5_gnt_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid6_gnt_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid7_gnt_stat;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x0;
	i++;

	eponMacRegTable[i].addr = (__u32)e_snf_mpcp_oam_ctl;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_rpt_adj;
	eponMacRegTable[i].def_value = 0x01ff0000;
	eponMacRegTable[i].rwmask = 0x1ffffff;
	i++;
    eponMacRegTable[i].addr = (__u32)e_rpt_cnt;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0x00000000;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w1;
	eponMacRegTable[i].def_value = 0x88090300;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w2;
	eponMacRegTable[i].def_value = 0x52000110;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w3;
	eponMacRegTable[i].def_value = 0x01000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w4;
	eponMacRegTable[i].def_value = 0x0f05ee00;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w5;
	eponMacRegTable[i].def_value = 0x13250022;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w6;
	eponMacRegTable[i].def_value = 0x01000210;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w7;
	eponMacRegTable[i].def_value = 0x01000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w8;
	eponMacRegTable[i].def_value = 0x0f05ee00;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w9;
	eponMacRegTable[i].def_value = 0x13250000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w10;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w11;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_dyinggsp_w12;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w1;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w2;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w3;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w4;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w5;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w6;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w7;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w8;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w9;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w10;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w11;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_w12;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_oam_kpalv_ctrl;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffff00;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tod_1pps_ctrl;
	eponMacRegTable[i].def_value = 0x01312d00;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_short_pkt_ctrl;
	eponMacRegTable[i].def_value = 0x00000040;
	eponMacRegTable[i].rwmask = 0xff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_sniff_sp_tag;
	eponMacRegTable[i].def_value = 0x00010000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid01_rpt_ctl;
	eponMacRegTable[i].def_value = 0x01ff01ff;
	eponMacRegTable[i].rwmask = 0x01ff01ff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid23_rpt_ctl;
	eponMacRegTable[i].def_value = 0x01ff01ff;
	eponMacRegTable[i].rwmask = 0x01ff01ff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid45_rpt_ctl;
	eponMacRegTable[i].def_value = 0x01ff01ff;
	eponMacRegTable[i].rwmask = 0x01ff01ff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_llid67_rpt_ctl;
	eponMacRegTable[i].def_value = 0x01ff01ff;
	eponMacRegTable[i].rwmask = 0x01ff01ff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_utili_check;
	eponMacRegTable[i].def_value = 0x00000004;
	eponMacRegTable[i].rwmask = 0x00000005;
	i++;

#if defined(TCSUPPORT_CPU_EN7528) || defined(TCSUPPORT_CPU_EN7523) || defined(TCSUPPORT_CPU_AN7552)
	eponMacRegTable[i].addr = (__u32)e_tx_prmb_ctl;
	eponMacRegTable[i].def_value = 0x55555500;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tx_crc32_cfg;
	eponMacRegTable[i].def_value = 0x00000000;
	eponMacRegTable[i].rwmask = 0xffffffff;
	i++;
	eponMacRegTable[i].addr = (__u32)e_tx_prmb_ctl2;
	eponMacRegTable[i].def_value = 0xd5000000;
	eponMacRegTable[i].rwmask = 0xff000007;
	i++;
#endif

	/*add mac register before here!
	Also need increase the kmalloc size!! */
	
	eponMacRegTable[i].addr = 0;
	eponMacRegTable[i].rwmask = 0x0;

}

void eponMacTableExit(void)
{
	kfree(eponMacRegTable);
	eponMacRegTable = NULL;
}

int eponMacDumpAllReg(void){
	__u32 n = 0;
	__u32 Raw = 0;

	if(NULL == eponMacRegTable){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: dump epon mac register fail");
		return -1;
	}
	
	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "Dump EPON MAC REG");
		
	n = 0;
	while(eponMacRegTable[n].addr != 0){
		Raw = READ_REG_WORD(eponMacRegTable[n].addr);
		printk("\r\n %X : %X" , eponMacRegTable[n].addr, Raw);
		n++;
	}
	
//	printk("\ndump all llid threshold\n");
//	eponDumpAllLlidQueThrod();
	return 0;
}

int eponCmdInit(void)
{
	cmds_t eponCmd;
	int ret = -1;
	memset(&eponCmd , 0 , sizeof(cmds_t));

	dbgtoMem(EPON_DEBUG_LEVEL_TRACE, "enter eponCmdInit");

	
	/* init  root ci-cmd */ 
	eponCmd.name = "epon";
	eponCmd.func = doEpon;
	eponCmd.flags = 0x12;
	eponCmd.argcmin = 0;
	eponCmd.argc_errmsg = NULL;

	/* register  ci-cmd */
	ret = cmd_register(&eponCmd); 
	if(ret <0){
		dbgtoMem(EPON_DEBUG_LEVEL_ERR, "ERR: eponCmdInit ret=%d \n", ret);
	}
	return ret;
}

void eponCmdExit(void)
{
	cmd_unregister("epon");
}
