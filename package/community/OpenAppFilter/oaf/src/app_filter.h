#ifndef APP_FILTER_H
#define APP_FILTER_H

#define AF_VERSION "5.0.1"
#define AF_FEATURE_CONFIG_FILE "/etc/appfilter/feature.cfg"

#define MAX_PARSE_PKT_NUM 16
#define MIN_HTTP_DATA_LEN 16
#define MAX_APP_NAME_LEN 64
#define MAX_FEATURE_NUM_PER_APP 16 
#define MIN_FEATURE_STR_LEN 16
#define MAX_FEATURE_STR_LEN 128
#define MAX_HOST_URL_LEN 128
#define MAX_REQUEST_URL_LEN 128
#define MAX_FEATURE_BITS 16
#define MAX_POS_INFO_PER_FEATURE 16
#define MAX_FEATURE_LINE_LEN 256
#define MIN_FEATURE_LINE_LEN 16
#define MAX_URL_MATCH_LEN 64

//#define CONFIG_KERNEL_FUNC_TEST 1

#define HTTP_GET_METHOD_STR "GET"
#define HTTP_POST_METHOD_STR "POST"
#define HTTP_HEADER "HTTP"
#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"
#define MAC_ARRAY(a) (a)[0], (a)[1], (a)[2], (a)[3], (a)[4], (a)[5]
#define MAC_FMT "%02x:%02x:%02x:%02x:%02x:%02x"

#define AF_TRUE 1
#define AF_FALSE 0

#define AF_APP_TYPE(a) (a) / 1000
#define AF_APP_ID(a) (a) % 1000
#define MAC_ADDR_LEN      		6

#define HTTPS_URL_OFFSET		9
#define HTTPS_LEN_OFFSET		7

enum AF_FEATURE_PARAM_INDEX{
	AF_PROTO_PARAM_INDEX,
	AF_SRC_PORT_PARAM_INDEX,
	AF_DST_PORT_PARAM_INDEX,
	AF_HOST_URL_PARAM_INDEX,
	AF_REQUEST_URL_PARAM_INDEX,
	AF_DICT_PARAM_INDEX,
};


#define OAF_NETLINK_ID 29
#define MAX_OAF_NL_MSG_LEN 1024

enum E_MSG_TYPE{
	AF_MSG_INIT,
	AF_MSG_MAX
};

typedef struct af_msg{
	int action;
	void *data;
}af_msg_t;

struct af_msg_hdr{
    int magic;
    int len;
};

enum e_http_method{
	HTTP_METHOD_GET = 1,
	HTTP_METHOD_POST,
};
typedef struct http_proto{
	int match;
	int method;
	char *url_pos;
	int url_len;
	char *host_pos;
	int host_len;
	char *data_pos;
	int data_len;
}http_proto_t;

typedef struct https_proto{
	int match;
	char *url_pos;
	int url_len;
}https_proto_t;

typedef struct flow_info{
	struct nf_conn *ct; // Á¬½Ó¸ú×ÙÖ¸Õë
	u_int32_t src; 
	u_int32_t dst;
	int l4_protocol;
	u_int16_t sport;
	u_int16_t dport;
	unsigned char *l4_data;
	int l4_len;
	http_proto_t http;
	https_proto_t https;
	u_int32_t app_id;
	u_int8_t app_name[MAX_APP_NAME_LEN];
	u_int8_t drop;
	u_int8_t dir;
	u_int16_t total_len;
}flow_info_t;



typedef struct af_pos_info{
	int pos;
	unsigned char value;
}af_pos_info_t;

typedef struct af_feature_node{
	struct list_head  		head;
	u_int32_t app_id;
	char app_name[MAX_APP_NAME_LEN];
	char feature_str[MAX_FEATURE_NUM_PER_APP][MAX_FEATURE_STR_LEN];
	u_int32_t proto;
	u_int32_t sport;
	u_int32_t dport;
	char host_url[MAX_HOST_URL_LEN];
	char request_url[MAX_REQUEST_URL_LEN];
	int pos_num;
	af_pos_info_t pos_info[MAX_POS_INFO_PER_FEATURE];
}af_feature_node_t;

typedef struct af_mac_info {
    struct list_head   hlist;
    unsigned char      mac[MAC_ADDR_LEN];
}af_mac_info_t;

int af_register_dev(void);
void af_unregister_dev(void);
void af_init_app_status(void);
int af_get_app_status(int appid);
int regexp_match(char *reg, char *text);
void af_mac_list_init(void);
void af_mac_list_clear(void);
af_mac_info_t * find_af_mac(unsigned char *mac);
int is_user_match_enable(void);
extern int g_oaf_enable;

#endif
