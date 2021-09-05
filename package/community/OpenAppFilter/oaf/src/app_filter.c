
/*
	author: derry
	date:2019/1/10
*/
#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <net/tcp.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack.h>
#include <net/netfilter/nf_conntrack_acct.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/etherdevice.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include "app_filter.h"
#include "af_utils.h"
#include "af_log.h"
#include "af_client.h"
#include "af_client_fs.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("destan19@126.com");
MODULE_DESCRIPTION("app filter module");
MODULE_VERSION("3.0.1");
struct list_head af_feature_head = LIST_HEAD_INIT(af_feature_head);

DEFINE_RWLOCK(af_feature_lock);

#define feature_list_read_lock() 		read_lock_bh(&af_feature_lock);
#define feature_list_read_unlock() 		read_unlock_bh(&af_feature_lock);
#define feature_list_write_lock() 		write_lock_bh(&af_feature_lock);
#define feature_list_write_unlock()		write_unlock_bh(&af_feature_lock);


#define SET_APPID(mark, appid) (mark = appid)
#define GET_APPID(mark) (mark)

#if 0
static void show_feature_list(void)
{
	af_feature_node_t *n,*node;
	unsigned int count = 0;
	feature_list_read_lock();
	if(!list_empty(&af_feature_head)) { // handle qos
		list_for_each_entry_safe(node, n, &af_feature_head, head) {
			count ++;
			printk("[%d] id=%d appname:%s, dport:%d, host:%s, request:%s\n", 
				count,
				node->app_id, node->app_name,
				node->dport,node->host_url, node->request_url);
			int i;
			for (i = 0;i < node->pos_num;i++){
				printk("(%d:%x)-->",
					node->pos_info[i].pos,
					node->pos_info[i].value);
				
			}
			printk("\n----------------------------------------\n\n\n");
		}
	}
	feature_list_read_unlock();
}
static af_feature_node_t* af_find_feature(char *app_id) 
{
	af_feature_node_t *node;
	feature_list_read_lock();

	if (!list_empty(&af_feature_head)) {
		list_for_each_entry(node, &af_feature_head, head) {
			if (node->app_id == app_id){
				feature_list_read_unlock();
				return node;
			}
		}
	}
	feature_list_read_unlock();
	return NULL;
}
#endif



int __add_app_feature(int appid,
					char *name,
					int proto,
					int src_port,
					int dst_port,
					char *host_url,
					char *request_url,
					char *dict)
{
	af_feature_node_t *node = NULL;
	char *p = dict;
	char *begin = dict;
	char pos[32] = {0};
	int index = 0;
	int value = 0;
	node = kzalloc(sizeof(af_feature_node_t), GFP_KERNEL);
	if (node == NULL) {
		printk("malloc feature memory error\n");
		return -1;
	}
	else {
		node->app_id = appid;
		strcpy(node->app_name, name);
		node->proto = proto;
		node->dport = dst_port;
		node->sport = src_port;
		strcpy(node->host_url, host_url);
		strcpy(node->request_url, request_url);
		// 00:0a-01:11
		p = dict;
		begin = dict;
		index = 0;
		value = 0;
		
		while (*p++) {
			if (*p == '|'){
				memset(pos, 0x0, sizeof(pos));
				strncpy(pos, begin, p - begin);
				k_sscanf(pos, "%d:%x",&index, &value);
				begin = p + 1;
				node->pos_info[node->pos_num].pos = index;
				node->pos_info[node->pos_num].value = value;
				node->pos_num++;
			}
		}
		
		if (begin != dict) {
			strncpy(pos, begin, p - begin);
			k_sscanf(pos, "%d:%x",&index, &value);
			node->pos_info[node->pos_num].pos = index;
			node->pos_info[node->pos_num].value = value;
			node->pos_num++;
		}
		feature_list_write_lock();
		list_add(&(node->head), &af_feature_head);
		feature_list_write_unlock();
	}
	return 0;
}
//[tcp;;443;baidu.com;;]
int add_app_feature(int appid, char *name, char *feature)
{
	char proto_str[16] = {0};
	char src_port_str[16] = {0};
	
	char dst_port_str[16] = {0};
	char host_url[32] = {0};
	char request_url[128] = {0};
	char dict[128] = {0};
	int proto = IPPROTO_TCP;
	char *p = feature;
	char *begin = feature;
	int param_num = 0;
	int dst_port = 0;
	int src_port = 0;
	
	if (!name || !feature) {
		AF_ERROR("error, name or feature is null\n");
		return -1;
	}
	// tcp;8000;www.sina.com;0:get_name;00:0a-01:11

	while(*p++) {
		if (*p != ';')
			continue;
		
		switch(param_num){
		
		case AF_PROTO_PARAM_INDEX:
			strncpy(proto_str, begin, p - begin);
			break;
		case AF_SRC_PORT_PARAM_INDEX:
			strncpy(src_port_str, begin, p - begin);
			break;
		case AF_DST_PORT_PARAM_INDEX:
			strncpy(dst_port_str, begin, p - begin);
			break;
			
		case AF_HOST_URL_PARAM_INDEX:
			strncpy(host_url, begin, p - begin);
			break;
		
		case AF_REQUEST_URL_PARAM_INDEX:
			strncpy(request_url, begin, p - begin);
			break;
		}
		param_num ++;
		begin = p + 1;
	}
	if (AF_DICT_PARAM_INDEX != param_num && strlen(feature) > MIN_FEATURE_STR_LEN) {
		AF_ERROR("invalid feature:%s\n", feature);
		return -1;
	}
	strncpy(dict, begin, p - begin);

	if (0 == strcmp(proto_str, "tcp"))
		proto = IPPROTO_TCP;
	else if (0 == strcmp(proto_str, "udp"))
		proto = IPPROTO_UDP;
	else {
		AF_DEBUG("proto %s is not support\n", proto_str);
		return -1;
	}

	sscanf(src_port_str, "%d", &src_port);
	
	sscanf(dst_port_str, "%d", &dst_port);

	__add_app_feature(appid,
					name,
					proto,
					src_port,
					dst_port,
					host_url,
					request_url,
					dict);

	return 0;
}


void af_init_feature(char *feature_str)
{
	int app_id;
	char app_name[128] = {0};
	char feature_buf[MAX_FEATURE_LINE_LEN] = {0};
	char *p = feature_str;
	char *pos = NULL;	
	int len = 0;
	char *begin = NULL;
	char feature[MAX_FEATURE_STR_LEN];;

	if (strstr(feature_str,"#"))
		return;
	
	k_sscanf(feature_str, "%d%[^:]", &app_id, app_name);

	while(*p++) {
		if (*p == '['){
			pos = p + 1;
			continue;
		}
		if (*p == ']' && pos != NULL) {
			len = p - pos;
		}
	}

	if (pos && len)
		strncpy(feature_buf, pos, len);
	memset(feature, 0x0, sizeof(feature));
	p = feature_buf;
	begin = feature_buf;

	while(*p++){
		if (*p == ',') {
			memset(feature, 0x0, sizeof(feature));
			strncpy((char *)feature, begin, p - begin);	
			
			add_app_feature(app_id, app_name, feature);
			begin = p + 1;
		}	
	}
	if (p != begin){
		memset(feature, 0x0, sizeof(feature));
		strncpy((char *)feature, begin, p - begin);	
		add_app_feature(app_id, app_name, feature);
	}
}

void load_feature_buf_from_file(char **config_buf)
{
	struct inode *inode = NULL;
	struct file *fp = NULL;
	mm_segment_t fs;
	off_t size;
	fp = filp_open(AF_FEATURE_CONFIG_FILE, O_RDONLY, 0);

	if(IS_ERR(fp)) {
		printk("open feature file failed\n");
		return;
	}

	inode = fp->f_inode;
	size = inode->i_size;
	if (size == 0) {
		return;
	}
	*config_buf = (char *) kzalloc( sizeof(char) * size, GFP_KERNEL);
	if(NULL == *config_buf ) {
		AF_ERROR("alloc buf fail\n");
		filp_close(fp, NULL);
		return;
	}

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,7,19)
	fs = get_fs();
	set_fs(KERNEL_DS);
#endif
	
// 4.14rc3 vfs_read-->kernel_read
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,14,0)
	kernel_read(fp, *config_buf, size, &(fp->f_pos));
#else
	vfs_read(fp, *config_buf, size, &(fp->f_pos));
#endif

#if LINUX_VERSION_CODE <= KERNEL_VERSION(5,7,19)
	set_fs(fs);
#endif

	filp_close(fp, NULL);
}

int load_feature_config(void)
{
	char *feature_buf = NULL;
	char *p;
	char *begin;
	char line[MAX_FEATURE_LINE_LEN] = {0};
	AF_INFO("begin load feature config.....\n");
	load_feature_buf_from_file(&feature_buf);
	if (!feature_buf) {
		AF_ERROR("error, feature buf is null\n");
		return -1;
	}
	

	p = begin = feature_buf;
	while(*p++) {
		if (*p == '\n'){
			if (p - begin < MIN_FEATURE_LINE_LEN || p - begin > MAX_FEATURE_LINE_LEN ) {
				begin = p + 1;
				continue;
			}
			memset(line, 0x0, sizeof(line));
			strncpy(line, begin, p - begin);
			af_init_feature(line);
			begin = p + 1;
		}
	}
	if (p != begin) {
		if (p - begin < MIN_FEATURE_LINE_LEN || p - begin > MAX_FEATURE_LINE_LEN ) 
			return 0;
		memset(line, 0x0, sizeof(line));
		strncpy(line, begin, p - begin);
		af_init_feature(line);
		begin = p + 1;
	}
	if (feature_buf)
		kfree(feature_buf);
	return 0;
}

static void af_clean_feature_list(void)
{
	af_feature_node_t *node;
	feature_list_write_lock();
	while(!list_empty(&af_feature_head)) {
		node = list_first_entry(&af_feature_head, af_feature_node_t, head);
		list_del(&(node->head));
		kfree(node);
	}
	feature_list_write_unlock();
}


int parse_flow_base(struct sk_buff *skb, flow_info_t *flow) 
{
	struct tcphdr * tcph = NULL;
	struct udphdr * udph = NULL;
	struct nf_conn *ct = NULL;
	struct iphdr *iph = NULL;
	if (!skb) {
		return -1;
	}
	
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
	ct = (struct nf_conn *)skb->_nfct;
#else
	ct = (struct nf_conn *)skb->nfct;
#endif
	if (!ct) {
		return -1;
	}
	iph = ip_hdr(skb);
	if (!iph) {
		return -1;
	}
	flow->ct = ct;
	flow->src = iph->saddr;
	flow->dst = iph->daddr;
	flow->l4_protocol = iph->protocol;
	switch (iph->protocol) {
		case IPPROTO_TCP:
			tcph = (struct tcphdr *)(iph + 1);
			flow->l4_data = skb->data + iph->ihl * 4 + tcph->doff * 4;
			flow->l4_len =  ntohs(iph->tot_len) - iph->ihl * 4 - tcph->doff * 4;
			flow->dport = htons(tcph->dest);
			flow->sport = htons(tcph->source);
			return 0;
		case IPPROTO_UDP:
			udph = (struct udphdr *)(iph + 1);
			flow->l4_data = skb->data + iph->ihl * 4 + 8;
			flow->l4_len = ntohs(udph->len) - 8;
			flow->dport = htons(udph->dest);
			flow->sport = htons(udph->source);
			return 0;
		case IPPROTO_ICMP:
			break;
		default:
			return -1;
	}
	return -1;
}


int parse_https_proto(flow_info_t *flow) {
	int i ;
	short url_len = 0 ;
	char * p = flow->l4_data;
	int data_len = flow->l4_len;
	
	if (NULL == flow) {
		AF_ERROR("flow is NULL\n");
		return -1;
	}
	if (NULL == p || data_len == 0) {
		return -1;
	}
	if (!(p[0] == 0x16 && p[1] == 0x03 && p[2] == 0x01))
		return -1;
	
	for(i = 0; i < data_len; i++) {
		if(i + HTTPS_URL_OFFSET >= data_len) {
			return -1;
		}
		
		if(p[i] == 0x0 && p[i + 1] == 0x0 && p[i + 2] == 0x0 && p[i + 3] != 0x0) {
			// 2 bytes
			memcpy(&url_len , p + i + HTTPS_LEN_OFFSET, 2);
			if(ntohs(url_len) <= 0 || ntohs(url_len) > data_len) {
				continue ;
			}
			
			if(i + HTTPS_URL_OFFSET + ntohs(url_len) < data_len) {
				//dump_hex("https hex", p, data_len);
				flow->https.match = AF_TRUE;
				flow->https.url_pos = p + i + HTTPS_URL_OFFSET;
			//	dump_str("https url2", flow->https.url_pos, ntohs(url_len));
				flow->https.url_len = ntohs(url_len);
				return 0;              
			}
		}
	}
	return -1;
}


void parse_http_proto(flow_info_t *flow) 
{
	int i = 0;
	int start = 0;
	char *data = NULL;
	int data_len = 0;
	if (!flow) {
		AF_ERROR("flow is null\n");
		return;
	}
	if (flow->l4_protocol != IPPROTO_TCP) {
		return;
	}

	data = flow->l4_data;
	data_len = flow->l4_len;
	if (data_len < MIN_HTTP_DATA_LEN) {
		return;
	}
	if (flow->sport != 80 && flow->dport != 80)
		return;
	for (i = 0; i < data_len; i++) {
		if (data[i] == 0x0d && data[i + 1] == 0x0a){
			if (0 == memcmp(&data[start], "POST ", 5)) {
				flow->http.match = AF_TRUE;
				flow->http.method = HTTP_METHOD_POST;
				flow->http.url_pos = data + start + 5;
				flow->http.url_len = i - start - 5;
				//dump_str("get request", flow->http.url_pos, flow->http.url_len);
			}
			else if(0 == memcmp(&data[start], "GET ", 4)) {
				flow->http.match = AF_TRUE;
				flow->http.method = HTTP_METHOD_GET;
				flow->http.url_pos = data + start + 4;
				flow->http.url_len = i - start - 4;
				//dump_str("post request", flow->http.url_pos, flow->http.url_len);
			}
			else if (0 == memcmp(&data[start], "Host:", 5) ){
				flow->http.host_pos = data + start + 6;
				flow->http.host_len = i - start - 6;
				//dump_str("host ", flow->http.host_pos, flow->http.host_len);
			}
			if (data[i + 2] == 0x0d && data[i + 3] == 0x0a){
				flow->http.data_pos = data + i + 4;
				flow->http.data_len = data_len - i - 4;
				break;
			}
			// 0x0d 0x0a
			start = i + 2; 
		}
	}
}

static void dump_http_flow_info(http_proto_t *http) {
	if (!http) {
		AF_ERROR("http ptr is NULL\n");
		return ;
	}
	if (!http->match)
		return;	
	if (http->method == HTTP_METHOD_GET){
		printk("Http method: "HTTP_GET_METHOD_STR"\n");
	}
	else if (http->method == HTTP_METHOD_POST) {
		printk("Http method: "HTTP_POST_METHOD_STR"\n");
	}
	if (http->url_len > 0 && http->url_pos){
		dump_str("Request url", http->url_pos, http->url_len);
	}

	if (http->host_len > 0 && http->host_pos){
		dump_str("Host", http->host_pos, http->host_len);
	}

	printk("--------------------------------------------------------\n\n\n");
}

static void dump_https_flow_info(https_proto_t *https) {
	if (!https) {
		AF_ERROR("https ptr is NULL\n");
		return ;
	}
	if (!https->match)
		return;	
	

	if (https->url_len > 0 && https->url_pos){
		dump_str("https server name", https->url_pos, https->url_len);
	}

	printk("--------------------------------------------------------\n\n\n");
}
static void dump_flow_info(flow_info_t *flow)
{
	if (!flow) {
		AF_ERROR("flow is null\n");
		return;
	}
	if (flow->l4_len > 0){
		AF_LMT_INFO("src="NIPQUAD_FMT",dst="NIPQUAD_FMT",sport: %d, dport: %d, data_len: %d\n", 
			NIPQUAD(flow->src), NIPQUAD(flow->dst),	flow->sport, flow->dport, flow->l4_len);
	}

	if (flow->l4_protocol == IPPROTO_TCP) {
		if (AF_TRUE == flow->http.match) {
			printk("-------------------http protocol-------------------------\n");
			printk("protocol:TCP , sport: %-8d, dport: %-8d, data_len: %-8d\n",
					flow->sport, flow->dport, flow->l4_len);
			dump_http_flow_info(&flow->http);
		}
		if (AF_TRUE == flow->https.match) {
			printk("-------------------https protocol-------------------------\n");
			dump_https_flow_info(&flow->https);
		}
	}
}
int af_match_by_pos(flow_info_t *flow, af_feature_node_t *node)
{
	int i;
	unsigned int pos = 0;
	
	if (!flow || !node)
		return AF_FALSE;
	if (node->pos_num > 0) {
		for (i = 0;i < node->pos_num; i++){
			// -1
			if(node->pos_info[i].pos < 0) {
				pos = flow->l4_len + node->pos_info[i].pos;
			}
			else{
				pos = node->pos_info[i].pos;
			}
			if (pos >= flow->l4_len){
				return AF_FALSE;
			}			
			if (flow->l4_data[pos] != node->pos_info[i].value){
				return AF_FALSE;
			}
		}
		AF_DEBUG("match by pos, appid=%d\n", node->app_id);
		return AF_TRUE;
	}
	return AF_FALSE;
}

int af_match_by_url(flow_info_t *flow, af_feature_node_t *node)
{
	char reg_url_buf[MAX_URL_MATCH_LEN] = {0};

	if (!flow || !node)
		return AF_FALSE;
	// match host or https url
	if (flow->https.match == AF_TRUE && flow->https.url_pos) {
		if (flow->https.url_len >= MAX_URL_MATCH_LEN)
			strncpy(reg_url_buf, flow->https.url_pos, MAX_URL_MATCH_LEN - 1);
		else
			strncpy(reg_url_buf, flow->https.url_pos, flow->https.url_len);
	}
	else if (flow->http.match == AF_TRUE && flow->http.host_pos) {
		if (flow->http.host_len >= MAX_URL_MATCH_LEN)
			strncpy(reg_url_buf, flow->http.host_pos, MAX_URL_MATCH_LEN - 1);
		else
			strncpy(reg_url_buf, flow->http.host_pos, flow->http.host_len);
	}
	if (strlen(reg_url_buf) > 0 && strlen(node->host_url) > 0 
				&& regexp_match(node->host_url, reg_url_buf)){
		AF_DEBUG("match url:%s	 reg = %s, appid=%d\n", 
			reg_url_buf, node->host_url, node->app_id);
		return AF_TRUE;
	}
	
	// match request url
	if (flow->http.match == AF_TRUE && flow->http.url_pos) {
		memset(reg_url_buf, 0x0, sizeof(reg_url_buf));
		if (flow->http.url_len >= MAX_URL_MATCH_LEN)
			strncpy(reg_url_buf, flow->http.url_pos, MAX_URL_MATCH_LEN - 1);
		else
			strncpy(reg_url_buf, flow->http.url_pos, flow->http.url_len);
		if(strlen(reg_url_buf) > 0 && strlen(node->request_url)
				&& regexp_match(node->request_url, reg_url_buf)){
			AF_DEBUG("match request:%s   reg:%s appid=%d\n",
						reg_url_buf, node->request_url, node->app_id);
			return AF_TRUE;
		}
	}
	return AF_FALSE;
}

int af_match_one(flow_info_t *flow, af_feature_node_t *node)
{
	int ret = AF_FALSE;
	if (!flow || !node){
		AF_ERROR("node or flow is NULL\n");
		return AF_FALSE;
	}
	
	if (flow->l4_len == 0)
		return AF_FALSE;

	if (node->sport != 0 && flow->sport != node->sport ){
		return AF_FALSE;
	}

	if (node->dport != 0 && flow->dport != node->dport)	{
		return AF_FALSE;
	}
	
	if (strlen(node->request_url) > 0 ||
		strlen(node->host_url) > 0){
		ret = af_match_by_url(flow, node);
	}
	else if (node->pos_num > 0){
		ret = af_match_by_pos(flow, node);
	}
	else{
		AF_DEBUG("node is empty, match sport:%d,dport:%d, appid = %d\n",
			node->sport, node->dport, node->app_id);
		return AF_TRUE;
	}
	return ret;
}

int app_filter_match(flow_info_t *flow)
{
	af_feature_node_t *n,*node;
	af_client_info_t *client = NULL;
	feature_list_read_lock();
	if(!list_empty(&af_feature_head)) { 
		list_for_each_entry_safe(node, n, &af_feature_head, head) {
			if(af_match_one(flow, node)) 
			{
				flow->app_id = node->app_id;
				strncpy(flow->app_name, node->app_name, sizeof(flow->app_name) - 1);
				client = find_af_client_by_ip(flow->src);
				if (!client){
					goto EXIT;
				}
				if (is_user_match_enable() && !find_af_mac(client->mac)){
					AF_DEBUG("not match mac:"MAC_FMT"\n", MAC_ARRAY(client->mac));
					goto EXIT;
				}
				if (af_get_app_status(node->app_id)){
					flow->drop = AF_TRUE;
					feature_list_read_unlock();
					return AF_TRUE;
				}
				else {
					goto EXIT;
				}
			}
			
		}
	}
EXIT:
	flow->drop = AF_FALSE;
	feature_list_read_unlock();
	return AF_FALSE;
}

#define APP_FILTER_DROP_BITS 0x80000000



static int af_get_visit_index(af_client_info_t *node, int app_id){
	int i;
	for(i = 0; i < MAX_RECORD_APP_NUM; i++){
		if(node->visit_info[i].app_id == app_id || node->visit_info[i].app_id == 0){
			return i;
		}
	}
	// default 0
	return 0;
}


int __af_update_client_app_info(flow_info_t *flow, af_client_info_t *node)
{
	int i;
	int index = -1;
	if(!node)
		return -1;
	if(!flow)
		return -1;
	int found = 0;
	index = af_get_visit_index(node, flow->app_id);

	if(index < 0 || index >= MAX_RECORD_APP_NUM){
		AF_ERROR("invalid index:%d\n\n", index);
		return 0;
	}
	// todo: up bytes
	node->visit_info[index].total_down_bytes += flow->l4_len + 66;
	//AF_ERROR("%s %pI4(%d)--> %pI4(%d) len = %d\n ", IPPROTO_TCP == flow->l4_protocol ? "tcp" :"udp",
	//	&flow->src, flow->sport, &flow->dst, flow->dport,  flow->l4_len);
//	AF_ERROR("index = %d, appid:%d, total:%d KB, cur len=%d\n",index,
	///	flow->app_id, node->visit_info[index].total_down_bytes / 1024, flow->l4_len);
	//}
	node->visit_info[index].total_num++;
	if(flow->drop)
		node->visit_info[index].drop_num++;
	
	node->visit_info[index].app_id = flow->app_id;
	node->visit_info[index].latest_time = af_get_timestamp_sec();
	node->visit_info[index].latest_action = flow->drop;
	AF_INFO("[%d] %pI4 visit %s(%d), time=%d action=%s, %d/%d\n", index, &node->ip, flow->app_name, flow->app_id, 
		node->visit_info[index].latest_time, node->visit_info[index].latest_action ? "Drop" : "Accept",
		node->visit_info[index].drop_num, node->visit_info[index].total_num);
	// todo: history
	return 0;
}

void af_update_client_app_info(flow_info_t *flow)
{
	int i;
	int index = 0;
	af_client_info_t *node = NULL;
	if(!flow)
		return;
	if(flow->app_id <= 0)
		return;
	AF_CLIENT_LOCK_W();
	node = find_af_client_by_ip(flow->src);
	if(node){
		__af_update_client_app_info(flow, node);
	}
	AF_CLIENT_UNLOCK_W();
}



int af_send_msg_to_user(char *pbuf, uint16_t len);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static u_int32_t app_filter_hook(void *priv,
			       struct sk_buff *skb,
			       const struct nf_hook_state *state) {
#else
static u_int32_t app_filter_hook(unsigned int hook,
						    	struct sk_buff *skb,
					           const struct net_device *in,
					           const struct net_device *out,
					           int (*okfn)(struct sk_buff *)){
#endif
	static int bytes1 = 0;
	unsigned long long total_packets = 0;
	flow_info_t flow;
// 4.10-->4.11 nfct-->_nfct
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
	struct nf_conn *ct = (struct nf_conn *)skb->_nfct;
#else
	struct nf_conn *ct = (struct nf_conn *)skb->nfct;
#endif
	if (!g_oaf_enable){
		return NF_ACCEPT;
	}
	if(ct == NULL) {
        return NF_ACCEPT;
    }
	if(!nf_ct_is_confirmed(ct)){
		
		return NF_ACCEPT;
	}
	
#if defined(CONFIG_NF_CONNTRACK_MARK)
	if(ct->mark != 0){
	//AF_LMT_ERROR("mark = %x, appid = %x\n", ct->mark, GET_APPID(ct->mark));
	if(APP_FILTER_DROP_BITS == (ct->mark & APP_FILTER_DROP_BITS)){
		return NF_DROP;
	}
}
#endif
#if 0
// 3.12.74-->3.13-rc1
#if LINUX_VERSION_CODE >= KERNEL_VERSION(3,13,0)
	struct nf_conn_acct *acct;
	acct = nf_conn_acct_find(ct);
	if(!acct)
		return NF_ACCEPT;
	total_packets = (unsigned long long)atomic64_read(&acct->counter[IP_CT_DIR_ORIGINAL].packets) 
		+ (unsigned long long)atomic64_read(&acct->counter[IP_CT_DIR_REPLY].packets);
#else

	struct nf_conn_counter *counter;
	counter = nf_conn_acct_find(ct);

	if (!counter)
		return NF_ACCEPT;	

	total_packets = (unsigned long long)atomic64_read(&counter[IP_CT_DIR_ORIGINAL].packets) 
		+ (unsigned long long)atomic64_read(&counter[IP_CT_DIR_REPLY].packets);

#endif
	if(total_packets > MAX_PARSE_PKT_NUM){
		return NF_ACCEPT;
	}

#endif
	memset((char *)&flow, 0x0, sizeof(flow_info_t));
	if(parse_flow_base(skb, &flow) < 0){
		return NF_ACCEPT;
	}

//	int appid = GET_APPID(ct->mark);
	parse_http_proto(&flow);
	parse_https_proto(&flow);
	if (TEST_MODE())
		dump_flow_info(&flow);
	app_filter_match(&flow);

	if (flow.app_id != 0){
		//SET_APPID(ct->mark, flow.app_id);
		if (flow.app_id > 1000 && flow.app_id <= 8999){
			af_update_client_app_info(&flow);
			AF_LMT_INFO("match %s %pI4(%d)--> %pI4(%d) len = %d, %d\n ",  IPPROTO_TCP == flow.l4_protocol ? "tcp" :"udp",
						&flow.src, flow.sport, &flow.dst, flow.dport, skb->len, flow.app_id);
		}
	}
	if(flow.drop){
#if defined(CONFIG_NF_CONNTRACK_MARK)
		ct->mark |= APP_FILTER_DROP_BITS;
#endif
		AF_LMT_INFO("##Drop app %s flow, appid is %d\n", flow.app_name, flow.app_id);
		return NF_DROP;
	}
	return NF_ACCEPT;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static struct nf_hook_ops app_filter_ops[] __read_mostly = {
	{
		.hook		= app_filter_hook,
		.pf			= PF_INET,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_MANGLE + 1,
	},
};
#else
static struct nf_hook_ops app_filter_ops[] __read_mostly = {
	{
		.hook		= app_filter_hook,
		.owner		= THIS_MODULE,
		.pf			= PF_INET,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_MANGLE + 1,
	},
};
#endif


#include "cJSON.h"
void TEST_cJSON(void)
{
	cJSON * root = NULL;
	char *out = NULL;
	root = cJSON_CreateObject();
	if (!root) {
		AF_ERROR("create obj failed\n");
		return;
	}
	cJSON_AddNumberToObject(root, "id", 123);
	cJSON_AddStringToObject(root, "name", "derry");
	out = cJSON_Print(root);
	printk("out = %s\n", out);
	cJSON_Delete(root);
	kfree(out);
}


struct timer_list oaf_timer;   
int report_flag = 0;
#define OAF_TIMER_INTERVAL 1
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
static void oaf_timer_func(struct timer_list *t)
#else
static void oaf_timer_func(unsigned long ptr)
#endif
{
	static int count = 0;
	if (count % 60 == 0)
		check_client_expire();
	if (count % 60 == 0 || report_flag){
		report_flag = 0;
		af_visit_info_report();
	}
	count++;
    mod_timer(&oaf_timer,  jiffies + OAF_TIMER_INTERVAL * HZ);
}


void init_oaf_timer(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,15,0)
	timer_setup(&oaf_timer, oaf_timer_func, 0);
#else
    setup_timer(&oaf_timer, oaf_timer_func, OAF_TIMER_INTERVAL * HZ);
#endif
    mod_timer(&oaf_timer,  jiffies + OAF_TIMER_INTERVAL * HZ);
	AF_INFO("init oaf timer...ok");
}

void fini_oaf_timer(void)
{
    del_timer_sync(&oaf_timer);
	AF_INFO("del oaf timer...ok");
}


static struct sock *oaf_sock = NULL;


int af_send_msg_to_user(char *pbuf, uint16_t len)
{
    struct sk_buff *nl_skb;
    struct nlmsghdr *nlh;

    int ret;
	if (len > MAX_OAF_NL_MSG_LEN){

		return -1;
    }

    nl_skb = nlmsg_new(len + sizeof(struct af_msg_hdr), GFP_ATOMIC);
    if(!nl_skb)
    {
        printk("netlink alloc failure\n");
        return -1;
    }

    nlh = nlmsg_put(nl_skb, 0, 0, OAF_NETLINK_ID, len + sizeof(struct af_msg_hdr), 0);
    if(nlh == NULL)
    {
        printk("error, nlh is NULL\n");
        nlmsg_free(nl_skb);
        return -1;
    }

	char msg_buf[MAX_OAF_NL_MSG_LEN] = {0};
	struct af_msg_hdr *hdr = (struct af_msg_hdr *)msg_buf;
	hdr->magic = 0xa0b0c0d0;
	hdr->len = len;
	char *p_data = msg_buf + sizeof(struct af_msg_hdr);
	memcpy(p_data, pbuf, len);
    memcpy(nlmsg_data(nlh), msg_buf, len + sizeof(struct af_msg_hdr));
    ret = netlink_unicast(oaf_sock, nl_skb, 999, MSG_DONTWAIT);
    return ret;
}
#define MAX_OAF_NETLINK_MSG_LEN 1024

static void oaf_user_msg_handle(af_msg_t *msg){
	printk("oaf msg handle, action = %d\n", msg->action);
	switch(msg->action){
		case AF_MSG_INIT:
			printk("module init.........\n");
			af_client_list_reset_report_num();
			report_flag = 1;
			break;
		default:
			break;
	}
}
static void oaf_msg_rcv(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    char *umsg = NULL;
	printk("recv user msg\n");
    if(skb->len >= nlmsg_total_size(0))
    {
        nlh = nlmsg_hdr(skb);
        umsg = NLMSG_DATA(nlh);

		struct af_msg_hdr *af_hdr = (struct af_msg_hdr *)umsg;
		if (af_hdr->magic != 0xa0b0c0d0){
			printk("magic error %x\n", af_hdr->magic);
			return;
		}
		if (af_hdr->len <= 0 || af_hdr->len >= MAX_OAF_NETLINK_MSG_LEN){
			printk("data len error\n");
			return;
		}
		void *udata = umsg + sizeof(struct af_msg_hdr);

        if(udata)
        	oaf_user_msg_handle((af_msg_t *)udata);
    }
}

int netlink_oaf_init(void)
{
    struct netlink_kernel_cfg nl_cfg = {0};
    nl_cfg.input = oaf_msg_rcv;
    oaf_sock = netlink_kernel_create(&init_net, OAF_NETLINK_ID, &nl_cfg);

    if (NULL == oaf_sock)
    {
		AF_ERROR("init oaf netlink failed, id=%d\n", OAF_NETLINK_ID);
        return -1;
    }
    AF_INFO("init oaf netlink ok, id = %d\n", OAF_NETLINK_ID);
    return 0;
}


static int __init app_filter_init(void)
{
	printk("appfilter version:"AF_VERSION"\n");
	if (0 != load_feature_config()){
		printk("load feature failed\n");
		return -1;
	}

	netlink_oaf_init();
	af_log_init();
	af_register_dev();
	af_mac_list_init();
	af_init_app_status();

	init_af_client_procfs();
//	show_feature_list();
	af_client_init();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
    nf_register_net_hooks(&init_net, app_filter_ops, ARRAY_SIZE(app_filter_ops));
#else
	nf_register_hooks(app_filter_ops, ARRAY_SIZE(app_filter_ops));
#endif
	init_oaf_timer();

	AF_INFO("init app filter ........ok\n");
	return 0;
}


static void app_filter_fini(void)
{
	AF_INFO("app filter module exit\n");
	fini_oaf_timer();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
    nf_unregister_net_hooks(&init_net, app_filter_ops, ARRAY_SIZE(app_filter_ops));
#else
	nf_unregister_hooks(app_filter_ops, ARRAY_SIZE(app_filter_ops));
#endif

	af_clean_feature_list();
	af_mac_list_clear();
	af_unregister_dev();
	af_log_exit();
	af_client_exit();
	finit_af_client_procfs();
	  if (oaf_sock)
        netlink_kernel_release(oaf_sock);
	return ;
}


module_init(app_filter_init);
module_exit(app_filter_fini);

