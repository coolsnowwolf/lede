/*
	Author:Derry
	Date: 2019/11/12
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
#include <linux/list.h>

#include "af_client.h"
#include "af_client_fs.h"
#include "af_log.h"
#include "af_utils.h"
#include "app_filter.h"
#include "cJSON.h"

DEFINE_RWLOCK(af_client_lock);            

u32 total_client = 0;
struct list_head af_client_list_table[MAX_AF_CLIENT_HASH_SIZE];

int af_send_msg_to_user(char *pbuf, uint16_t len);

static void 
nf_client_list_init(void)
{
	int i;
	AF_CLIENT_LOCK_W();
	for(i = 0; i < MAX_AF_CLIENT_HASH_SIZE; i ++){
        INIT_LIST_HEAD(&af_client_list_table[i]);
    }
	AF_CLIENT_UNLOCK_W();
	AF_INFO("client list init......ok\n");
}

static void 
nf_client_list_clear(void)
{
	int i;
	af_client_info_t * p = NULL;
	char mac_str[32] = {0};
	
	AF_DEBUG("clean list\n");
	AF_CLIENT_LOCK_W();
	for (i = 0; i < MAX_AF_CLIENT_HASH_SIZE;i++){
		while(!list_empty(&af_client_list_table[i])){
			p = list_first_entry(&af_client_list_table[i], af_client_info_t, hlist);
			memset(mac_str, 0x0, sizeof(mac_str));
			sprintf(mac_str, MAC_FMT, MAC_ARRAY(p->mac));
			AF_DEBUG("clean mac:%s\n", mac_str);
			list_del(&(p->hlist));
			kfree(p);
		}
	}
	AF_CLIENT_UNLOCK_W();
}


void af_client_list_reset_report_num(void)
{
	int i;
	af_client_info_t * node = NULL;
	char mac_str[32] = {0};
	
	AF_INFO("reset report num");
	AF_CLIENT_LOCK_W();
	for (i = 0; i < MAX_AF_CLIENT_HASH_SIZE;i++){
		list_for_each_entry(node, &af_client_list_table[i], hlist){
			node->report_count = 0;
			printk("reset  mac="MAC_FMT" report num to 0\n", MAC_ARRAY(node->mac));
		}
	}
	AF_CLIENT_UNLOCK_W();
}

int get_mac_hash_code(unsigned char *mac)
{
	if (!mac)
		return 0;
	else
		return mac[5] & (MAX_AF_CLIENT_HASH_SIZE - 1);
}

af_client_info_t * find_af_client(unsigned char *mac)
{
    af_client_info_t *node;
    unsigned int index;

    index = get_mac_hash_code(mac);
    list_for_each_entry(node, &af_client_list_table[index], hlist){
    	if (0 == memcmp(node->mac, mac, 6)){
			node->update_jiffies = jiffies;
			return node;
    	}
    }
    return NULL;
}

af_client_info_t *find_af_client_by_ip(unsigned int ip)
{
    af_client_info_t *node;
	int i;
	
	for(i = 0; i < MAX_AF_CLIENT_HASH_SIZE; i++){
	    list_for_each_entry(node, &af_client_list_table[i], hlist){
	    	if (node->ip == ip){
				AF_LMT_DEBUG("match node->ip=%pI4, ip=%pI4\n", &node->ip, &ip);
				return node;
	    	}
	    }
	}
    return NULL;
}

static af_client_info_t *
nf_client_add(unsigned char *mac)
{
    af_client_info_t *node;
	int index = 0;
	
	node = (af_client_info_t *)kmalloc(sizeof(af_client_info_t), GFP_ATOMIC);
    if (node == NULL) {
        AF_ERROR("kmalloc failed\n");
        return NULL;
    }

	memset(node, 0, sizeof(af_client_info_t));
	memcpy(node->mac, mac, MAC_ADDR_LEN);
	
	node->create_jiffies = jiffies;
	node->update_jiffies = jiffies;
    index = get_mac_hash_code(mac);
	
	AF_LMT_INFO("new client mac="MAC_FMT"\n", MAC_ARRAY(node->mac));
	total_client++;
	list_add(&(node->hlist), &af_client_list_table[index]);
    return node;
}

void check_client_expire(void)
{
	af_client_info_t  *node;
	int i;
	AF_CLIENT_LOCK_W();
	for (i = 0; i < MAX_AF_CLIENT_HASH_SIZE; i++){
		list_for_each_entry(node, &af_client_list_table[i], hlist) {
			AF_DEBUG("mac:"MAC_FMT" update:%lu interval:%lu\n", MAC_ARRAY(node->mac),
				node->update_jiffies, (jiffies - node->update_jiffies) / HZ);
			if (jiffies > (node->update_jiffies + MAX_CLIENT_ACTIVE_TIME * HZ)) {
				AF_INFO("del client:"MAC_FMT"\n", MAC_ARRAY(node->mac));
				list_del(&(node->hlist));
				kfree(node);
				AF_CLIENT_UNLOCK_W();
				return;
			}
		}
	}
	AF_CLIENT_UNLOCK_W();
}

#define MAX_EXPIRED_VISIT_INFO_COUNT 10
void flush_expired_visit_info(af_client_info_t *node)
{
	int i;
	int count = 0;
	u_int32_t cur_timep = 0;
	int timeout = 0;
	cur_timep = af_get_timestamp_sec();
	for (i = 0; i < MAX_RECORD_APP_NUM; i++){
		if (node->visit_info[i].app_id == 0){
			return;
		}
	}
	for (i = 0; i < MAX_RECORD_APP_NUM; i++){
		if (count >= MAX_EXPIRED_VISIT_INFO_COUNT)
			break;
		
		if (node->visit_info[i].total_num > 3){
			timeout = 180;
		}
		else{
			timeout = 60;
		}
		
		if (cur_timep - node->visit_info[i].latest_time > timeout){
			// 3?¡§o?¨¤??3y????
			memset(&node->visit_info[i], 0x0, sizeof(app_visit_info_t));
			count++;
		}
	}

}

int __af_visit_info_report(af_client_info_t *node){
	unsigned char mac_str[32] = {0};
	unsigned char ip_str[32] = {0};
	int i, j;
	cJSON *root_obj = cJSON_CreateObject();
	if(!root_obj){
		AF_ERROR("create json obj failed");
		return 0;
	}
	sprintf(mac_str, MAC_FMT, MAC_ARRAY(node->mac));
	sprintf(ip_str, "%pI4", &node->ip);
	cJSON_AddStringToObject(root_obj, "mac", mac_str);
	cJSON_AddStringToObject(root_obj, "ip", ip_str);
	cJSON_AddNumberToObject(root_obj, "app_num", node->visit_app_num);
	cJSON *visit_info_array = cJSON_CreateArray();
	int count = 0;
	for(i = 0; i < MAX_RECORD_APP_NUM; i++){
		if(node->visit_info[i].app_id == 0)
			continue;
		if(node->visit_info[i].total_num < 3)
			continue;
		count++;
		cJSON *visit_obj = cJSON_CreateObject();
		cJSON_AddNumberToObject(visit_obj, "appid", node->visit_info[i].app_id);
		cJSON_AddNumberToObject(visit_obj, "latest_action", node->visit_info[i].latest_action);
		//cJSON_AddNumberToObject(visit_obj, "latest_time", node->visit_info[i].latest_time);
		//cJSON_AddNumberToObject(visit_obj, "total_num", node->visit_info[i].total_num);
		//cJSON_AddNumberToObject(visit_obj, "drop_num", node->visit_info[i].drop_num);
		
		cJSON_AddNumberToObject(visit_obj, "up_bytes", node->visit_info[i].total_up_bytes);
		
		cJSON_AddNumberToObject(visit_obj, "down_bytes", node->visit_info[i].total_down_bytes);
		//clear
		memset((char *)&node->visit_info[i], 0x0, sizeof(app_visit_info_t));
		cJSON_AddItemToArray(visit_info_array, visit_obj);
	}
	
	cJSON_AddItemToObject(root_obj, "visit_info", visit_info_array);
	char *out = cJSON_Print(root_obj);
	if(!out)
		return 0;
	//cJSON_Minify(out);
	if (count > 0 || node->report_count == 0){
		AF_INFO("report:%s count=%d\n", out, node->report_count);
		node->report_count++;
		af_send_msg_to_user(out, strlen(out));
	}
	cJSON_Delete(root_obj);
	kfree(out);
	return 0;
}
void af_visit_info_report(void){
	af_client_info_t  *node;
	int i;
	AF_CLIENT_LOCK_W();
	for (i = 0; i < MAX_AF_CLIENT_HASH_SIZE; i++){
		list_for_each_entry(node, &af_client_list_table[i], hlist) {
			//flush_expired_visit_info(node);
			AF_INFO("report %s\n", node->mac);
			__af_visit_info_report(node);
		}
	}
	AF_CLIENT_UNLOCK_W();
}
static inline int get_packet_dir(struct net_device *in)
{
	if (0 == strncmp(in->name, "br", 2)){
		return PKT_DIR_UP;
	}
	else{
		return PKT_DIR_DOWN;
	}
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static u_int32_t af_client_hook(void *priv,
			       struct sk_buff *skb,
			       const struct nf_hook_state *state) {
#else
static u_int32_t af_client_hook(unsigned int hook,
						    	struct sk_buff *skb,
					           const struct net_device *in,
					           const struct net_device *out,
					           int (*okfn)(struct sk_buff *)){
#endif
	struct ethhdr *ethhdr = NULL;
	unsigned char smac[ETH_ALEN];
	af_client_info_t *nfc = NULL;
	int pkt_dir = 0;
	struct iphdr *iph = NULL;

// 4.10-->4.11 nfct-->_nfct
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,11,0)
	struct nf_conn *ct = (struct nf_conn *)skb->_nfct;
#else
	struct nf_conn *ct = (struct nf_conn *)skb->nfct;
#endif
	if (ct == NULL) {
		return NF_ACCEPT;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
	if(!skb->dev)
		return NF_ACCEPT;

	pkt_dir = get_packet_dir(skb->dev);
#else
	if (!in){
		AF_ERROR("in is NULL\n");
		return NF_ACCEPT;
	}
	pkt_dir = get_packet_dir(in);
#endif

	if(PKT_DIR_UP != pkt_dir)
		return NF_ACCEPT;

    ethhdr = eth_hdr(skb);
    if (ethhdr) {
        memcpy(smac, ethhdr->h_source, ETH_ALEN);
    } else {
        memcpy(smac, &skb->cb[40], ETH_ALEN);
    }

	iph = ip_hdr(skb);
	if (!iph) {
		return NF_ACCEPT;
	}

	AF_CLIENT_LOCK_W();
	nfc = find_af_client(smac);
	if (!nfc){
		if (skb->dev)
		AF_DEBUG("from dev:%s [%s] %pI4--->%pI4", skb->dev->name, (iph->protocol == IPPROTO_TCP ? "TCP" : "UDP"), 
			&iph->saddr, &iph->daddr);
		nfc = nf_client_add(smac);
	}
	if(nfc && nfc->ip != iph->saddr){
		AF_DEBUG("update node "MAC_FMT" ip %pI4--->%pI4\n", MAC_ARRAY(nfc->mac), &nfc->ip, &iph->saddr);
		nfc->ip = iph->saddr;
	}
	AF_CLIENT_UNLOCK_W();

	return NF_ACCEPT;
}


#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,4,0)
static struct nf_hook_ops af_client_ops[] = {
	{
		.hook		= af_client_hook,
		.pf			= PF_INET,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_FIRST + 1,
	},
};
#else
static struct nf_hook_ops af_client_ops[] = {
	{
		.hook		= af_client_hook,
		.owner		= THIS_MODULE,
		.pf			= PF_INET,
		.hooknum	= NF_INET_FORWARD,
		.priority	= NF_IP_PRI_FIRST + 1,
	},
};
#endif


int  af_client_init(void)
{
	nf_client_list_init();
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
    nf_register_net_hooks(&init_net, af_client_ops, ARRAY_SIZE(af_client_ops));
#else
	nf_register_hooks(af_client_ops, ARRAY_SIZE(af_client_ops));
#endif
	AF_INFO("init app afclient ........ok\n");
	
	return 0;
}


void af_client_exit(void)
{
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,13,0)
    nf_unregister_net_hooks(&init_net, af_client_ops, ARRAY_SIZE(af_client_ops));
#else
	nf_unregister_hooks(af_client_ops, ARRAY_SIZE(af_client_ops));
#endif
	nf_client_list_clear();
	return ;
}


