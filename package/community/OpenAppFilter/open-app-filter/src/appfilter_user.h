/*
Copyright (C) 2020 Derry <destan19@126.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#ifndef __FILTER_USER_H__
#define __FILTER_USER_H__
#define MAX_IP_LEN 32
#define MAX_MAC_LEN 32
#define MAX_VISIT_HASH_SIZE 64
#define MAX_DEV_NODE_HASH_SIZE 64
#define MAX_HOSTNAME_SIZE 64
#define MAX_SUPPORT_USER_NUM 64
#define OAF_VISIT_LIST_FILE "/tmp/visit_list"
#define OAF_DEV_LIST_FILE "/tmp/dev_list"
#define MIN_VISIT_TIME 5 // default 5s
#define MAX_APP_STAT_NUM 8
#define MAX_VISITLIST_DUMP_NUM 16
#define MAX_APP_TYPE 16
#define MAX_APP_ID_NUM 128
#define MAX_SUPPORT_DEV_NUM 64

//extern dev_node_t *dev_hash_table[MAX_DEV_NODE_HASH_SIZE];

/*
{
"mac":	"10:bf:48:37:0c:94",
"ip":	"192.168.100.244",
"app_num":	0,
"visit_info":	[{
"appid":	8002,
"latest_action":	1,
"latest_time":	1602604293,
"total_num":	4,
"drop_num":	4,
"history_info":	[]
}]
}
*/
/* 单个访问记录结构 */
typedef struct visit_info
{
    int appid;
    int first_time;
    int latest_time;
    int action;
    int expire; /*定期清除无效数据*/
    struct visit_info *next;
} visit_info_t;

/* 用于记录某个app总时间和总流量 */
typedef struct visit_stat
{
    unsigned long long total_time;
    unsigned long long total_down_bytes;
    unsigned long long total_up_bytes;
} visit_stat_t;

typedef struct dev_node
{
    char mac[MAX_MAC_LEN];
    char ip[MAX_IP_LEN];
    char hostname[MAX_HOSTNAME_SIZE];
    int online;
    int offline_time;
    int online_time;
    visit_info_t *visit_htable[MAX_VISIT_HASH_SIZE];
    visit_stat_t stat[MAX_APP_TYPE][MAX_APP_ID_NUM];
    struct dev_node *next;
} dev_node_t;

struct app_visit_info
{
    int app_id;
    char app_name[32];
    int total_time;
};

struct app_visit_stat_info
{
    int num;
    struct app_visit_info visit_list[MAX_APP_STAT_NUM];
};
typedef void (*iter_func)(void *arg, dev_node_t *dev);
//todo:dev for each
extern dev_node_t *dev_hash_table[MAX_DEV_NODE_HASH_SIZE];

dev_node_t *add_dev_node(char *mac);
void init_dev_node_htable();
void dump_dev_list(void);
void dump_dev_visit_list(void);
dev_node_t *find_dev_node(char *mac);
void dev_foreach(void *arg, iter_func iter);
void add_visit_info_node(visit_info_t **head, visit_info_t *node);
void check_dev_visit_info_expire(void);
void flush_expire_visit_info();

#endif
