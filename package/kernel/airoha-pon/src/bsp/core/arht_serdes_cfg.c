/***************************************************************
Copyright Statement:

This software/firmware and related documentation (��AIROHA Software��) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to AIROHA Limited (��AIROHA��) and/or 
its licensors. Without the prior written permission of AIROHA and/or its licensors, 
any reproduction, modification, use or disclosure of AIROHA Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

AIROHA Limited  AIROHA. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (��AIROHA SOFTWARE��) RECEIVED FROM AIROHA 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ��AS IS�� 
BASIS ONLY. AIROHA EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES AIROHA PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE AIROHA SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER��S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN AIROHA SOFTWARE.

AIROHA SHALL NOT BE RESPONSIBLE FOR ANY AIROHA SOFTWARE RELEASES 
MADE TO RECEIVER��S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
AIROHA'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE AIROHA 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT AIROHA'S SOLE OPTION, TO 
REVISE OR REPLACE THE AIROHA SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO AIROHA FOR SUCH 
AIROHA SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/tc3162/tc3162.h>
#include <linux/uaccess.h>
#include <ecnt_event_global/ecnt_event_system.h>
#include <ecnt_event_global/ecnt_event_board.h>
#include <asm/uaccess.h>
#include <linux/limits.h>
#include <asm/ptrace.h>
#include <linux/proc_fs.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/
#define SERDES_IF_IGNORE			(U8_MAX)

#define SERDES_CFG_DBG_LEVEL_NONE	(1 << 0)
#define SERDES_CFG_DBG_LEVEL_ERROR	(1 << 1)
#define SERDES_CFG_DBG_LEVEL_NOTICE	(1 << 2)	//for verify
#define SERDES_CFG_DBG_LEVEL_INFO	(1 << 3)	//for dump array
#define SERDES_CFG_DBG_LEVEL_TRACE	(1 << 4)	//for trace code flow
#define SERDES_CFG_DBG_LEVEL_CFG	(SERDES_CFG_DBG_LEVEL_ERROR)

#define ETHERTYPE_PHYTYPE	(0)
#define SRDS_PARM_ETHERTYPE	(1)
#define SRDS_PARM_IF		(2)

#define USAGE_BUF_SIZE		 (512)
#define USAGE_TEMP_SIZE		 (128)
/************************************************************************
*                  M A C R O S
*************************************************************************
*/
#define ARHT_SERDES_CFG_DEBUG
#define SERDES_MSG(lv, arg...) { \
	if(serdes_cfg_dbg_lv & lv) { \
		printk("[%s]:%d ", __func__, __LINE__); \
		printk(arg); \
	} \
}

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

typedef struct SRDS_PARM {
	uint8_t type;
} SRDS_PARM_t;

typedef struct SRDS_PORT {
	uint8_t if_combo_cnt; 
	uint8_t *if_combo;
	char *name;
	SRDS_PARM_t *srds_parm;
} SRDS_PORT_t;

typedef struct IC_SRDS {
	uint8_t srdsPortCnt;
	uint8_t srdsParmCntPerPort;
	char *srdsPortCombo;
	SRDS_PORT_t *srds_port;
} IC_SRDS_t;

typedef struct SRDS_IF_NAME_ALL_COMP {
	uint8_t cnt;
	char *all_comp;
	char **comp;
} SRDS_IF_NAME_ALL_COMP_t;

typedef struct ETHER_TYPE_NAME_ALL_COMP {
	uint8_t cnt;
	char *all_comp;
	char **comp;
} ETHER_TYPE_NAME_ALL_COMP_t;

typedef struct ETHER_PHY_NAME_ALL_COMP {
	uint8_t cnt;
	char *all_comp;
	char **comp;
} ETHER_PHY_NAME_ALL_COMP_t;

static int init_srds_if_name_all_comp(SRDS_IF_NAME_ALL_COMP_t *);
static int init_ether_type_name_all_comp(ETHER_TYPE_NAME_ALL_COMP_t *);
static int init_ether_phy_name_all_comp(ETHER_PHY_NAME_ALL_COMP_t *);
static int init_ic_srds(IC_SRDS_t *);
static int init_srds_port(IC_SRDS_t *);
static int init_srds_port_name(char *, IC_SRDS_t *);
static int init_srds_usage_postfix(IC_SRDS_t *);
static int init_srds_status(IC_SRDS_t *);
static int parse_srds_if_combo(SRDS_PORT_t *);
static int parse_srds_parm(SRDS_PORT_t *, uint8_t);
static int parse_rfb_cfg(void);
static void insertStrToBuffer(char *, size_t , const char *, ...);
/************************************************************************
*                  STATIC VARIABLE DECLARATIONS
*************************************************************************
*/
static uint32_t serdes_cfg_dbg_lv = SERDES_CFG_DBG_LEVEL_CFG;

static char *rfb_cfg_cur = NULL, *rfb_cfg_ori = NULL;
static IC_SRDS_t ic_srds;
static SRDS_IF_NAME_ALL_COMP_t if_name_all_comp;
static ETHER_TYPE_NAME_ALL_COMP_t ether_type_name_all_comp;
static ETHER_PHY_NAME_ALL_COMP_t ether_phy_name_all_comp;
static char *srdsPortNameComp;

static const char * const srdsPortPartUsagePrefix =
	"get or set serdes port.\n" \
	"  sys serdes: get all serdes port configurations.\n" \
	"  sys serdes <port> <I/F> [ether type] [ether phy type] [-n]: set serdes port\n" \
	"  port:\n";
static const char * const srdsEtherTypePartUsagePrefix =
	"  ether type:\n";
static const char * const srdsEtherPhyTypePartUsagePrefix =
	"  ether ether type:\n";
static const char * const srds_if_str="      I/F:\n";
static size_t srdsPortPartUsagePostfixLen = 0;
static char** srdsPortPartUsagePostfixArr = NULL;
static char *srdsEtherTypePartUsagePostfix = NULL;
static size_t srdsEtherTypePartUsagePostfixLen = 0;
static char *srdsEtherPhyTypePartUsagePostfix = NULL;
static size_t srdsEtherPhyTypePartUsagePostfixLen = 0;
static char *srdsSts = NULL;
static size_t srdsStsLen = 0;

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/
/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/
#ifdef ARHT_SERDES_CFG_DEBUG
void set_serdes_cfg_dbg_lv(uint32_t dbg_lv)
{
	serdes_cfg_dbg_lv = dbg_lv;
}
#else
#define set_serdes_cfg_dbg_lv(...) do{}while(0)
#endif

static int __init early_rfb_cfg(char *p)
{
	rfb_cfg_ori=p;
	printk("uboot rfb_cfg:%s\n", rfb_cfg_ori);
	return 0;
}
early_param(BOOTARGS_RFB_CFG_STR, early_rfb_cfg);

static int __init early_serdes_cnt(char *p)
{
	int ret;

	ret = sscanf(p, "%d", &ic_srds.srdsPortCnt);
	if(ret != 1) {
		printk("parse %s error:%d\n", BOOTARGS_SRDS_PORT_CNT_STR, ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_SRDS_PORT_CNT_STR, early_serdes_cnt);

static int __init early_serdes_param_num(char *p)
{
	int ret;

	ret = sscanf(p, "%d", &ic_srds.srdsParmCntPerPort);
	if(ret != 1) {
		printk("parse %s error:%d\n", BOOTARGS_SRDS_PARM_PER_PORT_STR, ret);
		return -1;
	}

	return 0;
}
early_param(BOOTARGS_SRDS_PARM_PER_PORT_STR, early_serdes_param_num);

static int __init early_serdes_port_name_all_comp(char *p)
{
	srdsPortNameComp = p;
	return 0;
}
early_param(BOOTARGS_SRDS_PORT_NAME_ALL_COMP_STR, early_serdes_port_name_all_comp);

static int __init early_serdes_if_name_all_comp(char *p)
{
	if_name_all_comp.all_comp = p;
	return 0;
}
early_param(BOOTARGS_SRDS_IF_NAME_ALL_COMP_STR, early_serdes_if_name_all_comp);

static int __init early_ether_type_name_all_comp(char *p)
{
	ether_type_name_all_comp.all_comp = p;
	return 0;
}
early_param(BOOTARGS_ETHER_TYPE_NAME_ALL_COMP_STR, early_ether_type_name_all_comp);

static int __init early_ether_phy_name_all_comp(char *p)
{
	ether_phy_name_all_comp.all_comp = p;
	return 0;
}
early_param(BOOTARGS_ETHER_PHY_NAME_ALL_COMP_STR, early_ether_phy_name_all_comp);

static int __init early_serdes_port_combo(char *p)
{
	ic_srds.srdsPortCombo = p;
	return 0;
}
early_param(BOOTARGS_SRDS_PORT_COMBO_STR, early_serdes_port_combo);

uint8_t get_serdes_port_count(void)
{
	return ic_srds.srdsPortCnt;
}
EXPORT_SYMBOL(get_serdes_port_count);

static int check_serdes_if(uint8_t port_idx, uint8_t if_idx)
{
	if(port_idx >= get_serdes_port_count()) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error port_idx:%u.\n", port_idx);
		return -1;
	}

	if(if_idx != SERDES_IF_IGNORE) {
		if(if_idx >= ic_srds.srds_port[port_idx].if_combo_cnt) {
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error if_idx:%u.\n", if_idx);
			return -1;
		}
	}

	return 0;
}

static int get_serdes_interface_count(uint8_t port_idx)
{	
	if(check_serdes_if(port_idx, SERDES_IF_IGNORE)) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error port_idx:%u.\n", port_idx);
		return -1;
	}

	return ic_srds.srds_port[port_idx].if_combo_cnt;
}

ECNT_EVENT_SYSTEM_SERDES_ETHER_TYPE_SEL_t get_serdes_ether_type_sel(uint8_t port_idx)
{
	uint8_t ether_type = 0;

	if(check_serdes_if(port_idx, SERDES_IF_IGNORE)) {
		return ECNT_EVENT_SERDES_ETHER_TYPE_ERROR;
	}

	ether_type = ic_srds.srds_port[port_idx].srds_parm[SRDS_PARM_ETHERTYPE].type;

	switch(ether_type) {
		case ECNT_EVENT_SERDES_ETHER_TYPE_LAN:
			return ECNT_EVENT_SERDES_ETHER_TYPE_LAN;
		case ECNT_EVENT_SERDES_ETHER_TYPE_WAN:
			return ECNT_EVENT_SERDES_ETHER_TYPE_WAN;
		default :
			return ECNT_EVENT_SERDES_ETHER_TYPE_NONE;
	}
}
EXPORT_SYMBOL(get_serdes_ether_type_sel);

/* forced updating ether type region */
int set_serdes_ether_type_sel(uint8_t port_idx, uint8_t value)
{
	if(check_serdes_if(port_idx, SERDES_IF_IGNORE)) {
		return -1; 
	}
	
	if (value > ether_type_name_all_comp.cnt) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error, ether type value=%u more than ether type components:%u.\n", value, ether_type_name_all_comp.cnt);
		return -1;
	}

	ic_srds.srds_port[port_idx].srds_parm[SRDS_PARM_ETHERTYPE].type = value;
	
	return 0;		
}


uint8_t get_serdes_interface_sel(uint8_t port_idx)
{
	if(check_serdes_if(port_idx, SERDES_IF_IGNORE)) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error port_idx:%u.\n", port_idx);
		return SERDES_IF_STS_ERROR;
	}

	return (uint8_t)ic_srds.srds_port[port_idx].srds_parm[SRDS_PARM_IF].type;
}
EXPORT_SYMBOL(get_serdes_interface_sel);

ECNT_EVENT_SYSTEM_SERDES_PHY_SEL_t get_serdes_phy_sel(uint8_t port_idx)
{
	uint8_t ether_phy = 0;

	if(check_serdes_if(port_idx, SERDES_IF_IGNORE)) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error port_idx:%u.\n", port_idx);
		return ECNT_EVENT_SERDES_PHY_ERROR;
	}

	ether_phy = ic_srds.srds_port[port_idx].srds_parm[ETHERTYPE_PHYTYPE].type;

	if(ether_phy >= ether_phy_name_all_comp.cnt) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error serdes_phy_sel:%d.\n", (int)ether_phy);
		return ECNT_EVENT_SERDES_PHY_NONE;
	}

	return (ECNT_EVENT_SYSTEM_SERDES_PHY_SEL_t)ether_phy;
}
EXPORT_SYMBOL(get_serdes_phy_sel);

const char *get_serdes_port_name(uint8_t port_idx)
{	
	if(check_serdes_if(port_idx, SERDES_IF_IGNORE)) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error port_idx:%u.\n", port_idx);
		return NULL;
	}
	
	return ic_srds.srds_port[port_idx].name;
}
EXPORT_SYMBOL(get_serdes_port_name);

const char *get_serdes_interface_name(uint8_t port_idx, uint8_t serdes_if)
{
	if(check_serdes_if(port_idx, serdes_if)) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error port_idx:%d, if_idx:%d.\n", port_idx, serdes_if);
		return NULL;
	}

	return if_name_all_comp.comp[ic_srds.srds_port[port_idx].if_combo[serdes_if]];
}
EXPORT_SYMBOL(get_serdes_interface_name);

static const char *get_serdes_ether_type_name(uint8_t ether_type_sel)
{
	if(ether_type_sel > ether_type_name_all_comp.cnt) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error ether_type_sel:%u, MAX value is %u.\n", ether_type_sel, ether_type_name_all_comp.cnt);
		return NULL;
	}
	
	return ether_type_name_all_comp.comp[ether_type_sel];
}

static const char *get_serdes_phy_name(ECNT_EVENT_SYSTEM_SERDES_PHY_SEL_t phy_sel)
{
	if(phy_sel >= ether_phy_name_all_comp.cnt) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error phy_sel:%d, MAX value is %u.\n", (int)phy_sel, ether_phy_name_all_comp.cnt);
		return NULL;
	}

	return ether_phy_name_all_comp.comp[phy_sel];
}

static void getSerdesSelUsage(void)
{
	int i=0;
	/* print serdes port part */
	printk(srdsPortPartUsagePrefix);
	for(i=0;i<get_serdes_port_count();i++){
		printk(srdsPortPartUsagePostfixArr[i]);
	}

	/* print ether type part */
	printk(srdsEtherTypePartUsagePrefix);
	printk(srdsEtherTypePartUsagePostfix);

	/* print ether phy part */
	printk(srdsEtherPhyTypePartUsagePrefix);
	printk(srdsEtherPhyTypePartUsagePostfix);
}

static void getSerdesSelStatus(void)
{
	printk(srdsSts);
}

long serdesCfgIoctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	long ret = 0;
	serdesCfgIoctl_t __serdesCfg;
	serdesCfgIoctl_t *userCfg = (serdesCfgIoctl_t *)arg;
	uint8_t result;
	int port_cnt;
	ECNT_EVENT_SYSTEM_SERDES_ETHER_TYPE_SEL_t ether_type;
	ECNT_EVENT_SYSTEM_SERDES_PHY_SEL_t phy_sel;
	ECNT_EVENT_SYSTEM_SERDES_SEL_t port_idx;
	char *port_name;
	
	if (_IOC_TYPE(cmd) != SERDES_CFG_IOC_MAGIC) {
		return -ENOTTY;
	}

	if ((_IOC_DIR(cmd) & _IOC_READ) || 
		(_IOC_DIR(cmd) & _IOC_WRITE)) {
		ret = !access_ok( (void __user *)userCfg, _IOC_SIZE(cmd));
		if (ret) {
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "_IOC_READ or _IOC_WRITE check error.\n");
			return -EFAULT;
		}
	}


	if(copy_from_user(&__serdesCfg, userCfg, sizeof(serdesCfgIoctl_t))) {
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "copy_from_user failed\n");
		return -EFAULT;
	}

	
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "__serdesCfg.port_idx:%d\n", __serdesCfg.port_idx);
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "__serdesCfg.port_cnt:%d\n", __serdesCfg.port_cnt);
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "__serdesCfg.ether_type:%d\n", __serdesCfg.ether_type);
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "__serdesCfg.ether_type_cnt:%d\n", __serdesCfg.ether_type_cnt);
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "__serdesCfg.serdes_if:%d\n", __serdesCfg.serdes_if);
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "__serdesCfg.serdes_if_cnt:%d\n", __serdesCfg.serdes_if_cnt);

	switch(cmd)
	{
		case SERDES_CFG_IOCTL_GET_IF_SEL:
			result = get_serdes_interface_sel(__serdesCfg.port_idx);
			if(SERDES_IF_STS_ERROR == result) {
				return -EFAULT;
			}
			ret = copy_to_user((void __user *)(&userCfg->serdes_if), &result, sizeof(userCfg->serdes_if));			
			if(ret) {
				SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "copy serdes interface selection error.\n");
			}
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "serdes_if:%u\n", userCfg->serdes_if);
			break;
		case SERDES_CFG_IOCTL_GET_ETHER_TYPE_SEL:
			ether_type = get_serdes_ether_type_sel(__serdesCfg.port_idx);
			if(ECNT_EVENT_SERDES_ETHER_TYPE_ERROR == ether_type) {
				return -EFAULT;
			}
			ret = copy_to_user((void __user *)(&userCfg->ether_type), &ether_type, sizeof(userCfg->ether_type));			
			if(ret) {
				SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "copy serdes ether type selection error.\n");
			}
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "ether_type:%d\n", (int)userCfg->ether_type);
			break;
		case SERDES_CFG_IOCTL_GET_PHY_TYPE_SEL:
			phy_sel = get_serdes_phy_sel(__serdesCfg.port_idx);
			if(ECNT_EVENT_SERDES_PHY_ERROR == phy_sel) {
				return -EFAULT;
			}
			ret = copy_to_user((void __user *)(&userCfg->phy_sel), &phy_sel, sizeof(userCfg->phy_sel));			
			if(ret) {
				SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "copy serdes phy type selection error.\n");
			}
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "phy_type:%d\n", (int)userCfg->phy_sel);
			break;
		case SERDES_CFG_IOCTL_SET_ETHSEL_ETHTYPE:
			ret = set_serdes_ether_type_sel(ECNT_EVENT_SERDES_PORT_1, __serdesCfg.ether_type);
			if(ret) {
				SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "error setting ether type in serdes-ethernet .\n");
			}
			break;
		case SERDES_CFG_IOCTL_GET_USAGE:
			getSerdesSelUsage();
			break;
		case SERDES_CFG_IOCTL_GET_PORT_ENV_NAME:
			port_name = get_serdes_port_name(__serdesCfg.port_idx);
			ret = copy_to_user((void __user *)userCfg->name, port_name, sizeof(userCfg->name));			
			if(ret) {
				SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "copy serdes port name error.\n");
			}
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "SERDES_CFG_IOCTL_GET_PORT_ENV_NAME name:%s\n", userCfg->name);
			break;
		case SERDES_CFG_IOCTL_GET_STATUS:
			getSerdesSelStatus();
			break;
		case SERDES_CFG_IOCTL_GET_PORT_CNT:
			result = get_serdes_port_count();
			port_cnt = (int)result;
			ret = copy_to_user((void __user *)userCfg->port_cnt, &port_cnt, sizeof(userCfg->port_cnt));			
			if(ret) {
				SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "copy serdes port cnt error.\n");
			}
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "SERDES_CFG_IOCTL_GET_PORT_CNT name:%d\n", userCfg->port_cnt);
			
			break;
		default:
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "unknown cmd:%d.\n", cmd);
			return -EFAULT;
	}
	return ret;
}


static struct file_operations serdesCfgFops = {
	.owner =			THIS_MODULE,
	.write =			NULL,
	.read =				NULL,
	.unlocked_ioctl =	serdesCfgIoctl,	
#ifdef TCSUPPORT_CPU_ARMV8_64
	.compat_ioctl	= 	serdesCfgIoctl,
#endif
	.open =				NULL,
	.release =			NULL,
};

unsigned char serdes_dsl_enable = 0;
EXPORT_SYMBOL(serdes_dsl_enable);

static void check_serdes_dsl_enable(void)
{
	ECNT_EVENT_SYSTEM_SERDES_PHY_SEL_t phy_sel;
	ECNT_EVENT_SYSTEM_SERDES_ETHER_TYPE_SEL_t ether_idx = 0;
	int i = 0;

	for(i = 0; i < ic_srds.srdsPortCnt; i++){
		
		phy_sel = get_serdes_phy_sel(i);
		ether_idx = get_serdes_ether_type_sel(i);
		
		if ( ECNT_EVENT_SERDES_ETHER_TYPE_WAN == ether_idx
				&& ECNT_EVENT_SERDES_PHY_DSL == phy_sel )
		{
				serdes_dsl_enable = 1;
		}
	}
}

static ssize_t serdes_wan_type_read_proc(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	char user_buf[32] = {0};
	size_t len = count, buf_length = 0;
	loff_t pos = *ppos;
	unsigned long ret = 0;
	ssize_t retval = 0;
	
	/* use memory size for data stream or use length of string buffer */
	buf_length = snprintf(user_buf, sizeof(user_buf), "%d", serdes_dsl_enable);
	
	/* valid check */
	if ( pos >= buf_length )
		return 0;
	if ( len >= buf_length )
		len = buf_length;
	if ( pos + len > buf_length )
		len = buf_length - pos;
	
	/* ret contains the amount of chars wasn't successfully written to buf */
	ret = copy_to_user(buf, user_buf + pos, len);
	
	*ppos += len;
	retval = len - ret;

	return retval;
}

static const struct file_operations proc_wan_type_ops = {
	.owner =			THIS_MODULE,
	.write =			NULL,
	.read =				serdes_wan_type_read_proc,
	.unlocked_ioctl =	NULL,	
	.compat_ioctl	= 	NULL,
	.open =				NULL,
	.release =			NULL,
};

static int ECNT_SERDES_CFG_PROBE(void)
{
	int ret = 0;
	struct proc_dir_entry *serdes_proc = NULL;
	
	ret = register_chrdev(SERDES_CFG_MAJOR, "serdes_cfg", &serdesCfgFops);
	if (ret < 0) {    
    	SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "serdes cfg create ioctrl fail\n");
		return ret;
    }
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "%s:%u\n", BOOTARGS_SRDS_PORT_CNT_STR, ic_srds.srdsPortCnt);

	if(-1 == parse_rfb_cfg()){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "parse rfb cfg fail\n");
		return -1;
	}
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "parse rfb_cfg:%s\n", rfb_cfg_cur);
	
	if(-1 == init_ic_srds(&ic_srds)){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "init ic srds fail\n");
		return -1;
	}
	
	if(-1 == init_srds_if_name_all_comp(&if_name_all_comp)){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "init srds if name all comp fail\n");
		return -1;
	}

	if(-1 == init_ether_type_name_all_comp(&ether_type_name_all_comp)){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "init ether type name all comp fail\n");
		return -1;
	}

	if(-1 == init_ether_phy_name_all_comp(&ether_phy_name_all_comp)){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "init ether phy name all comp fail\n");
		return -1;
	}

	if(-1 == init_srds_usage_postfix(&ic_srds)){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "init srds usage postfix fail\n");
		return -1;
	}

	if(-1 == init_srds_status(&ic_srds)){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "init srds status fail\n");
		return -1;
	}
	check_serdes_dsl_enable();
	
	serdes_proc = proc_create("tc3162/wan_type", 0, NULL, &proc_wan_type_ops);
	if ( !serdes_proc )
	{
		printk("create proc tc3162/wan_type failed!\n");
		return -1;
	}
	return 0;
}
subsys_initcall(ECNT_SERDES_CFG_PROBE);

static void insertStrToBuffer(char *buf, size_t bufSize, const char *strFmt, ...)
{
	va_list args;
	va_start(args, strFmt);
	vsnprintf(buf, bufSize, strFmt, args);
	va_end(args);
}

static int init_srds_port(IC_SRDS_t *ic_srds)
{
	int i = 0;
	
	ic_srds->srds_port = (SRDS_PORT_t*)vmalloc(sizeof(SRDS_PORT_t)*(ic_srds->srdsPortCnt));
	
	if(NULL == ic_srds->srds_port){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "srds_port malloc failed\n");
		return -1;
	}

	//memset(ic_srds, 0, sizeof(IC_SRDS_t));

	for(i = 0; i < ic_srds->srdsPortCnt; i++){
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "port%d\n",i+1);
	
		if(-1 == parse_srds_if_combo(&ic_srds->srds_port[i])){
			vfree(ic_srds->srds_port);
			return -1;
		}

		if(-1 == parse_srds_parm(&ic_srds->srds_port[i], ic_srds->srdsParmCntPerPort)){
			vfree(ic_srds->srds_port);
			return -1;
		}
	}

	if(-1 == init_srds_port_name(srdsPortNameComp, ic_srds)) {
		vfree(ic_srds->srds_port);
		return -1;
	}

	return 0;
}

static int init_ic_srds(IC_SRDS_t *ic_srds)
{
	int ret = 0;

	if(-1 == init_srds_port(ic_srds)){
		ret = -1;
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "init error for srds\n");
	}

	return ret;
}

static int init_srds_usage_postfix(IC_SRDS_t *ic_srds)
{
	int ret=0, port_idx = 0, if_idx = 0, ether_idx = 0;
	char* srdsUsageTemp=NULL;
	char* srdsUsageTotal=NULL;
	size_t srdsUsageTmpSize = USAGE_TEMP_SIZE;

	srdsPortPartUsagePostfixArr = (char **)vzalloc(sizeof(char*) * get_serdes_port_count());
	if(NULL==srdsPortPartUsagePostfixArr){
		ret=-1;
		goto init_usage_postfix_finish;
	}

	srdsUsageTemp = (char *)vzalloc(sizeof(char) * (USAGE_TEMP_SIZE));
	if(NULL==srdsUsageTemp){
		ret=-1;
		goto init_usage_postfix_finish;
	}

	srdsUsageTotal = (char *)vzalloc(sizeof(char) * (USAGE_BUF_SIZE));
	if(NULL==srdsUsageTotal){
		ret=-1;
		goto init_usage_postfix_finish;
	}
	memset(srdsUsageTotal, '\0', USAGE_BUF_SIZE);
	memset(srdsUsageTemp, '\0', USAGE_TEMP_SIZE);
	
	/* === srdsPortPart === */
	for(port_idx = 0; port_idx < ic_srds->srdsPortCnt; port_idx++) {
		/* serdes port index and its name */
		insertStrToBuffer(srdsUsageTemp, srdsUsageTmpSize, "    %d:%s\n", port_idx, get_serdes_port_name(port_idx));
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,"port_idx:%d %s len:%d\n",port_idx,srdsUsageTemp,strlen(srdsUsageTemp));
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,"srdsUsageTotal len :%d\n",strlen(srdsUsageTotal));
		insertStrToBuffer(srdsUsageTotal, USAGE_BUF_SIZE, "%s%s", srdsUsageTotal,srdsUsageTemp);
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,"concate srdsUsageTotal len :%d\n",strlen(srdsUsageTotal));
		memset(srdsUsageTemp, '\0', USAGE_TEMP_SIZE);
		insertStrToBuffer(srdsUsageTotal, USAGE_BUF_SIZE, "%s%s", srdsUsageTotal,srds_if_str);

		/* serdes interface name */
		for(if_idx = 0; if_idx < ic_srds->srds_port[port_idx].if_combo_cnt; if_idx++) {
			insertStrToBuffer(srdsUsageTemp, srdsUsageTmpSize, "        %d:%s\n", if_idx, get_serdes_interface_name(port_idx, if_idx));
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,"port_idx:%d if_idx:%d %s len:%d\n",port_idx,if_idx,srdsUsageTemp,strlen(srdsUsageTemp));
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,"srdsUsageTotal len :%d\n",strlen(srdsUsageTotal));
			insertStrToBuffer(srdsUsageTotal, USAGE_BUF_SIZE, "%s%s", srdsUsageTotal,srdsUsageTemp);
			SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,"concate srdsUsageTotal len :%d\n",strlen(srdsUsageTotal));
			memset(srdsUsageTemp, '\0', USAGE_TEMP_SIZE);
			
		}

		/* string copy */
		srdsPortPartUsagePostfixLen = strlen(srdsUsageTotal);
		srdsPortPartUsagePostfixArr[port_idx] = (char *)vzalloc(sizeof(char) * (srdsPortPartUsagePostfixLen + 1));
		strncpy(srdsPortPartUsagePostfixArr[port_idx], srdsUsageTotal, srdsPortPartUsagePostfixLen);

		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,	"postfix of srds port usage:\n"
												"  length:%zu, string:%s\n", 
												srdsPortPartUsagePostfixLen, 
												srdsPortPartUsagePostfixArr[port_idx]);

		/* reset buffer */
		memset(srdsUsageTotal, '\0', USAGE_BUF_SIZE);
	}

	/* === ether type Part === */

	/* ether type name */
	for(ether_idx = 0; ether_idx < ether_type_name_all_comp.cnt; ether_idx++) {
		insertStrToBuffer(srdsUsageTemp, srdsUsageTmpSize, "    %d:%s\n", ether_idx, get_serdes_ether_type_name((uint8_t)ether_idx));
		strncat(srdsUsageTotal, srdsUsageTemp, strlen(srdsUsageTemp));
		memset(srdsUsageTemp, '\0', USAGE_TEMP_SIZE);
	}

	/* string copy */
	srdsEtherTypePartUsagePostfixLen = strlen(srdsUsageTotal);
	srdsEtherTypePartUsagePostfix = (char *)vzalloc(sizeof(char) * (srdsEtherTypePartUsagePostfixLen + 1));
	strncpy(srdsEtherTypePartUsagePostfix, srdsUsageTotal, srdsEtherTypePartUsagePostfixLen);

	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,	"postfix of ether type usage:\n"
											"  length:%zu, string:%s\n", 
											srdsEtherTypePartUsagePostfixLen, 
											srdsEtherTypePartUsagePostfix);

	/* === ether phy Part === */

	/* reset buffer */
	memset(srdsUsageTotal, '\0', USAGE_BUF_SIZE);
	
	/* ether phy name */
	for(ether_idx = 0; ether_idx < ether_phy_name_all_comp.cnt; ether_idx++) {
		insertStrToBuffer(srdsUsageTemp, srdsUsageTmpSize, "    %d:%s\n", ether_idx, get_serdes_phy_name(ether_idx));
		strncat(srdsUsageTotal, srdsUsageTemp, strlen(srdsUsageTemp));
		memset(srdsUsageTemp, '\0', USAGE_TEMP_SIZE);
	}

	/* string copy */
	srdsEtherPhyTypePartUsagePostfixLen = strlen(srdsUsageTotal);
	srdsEtherPhyTypePartUsagePostfix = (char *)vzalloc(sizeof(char) * (srdsEtherPhyTypePartUsagePostfixLen + 1));
	strncpy(srdsEtherPhyTypePartUsagePostfix, srdsUsageTotal, srdsEtherPhyTypePartUsagePostfixLen);

	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,	"postfix of phy type usage:\n"
											"  length:%zu, string:%s\n", 
											srdsEtherPhyTypePartUsagePostfixLen, 
											srdsEtherPhyTypePartUsagePostfix);

init_usage_postfix_finish:
	if(srdsUsageTemp) {
		vfree(srdsUsageTemp);
	}
	if(srdsUsageTotal) {
		vfree(srdsUsageTotal);
	}

	return ret;
}

/* 
 * show serdes status
 * <serdes port name>: I/F=<I/F name>, EtherType=<ether type name>, Ether PHY type=<ether phy type name>
 */
static int init_srds_status(IC_SRDS_t *ic_srds)
{
	int ret = -1, port_idx = 0, if_idx = 0;
	ECNT_EVENT_SYSTEM_SERDES_ETHER_TYPE_SEL_t ether_idx = 0;
	ECNT_EVENT_SYSTEM_SERDES_PHY_SEL_t phy_sel;
	char* srdsUsageTemp=NULL;
	char* srdsUsageTotal=NULL;
	size_t srdsUsageTmpSize = USAGE_TEMP_SIZE;

	srdsUsageTemp = (char *)vzalloc(sizeof(char) * (USAGE_TEMP_SIZE));
	if(NULL==srdsUsageTemp){
		goto init_srds_sts_finish;
	}

	srdsUsageTotal = (char *)vzalloc(sizeof(char) * (USAGE_BUF_SIZE));
	if(NULL==srdsUsageTotal){
		goto init_srds_sts_finish;
	}
	srdsUsageTotal[0]='\0';
	
	ret = -1;
	/* === srdsPortPart === */
	for(port_idx = 0; port_idx < ic_srds->srdsPortCnt; port_idx++) {
		if_idx = get_serdes_interface_sel(port_idx);
		ether_idx = get_serdes_ether_type_sel(port_idx);
		phy_sel = get_serdes_phy_sel(port_idx);
		if(ECNT_EVENT_SERDES_ETHER_TYPE_ERROR == ether_idx ||ECNT_EVENT_SERDES_PHY_ERROR == phy_sel) {
			goto init_srds_sts_finish;
		}
		/* serdes port index and its name */
		insertStrToBuffer(srdsUsageTemp, srdsUsageTmpSize, "%s: IF=%s, EtherType=%s, Ether PHY Type=%s\n", 
											get_serdes_port_name(port_idx),
											get_serdes_interface_name(port_idx, if_idx),
											get_serdes_ether_type_name((uint8_t)ether_idx),
											get_serdes_phy_name(phy_sel));
		strncat(srdsUsageTotal, srdsUsageTemp, strlen(srdsUsageTemp));

		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE,	"srds status: length:%zu, string:%s\n", 
												strlen(srdsUsageTotal), 
												srdsUsageTotal);
	}

	/* string copy */
	srdsStsLen = strlen(srdsUsageTotal);
	srdsSts = (char *)vzalloc(sizeof(char) * (srdsStsLen + 1));
	strncpy(srdsSts, srdsUsageTotal, srdsStsLen);

	ret = 0;
	
init_srds_sts_finish:
	if(srdsUsageTemp) {
		vfree(srdsUsageTemp);
	}
	if(srdsUsageTotal) {
		vfree(srdsUsageTotal);
	}

	return ret;
}

/* when srdsIfNameAllComp in mi.conf is 
 * 10,NONE,XFI,USXGMII,HSGMII,PON,PCIE_G3_1LANE,PCIE_G2_1LANE,USB3.0,USB2.0_only,HSGMII+USB2.0
 * srdsIfNameAllComp(Decimal cnt, string) represents the interface name of all of the serdes port can be selected, 
 * this is the fix value;
 * 00H=NONE, 01H=XFI, 
 * 02H=USXGMII, 
 * 03H=HSGMII, 
 * 04H=PON, 
 * 05H=PCIE_G3_1LANE, 
 * 06H=PCIE_G2_1LANE, 
 * 07H=USB3.0, 
 * 08H=USB2.0_only, 
 * 09H=HSGMII+USB2.0, 
 * 0AH=10GPON;
 * First number(Decimal) is the count of interface names. 
 * ex: 10 means that srdsIfNameAllComp have ten interface names.
 */
static int init_srds_if_name_all_comp(SRDS_IF_NAME_ALL_COMP_t *_if_name_all_comp)
{
	int i=0;
	char *token_str;
	const char *delim=",";

	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "%s=%s\n", BOOTARGS_SRDS_IF_NAME_ALL_COMP_STR, _if_name_all_comp->all_comp);
	token_str=strsep(&_if_name_all_comp->all_comp, delim);
	if(NULL==token_str){
		return -1;
	}
	_if_name_all_comp->cnt=simple_strtol(token_str, NULL, 10);

	/*row*/
	_if_name_all_comp->comp=(char**)vmalloc(sizeof(char*) * _if_name_all_comp->cnt);
	if(NULL==_if_name_all_comp->comp){
		return -1;
	}
	for(i=0;i<_if_name_all_comp->cnt;i++){
		token_str=strsep(&_if_name_all_comp->all_comp, delim);
		if(NULL==token_str){
			vfree(_if_name_all_comp->comp);
			return -1;
		}
		/*column*/
		_if_name_all_comp->comp[i]=(char*)vmalloc(sizeof(char)*(strlen(token_str)+1));
		strcpy(_if_name_all_comp->comp[i], token_str);
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "if_name_all_comp->comp[%d]=%s\n",i,_if_name_all_comp->comp[i]);
	}

	return 0;
}

/* when srdsEtherTypeAllComp in mi.conf is 3,NONE,LAN,WAN
 * srdsEtherTypeAllComp(Decimal cnt, string) represents the ethertype name can be selected, 
 * this is the fix value;
 * 00H=NONE, 
 * 01H=ether-LAN, 
 * 02H=ether-WAN, 
 * First number(Decimal) is the count of interface names. \
 */
static int init_ether_type_name_all_comp(ETHER_TYPE_NAME_ALL_COMP_t *_ether_type_name_all_comp)
{
	int i=0;
	char *token_str;
	const char *delim=",";

	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "%s=%s\n", BOOTARGS_SRDS_IF_NAME_ALL_COMP_STR, _ether_type_name_all_comp->all_comp);
	token_str=strsep(&_ether_type_name_all_comp->all_comp, delim);
	if(NULL==token_str){
		return -1;
	}
	_ether_type_name_all_comp->cnt=simple_strtol(token_str, NULL, 10);

	/*row*/
	_ether_type_name_all_comp->comp=(char**)vmalloc(sizeof(char*) * _ether_type_name_all_comp->cnt);
	if(NULL==_ether_type_name_all_comp->comp){
		return -1;
	}
	for(i=0;i<_ether_type_name_all_comp->cnt;i++){
		token_str=strsep(&_ether_type_name_all_comp->all_comp, delim);
		if(NULL==token_str){
			vfree(_ether_type_name_all_comp->comp);
			return -1;
		}
		/*column*/
		_ether_type_name_all_comp->comp[i]=(char*)vmalloc(sizeof(char)*(strlen(token_str)+1));
		strcpy(_ether_type_name_all_comp->comp[i], token_str);
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "if_name_all_comp->comp[%d]=%s\n",i,_ether_type_name_all_comp->comp[i]);
	}

	return 0;
}

/* when srdsEtherPhyTypeAllComp in mi.conf is 7,NONE,an8811,A,B,C,D,E
 * srdsEtherPhyTypeAllComp(Decimal cnt, string) represents the ether phy name can be selected, 
 * this is the fix value;
 * 00H=NONE, 
 * 01H=an8811, 
 * 02H=B, 
 * 02H=C, 
 * 02H=D, 
 * 02H=E, 
 * First number(Decimal) is the count of interface names. \
 */
static int init_ether_phy_name_all_comp(ETHER_PHY_NAME_ALL_COMP_t *_ether_phy_name_all_comp)
{
	int i=0;
	char *token_str;
	const char *delim=",";

	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "%s=%s\n", BOOTARGS_ETHER_PHY_NAME_ALL_COMP_STR, _ether_phy_name_all_comp->all_comp);
	token_str=strsep(&_ether_phy_name_all_comp->all_comp, delim);
	if(NULL==token_str){
		return -1;
	}
	_ether_phy_name_all_comp->cnt=simple_strtol(token_str, NULL, 10);

	/*row*/
	_ether_phy_name_all_comp->comp=(char**)vmalloc(sizeof(char*) * _ether_phy_name_all_comp->cnt);
	if(NULL==_ether_phy_name_all_comp->comp){
		return -1;
	}
	for(i=0;i<_ether_phy_name_all_comp->cnt;i++){
		token_str=strsep(&_ether_phy_name_all_comp->all_comp, delim);
		if(NULL==token_str){
			vfree(_ether_phy_name_all_comp->comp);
			return -1;
		}
		/*column*/
		_ether_phy_name_all_comp->comp[i]=(char*)vmalloc(sizeof(char)*(strlen(token_str)+1));
		strcpy(_ether_phy_name_all_comp->comp[i], token_str);
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "_ether_phy_name_all_comp->comp[%d]=%s\n",i,_ether_phy_name_all_comp->comp[i]);
	}

	return 0;
}


static int init_srds_port_name(char *srdsPortNameComp, IC_SRDS_t *ic_srds)
{
	int i=0, ret = 0;
	char *token_str;
	const char *delim=",";

	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "%s=%s\n", BOOTARGS_SRDS_PORT_NAME_ALL_COMP_STR, srdsPortNameComp);

	ret = 0;
	for(i = 0; i < ic_srds->srdsPortCnt; i++){
		token_str=strsep(&srdsPortNameComp, delim);
		if(NULL==token_str){
			ret = -1;
			break;
		}
		ic_srds->srds_port[i].name=(char*)vmalloc(sizeof(char)*(strlen(token_str)+ sizeof(SRDS_PORT_NAME_PREFIX_STR) +1));
		strcpy(ic_srds->srds_port[i].name, SRDS_PORT_NAME_PREFIX_STR);
		strcat(ic_srds->srds_port[i].name, token_str);
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "ic_srds->srds_port[%d].name=%s\n",i, ic_srds->srds_port[i].name);
	}

	if(-1 == ret) {
		for(i = 0; i < ic_srds->srdsPortCnt; i++){
			if(NULL != ic_srds->srds_port[i].name) {
				vfree(ic_srds->srds_port[i].name);
			}
		}
	}

	return 0;
}

/*  
 * When srdsPortCombo in mi.conf is 05,00,01,02,03,04,04,00,01,02,03,03,00,02,05,03,00,03,06,03,00,07,09
 * srdsPortCombo(Hexmal) represents the interface selections supported by each serdes port,
 * this is the fix value
 * 05,00,01,02,03,04 is the serdes port1, 05H means the serdes port1 interface count, 
 * 00,01,02,03,04 is serdes interface combination, and it represent in srdsIfNameAllComp.
 * when srdsIfNameAllComp=10,NONE,XFI,USXGMII,HSGMII,PON,PCIE_G3_1LANE,PCIE_G2_1LANE,USB3.0,USB2.0_only,HSGMII+USB2.0
 * 00H is NONE, 01H is XFI, 02H isUSXGMII, 03H is HSGMII, 04H is PON;
 * therefore that serdes port1 can support interface=NONE/XFI/USXGMII/HSGMII;
 */
static int parse_srds_if_combo(SRDS_PORT_t *port)
{
	int i=0;
	char *token_str;
	const char *delim=",";

	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "%s=%s\n", BOOTARGS_SRDS_PORT_COMBO_STR, ic_srds.srdsPortCombo);
	token_str=strsep(&ic_srds.srdsPortCombo, delim);
	if(NULL == token_str){
		return -1;
	}
	port->if_combo_cnt=simple_strtol(token_str, NULL, 16);
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "port->if_combo_cnt=%xH\n",port->if_combo_cnt);
	
	port->if_combo=(uint8_t*) vmalloc(sizeof(uint8_t)*port->if_combo_cnt);
	if(NULL == port->if_combo){
		return -1;
	}

	for(i=0; i<port->if_combo_cnt; i++){
		token_str=strsep(&ic_srds.srdsPortCombo, delim);
		if(NULL==token_str){
			return -1;
		}
		port->if_combo[i]=simple_strtol(token_str, NULL, 16);
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "port->if_combo[%d]=%xH\n",i,port->if_combo[i]);
	}

	return 0;
}

/* the first digit(Hexmal) of the first port, 1H is for phy-type setting;
 * the second digit(Hexmal) of the first port, 2H is for ether-type setting;
 * the third digit(Hexmal) of the first port, 3H is for interface setting
 * rfb_cfg=123,112,001,001,000,aab,bbb
 */
static int parse_srds_parm(SRDS_PORT_t *port, uint8_t srdsParmCnt)
{
	int i=0;
	char *token_str;
	const char *delim=",";
	char param_type[2]={0};
	
	SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "rfb_cfg=%s\n",rfb_cfg_cur);
	token_str=strsep(&rfb_cfg_cur, delim);
	if(NULL==token_str){
		return -1;
	}

	port->srds_parm=(SRDS_PARM_t*) vmalloc(srdsParmCnt*sizeof(SRDS_PARM_t));
	if(NULL==port->srds_parm){
		return -1;
	}

	for(i=0;i<srdsParmCnt;i++){
		param_type[0]=token_str[i];
		param_type[1]='\0';
		port->srds_parm[i].type=simple_strtol(param_type, NULL, 16);
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_TRACE, "port->srds_parm[%d].type=%xH\n",i,port->srds_parm[i].type);
	}

	return 0;
}

static int parse_rfb_cfg(void)
{

	rfb_cfg_cur=(char*)vmalloc(sizeof(char)*(strlen(rfb_cfg_ori)+1));
	if(NULL==rfb_cfg_cur){
		return -1;
		SERDES_MSG(SERDES_CFG_DBG_LEVEL_ERROR, "rfb_cfg malloc failed!\n");
	}

	strcpy(rfb_cfg_cur, rfb_cfg_ori);

	return 0;
}
