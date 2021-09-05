#ifndef __AF_CLIENT_H__
#define __AF_CLIENT_H__
#include "app_filter.h"

extern rwlock_t af_client_lock;  

extern u32 nfc_debug_level;
	
#define MAX_AF_CLIENT_HASH_SIZE 64
#define NF_CLIENT_TIMER_EXPIRE  1
#define MAX_CLIENT_ACTIVE_TIME  90


#define AF_CLIENT_LOCK_R() 		read_lock_bh(&af_client_lock);
#define AF_CLIENT_UNLOCK_R() 	read_unlock_bh(&af_client_lock);
#define AF_CLIENT_LOCK_W() 		write_lock_bh(&af_client_lock);
#define AF_CLIENT_UNLOCK_W()	write_unlock_bh(&af_client_lock);

#define NIPQUAD(addr) \
	((unsigned char *)&addr)[0], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[3]
#define NIPQUAD_FMT "%u.%u.%u.%u"

enum NFC_PKT_DIR{
	PKT_DIR_DOWN,
	PKT_DIR_UP
};


#define MAX_VISIT_HISTORY_TIME 24
#define MAX_RECORD_APP_NUM 64


typedef struct app_visit_info{
	unsigned int app_id;
	unsigned int total_num;
	unsigned int drop_num;
	unsigned long latest_time;
	unsigned int latest_action;
	unsigned int total_down_bytes;
	unsigned int total_up_bytes;
	unsigned long history_time[MAX_VISIT_HISTORY_TIME];
	unsigned int  action[MAX_VISIT_HISTORY_TIME];
}app_visit_info_t;

typedef struct af_client_info {
    struct list_head   hlist;
    unsigned char      mac[MAC_ADDR_LEN];
	unsigned int 	   ip;
    unsigned long      create_jiffies;
	unsigned long	   update_jiffies;
	unsigned int	   visit_app_num;
	int report_count;
	app_visit_info_t   visit_info[MAX_RECORD_APP_NUM];
}af_client_info_t;




int af_client_init(void);

void af_client_exit(void);
af_client_info_t * find_af_client_by_ip(unsigned int ip);

void check_client_expire(void);

void af_visit_info_report(void);

void af_client_list_reset_report_num(void);

#endif
