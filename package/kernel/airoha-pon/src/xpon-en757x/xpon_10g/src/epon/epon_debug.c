#include <linux/spinlock.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/netdevice.h>

#include "common/xpon_global.h"
#include "epon/epon_debug.h"
#include "epon/epon.h"
#include "epon/epon_msg_route.h"
#include "epon/epon_act.h"
#include "common/phy_if_wrapper.h"
#include "epon/epon_dev.h"
#include "epon/epon_msg_route.h"
#include "epon/epon_compile_option_wrapper.h"


/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
static int doEponSetLlidThrshldNum(int argc, char *argv[], void *p);
static int doEponSetLlidQueThrshld(int argc, char *argv[], void *p);
static int doEponRegCheck(int argc, char *argv[], void *p);
static int doEponWanReset(int argc, char *argv[], void *p);
static int doEponStatistic(int argc, char *argv[], void *p);
static int doEponQdmaIntEnableShow(int argc, char *argv[], void *p);
static int doEponState(int argc, char *argv[], void *p);
static int doEponSniffer(int argc, char *argv[], void *p);
static int doEponReg(int argc, char *argv[], void *p);
static int doEponLsrTm(int argc, char *argv[], void *p);
static int doEponStaticRpt(int argc, char *argv[], void *p);

void add_fix_reg_list(uint32_t addr, uint32_t value);
static void show_fix_reg_list(void);
void del_fix_reg_list(uint32_t addr);
static uint32_t strToHex(char *str);

static int doEpon(int argc, char *argv[], void *p);
static int doEponSet(int argc, char *argv[], void *p);
static int doEponMpcp(int argc, char *argv[], void *p);


int eponRegDefCheck(void);
int eponRegRWTest(uint32_t parttern);
int eponRegIndRWTest(uint32_t parttern);
static void xpon_debug_print_timeout(TIMER_FUN_PAAM arg);
static void doEponStatisticClean(void);

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
extern int is_hwnat_dont_clean;
extern uint16_t qdma_int_enable;

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
XPON_DEBUG_PRINT_DATA g_xpon_debug_print = {0};
XPON_DEBUG_PRINT_DATA *gp_xpon_debug_print = &g_xpon_debug_print;
unsigned long xpon_debug_print_flag;

char epon_debug_level;
uint32_t ignoreIntMask = 0xffffffff;
uint32_t debug_off = 0;

struct list_head fix_reg_list;

#define RANDOM_DLY_TEST_CNT_MAX		2000
uint32_t random_dly_test_cnt = 0;
uint32_t random_dly_val[RANDOM_DLY_TEST_CNT_MAX] = {0};

uint32_t tx_favor_oam_enable = 1;
uint32_t llid_rcv_rgst_int_sts_mark = 0;
uint8_t RgstrAckFlag[EPON_LLID_MAX_NUM] = {ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,
										   ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK,ACK};

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/
static struct proc_dir_entry *epon_proc_dir = NULL;
eponMacHwtestReg_t *eponMacRegTable = NULL;
#if LINUX_VERSION_CODE <= KERNEL_VERSION(3,18,19)
spinlock_t dbg_print_lock = SPIN_LOCK_UNLOCKED;
#else
spinlock_t dbg_print_lock = __SPIN_LOCK_UNLOCKED(dbg_print_lock);
#endif

char empty_char[XPON_DEBUG_PRINT_MAX_CHARS] = {0};
char temp[XPON_DEBUG_PRINT_MAX_CHARS] = {0};

char hw_llid_state[3][16]=
{
	"UN_REGISTERED",
    "REGISTERING",
    "REGISTERED",
};

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
/*______________________________________________________________________________
**  function name
**      strToHex
**  description:
**      string to hex
**  parameters:
**      None
**  global:
**      
**  return:
**      None
**  call:
**      None
**  revision:
**      v1.0
**____________________________________________________________________________*/
static uint32_t strToHex(char *str)
{
    uint32_t addr=0;
    uint32_t i=0;
    uint32_t tmp=0;
    uint32_t len=0;

    len = strlen(str);

    while(str[i]!='\0')
    {
        if(str[i]<='9'){
            tmp = str[i]-48;
        }else{
            tmp = (str[i]>'F')?(str[i]-87):(str[i]-55);
        }
        addr += tmp << (32-(i+1+8-len)*4);
        i++;
    }

    return addr;
}

/*______________________________________________________________________________
**	function name
**		epon_debug_print_exit
**	description:
**		delete timer that create in epon_debug_print_init
**	parameters:
**		None
**	global:
**		gp_xpon_debug_print
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_debug_print_exit(void)
{
	del_timer(&gp_xpon_debug_print->xpon_debug_output_timer);
    return;
}
/*______________________________________________________________________________
**	function name
**		epon_debug_print_init
**	description:
**		init epon debug print function
**	parameters:
**		None
**	global:
**		epon_debug_level
**		gp_xpon_debug_print
**		empty_char
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_debug_print_init(void)
{
	struct timer_list *output_timer = NULL;
    epon_debug_level = MSG_LEVEL_ERROR;

    memset(gp_xpon_debug_print,0,sizeof(XPON_DEBUG_PRINT_DATA));
    memset(empty_char,0,sizeof(empty_char));
    
    output_timer = &gp_xpon_debug_print->xpon_debug_output_timer;
	EPON_CREATE_TIMER(output_timer,xpon_debug_print_timeout,(jiffies + (HZ >> 1)));	
	add_timer(output_timer);

	gp_xpon_debug_print->pInput  = gp_xpon_debug_print->xpon_debug_total_mem;
	gp_xpon_debug_print->pOutput = gp_xpon_debug_print->xpon_debug_total_mem;
    return;
}

/*______________________________________________________________________________
**	function name
**		print_time_information
**	description:
**		print time
**	parameters:
**		time
**	global:
**		None
**	return:
**		None
**	call:
**		ktime_to_sec
**	revision:
**		v1.0
**____________________________________________________________________________*/

static inline void print_time_information(ktime_t * time)
{            
#if LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,36)
	u32 nsec = time->tv.nsec % 1000;
	u32 usec = (time->tv.nsec / 1000) % 1000;    
	u32 msec = (time->tv.nsec / 1000000) % 1000;   
	u32 sec = time->tv.sec % 60;
	u32 min = time->tv.sec / 60;
#else
    u32 nsec =(u32) ktime_to_ns(*time) % 1000;
    u32 usec =(u32) ktime_to_us(*time) % 1000;
    u32 msec =(u32) ktime_to_ms(*time) % 1000;
    u32 sec  =(u32) ktime_to_sec(*time) % 60;
    u32 min = (u32) ktime_to_sec(*time) / 60;
#endif
    printk("[%03dmin : %03ds : %03dms: %03dus : %03dns] ", min, sec, msec, usec, nsec);
    return;

}

/*______________________________________________________________________________
**	function name
**		xpon_debug_print_input
**	description:
**		input print debug to memory
**	parameters:
**		None
**	global:
**		gp_xpon_debug_print
**		empty_char
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

static void xpon_debug_print_input(void)
{
	if (gp_xpon_debug_print->pInput == (gp_xpon_debug_print->xpon_debug_total_mem + XPON_DEBUG_PRINT_MAX_LINE))
    {
		gp_xpon_debug_print->pInput = gp_xpon_debug_print->xpon_debug_total_mem;
	}
	
	if(strcmp(gp_xpon_debug_print->pInput->debug_info, empty_char) != 0){
		printk("Memory out .\n");
	}
	else
	{
		gp_xpon_debug_print->pInput->time_info = ktime_get();
		memcpy(gp_xpon_debug_print->pInput->debug_info, gp_xpon_debug_print->xpon_debug_temp, XPON_DEBUG_PRINT_MAX_CHARS);
		gp_xpon_debug_print->pInput++;
	}
    return;
}

/*______________________________________________________________________________
**	function name
**		xpon_debug_print_output
**	description:
**		output print debug from memory
**	parameters:
**		None
**	global:
**		gp_xpon_debug_print
**		empty_char
**          debug_off
**	return:
**		None
**	call:
**		print_time_information
**	revision:
**		v1.0
**____________________________________________________________________________*/

static void xpon_debug_print_output(void)
{
	int i;
	if(debug_off)
	{
		while(gp_xpon_debug_print->pOutput < (gp_xpon_debug_print->xpon_debug_total_mem + XPON_DEBUG_PRINT_MAX_LINE)){
						
			if(strcmp(gp_xpon_debug_print->pOutput->debug_info, empty_char) == 0){
				continue;
			}
				
			print_time_information(&(gp_xpon_debug_print->pOutput->time_info));
			printk("%s\n", gp_xpon_debug_print->pOutput->debug_info);
			mb();
			memset(gp_xpon_debug_print->pOutput, 0, sizeof(XPON_PRINT));
			gp_xpon_debug_print->pOutput++;
		} 
	}else 
	{
		for (i = 0; i < 60; i++)
		{
			if(gp_xpon_debug_print->pOutput >= (gp_xpon_debug_print->xpon_debug_total_mem + XPON_DEBUG_PRINT_MAX_LINE)){
				gp_xpon_debug_print->pOutput = gp_xpon_debug_print->xpon_debug_total_mem;
			}
			
			if(strcmp(gp_xpon_debug_print->pOutput->debug_info, empty_char) == 0){
				continue;
			}
			
			print_time_information(&(gp_xpon_debug_print->pOutput->time_info));
			printk("%s\n", gp_xpon_debug_print->pOutput->debug_info);
			mb();
			memset((char *)gp_xpon_debug_print->pOutput, 0, sizeof(XPON_PRINT));
			gp_xpon_debug_print->pOutput++;
		}
	}
    return;
}

/*______________________________________________________________________________
**	function name
**		xpon_debug_print_timeout
**	description:
**		start to print message from memory when timeout
**	parameters:
**		None
**	global:
**		gp_xpon_debug_print
**	return:
**		None
**	call:
**		xpon_debug_print_output
**	revision:
**		v1.0
**____________________________________________________________________________*/

static void xpon_debug_print_timeout(TIMER_FUN_PAAM arg)
{
	if(debug_off == 0){
		xpon_debug_print_output();
		mod_timer(&gp_xpon_debug_print->xpon_debug_output_timer, jiffies + (HZ >> 5));
	}
    return;
}

/*______________________________________________________________________________
**	function name
**		xpon_debug_print
**	description:
**		print to console
**	parameters:
**		fmt
**	global:
**		dbg_print_lock
**		xpon_debug_print_flag
**		gp_xpon_debug_print
**	return:
**		None
**	call:
**		spin_lock_irqsave
**		va_start
**		va_end
**		vsnprintf
**		xpon_debug_print_input
**	revision:
**		v1.0
**____________________________________________________________________________*/

void  xpon_debug_print(char *fmt,...)
{
    va_list wp;	
	spin_lock_irqsave(&dbg_print_lock, xpon_debug_print_flag);	
	
	memset(gp_xpon_debug_print->xpon_debug_temp, 0, XPON_DEBUG_PRINT_MAX_CHARS);
	va_start(wp, fmt);
	vsnprintf(gp_xpon_debug_print->xpon_debug_temp, XPON_DEBUG_PRINT_MAX_CHARS, fmt, wp);
	xpon_debug_print_input();
	va_end(wp);
	
	spin_unlock_irqrestore(&dbg_print_lock, xpon_debug_print_flag);
    return;
}


/*************************PUBLIC PRINT END******************************************/

/*______________________________________________________________________________
**	function name
**		show_all_mac_regs
**	description:
**		show reg value
**	parameters:
**		None
**	global:
**		eponMacRegTable
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/


void show_all_mac_regs(void)
{
	uint32_t n = 0;
	uint32_t Raw = 0;
	int i = 0;

	if(NULL == eponMacRegTable){
		DEBUG_MSG_EPON(MSG_LEVEL_DEBUG, "ERR: dump epon mac register fail");
		return;
	}
	
	printk("--------Dump EPON MAC REG begin---------");
		
	n = 0;
	while(eponMacRegTable[n].addr != 0){
		Raw = READ_REG_WORD(eponMacRegTable[n].addr);
		printk("\r\n %X : %X" , eponMacRegTable[n].addr, Raw);
		n++;
	}
	printk("\r\n BFB66138 : %X" , READ_REG_WORD(0xbfb66138));
	for(i = 0; i <= 8; i++)
		printk("\r\n %X : %X" , (0xbfb66200+(i*4)), READ_REG_WORD(0xbfb66200+(i*4)));
	for(i = 0; i <= 8; i++)
		printk("\r\n %X : %X" , (0xbfb66230+(i*4)), READ_REG_WORD(0xbfb66230+(i*4)));
	for(i = 0; i <= 20; i++)
		printk("\r\n %X : %X" , (0xbfb66334+(i*4)), READ_REG_WORD(0xbfb66334+(i*4)));
	printk("--------Dump EPON MAC REG end---------");
    
	return;
}



/*______________________________________________________________________________
**	function name
**		show_oam_counters
**	description:
**		show oam cnt
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void show_oam_counters(void)
{
	uint32_t raw1 = 0;
    raw1 = READ_REG_WORD(e_oam_stat);
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"epon OAM counters: TX : %d RX: %d",raw1&0xffff,(raw1>>16)&0xffff);
    return;
}
/*______________________________________________________________________________
**	function name
**		show_mpcp_tx_rx_counters
**	description:
**		show mpcp cnt
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void show_mpcp_tx_rx_counters(void)
{
	uint32_t raw1 = 0;
    uint32_t raw2 = 0;
    raw1 = READ_REG_WORD(e_mpcp_stat);
    raw2 = READ_REG_WORD(e_mpcp_rgst_stat);
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"epon TX mpcp counters: register_req: %d register Ack: %d",(raw2>>8)&0xff,raw2&0xff);
    DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"epon RX mpcp counters: discovry_gate: %d register: %d",raw1&0xffff,(raw1>>16)&0xff);
    return;
}

/*______________________________________________________________________________
**	function name
**		show_current_hw_disc_state
**	description:
**		show hw reg status
**	parameters:
**		llid_index
**	global:
**		hw_llid_state
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void show_current_hw_disc_state(__u8 llid_index)
{
	REG_e_llid0_dscvry_sts sdcvSts;
	if(llid_index < 32) 
		sdcvSts.Raw = READ_REG_WORD(g_epon_llid_dscv_stat[llid_index]);
	else {
		DEBUG_MSG_EPON(MSG_LEVEL_ERROR, "ERR: %s fail llidIndex=%d", __FUNCTION__, llid_index);
		return;
	}
    DEBUG_MSG_EPON(MSG_LEVEL_NOTIFY,"DEBUG:llid %d now discv reg is %x state is %s \n",
        llid_index ,sdcvSts.Raw,hw_llid_state[sdcvSts.Bits.llid0_dscvry_sts]); 
    return;
}

/*______________________________________________________________________________
**	function name
**		show_onu_mac_addr
**	description:
**		show onu mac
**	parameters:
**		addr
**	global:
**		None
**	return:
**		None
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

void show_onu_mac_addr(__u8 *addr)
{
	DEBUG_MSG_EPON(MSG_LEVEL_DEBUG,"onu mac addr is %x:%x:%x:%x:%x:%x\n",addr[0],addr[1],addr[2],addr[3],addr[4],addr[5]);
    return;
}
/*______________________________________________________________________________
**	function name
**		route_test_entry
**	description:
**		test for route entry
**	parameters:
**		data
**	global:
**		None
**	return:
**		0:success
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int route_test_entry(void *data)
{
	printk("func %s\n",__FUNCTION__);
    return EPON_SUCCESS;
}

static const cmds_t eponSetCmds[] = {
	{"llidThrshldNum",		doEponSetLlidThrshldNum,	0x02,  	1,  "<llidIndex(0~7)> <llidThrshldNum(1~3)>"},
	{"llidQueThrshld",		doEponSetLlidQueThrshld, 	0x02,	1,	"<llidIndex(0~7)> <thrshldIndex(0~2)> <q0 value> ... <q7 value>"},

	{NULL,			NULL,					0,	0,	NULL},
};

static const cmds_t eponMpcpCmds[] = {
    {"wanreset",    doEponWanReset,     0x02,   0,  NULL},

    {NULL,          NULL,               0x10,   0,  NULL},
};

static const cmds_t eponCmds[] = {
    /*Sub cmd*/
    {"set",             doEponSet,          0x12,   0,  NULL},
    {"mpcp",            doEponMpcp,         0x12,   0,  NULL},
    /*attribute*/
    {"reg_check",       doEponRegCheck,     0x12,   0,  NULL},
    
    {"statistic",       doEponStatistic,    0x12,   0,  "<NULL/clean>"},
    {"state",           doEponState,        0x12,   0,  NULL},
    {"sniffer",         doEponSniffer,      0x12,   0,  "<enable/disable> <othmpcp/otheth>"},
    {"register",        doEponReg,          0x12,   0,  "<set/show/del> <Addr> <Value>"},
    {"lasertime",       doEponLsrTm,        0x12,   0,  "<off-on(0~0x20)>"},
    {"staticrpt",       doEponStaticRpt,    0x12,   0,  "<enable/disable> <Value>"},
	 {"qdma_int_enable_show",    doEponQdmaIntEnableShow,    0x12,   0,  "<NULL/clean>"},
    {NULL,			    NULL,				0x10,	0,	NULL},
};

static int doEpon(int argc, char *argv[], void *p){	
	return subcmd(eponCmds, argc, argv, p);
}

static int doEponSet(int argc, char *argv[], void *p){	
	return subcmd(eponSetCmds, argc, argv, p);
}

static int doEponMpcp(int argc, char *argv[], void *p){
	return subcmd(eponMpcpCmds, argc, argv, p);
}

/*______________________________________________________________________________
**	function name
**		eponCmdInit
**	description:
**		init epon cmd 
**	parameters:
**		None
**	global:
**		None
**	return:
**		0:success
**	call:
**		cmd_register
**	revision:
**		v1.0
**____________________________________________________________________________*/

int eponCmdInit(void)
{
	cmds_t eponCmd;
	int ret = -1;
	memset(&eponCmd , 0 , sizeof(cmds_t));

	DEBUG_MSG_EPON(MSG_LEVEL_TRACE, "enter eponCmdInit");

	
	/* init  root ci-cmd */ 
	eponCmd.name = "epon";
	eponCmd.func = doEpon;
	eponCmd.flags = 0x12;
	eponCmd.argcmin = 0;
	eponCmd.argc_errmsg = NULL;

	/* register  ci-cmd */
	ret = cmd_register(&eponCmd); 
	if(ret <0){
		printk(">>>>>ERR: eponCmdInit ret=%d \n", ret);
		DEBUG_MSG_EPON(MSG_LEVEL_TRACE,"ERR: eponCmdInit ret=%d \n", ret);
	}
	return ret;
}
/*______________________________________________________________________________
**	function name
**		eponCmdExit
**	description:
**		deinit epon cmd 
**	parameters:
**		None
**	global:
**		None
**	return:
**		None
**	call:
**		cmd_unregister
**	revision:
**		v1.0
**____________________________________________________________________________*/

void eponCmdExit(void)
{
	cmd_unregister("epon");
}
/*______________________________________________________________________________
**	function name
**		doEponRegCheck
**	description:
**		check reg value 
**	parameters:
**		argc
**		argv[]
**		p
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		pon_mac_reset
**		eponRegDefCheck
**		eponRegRWTest
**		v1.0
**____________________________________________________________________________*/

static int 
doEponRegCheck(int argc, char *argv[], void *p)
{
	int epon_logic_state = EPON_LOGIC_RESET_HOLD_OFF;
	int ret = 0;
	if(argc <= 1){
		UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_PON_MAC_SCU_RESET,NULL,NULL);
		if(eponRegDefCheck() == -1){
			printk("XEPON Register default value verification is failure!!\r\n");
		}
		else{
			printk("XEPON Register default value verification is ok!!\r\n");
		}
	}else if(argc == 2){
		UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_PON_MAC_SCU_RESET,NULL,NULL);
		if(eponRegRWTest((unsigned long)simple_strtoul(argv[1], NULL, 16))==-1){
			printk("XEPON Register  Read/Write verification is failure!!\r\n");
		}
		else{
			printk("XEPON Register Read/Write verification is ok!!\r\n");
		}
		UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_PON_MAC_SCU_RESET,NULL,NULL);

		UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_MAC_LOGIC_RESET,&epon_logic_state,NULL);
		ret = eponRegIndRWTest((unsigned long)simple_strtoul(argv[1], NULL, 16));
		if(ret==-1){
			printk("XEPON Register  Indirect Read/Write verification is failure!!\r\n");
		}else if(ret==-2){
			printk("XEPON Register  Indirect Read/Write verification TABLE IS NOT EXIST!!\r\n");
		}
		else{
			printk("XEPON Register Indirect Read/Write verification is ok!!\r\n");
		}
	}
	else{
		printk("Usage:XEPON reg_check <partten>\n");
	}
	return 0;
}/*end doEponRegCheck*/

/*______________________________________________________________________________
**	function name
**		doEponWanReset
**	description:
**		Reset wan 
**	parameters:
**		argc
**		argv[]
**		p
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		v1.0
**____________________________________________________________________________*/

static int
doEponWanReset(int argc, char *argv[], void *p)
{
	epon_los_state(NULL);
	epon_stop(NULL);
	epon_reset(NULL);
	if(XPON_PHY_GET(PON_GET_PHY_READY_STATUS) == PHY_TRUE){
		gpPhyData->phy_link_status = PHY_LINK_STATUS_READY;
	}
	return epon_msg_route_dispatch(EPON_MSG_DETECT_READY,NULL);
}

#define DROP_CALC(a, b)         a>b?(a-b):0
/*______________________________________________________________________________
**	function name
**		doEponStatistic
**	description:
**		Statistic 
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
static int doEponStatistic(int argc, char *argv[], void *p)
{
    epon_debug_statistic_t tmpReg;
    memset(&tmpReg,0,sizeof(epon_debug_statistic_t));
    
    if(argv[1]==NULL)
    {
        UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DBG_GET_STATIS_CNT,NULL,&tmpReg);

        /*sw state*/
        printk("\n\033[1;34m-> SW State \033[0m\n");
        printk("llid0 sw mpcp state: %d\n", gp_epon_global_data->llid_entry[0].eponMpcp.mpcpDiscState);
        
        /*mpcp cnt*/
        printk("\n\033[1;34m-> Mpcp Counter \033[0m\n");
        
        printk("mpcpDiscGateCnt:\033[1;33m 0x%-8x \033[0m\t", tmpReg.mpcpDscvGateCnt);
        printk("mpcpRgstReqCnt:\033[1;33m 0x%-8x \033[0m\t", tmpReg.mpcpRgstReqCnt);
        printk("mpcpRgstCnt:\033[1;33m 0x%-8x \033[0m\t", tmpReg.mpcpRgstCnt);      
        printk("mpcpRgstAckCnt:\033[1;33m 0x%-8x \033[0m\n", tmpReg.mpcpRgstAckCnt);

        printk("rxMpiNrlGateCnt: 0x%-8x\t", tmpReg.rxMpiNrlGateCnt);
        printk("rxMpiGateCnt: 0x%-8x\t", tmpReg.rxMpiGateCnt);
        printk("txMpiRgstReqCnt: 0x%-8x\t", tmpReg.txMpiRgstReqCnt);
        printk("txMpiRgstAckCnt: 0x%-8x\n", tmpReg.txMpiRgstAckCnt);
               
        printk("txMbiRptCnt: 0x%-8x\t\t", tmpReg.txMbiRptCnt);
        printk("txMpiRptCnt: 0x%-8x\t\t", tmpReg.txMpiRptCnt);
        printk("rxMpiMpcpCnt: 0x%-8x\t", tmpReg.rxMpiMpcpCnt);
        printk("mpcpErrCnt:\033[1;31m 0x%-8x \033[0m\n", tmpReg.mpcpErrCnt);

        /*oam cnt*/
        printk("\n\033[1;34m-> Oam Counter \033[0m\n");
        
        printk("rxMpiOamCnt:\033[1;33m 0x%-8x \033[0m\t", tmpReg.rxMpiOamCnt);
        printk("txMpiOamCnt:\033[1;33m 0x%-8x \033[0m\t", tmpReg.txMpiOamCnt);
        printk("rxOamCnt:\033[1;33m 0x%-8x \033[0m\t\t", tmpReg.rxOamCnt);
        printk("txOamCnt:\033[1;33m 0x%-8x \033[0m\n", tmpReg.txOamCnt);

        /*ethernet cnt*/
        printk("\n\033[1;34m-> Ethernet Counter \033[0m\n");
        
        printk("rxMpiEth: 0x%-8x\t\t", tmpReg.rxMpiEth);
        printk("txMpiEth: 0x%-8x\t\t", tmpReg.txMpiEth);
        printk("rxMbiEth: 0x%-8x\t\t", tmpReg.rxMbiEth);
        printk("txMbiEth: 0x%-8x\n", tmpReg.txMbiEth);

        printk("rxMpiUcEth: 0x%-8x\t\t", tmpReg.rxMpiUcEthCnt);
        printk("rxMpiBcEth: 0x%-8x\t\t", tmpReg.rxMpiBcEthCnt);
        printk("rxMpiMcEth: 0x%-8x\n", tmpReg.rxMpiMcEthCnt);
        
        printk("txMpiUcEth: 0x%-8x\t\t", tmpReg.txMpiUcEthCnt);
        printk("txMpiBcEthCnt: 0x%-8x\t", tmpReg.txMpiBcEthCnt);
        printk("txMpiMcEthCnt: 0x%-8x\n", tmpReg.txMpiMcEthCnt);

        printk("txMbiUcEth: 0x%-8x\t\t", tmpReg.txMbiUcEthCnt);
        printk("txMbiBcEthCnt: 0x%-8x\t", tmpReg.txMbiBcEthCnt);
        printk("txMbiMcEthCnt: 0x%-8x\n", tmpReg.txMbiMcEthCnt);
        
        /*error/drop cnt*/
        printk("\n\033[1;34m-> Error/Drop Counter \033[0m\n");

        printk("rxMpiEofDropCnt:\033[1;31m 0x%-8x \033[0m\t", tmpReg.rxMpiEofDropCnt);
        printk("rxMpiFifoOvRunCnt(AN7581):\033[1;31m 0x%-8x \033[0m\t", tmpReg.rxMpiFifoOvRunCnt);

        printk("rxMbiSofDropCnt:\033[1;31m 0x%-8x \033[0m\n", tmpReg.rxMbiSofDropCnt);
        
        printk("rxMpiCrc32ErrCnt(AN7581):\033[1;31m 0x%-8x \033[0m\t", tmpReg.rxMpiCrc32ErrCnt);
        printk("rxMpiCrc8ErrCnt:\033[1;31m 0x%-8x \033[0m\t", tmpReg.rxMpiCrc8ErrCnt);
        printk("txMbiErrCnt:\033[1;31m 0x%-8x \033[0m\n", tmpReg.txMbiErrCnt);

        /*Churning cnt*/
        printk("\n\033[1;34m-> Triple Churning Counter \033[0m\n");
        
        printk("rxMpiChurnOkCnt(AN7581): 0x%-8x\t", tmpReg.rxMpiChurnOkCnt);
        printk("rxMpiChurnErrCnt(AN7581):\033[1;31m 0x%-8x \033[0m\n", tmpReg.rxMpiChurnErrCnt);

        /*sniffer cnt*/
        printk("\n\033[1;34m-> Sniffer Counter \033[0m\n");
        
        printk("rxMbiSnfCnt: 0x%-8x\t\t", tmpReg.rxMbiSnfCnt);
        printk("rxMbiSnfDropCnt:\033[1;31m 0x%-8x \033[0m\n", tmpReg.rxMbiSnfDropCnt);
        
    }else if(!strcmp(argv[1],"clean")){
        doEponStatisticClean();
    }else if(!strcmp(argv[1],"graph")){
        UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DBG_GET_STATIS_CNT,NULL,&tmpReg);
        
        printk("*** EPON MAC *****************************************************************\n");        
        printk("* MBI                                    \033[1;34m Report      Oam        Eth \033[0m        *\n");
        printk("*                    0x%-8x 0x%-8x 0x%-8x 0x%-8x 0x%-8x  *\n", tmpReg.rxOamCnt, tmpReg.rxMbiEth, tmpReg.txMbiRptCnt, tmpReg.txOamCnt, tmpReg.txMbiEth);
        printk("*                    %-8d   %-8d   %-8d   %-8d   %-8d    *\n", tmpReg.rxOamCnt, tmpReg.rxMbiEth, tmpReg.txMbiRptCnt, tmpReg.txOamCnt, tmpReg.txMbiEth);
        printk("*          ^          ^          ^          |          |          |          *\n");
        printk("*          |          |          |          |          |          |          *\n");
        printk("*          |          |          |          v          v          v          *\n");
        printk("* Drop \033[1;31m              0x%-8x 0x%-8x 0x%-8x 0x%-8x 0x%-8x  \033[0m*\n", DROP_CALC(tmpReg.rxMpiOamCnt, tmpReg.rxOamCnt), DROP_CALC(tmpReg.rxMpiEth, tmpReg.rxMbiEth), \
                                                                                                DROP_CALC(tmpReg.txMbiRptCnt, tmpReg.txMpiRptCnt), DROP_CALC(tmpReg.txOamCnt, tmpReg.txMpiOamCnt), DROP_CALC(tmpReg.txMbiEth, tmpReg.txMpiEth));
        printk("*\033[1;31m                    %-8d   %-8d   %-8d   %-8d   %-8d    \033[0m*\n", DROP_CALC(tmpReg.rxMpiOamCnt, tmpReg.rxOamCnt), DROP_CALC(tmpReg.rxMpiEth, tmpReg.rxMbiEth), \
                                                                                                DROP_CALC(tmpReg.txMbiRptCnt, tmpReg.txMpiRptCnt), DROP_CALC(tmpReg.txOamCnt, tmpReg.txMpiOamCnt), DROP_CALC(tmpReg.txMbiEth, tmpReg.txMpiEth));
        printk("*          ^          ^          ^          |          |          |          *\n");
        printk("*          |          |          |          |          |          |          *\n");
        printk("*          |          |          |          v          v          v          *\n");
        printk("*         0x%-8x 0x%-8x 0x%-8x 0x%-8x 0x%-8x 0x%-8x  *\n", tmpReg.rxMpiNrlGateCnt, tmpReg.rxMpiOamCnt, tmpReg.rxMpiEth, tmpReg.txMpiRptCnt, tmpReg.txMpiOamCnt, tmpReg.txMpiEth);
        printk("*         %-8d   %-8d   %-8d   %-8d   %-8d   %-8d    *\n", tmpReg.rxMpiNrlGateCnt, tmpReg.rxMpiOamCnt, tmpReg.rxMpiEth, tmpReg.txMpiRptCnt, tmpReg.txMpiOamCnt, tmpReg.txMpiEth);
        printk("* MPI  \033[1;34m Nrl_Gate     Oam        Eth \033[0m                                         *\n");
        printk("******************************************************************************\n");
    }else{
        printk("Cmd error.\n");
    }
    return 0;
}
static int doEponQdmaIntEnableShow(int argc, char *argv[], void *p)
{
	if(argv[1]==NULL)
    {
		printk("QDMA INT ENABLE FLAG = %d\n", qdma_int_enable);
		
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
    epon_dbg_state_t tmpReg;
    memset(&tmpReg,0,sizeof(epon_dbg_state_t));

	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DBG_GET_STATE,NULL,&tmpReg);

    /*globle config*/
    printk("\t--- globle config ---\n");
    printk("glbCfg: 0x%x\t\t", tmpReg.glbCfg);
    printk("glbCfg: 0x%x\n", tmpReg.glbCfg2);
    printk("glbSts1(AN7581): 0x%x\t", tmpReg.glbSts1);
    printk("glbSts2(AN7581): 0x%x\n", tmpReg.glbSts2);
    printk("rdmDlyCfg(AN7581): 0x%x\t\t", tmpReg.rdmDlyCfg);
    printk("rdmDlySts: 0x%x\n", tmpReg.rdmDlySts);
    printk("txCnst: 0x%x\t\t", tmpReg.txCnst);
    printk("pwrSvCfg: 0x%x\n", tmpReg.pwrSvCfg);
    printk("dygspCfg: 0x%x\n", tmpReg.dygspCfg);

    /*interrupt status*/
    printk("\t--- interrupt status ---\n");
    printk("intSts: 0x%x\t\t", tmpReg.intSts);
    printk("intEn: 0x%x\n", tmpReg.intEn);
    printk("intSts2: 0x%x\t\t", tmpReg.intSts2);
    printk("intEn2: 0x%x\n", tmpReg.intEn2);
    printk("intSts3: 0x%x\t\t", tmpReg.intSts3);
    printk("intEn3: 0x%x\n", tmpReg.intEn3);

    /*llid0 status*/
    printk("\t--- llid0 status ---\n");
    printk("e_llid0_3_cfg: 0x%x\t\t", tmpReg.llidCfg0_3);
    printk("rptCfg: 0x%x\n", tmpReg.rptCfg);
    printk("laserTm: 0x%x\t\t", tmpReg.laserTm);
    printk("syncTm: 0x%x\n", tmpReg.syncTm);
    printk("rptBmp(AN7581): 0x%x\t\t", tmpReg.rptBmp);
    printk("oltOvTm(AN7581): 0x%x\n", tmpReg.oltOvTm);

    /*security status*/
    printk("\t--- security status(AN7581) ---\n");
    printk("dsKeyChg: 0x%x\t\t", tmpReg.dsKeyChg);
    printk("usKeyChg: 0x%x\n", tmpReg.usKeyChg);
    printk("ensecCfg: 0x%x\t\t", tmpReg.ensecCfg);
    printk("desecCfg: 0x%x\n", tmpReg.desecCfg);
    printk("crptCfg: 0x%x\n", tmpReg.crptCfg);
    return 0;
}

/*______________________________________________________________________________
**	function name
**		doEponStatisticClean
**	description:
**		clean count 
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
static void doEponStatisticClean(void)
{
    WRITE_REG_WORD(e_cnt_clr, 1);
    printk("Epon MAC statistic counter clean.\n");
}

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
	EPON_SNIFFER_MODE_T sniffer_mode = 0;
    if (argc < 2){
		printk("Usage: <enable/disable> <NULL/othmpcp/otheth>\n");
		return -1;
	}
	
    if(!strcmp(argv[1],"enable"))
    {
		if(argv[2] == NULL){
            sniffer_mode = EPON_SNIFFER_ENABLE_SELF_ALL;
		}
		else if(!strcmp(argv[2],"othmpcp")){
            sniffer_mode = EPON_SNIFFER_ENABLE_OTHER_MPCP;
        }
        else if(!strcmp(argv[2],"otheth")){
			sniffer_mode = EPON_SNIFFER_ENABLE_OTHER_ETHERNET;
        }
		else{
			printk("unknow EPON sniffer parameter.\n");
			return -1;
		}
    }
    else if(!strcmp(argv[1],"disable"))
    {
		sniffer_mode = EPON_SNIFFER_DISABLE_ALL;		
    }
    else{
        printk("Cmd error.\n");
		return -1;
    }

	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DBG_SET_SNIFFER,&sniffer_mode,NULL);
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
    uint32_t addr=0, val=0;

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
        addr = strToHex(argv[2]);
        if(argc < 4){
            printk("Usage: <value>\n");
            return -1;
        }
        val = strToHex(argv[3]);
        add_fix_reg_list(addr, val);
    }else if(!strcmp(argv[1],"show")){
        show_fix_reg_list();
    }else if(!strcmp(argv[1],"del")){
    	addr = strToHex(argv[2]);
        del_fix_reg_list(addr);
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
    uint32_t tmp = 0;

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
        tmp = strToHex(argv[2]);
        gp_epon_global_data->laser_on = tmp&0xff;
        gp_epon_global_data->laser_off = (tmp&0xff00)>>8;
        WRITE_REG_WORD(e_laser_onoff_time, tmp);
        add_fix_reg_list((uint32_t)e_laser_onoff_time, tmp);
        printk("laser on 0x%x, laser off 0x%x\n",gp_epon_global_data->laser_on, gp_epon_global_data->laser_off);
    }else if(!strcmp(argv[1],"get")){
        printk("laser on 0x%x, laser off 0x%x\n",gp_epon_global_data->laser_on, gp_epon_global_data->laser_off);
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
static int doEponStaticRpt(int argc, char *argv[], void *p)
{
    static_report_info_t static_rpt = {0};
    
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
		static_rpt.enable = TRUE;
        static_rpt.report_val= strToHex(argv[2]);
        
    }else if(!strcmp(argv[1],"disable")){
        static_rpt.enable = FALSE;
    }else{
        printk("Cmd error.\n");
		return -1;
    }
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DBG_SET_STATIC_RPT,&static_rpt,NULL);
	return 0;
}

/*______________________________________________________________________________
**	function name
**		eponRegDefCheck
**	description:
**		check reg default valuet 
**	parameters:
**		None
**	global:
**		epon_reg
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int
eponRegDefCheck(void)
{
	int i=0;
	uint32_t reg_addr=0;
	int retval=0;

	if(epon_reg == NULL)
		return -1;

	for(i=0; epon_reg[i].name!=NULL; i++){
		reg_addr = epon_reg[i].addr;
		if(epon_reg[i].type & NO_DEF){
			/*If the register is no default value, we skip this register.*/
			continue;
		}else {
    		if(READ_REG_WORD(reg_addr) != epon_reg[i].def_value){
    			printk("Error(i:%d):%s is error, Default: 0x%08x Real:0x%08x\n", i, epon_reg[i].name, epon_reg[i].def_value, READ_REG_WORD(reg_addr));
    			retval=-1;
    		}
        }
	}
	return retval;
}/*end eponRegDefCheck*/
/*______________________________________________________________________________
**	function name
**		eponRegRWTest
**	description:
**		check reg R/W valuet 
**	parameters:
**		pattern
**	global:
**		epon_reg
**	return:
**		0:success
**		-1:fail
**	call:
**		None
**	revision:
**		v1.0
**____________________________________________________________________________*/

int
eponRegRWTest(uint32_t pattern)
{
	int i=0;
	uint32_t befVal=0;
	uint32_t reg_addr=0x0;
	int retval=0;
	int err=0;
	long unsigned int val1 =0;
	long unsigned int val2 =0;

	if(epon_reg == NULL)
		return -1;
	
	for(i=0; epon_reg[i].name!=NULL; i++){
			err=0;
			reg_addr= epon_reg[i].addr;
			befVal= (uint32_t)READ_REG_WORD(reg_addr);

            if(epon_reg[i].type & W1C){
                printk("addr: W1C REG: 0x%x\n",epon_reg[i].addr);
                continue;
            }else if((epon_reg[i].type & RO) && (epon_reg[i].type & RW)){
				WRITE_REG_WORD(reg_addr,pattern);
                if((READ_REG_WORD(reg_addr) & epon_reg[i].mask) != (pattern & epon_reg[i].mask)){
                    val1 = (READ_REG_WORD(reg_addr) & epon_reg[i].mask);
                    val2 = (pattern & epon_reg[i].mask);
                    printk(" RW ERROR REG:0x%x value1: 0x%lx value2: 0x%lx\n",epon_reg[i].addr,val1,val2);
					retval=-1;
					err=1;
				}else if((READ_REG_WORD(reg_addr) & (~epon_reg[i].mask))!=(befVal & (~epon_reg[i].mask))){
				        val1 = (READ_REG_WORD(reg_addr) & (~epon_reg[i].mask));
                        val2 = (befVal & (~epon_reg[i].mask));
				        printk(" RO ERROR REG:0x%x value1: 0x%lx value2: 0x%lx\n",epon_reg[i].addr,val1,val2);
                        retval=-1;
        				err=1;
                }
            }else if(epon_reg[i].type & RO){
					WRITE_REG_WORD(reg_addr,~(READ_REG_WORD(reg_addr)));
        			if(READ_REG_WORD(reg_addr) != befVal){
        				retval=-1;
        				err=1;
        			}
			}
			else if(epon_reg[i].type & WO){
				WRITE_REG_WORD(reg_addr,pattern);
				if(READ_REG_WORD(reg_addr)!=0x00000000){
					retval=-1;
					err=1;
				}
			}
			else if(epon_reg[i].type & RW){
				WRITE_REG_WORD(reg_addr,pattern);
				if((READ_REG_WORD(reg_addr) & epon_reg[i].mask) != (pattern & epon_reg[i].mask)){
					retval=-1;
					err=1;
				}
			} else {
			    printk("Error: no match TYPE!!\n");
             }
			
			if(err==1){
			    val1 = (pattern & epon_reg[i].mask);
			    val2 = befVal;
				printk("Error: %s is error, Pattern: 0x%08lx before:0x%08lx After:0x%08x\n", \
					epon_reg[i].name, val1, val2, (uint32_t) READ_REG_WORD(reg_addr));	
			}
	}
	return retval;
}/*end eponRegRWTest*/

int eponRegIndRWTest(uint32_t pattern)
{
	int i = 0;
	int retval = 0;

	if(indirect_reg == NULL)
		return -2;
	for(i=0; indirect_reg[i].name != NULL; i++){
		if(indirect_reg[i].func != NULL){
			if(indirect_reg[i].func(pattern) != 0){				
				printk("Error:[%s][%d] Reg(%s) indirect check Fail!!\n",__FUNCTION__,__LINE__,indirect_reg[i].name);
				retval = -1;
			}
		}else {
			printk("[%s][%d]: Reg(%s) without indirect check funtion\n",__FUNCTION__,__LINE__,indirect_reg[i].name);
		}
	}
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_PON_MAC_SCU_RESET,NULL,NULL);
	return retval;
}

/*______________________________________________________________________________
**	function name
**		doEponSetLlidThrshldNum
**	description:
**		set report threshold number  
**	parameters:
**		argc
**		argv[]
**		p
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		simple_strtoul
**		eponSetLlidThrshldNum
**	revision:
**		v1.0
**____________________________________________________________________________*/

static int doEponSetLlidThrshldNum(int argc, char *argv[], void *p)
{
	eponMacIoctl_t opt3;
	__u8 llidIndex = 0;
	__u8 num = 0;
	
	if (argc < 3)
	{
		printk("Usage: llidThrshldNum <llidIndex(0~7)> <llidThrshldNum(1~3)>\n");
		return -1;
	}
	
	llidIndex = (__u8)simple_strtoul(argv[1], NULL, 10);
	num = (__u8)simple_strtoul(argv[2], NULL, 10);

	memset(&opt3, 0, sizeof(eponMacIoctl_t));
	opt3.llidIndex = llidIndex;
	opt3.param0 = num;

	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_SET_LLID_THRSHLD_NUM,&opt3,NULL);

	return 0;
}
/*______________________________________________________________________________
**	function name
**		doEponSetLlidThrshldNum
**	description:
**		set report threshold value  
**	parameters:
**		argc
**		argv[]
**		p
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		simple_strtoul
**		epon_set_queue_threshold
**	revision:
**		v1.0
**____________________________________________________________________________*/

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



/*======================================================================*/
#ifdef TCSUPPORT_EPON_POWERSAVING
void doEponEarlyWakeup(uint action1, uint action2, uint action3)
{
	printk("earlywakeup start %d %d %d\n", action1, action2, action3);
	gpEponPriv->eponCfg.earlyWakeupFlag = action1;
	gpEponPriv->eponCfg.earlyWakeupTimer = action2;
	gpEponPriv->eponCfg.earlyWakeupCount = action3;
}
#endif

void doEponIgnoreIntMask(uint action1, uint action2, uint action3)
{
	ignoreIntMask = action1;
    printk("\r\n ignoreIntMask =%x\n",ignoreIntMask);
}

void doEponDiscrgstAck(uint action1, uint action2, uint action3)
{
	printk("\r\n llid = %x , ack = %x\n",action1, action2);
	if(action1>31 || action2>1)
		return;
	RgstrAckFlag[action1] = action2;
}


void doEponderegister(uint action1, uint action2, uint action3)
{
	printk("\r\n llid = %d \n",action1);
	epon_mpcp_local_deregister(action1);
}

void doEponLlid_Mask(uint action1, uint action2, uint action3)
{
	printk("\r\n llid_mask = %x \n",action1);
	eponMacSetLlidEnableMask(action1);
	gp_epon_global_data->m_llid_flag = action2;
}

void doEponDbgToDram(uint action1, uint action2, uint action3)
{
	printk("\r\n set dbgToDram = %d \n",action1);
	xpon_debug_print_output();
}

void doEponDbgTmOff(uint action1, uint action2, uint action3)
{
	debug_off = action1;
	printk("\r\n set debug_off = %d \n",debug_off);
	if(debug_off == 0)
		xpon_debug_print_timeout(0);
}

void doEponShowTime(uint action1, uint action2, uint action3)
{
    __u8 local_time[MAX_CHAR_NUM] = {0};
	printk("\r\n ONU registered time : %s\n", gp_epon_global_data->register_time);
	printk("\r\n ONU last registered time : %s\n", gp_epon_global_data->last_register_time);
	printk("\r\n ONU deregistered time : %s\n", gp_epon_global_data->lost_time);
    epon_time_record(local_time);
    printk("\r\n ONU local_time : %s\n", local_time);
}

void doEponLostTime(uint action1, uint action2, uint action3)
{
	printk("\r\n ONU lost count : %d times\n", gp_epon_global_data->lost_count);
}

void doEponHwnatNoclean(uint action1, uint action2, uint action3)
{
	is_hwnat_dont_clean = action1;
    printk("\r\n no clean hw_nat done no_clean_action: %d\n", is_hwnat_dont_clean);
}

void doEponMacOam(uint action1, uint action2, uint action3)
{
    action1 = (uint32_t)READ_REG_WORD(e_oam_stat);
    action2 = (action1 & 0xffff);
    action3 = ((action1 & 0xffff0000) >> 16);
    printk("\r\n oam tx: %d   oam rx: %d\n", action2, action3);
}

void doEponEth(uint action1, uint action2, uint action3)
{
    uint mac_rxmbi_temp = (uint32_t)READ_REG_WORD(e_rxmbi_eth_cnt);
    uint mac_rxmpi_temp = (uint32_t)READ_REG_WORD(e_rxmpi_eth_cnt);
    uint mac_txmbi_temp = (uint32_t)READ_REG_WORD(e_txmbi_eth_cnt);
    uint mac_txmpi_temp = (uint32_t)READ_REG_WORD(e_txmpi_eth_cnt);

    printk("\r\n eth mbi_rx: %d   eth mpi_rx: %d   eth mbi_tx: %d   eth mpi_tx: %d\n", mac_rxmbi_temp, mac_rxmpi_temp,\
        mac_txmbi_temp, mac_txmpi_temp);
}

void doEponStaticrpt(uint action1, uint action2, uint action3)
{
	static_report_info_t static_rpt = {0};
	static_rpt.enable = !!action1;
	static_rpt.report_val = action2;
    if (gpPonSysData->sysLinkStatus == PON_LINK_STATUS_EPON){
        UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DBG_SET_STATIC_RPT,&static_rpt,NULL);
    }
}

void doEponDying(uint action1, uint action2, uint action3)
{
    if(action1 == 0) {
        gpPonSysData->dyingGaspData.isEponHwFlag = 0 ;  
        xmit_dying_gasp_oam();
        //eponHwDygaspCtrl(XPON_DISABLE);
        printk("The dying gasp message will be send by software\n") ;       
    } else if (action1 == 1) {
        gpPonSysData->dyingGaspData.isEponHwFlag = 1 ;  
        //eponHwDygaspCtrl(XPON_ENABLE);  
        printk("The dying gasp message will be send by hardware\n") ;   
    }
}
void doEponLaserTime(uint action1, uint action2, uint action3)
{
	gp_epon_global_data->laser_on = (action1&0xff);
    gp_epon_global_data->laser_off = (action1&0xff00)>>8;
	printk("epon laser time del laser on%x, laser off%x\n",gp_epon_global_data->laser_on,gp_epon_global_data->laser_off);
}
void doEponSetllid(uint action1, uint action2, uint action3)
{
	printk("epon set llid %d traffic up\n",action1);
	xmcs_set_epon_llid_config(action1);
}
void doEponSlientTime(uint action1, uint action2, uint action3)
{
	gp_epon_global_data->silent_time_config = action1;
	printk("epon set silent time %d\n",gp_epon_global_data->silent_time_config);
}
void doEponSetMacMode(uint action1, uint action2, uint action3)
{
	gp_epon_global_data->mac_mode = action1;
	printk("epon set continue mode %d\n",gp_epon_global_data->mac_mode);
}
void doEponMacAddr(uint action1, uint action2, uint action3)
{
    gp_epon_global_data->onu_mac_addr[0] = (action1 >> 8) & 0xff;
    gp_epon_global_data->onu_mac_addr[1] = (action1) & 0xff;
    gp_epon_global_data->onu_mac_addr[2] = (action2 >> 24) & 0xff;
    gp_epon_global_data->onu_mac_addr[3] = (action2 >> 16) & 0xff;
    gp_epon_global_data->onu_mac_addr[4] = (action2 >> 8) & 0xff;
    gp_epon_global_data->onu_mac_addr[5] = (action2) & 0xff;
    printk("mannual set onu mac address, global ");
    show_onu_mac_addr(gp_epon_global_data->onu_mac_addr);
}
void doEponShowMac(uint action1, uint action2, uint action3)
{
	pEPON_LLID_INFO_T llid_info = NULL;
	EPON_GET_LLID_BY_INDEX(llid_info,action1);
    epon_get_mac_address(llid_info);
}
void doEponTxCheckType(uint action1, uint action2, uint action3)
{
	gp_epon_global_data->mpcp_send_check_type = action1;
}
void doEponDumpReg(uint action1, uint action2, uint action3)
{
	show_all_mac_regs();
}
void doEponPhyReady(uint action1, uint action2, uint action3)
{
	printk("epon set phy %s\n",action1?"Enable":"Disable");
	if(action1 == 0)
    {
    	epon_msg_route_dispatch(EPON_MSG_TRUE_LOS,NULL);
    }
    else if(action1 == 1)
    {
    	gpPonSysData->sysStartup = PON_WAN_START;
        //epon_start(0);
    	epon_msg_route_dispatch(EPON_MSG_DETECT_READY,NULL);
    }
}
void doEponSetMode(uint action1, uint action2, uint action3)
{
	eponSetRateMode(action1);
}
void doEponGetMode(uint action1, uint action2, uint action3)
{
	eponMacIoctl_t opt3;
	memset(&opt3, 0, sizeof(eponMacIoctl_t));
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_GET_10G_LLID_KEY,NULL,&opt3);
}
void doEponDebug(uint action1, uint action2, uint action3)
{
	epon_debug_level = action1;
    printk("EPON set debug %d\n",action1);
}
void doEponBandUtilizationEnable(uint action1, uint action2, uint action3)
{
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DBG_SET_BAND_UTILIZATION,&action1,NULL);
}

void epon_Tx_Rate_expires(TIMER_FUN_PAAM data)
{
	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_EPON_DBG_SET_TX_RATE_ANALYZE,NULL,NULL);
    return;
}

void doEponTxRate(uint action1, uint action2, uint action3)
{
    printk("EPON setdoEponTxRate mode %d\n",action1);
	EPON_START_TIMER(eponTxRate,4000);
	gp_epon_global_data->tx_rate_mode= action1 ;
}

void doEponDpoeKey(uint action1, uint action2, uint action3)
{
	epon_event_report(XMCS_EVENT_EPON_dpoe_key,action1);
    printk("EPON set doEponDpoeKey llid index %d\n",action1);
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
void add_fix_reg_list(uint32_t addr, uint32_t value)
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
    if(new_entry == NULL){		
		printk("return null \n");	  
        return;
	}
        
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
void del_fix_reg_list(uint32_t addr)
{
    FIX_REG_LIST_ENTRY_T *entry = NULL;
    FIX_REG_LIST_ENTRY_T *tmp = NULL;

    rcu_read_lock();
    list_for_each_entry_rcu(entry, &fix_reg_list, list)
    {
		if(entry->regInfo.addr == addr)
		{
	        tmp = list_entry(&entry->rcu, FIX_REG_LIST_ENTRY_T, rcu);
	        printk("del 0x%x: 0x%x\n", entry->regInfo.addr, entry->regInfo.value);
	        list_del_rcu(&entry->list);
	        kfree(tmp);
	        break;
		}
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

void doEponRegSet(uint action1, uint action2, uint action3)
{
    add_fix_reg_list(action1, action2);
}
void doEponRegShow(uint action1, uint action2, uint action3)
{
    show_fix_reg_list();
}
void doEponRegDel(uint action1, uint action2, uint action3)
{
    del_fix_reg_list(action1);
}
void dump_random_dly_test_result(int clear_flag)
{
	int i = 0;

	for(i = 0; i < random_dly_test_cnt; i++)
		printk("%d\n", random_dly_val[i]);
	if(clear_flag)
	{
		random_dly_test_cnt = 0;
		memset(random_dly_val,0, sizeof(random_dly_val));
	}
}
void doRandom_delay_show(uint action1, uint action2, uint action3)
{
	dump_random_dly_test_result(0);
}

void doRandom_delay_test(uint action1, uint action2, uint action3)
{
	if(action1 == 0)
		dump_random_dly_test_result(1);
	random_dly_test_cnt = action1>RANDOM_DLY_TEST_CNT_MAX?RANDOM_DLY_TEST_CNT_MAX:action1;
	printk("set random delay test times=%d\n",random_dly_test_cnt);
}

void record_random_dly_test(void)
{
	static int cnt = 0;
	static int record_flag = 1;
	uint32_t random_dly = 0;

	if(!record_flag)
		return;
	if(cnt < random_dly_test_cnt)
	{
		random_dly = READ_REG_WORD(e_rdmdly_stat);
		printk("read random delay=%x, times=%d\n",random_dly, cnt);
		random_dly_val[cnt++] = random_dly&0xFFFF;
	}
	else if(cnt >= random_dly_test_cnt)
	{
		cnt = 0;
		record_flag = 0;
		dump_random_dly_test_result(1);
		record_flag = 1;
	}
}

void doTx_favor_oam(uint action1, uint action2, uint action3)
{
	REG_e_glb_cfg2 cfg2;

	tx_favor_oam_enable = action1;
	printk("tx favor oam [%s]\n", tx_favor_oam_enable?"ENABLE":"DISABLE");
	if(tx_favor_oam_enable)
	{
		del_fix_reg_list((uint32_t)e_glb_cfg2);
	}
	else
	{
		cfg2.Raw = READ_REG_WORD(e_glb_cfg2);
		cfg2.Bits.llidnum_sel = 0;
		WRITE_REG_WORD(e_glb_cfg2, cfg2.Raw);
		add_fix_reg_list((uint32_t)e_glb_cfg2, cfg2.Raw);
		QDMA_API_SET_OAM_MODIFY_FP_EN(ECNT_QDMA_WAN, QDMA_DISABLE);
	}
}

void doLlid_rcv_rgst_sts(uint action1, uint action2, uint action3)
{
	uint32_t llidindex = action1;
	uint32_t status = action2;

	if(status == 1)
	{
		llid_rcv_rgst_int_sts_mark = 0;
	}
	else
	{
		if(llidindex >= 32)
			llid_rcv_rgst_int_sts_mark=0xFFFFFFFF;
		else
			llid_rcv_rgst_int_sts_mark = 1 << llidindex;
	}
	printk("llid_rcv_rgst_sts mark bit = %#08x\n", llid_rcv_rgst_int_sts_mark);
}

/*======================================================================*/

static epon_cmds_t epondebugCmds[] = {
 //   {"reg_check",	doEponRegCheck,		0x12,  	0,  NULL},

    {"reg_set",   doEponRegSet},
    {"reg_show",   doEponRegShow},
    {"reg_del",   doEponRegDel},
#ifdef TCSUPPORT_EPON_POWERSAVING    
    {"earlywakeup",    doEponEarlyWakeup},
#endif
    {"ignoreIntMask",    doEponIgnoreIntMask},
    {"discrgstack",    doEponDiscrgstAck},
    {"deregister",    doEponderegister},
    {"llid_mask",    doEponLlid_Mask},
    {"dbgToDram",    doEponDbgToDram},
    {"dbgTmOff",    doEponDbgTmOff},
    {"show_time",    doEponShowTime},
    {"lost_count",    doEponLostTime},
    {"hw_nat_noclean",    doEponHwnatNoclean},
    {"mac_oam_cnt",    doEponMacOam},
    {"mac_eth_cnt",    doEponEth},
    {"staticrpt",    doEponStaticrpt},
    {"dying",    doEponDying},
    {"laser_time",    doEponLaserTime},
    {"set_llid",    doEponSetllid},
    {"silent_time",    doEponSlientTime},
    {"set_mac_mode",    doEponSetMacMode},
    {"mac_addr",    doEponMacAddr},
    {"show_hw_mac",    doEponShowMac},
    {"tx_check_type",    doEponTxCheckType},
    {"dump_reg",    doEponDumpReg},
    {"phyready",    doEponPhyReady},    
    {"set_mode",    doEponSetMode},
    {"get_mode",    doEponGetMode},
    {"debug",    doEponDebug},
    {"band_enable",    doEponBandUtilizationEnable},
	{"random_dly_test",    doRandom_delay_test},
	{"random_dly_show",    doRandom_delay_show},
 	{"tx_favor_oam",    doTx_favor_oam},
	{"llid_rcv_rgst_sts",    doLlid_rcv_rgst_sts},
 	{"rate",    doEponTxRate}, 	
 	{"dpoeKey",	 doEponDpoeKey},
    /*add cmd above this line*/
    {NULL,               NULL},
};
/*______________________________________________________________________________
**	function name
**		epon_debug_subcmd
**	description:
**		epon sub cmd 
**	parameters:
**		tab[]
**		subcmd
**		action1
**		action2
**		action3
**	global:
**		None
**	return:
**		0:success
**		-1:fail
**	call:
**		simple_strtoul
**		epon_set_queue_threshold
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_debug_subcmd(const epon_cmds_t tab[], char *subcmd, uint action1, uint action2, uint action3)
{
	const epon_cmds_t *cmdp = NULL;

	for(cmdp = tab;cmdp->name != NULL;cmdp++)
	{
		if(!strcmp(cmdp->name, subcmd))
		{
			cmdp->func(action1, action2, action3);
			break;
		}
	}
}
/*______________________________________________________________________________
**	function name
**		epon_debug_write_proc
**	description:
**		epon debug cmd write proc 
**	parameters:
**		file
**		buffer
**		count
**		data
**	global:
**		gp_epon_global_data
**		gpPonSysData
**		epondebugCmds
**	return:
**		0:success
**		-1:fail
**	call:
**		e_glb_cfg_SET_tx_default_rpt
**		e_tx_cal_cnst_SET_default_ovrhd
**		xmit_dying_gasp_oam
**		xmcs_set_epon_llid_config
**		show_onu_mac_addr
**		epon_get_mac_address
**		show_all_mac_regs
**		show_onu_mac_addr
**		show_all_route_type
**		show_route_table
**		show_all_route_type
**		insert_route_entry
**		delete_route_entry
**		epon_msg_route_dispatch
**		eponSetRateMode
**		eponGet10GLlidKey
**		epon_debug_subcmd
**	revision:
**		v1.0
**____________________________________________________________________________*/

static int epon_debug_write_proc(struct file *file, const char *buffer, unsigned long count, void *data)
{
    char val_string[128], cmd[32], subcmd[32] ;
    uint action1 = 0, action2 = 0, action3 = 0;
    
    if (count > sizeof(val_string) - 1)
        return -EINVAL ;

    if (copy_from_user(val_string, buffer, count))
        return -EFAULT ;

    sscanf(val_string, "%31s %31s %x %x %x", cmd, subcmd, &action1, &action2, &action3);
        
    if (!strcmp(cmd, "epon")){
			epon_debug_subcmd(epondebugCmds, subcmd, action1, action2, action3);
    }
	return count;
}

/*______________________________________________________________________________
**	function name
**		epon_proc_init
**	description:
**		epon proc init
**	parameters:
**		None
**	global:
**		epon_proc_dir
**	return:
**		0:success
**	call:
**		create_proc_entry
**	revision:
**		v1.0
**____________________________________________________________________________*/

int epon_proc_init(void)
{
	struct proc_dir_entry *epon_proc=NULL;

	epon_proc_dir = proc_mkdir("epon", NULL);
	if(NULL == epon_proc_dir){
        printk("ERR: epon proc_mkdir epon Fail\n");
	    return -1;
	}

	epon_proc = create_proc_entry("debug", 0, epon_proc_dir) ;
	if(NULL == epon_proc) {
        printk("ERR: epon create_proc_entry debug Fail\n");
	    return -1;
	}
	epon_proc->read_proc = NULL ;
	epon_proc->write_proc = epon_debug_write_proc ;
	return EPON_SUCCESS;
}
/*______________________________________________________________________________
**	function name
**		epon_proc_exit
**	description:
**		epon proc deinit
**	parameters:
**		None
**	global:
**		epon_proc_dir
**	return:
**		None
**	call:
**		remove_proc_entry
**	revision:
**		v1.0
**____________________________________________________________________________*/

void epon_proc_exit(void)
{   
    if(epon_proc_dir)
        remove_proc_entry("debug", epon_proc_dir);
    remove_proc_entry("epon", NULL);
    return;
}



