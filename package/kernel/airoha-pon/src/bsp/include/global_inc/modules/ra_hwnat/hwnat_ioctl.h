/*
    Module Name:
    hwnat_ioctl.h

    Abstract:

    Revision History:
    Who         When            What
    --------    ----------      ----------------------------------------------
    Name        Date            Modification logs
    Steven Liu  2006-10-06      Initial version
*/

#ifndef	__HW_NAT_IOCTL_H__
#define	__HW_NAT_IOCTL_H__
#define HW_NAT_ADD_ENTRY    		(0x01)
#define HW_NAT_DUMP_CACHE_ENTRY    	(0x02)
#define HW_NAT_DUMP_ENTRY    		(0x03)
#define HW_NAT_GET_ALL_ENTRIES 		(0x04)
#define HW_NAT_BIND_ENTRY		(0x05)
#define HW_NAT_UNBIND_ENTRY		(0x06)
#define HW_NAT_INVALID_ENTRY		(0x07)
#define HW_NAT_DEBUG	   		(0x08)

/*HNAT QOS*/
#define HW_NAT_GET_AC_CNT		(0x09)
#define HW_NAT_SET_MTU  		(0x0a)
#define HW_NAT_SET_FPORT_MTU  	(0x0b)
#define PPE_UNKNOW_MC_CNT         (0xe)
#define HW_NAT_QOS_TRTCM_RATE		(0x0f)
#define HW_NAT_WAN_ACNT                 (0x10)
#define HW_NAT_TRAFFIC_QOS_FLOW_CNT	(0x11)
#define HW_NAT_TRAFFIC_QOS_FLOW_RATE	(0x12)
#define HW_NAT_AWIFI_CNT		(0x13)
#define HW_NAT_LAN_TCP_BLACKLIST	(0x14)
#define HW_NAT_WAN_RATELIMIT        (0X15)
#define HW_NAT_BIND_THRESHOLD		(0x16)
#define HW_NAT_MAX_ENTRY_LMT		(0x17)
#define HW_NAT_RULE_SIZE		(0x18)
#define HW_NAT_KA_INTERVAL		(0x19)
#define HW_NAT_UB_LIFETIME		(0x1a)
#define HW_NAT_BIND_LIFETIME		(0x1b)
#define HW_NAT_BIND_DIRECTION		(0x1c)
#define HW_NAT_MULTICAST_EN		(0x1d)
#define HW_NAT_SET_IP_CHK		(0x1e)
#define HW_NAT_REG_DUMP			(0x1f)
#define HW_NAT_TAB_CLEAN			(0x20)
#define HW_NAT_FOE_REMOVE             (0x21)
#define HW_NAT_SNOOPING_EN             (0x22)
#define HW_NAT_L2BR_ETHTYPE_EN             (0x23)
#define HW_NAT_CLEAN_LAN			(0x24)
#define HW_NAT_CLEAN_WAN			(0x25)
#define HW_NAT_DUMP_CACHE_DATA             (0x26)
#define HW_NAT_CLEAR_CACHE             (0x27)
#define PPE_COMMAND_TLS         (0x28)
#define PPE_COMMAND_METER       (0x29)
#define PPE_COMMAND_TRTCM       (0x2a)
#define PPE_COMMAND_ACCOUNT     (0x2b)
#define PPE_COMMAND_CACHE       (0x2c)
#define PPE_COMMAND_HASH        (0x2d)
#define PPE_COMMAND_LOCAL_IN       (0x2e)
#define PPE_COMMAND_METER_GROUP    (0x2f)
#define PPE_COMMAND_METER_FLOW     (0x30)
#define PPE_COMMAND_TRTCM_GROUP    (0x31)
#define PPE_COMMAND_TRTCM_FLOW     (0x32)
#define PPE_COMMAND_ACCOUNT_STATS  (0x33)
#define PPE_COMMAND_BLACK_IP_ADD	(0x34)
#define PPE_COMMAND_BLACK_IP_DELETE	(0x35)
#define PPE_COMMAND_BLACK_IP_DELETE_SIGN  (0x36)
#define PPE_COMMAND_L2B_ETHTYPE_CLEAN   (0x37)
#define PPE_COMMAND_MULHIGHPRI  (0x38)
#define PPE_COMMAND_FAST_RULE  (0x39)
#define HW_NAT_UN_MULTI_DROP_BY_INDEX (0x3a)
#define HW_NAT_QUERY_MULTI_DROP_LIST (0x3b)
#define PPE_COMMAND_BLACK_IP_PROTOCOL_PORT	(0x3c)
#define PPE_COMMAND_MUL_STAT        (0x3d)
#define PPE_COMMAND_DLF_ALL_FLAG                  (0x3e)
#define PPE_COMMAND_UNKNOWN_MUL_ALL_FLAG          (0x3f)
#define PPE_COMMAND_MIRROR_EN  	(0x40)
#define PPE_COMMAND_WAN_MTU  	(0x41)
#define PPE_COMMAND_SET_NPTV6_PREFIX        (0x42)
#define PPE_COMMAND_NPTV6_MODE        (0x43)
#define PPE_CLEAN_UNI_ENTRY 		(0x44)
#define PPE_COMMAND_PPE2 		(0x45)
#define PPE_COMMAND_VXLAN_HASH 		(0x46)
#define PPE_COMMAND_L2TP_HASH 		(0x47)
#define HW_NAT_ENABLE_STATUS        (0x48)


#define HW_NAT_DEVNAME			"hwnat0"
#define HW_NAT_MAJOR			(220)
#define TLS_CFG_MAX     6
#define TLS_VIDS_MAX    16
#define TLS_TAGS_MAX    5

#define TLS_SP_QDMA1    0
#define TLS_SP_GDMA1    1
#define TLS_SP_GDMA2    2
#define TLS_SP_QDMA2    5
#define TLS_SP_GSW_P0   8
#define TLS_SP_GSW_P1   9
#define TLS_SP_GSW_P2   10
#define TLS_SP_GSW_P3   11

#define TLS_CONCATENATE_AND 1 /* used only for mode TLS_MODE_CTAG_NON */
#define TLS_CONCATENATE_OR  0 /* used for all modes except TLS_MODE_CTAG_NON */

#define TLS_MODE_CMD_UNI        "uni"
#define TLS_MODE_CMD_UUTAG      "un-tag"
#define TLS_MODE_CMD_PRI_TAG    "pri-tag"
#define TLS_MODE_CMD_CTAG       "c-tag"
#define TLS_MODE_CMD_STAG_SIN   "s-tag-sin"
#define TLS_MODE_CMD_DBL_TAG    "dbl-tag"
#define TLS_MODE_CMD_CTAG_NON   "non-c-tag"
#define TLS_MODE_CMD_STAG_SPE   "s-tag-spe"
#define TLS_MODE_CMD_MAX        8
    
#define TLS_SPORT_CMD_LAN0      "lan0"
#define TLS_SPORT_CMD_LAN1      "lan1"
#define TLS_SPORT_CMD_LAN2      "lan2"
#define TLS_SPORT_CMD_LAN3      "lan3"
#define TLS_SPORT_CMD_WAN       "wan"
#define TLS_SPORT_CMD_MAX       5

#define TLS_MA_CMD_NONTLS       "non-tls"    
#define TLS_MA_CMD_DROP         "drop"
#define TLS_MA_CMD_MAX          2

#define TLS_NOT_FOUND           "not found"

#define METER_QUEUE_MAX         8
#define METER_GROUP_MAX         63
#define TRTCM_GROUP_MAX         31
#define VID_MAX                 4096

#define METER_SCALE_K       (1000)
#define METER_SCALE_M       (METER_SCALE_K<<10)
#define METER_SCALE_G       (METER_SCALE_M<<10)

#define METER_PKT_RATE_BASE 2047
#define METER_SCALE_10      10
#define METER_SCALE_100     100
#define METER_SCALE_1000    1000

#define BYTE_STRING         "byte"
#define PACKET_STRING       "packet"
#define K_STRING            "K"
#define M_STRING            "M"
#define G_STRING            "G"
#define UP_STRING           "up"
#define DOWN_STRING         "down"
#define NULL_STRING         ""
#define YES_STRING          "yes"
#define NO_STRING           "no"

#define CACHE_MODE_MAX      4
#define HASH_MODE_MAX       4
#define HASH_BITSHIFT_MAX   16

#define PROTO_IPV4 4
#define PROTO_IPV6 6

/* EN7523 IFC index is 6 bit */
#define IFC_MAX_IDX (0xff)

#define FOE_ENTRY_NUM_INVALID 0xFFFFFFFE

enum {
    PPE_METHOD_ADD = 1,
    PPE_METHOD_DEL,
    PPE_METHOD_SHOW,
    PPE_METHOD_GET,
    PPE_METHOD_GET_ALL,
};

enum tlsMode
{
    TLS_MODE_SPORT = 0,
    TLS_MODE_UNTAG,
    TLS_MODE_PRI_TAG,
    TLS_MODE_CTAG,
    TLS_MODE_STAG_SIN,
    TLS_MODE_DBL_TAG,
    TLS_MODE_CTAG_NON,
    TLS_MODE_STAG_SPE,
    TLS_MODE_MAX
};

enum {
    TLS_MISS_NON_TLS = 0,
    TLS_MISS_DROP    
};

enum meterModeEnum {
    METER_MODE_BYTE = 0,
    METER_MODE_PACKET
};

enum meterScaleEnum {
    METER_SCALE_KBPS = 0,
    METER_SCALE_MBPS,
    METER_SCALE_GBPS,
    METER_SCALE_MAX
};
/* hwnat ppe api define up 0, down 1*/
enum meterDirEnum {
    METER_DIR_UP = 0,
    METER_DIR_DOWN
};

/*fe resource manager define up 1, down 0*/
enum FE_meterDirEnum {
    DOWNSTREAM = 0,
    UPSTREAM
};

enum {
    METER_HANDLE_BY_GROUP = 0,
    METER_HANDLE_BY_FLOW    
};

enum {
    CACHE_METHOD_ENABLE = 0,
    CACHE_METHOD_DISABLE,
    CACHE_METHOD_SET_MODE,
    CACHE_METHOD_CLEAR,
    CACHE_METHOD_SHOW,
};

enum {
    PPE_CACHE_TYPE_VALID = 0,
    PPE_CACHE_TYPE_INVALID,
    PPE_CACHE_TYPE_LOCKED,
    PPE_CACHE_TYPE_ALL,
    PPE_CACHE_TYPE_LINECNT,
    PPE_CACHE_TYPE_NUM
};

enum {
    HASH_METHOD_SET_MODE = 0,
    HASH_METHOD_SET_BITSHIFT,
    HASH_METHOD_SET_TLS,
    HASH_METHOD_SET_L2B,
};

enum {
    LOCAL_IN_METHOD_ENABLE = 0,
    LOCAL_IN_METHOD_DISABLE,    
    LOCAL_IN_METHOD_ADD_DPORT,    
    LOCAL_IN_METHOD_ADD_ALL,
    LOCAL_IN_METHOD_DEL_DPORT,    
    LOCAL_IN_METHOD_DEL_ALL,
    LOCAL_IN_METHOD_SHOW,
};

enum {
    WAN_RATELIMIT_METHOD_SET_RATE = 0,
    WAN_RATELIMIT_METHOD_SET_MTU,
    WAN_RATELIMIT_METHOD_SET_THRESHOLD,
};

enum actionEnum {
    PPE_SET = 0,
    PPE_GET,
};

enum {
    PPE_METHOD_READ = 0,
    PPE_METHOD_CLEAR,
};

enum countMethod{
    PPE_COUNT_READ = 0,
    PPE_COUNT_CLEAR,
};

enum {
	PPE_BLACK_IP_PORT_PROTOCOL_ADD = 0,
	PPE_BLACK_IP_PORT_PROTOCOL_DELETE,
	PPE_BLACK_IP_PORT_PROTOCOL_SHOW,
	PPE_BLACK_IP_PORT_PROTOCOL_DELETE_ALL,
};
enum {
	FAST_RULE_METHOD_ENABLE = 0,		
	FAST_RULE_METHOD_DISABLE,
	FAST_RULE_METHOD_SET_MARK_BIT,
	FAST_RULE_METHOD_SHOW,
	FAST_RULE_METHOD_SET_MARK_ENABLE,
	FAST_RULE_METHOD_SET_REVERSE_ENABLE,
};

struct cmdVal {
    char *cmd;
    unsigned int val;
};

struct stringVal {
    char *str;
    unsigned int val;
};

struct ppe_tls_cfg {
    unsigned int resv   :14;
    unsigned int retVal :1; /* 0:success, 1: fail */
    unsigned int method :2; /* command method from user */
    unsigned int tags   :3; /* number of tags (VIDs) */
    unsigned int uni    :4; /* source port */
    unsigned int mode   :3; /* (TLS) mode */
    unsigned int rng    :1; /* (tag) range*/
    unsigned int tagEn  :1; /* tag enable */
    unsigned int ma     :1; /* miss action (non-TLS action) */
    unsigned int cm     :1; /* concatenate mode */
    unsigned int en     :1; /* enable */

    unsigned short vids[TLS_TAGS_MAX];
};

struct ppe_meter_cfg {
    unsigned int trtcm      :1;
    unsigned int grpOrFlow  :1; /* 0: delete/show by group, 1: delete/show by flow(vlan,dir) */
    unsigned int method     :3; /* command method from user */
    unsigned int dir        :1; /* 0: upstream, 1:downstream */
    unsigned int vlan       :16; /* including DEI, Pbits, and vlanID */
    unsigned int queue      :4;	/* qos queue index;0xf:not care qos queue*/
    unsigned int scale      :2; /* 0:Kbps, 1:Mbps, 2:Gbps (K is 1000, M is 1000x1024, G is 1000x1024x1024) */
    unsigned int scale2     :2; /* in TRTCM, scale for pir, scale2 for cir */
    unsigned int mode       :1; /* 0:byte, 1:packet */
    unsigned int group      :6; /* 0~62 for Meter, 0~30 for TRTCM */

    unsigned int resv       :28;
    unsigned int retVal     :1; /* 0:success, 1: fail */
    unsigned int addFlowOnly:2; /* 0:add Meter & flow, 1: add flow only, 2: add meter only */
    unsigned int enable     :1; /* 0:disable, 1:enable */

    unsigned int rate; /*  16K ~ 2G bps for byte mode, 1 ~ 2047000 fps for packet mode */
    unsigned int rate2; /* in TRTCM, rate for pir, rate2 for cir */
};

struct ppe_account_cfg {
    unsigned int resv       :4;
    unsigned int retVal     :1; /* 0:success, 1: fail */
    unsigned int grpOrFlow  :1; /* 0: delete/show by group, 1: delete/show by flow(vlan,dir) */
    unsigned int method     :3; /* command method from user */
    unsigned int dir        :1; /* 0: upstream, 1:downstream */
    unsigned int vlan       :16; /* including DEI, Pbits, and vlanID */
    unsigned int group      :6; /* 0~62 */

    unsigned int byteCntH; /* byte counter high 32bits */
    unsigned int byteCntL; /* byte counter low 32bits */
    unsigned int pktCntH;  /* packet counter */
};

struct ppe_cache_cfg {
    unsigned int resv       :6;
    unsigned int retVal     :1; /* 0:success, 1: fail */
    unsigned int showType   :3; /* 0:valid, 1:invalid, 2:locked, 3:all, 4:lineCnt */
    unsigned int method     :4; /* command method from user */
    unsigned int foeIdx     :16; /* ppe foe idx */
    unsigned int mode       :2; /* 0~3 (cache hash mode) */
};

struct ppe_hash_cfg {
    unsigned int resv           :18;
    unsigned int retVal         :1; /* 0:success, 1: fail */
    unsigned int l2bSmacEn      :1; /* 0:disable, 1:enable (SMAC hash when L2B) */
    unsigned int tlsEtypeDis    :2; /* 2:enable, 3:disable, 0~1:dont care (including EthType and DSCP hash when TLS) */
    unsigned int tlsPbitsDis    :2; /* 2:enable, 3:disable, 0~1:dont care (VLAN Pbits hash when TLS) */
    unsigned int tlsDmacDis     :2; /* 2:enable, 3:disable, 0~1:dont care (DMAC hash when TLS) */
    unsigned int method         :4; /* command method from user */
    unsigned int bitShift       :4; /* 0~15 (foe hash bit shift) */
    unsigned int mode           :2; /* 0:hash seed, 1:(A&B)|(~A&C), 2:(B^(A&~C)), 3:CRC32 hash mode */
};

struct ppe_local_in_cfg {
    unsigned int resv           :23;
    unsigned int retVal         :1; /* 0:success, 1: fail */
    unsigned int method         :4; /* command method from user */
    unsigned int dport_num  :4;
    unsigned short *dport_ptr;
};

struct ppe_del_l2betype_cfg {
	unsigned int etype;
	unsigned int port;
	unsigned char retVal;/* 0:success, 1: fail */
};

struct ppe_mulhigh_cfg{
	unsigned int data;
	unsigned char retVal;
};
enum hwnat_status {
	HWNAT_SUCCESS = 0,
	HWNAT_FAIL = 1,
	HWNAT_ENTRY_NOT_FOUND = 2
};

struct hwnat_tuple {
	unsigned short hash_index;
	unsigned char pkt_type;

	/* egress layer2 */
	unsigned char dmac[6];
	unsigned char smac[6];
	unsigned char in_dmac[6];
	unsigned char in_smac[6];
    unsigned short in_vlan;
    unsigned int hsk_l2;
    unsigned char channel;
    unsigned char new_l2b;
    unsigned int hsk_mc;
    unsigned short in_gre_call_id;
    unsigned short out_gre_call_id;
	unsigned short stag;
	unsigned char  vlan_layer;
	unsigned short vlan1;
	unsigned short vlan2;
	unsigned short pppoe_id;

	/* ingress layer3 */
	unsigned int ing_sipv4;
	unsigned int ing_dipv4;

	unsigned int ing_sipv6_0;
	unsigned int ing_sipv6_1;
	unsigned int ing_sipv6_2;
	unsigned int ing_sipv6_3;

	unsigned int ing_dipv6_0;
	unsigned int ing_dipv6_1;
	unsigned int ing_dipv6_2;
	unsigned int ing_dipv6_3;

	/* egress layer3 */
	unsigned int eg_sipv4;
	unsigned int eg_dipv4;

	unsigned int eg_sipv6_0;
	unsigned int eg_sipv6_1;
	unsigned int eg_sipv6_2;
	unsigned int eg_sipv6_3;

	unsigned int eg_dipv6_0;
	unsigned int eg_dipv6_1;
	unsigned int eg_dipv6_2;
	unsigned int eg_dipv6_3;

	unsigned char prot;

	// ingress layer4
	unsigned short ing_sp;
	unsigned short ing_dp;

	// egress layer4
	unsigned short eg_sp;
	unsigned short eg_dp;

	unsigned char dst_port;
	unsigned char eg_dscp;
	unsigned int  ipv6_flowlabel;
	unsigned char is_udp;
	unsigned char tunnel_dscp; //IPv6 TRFC
	unsigned char tunnel_TTL; //IPv6 HPOL
	unsigned char tunnel_IPv4_flag;
	unsigned char RMT;
	unsigned char ag;
	unsigned char mg;
	unsigned char tsid;
	unsigned char qid;
	unsigned char nbq;
	unsigned char sta;
	unsigned char vpm;
	unsigned char pcp;
	unsigned int info2;
	unsigned char hash_index_shift;

	unsigned short ifc_idx;
	unsigned short ttl;
	unsigned char cnt_grp2;
	unsigned char dpi;
    unsigned char ppe2;
	enum hwnat_status result;
};

enum hwnat_debug_id{
	HWNAT_DBG_ID_SET_DBGLVL = 0,
	HWNAT_DBG_ID_SET_BINDFAILIDX,	
	HWNAT_DBG_ID_SET_FREEIDX,	
	HWNAT_DBG_ID_SET_DROPIDX,
	HWNAT_DBG_ID_SET_DUMP_PARSEINFO,
	HWNAT_DBG_ID_MAX_NUM,
};

struct hwnat_debug_args{
	unsigned int id;
	unsigned int dbg_lvl;
	unsigned int bind_fail_idx;	
	unsigned int free_idx;
	unsigned int drop_idx;
	unsigned int parse_dump_enable;
};

struct multi_drop_args{
	unsigned int entry_index;
	unsigned char foe_ai;
	unsigned int foe_magic;
	unsigned char proto; //ipv4 or ipv6
	unsigned char grp_addr[16];
};
struct hwnat_args {
	unsigned int debug:3;
	unsigned int bind_dir:2;	/* 0=upstream, 1=downstream, 2=bi-direction */
	unsigned int entry_state:2;	/* invalid=0, unbind=1, bind=2, fin=3 */
	enum hwnat_status result;
	unsigned int entry_num:16;
	unsigned int num_of_entries:16;
	unsigned char cache_type;
	struct multi_drop_args multi_drop;	
	union{
#ifdef CONFIG_COMPAT
		compat_uptr_t compat_ptr;
#endif
		struct hwnat_debug_args* p_dbg_arg;
		struct hwnat_tuple *entries;
	};
	unsigned int hwnat_enable_status;  /*0=off, 1=on*/
};


/*hnat qos*/
struct hwnat_qos_args{
	unsigned int enable:1;
	unsigned int up:3;
	unsigned int weight:3;	/*UP resolution */
	unsigned int dscp:6;
	unsigned int dscp_set:3;
	unsigned int vpri:3;
	unsigned int ac:2;
	unsigned int mode:2;
	unsigned int weight0:4;	/*WRR 4 queue weight */
	unsigned int weight1:4;
	unsigned int weight2:4;
	unsigned int weight3:4;
	enum hwnat_status result;
};

/*hnat config*/
struct hwnat_config_args {
	unsigned char enable:1;
	unsigned char ip_chk_type:1;
	unsigned char snoopingEn:1;
	unsigned char L2ethtypeEN:1;
	unsigned char resv:4;
	unsigned int bind_threshold:16;
	unsigned int foe_full_lmt:14;
	unsigned int foe_half_lmt:14;
	unsigned int foe_qut_lmt:14;
	unsigned int pre_acl:9;
	unsigned int pre_meter:9;
	unsigned int pre_ac:9;
	unsigned int post_meter:9;
	unsigned int post_ac:9;
	unsigned int foe_tcp_ka:8;	/*unit 4 sec */
	unsigned int foe_udp_ka:8;	/*unit 4 sec */
	unsigned int foe_unb_dlta:8;	/*unit 1 sec */
	unsigned int foe_tcp_dlta:16;	/*unit 1 sec */
	unsigned int foe_udp_dlta:16;	/*unit 1 sec */
	unsigned int foe_fin_dlta:16;	/*unit 1 sec */
	unsigned int foe_ntu_dlta:16;	/*unit 1 sec */
	enum hwnat_status result;
};

struct hwnat_ac_args {
	unsigned char ag_index;
	unsigned int ag_byte_cnt;
	unsigned int ag_pkt_cnt;
	enum hwnat_status result;
};

struct hwnat_tcp_lanPort {
	unsigned short lanPort;
	unsigned short enable;
};

struct hwnat_lanip_cnt {
	unsigned int ipaddr;
	unsigned long long cnt_byte_up;
	unsigned long long cnt_byte_down;
};

struct hwnat_tr_qos_flow_cnt {
	unsigned char direction;
	unsigned char flowid;
	unsigned long long cntbyte;
};

struct hwnat_tr_qos_flow_rate {
	unsigned char direction;
	unsigned char action;
	unsigned char flowid;
	unsigned int ratelimit;
};

struct black_ip_cnt {
	unsigned int ip;
	unsigned char u6_addr8[16];
	unsigned int mask;
	unsigned int type;
	unsigned int sign;
	unsigned int ipv6_mode;
};

struct black_ip_port_portocol{
	unsigned int retVal; /* 0:success, 1: fail */
	unsigned int action;
	unsigned int ip; /*0xFFFFFFFF:NULL*/
	unsigned int port; /*0~65536, 65536:NULL*/
	unsigned int protocol; /*1:TCP, 2:UDP, 3:ICMP, 4:ALL*/
};

struct hwnat_trqos_trtcm_rate {
	unsigned char enable;
	unsigned char flowid;
	unsigned char action;
	unsigned int ratelimit_assure;
	unsigned int ratelimit_max;
};

struct hwnat_byte_cnt_args {
	unsigned int count_pkt;
	unsigned long long count_byte;
	enum countMethod method;
};

struct hwnat_fast_rule_args {
	unsigned int method;
	unsigned char value;
};


struct hwnat_wan_ratelimit_args {
	unsigned int method;
	unsigned char wanid;
	unsigned char direction;
	unsigned int ratelimit;

	unsigned char wan_type;
	unsigned int mtu;
	unsigned int threshold;
};

struct hwnat_CDS_all_ratelimit {
	unsigned char dlf_flag;
	unsigned char UN_Mul_flag;
	unsigned char retVal;/* 0:success, 1: fail */
};

typedef enum
{
    PPE_MIRROR_FUNC_ID_ENABLE,
    PPE_MIRROR_FUNC_ID_DISABLE,
    PPE_MIRROR_FUNC_ID_MODE,
    PPE_MIRROR_FUNC_ID_ENABLE_CONFIG,
    PPE_MIRROR_FUNC_ID_MODE_CONFIG,
    PPE_MIRROR_FUNC_ID_MAX,
}PPE_MIRROR_FUNC_ID;

struct ppe_mirror_args {
    PPE_MIRROR_FUNC_ID func_id;
    union
    {
        int port;
        int mode;
        struct{
            int enable;
            int fport;
            int nbq;
            int sptag;
        }enable_config;
        struct{
            int mode;
            int mirror_dst;
            int mirror_src;
            int mirror_mask;
        }mode_config;
    };
};

struct ppe_wan_mtu_args {
	int wan_idx;
	unsigned short wan_mtu;
};

enum hwnat_nptv6_prefix_id{
	HWNAT_NPTV6_PREFIX_ID_ADD = 0,
	HWNAT_NPTV6_PREFIX_ID_DEL,		
	HWNAT_NPTV6_PREFIX_ID_DEL_ALL,	
	HWNAT_NPTV6_PREFIX_ID_MAX_NUM,
};

struct hwnat_nptv6_prefix_args
{
	unsigned int function_id;
	unsigned char ifname[16];
	unsigned short s_prefix[8];	
	unsigned int s_prefix_len;
	unsigned short d_prefix[8];	
	unsigned int d_prefix_len;
};

enum hwnat_nptv6_mode_id{
	HWNAT_NPTV6_MODE_ID_SET = 0,
	HWNAT_NPTV6_MODE_ID_GET,	
	HWNAT_NPTV6_MODE_ID_MAX_NUM,
};

struct hwnat_nptv6_mode_args
{
	unsigned int function_id;
	unsigned char mode;
};

struct hwnat_fport_mtu_args {
	int fport;
	unsigned short mtu;
};

enum hwnat_ppe2_cmd_func_id{
	HWNAT_PPE2_CMD_ID_ENABLE = 0,
	HWNAT_PPE2_CMD_ID_DISABLE,	
	HWNAT_PPE2_CMD_ID_MAX_NUM,
};

struct hwnat_ppe2_cmd_args
{
	unsigned int function_id;
};

enum hwnat_vxlan_hash_cmd_func_id{
	HWNAT_VXLAN_HASH_CMD_ID_ENABLE = 0,
	HWNAT_VXLAN_HASH_CMD_ID_DISABLE,
	HWNAT_VXLAN_HASH_CMD_ID_SHOW,
	HWNAT_VXLAN_HASH_CMD_ID_MAX_NUM,
};

struct hwnat_vxlan_hash_cmd_args
{
	unsigned int function_id;
};

enum hwnat_l2tp_hash_cmd_func_id{
	HWNAT_L2TP_HASH_CMD_ID_ENABLE = 0,
	HWNAT_L2TP_HASH_CMD_ID_DISABLE,
	HWNAT_L2TP_HASH_CMD_ID_SHOW,
	HWNAT_L2TP_HASH_CMD_ID_MAX_NUM,
};

struct hwnat_l2tp_hash_cmd_args
{
	unsigned int function_id;
};

int PpeRegIoctlHandler(void);
void PpeUnRegIoctlHandler(void);
int PpeGetAGCnt(struct hwnat_ac_args *opt3);
void PpeSetCacheEbl(int Ebl);
void PpeTrafficQosFlowCnt(struct hwnat_tr_qos_flow_cnt *opt);

#endif
