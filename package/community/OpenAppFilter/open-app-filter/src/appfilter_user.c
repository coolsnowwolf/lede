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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/socket.h>
#include <sys/socket.h>
#include "appfilter_user.h"

dev_node_t *dev_hash_table[MAX_DEV_NODE_HASH_SIZE];
int g_cur_user_num = 0;
unsigned int hash_mac(unsigned char *mac)
{
    if (!mac)
        return 0;
    else
        return mac[0] & (MAX_DEV_NODE_HASH_SIZE - 1);
}
int get_timestamp(void)
{
    struct timeval cur_time;
    gettimeofday(&cur_time, NULL);
    return cur_time.tv_sec;
}

int hash_appid(int appid)
{
    return appid % (MAX_VISIT_HASH_SIZE - 1);
}

void add_visit_info_node(visit_info_t **head, visit_info_t *node)
{
    if (*head == NULL)
    {
        *head = node;
    }
    else
    {
        node->next = *head;
        *head = node;
    }
}

void init_dev_node_htable()
{
    int i;
    for (i = 0; i < MAX_DEV_NODE_HASH_SIZE; i++)
    {
        dev_hash_table[i] = NULL;
    }
}

dev_node_t *add_dev_node(char *mac)
{
    unsigned int hash = 0;
    if (g_cur_user_num >= MAX_SUPPORT_USER_NUM)
    {
        printf("error, user num reach max %d\n", g_cur_user_num);
        return NULL;
    }
    hash = hash_mac(mac);
    if (hash >= MAX_DEV_NODE_HASH_SIZE)
    {
        printf("hash code error %d\n", hash);
        return NULL;
    }
    dev_node_t *node = (dev_node_t *)calloc(1, sizeof(dev_node_t));
    if (!node)
        return NULL;
    strncpy(node->mac, mac, sizeof(node->mac));
    node->online = 1;
    node->online_time = get_timestamp();
    if (dev_hash_table[hash] == NULL)
        dev_hash_table[hash] = node;
    else
    {
        node->next = dev_hash_table[hash];
        dev_hash_table[hash] = node;
    }
    g_cur_user_num++;
    printf("add mac:%s to htable[%d]....success\n", mac, hash);
    return node;
}

dev_node_t *find_dev_node(char *mac)
{
    unsigned int hash = 0;
    dev_node_t *p = NULL;
    hash = hash_mac(mac);
    if (hash >= MAX_DEV_NODE_HASH_SIZE)
    {
        printf("hash code error %d\n", hash);
        return NULL;
    }
    p = dev_hash_table[hash];
    while (p)
    {
        if (0 == strncmp(p->mac, mac, sizeof(p->mac)))
        {
            return p;
        }
        p = p->next;
    }
    return NULL;
}

void dev_foreach(void *arg, iter_func iter)
{
    int i, j;
    dev_node_t *node = NULL;

    for (i = 0; i < MAX_DEV_NODE_HASH_SIZE; i++)
    {
        dev_node_t *node = dev_hash_table[i];
        while (node)
        {
            iter(arg, node);
            node = node->next;
        }
    }
}

char *format_time(int timetamp)
{
    char time_buf[64] = {0};
    time_t seconds = timetamp;
    struct tm *auth_tm = localtime(&seconds);
    strftime(time_buf, sizeof(time_buf), "%Y %m %d %H:%M:%S", auth_tm);
    return strdup(time_buf);
}

void update_dev_hostname(void)
{
    char line_buf[256] = {0};
    char hostname_buf[128] = {0};
    char mac_buf[32] = {0};
    char ip_buf[32] = {0};

    FILE *fp = fopen("/tmp/dhcp.leases", "r");
    if (!fp)
    {
        printf("open dhcp lease file....failed\n");
        return;
    }
    while (fgets(line_buf, sizeof(line_buf), fp))
    {
        if (strlen(line_buf) <= 16)
            continue;
        sscanf(line_buf, "%*s %s %s %s", mac_buf, ip_buf, hostname_buf);
        dev_node_t *node = find_dev_node(mac_buf);
        if (!node)
            continue;
        if (strlen(hostname_buf) > 0)
        {
            strncpy(node->hostname, hostname_buf, sizeof(node->hostname));
        }
    }
    fclose(fp);
}

void clean_dev_online_status(void)
{
    int i;
    for (i = 0; i < MAX_DEV_NODE_HASH_SIZE; i++)
    {
        dev_node_t *node = dev_hash_table[i];
        while (node)
        {
            node->online = 0;
            node->offline_time = get_timestamp();
            node = node->next;
        }
    }
}

/*
Id   Mac                  Ip                  
1    10:bf:48:37:0c:94    192.168.66.244 
*/
void check_dev_expire(void)
{
    char line_buf[256] = {0};
    char mac_buf[32] = {0};
    char ip_buf[32] = {0};

    FILE *fp = fopen("/proc/net/af_client", "r");
    if (!fp)
    {
        printf("open dev file....failed\n");
        return;
    }
    fgets(line_buf, sizeof(line_buf), fp); // title
    while (fgets(line_buf, sizeof(line_buf), fp))
    {
        sscanf(line_buf, "%*s %s %s", mac_buf, ip_buf);
        if (strlen(mac_buf) < 17)
        {
            printf("invalid mac:%s\n", mac_buf);
            continue;
        }
        dev_node_t *node = find_dev_node(mac_buf);
        if (!node)
        {
            node = add_dev_node(mac_buf);
            if (!node)
                continue;
            strncpy(node->ip, ip_buf, sizeof(node->ip));
        }
        node->online = 1;
    }
    fclose(fp);
}
void dump_dev_list(void)
{
    int i, j;
    int count = 0;
    char hostname_buf[MAX_HOSTNAME_SIZE] = {0};
    char ip_buf[MAX_IP_LEN] = {0};
    clean_dev_online_status();
    update_dev_hostname();
    check_dev_expire();
    FILE *fp = fopen(OAF_DEV_LIST_FILE, "w");
    if (!fp)
    {
        return;
    }

    fprintf(fp, "%-4s %-20s %-20s %-32s %-8s\n", "Id", "Mac Addr", "Ip Addr", "Hostname", "Online");
    for (i = 0; i < MAX_DEV_NODE_HASH_SIZE; i++)
    {
        dev_node_t *node = dev_hash_table[i];
        while (node)
        {
            if (node->online != 0)
            {
                if (strlen(node->hostname) == 0)
                    strcpy(hostname_buf, "*");
                else
                    strcpy(hostname_buf, node->hostname);
                if (strlen(node->ip) == 0)
                    strcpy(ip_buf, "*");
                else
                    strcpy(ip_buf, node->ip);
                fprintf(fp, "%-4d %-20s %-20s %-32s %-8d\n",
                        i + 1, node->mac, ip_buf, hostname_buf, node->online);
                count++;
            }
            if (count >= MAX_SUPPORT_DEV_NUM)
            {
                goto EXIT;
            }
            node = node->next;
        }
    }
    for (i = 0; i < MAX_DEV_NODE_HASH_SIZE; i++)
    {
        dev_node_t *node = dev_hash_table[i];
        while (node)
        {
            if (node->online == 0)
            {
                if (strlen(node->hostname) == 0)
                    strcpy(hostname_buf, "*");
                else
                    strcpy(hostname_buf, node->hostname);

                if (strlen(node->ip) == 0)
                    strcpy(ip_buf, "*");
                else
                    strcpy(ip_buf, node->ip);

                fprintf(fp, "%-4d %-20s %-20s %-32s %-8d\n",
                        i + 1, node->mac, ip_buf, hostname_buf, node->online);
            }
            if (count >= MAX_SUPPORT_DEV_NUM)
                goto EXIT;
            node = node->next;
        }
    }
EXIT:
    fclose(fp);
}
// 记录最大保存时间 todo: support config
#define MAX_RECORD_TIME (7 * 24 * 60 * 60) // 7day
// 超过1天后清除短时间的记录
#define RECORD_REMAIN_TIME (24 * 60 * 60) // 1day
#define INVALID_RECORD_TIME (5 * 60) // 5min

void check_dev_visit_info_expire(void)
{
    int i, j;
    int count = 0;
    int cur_time = get_timestamp();
    for (i = 0; i < MAX_DEV_NODE_HASH_SIZE; i++)
    {
        dev_node_t *node = dev_hash_table[i];
        while (node)
        {
            for (j = 0; j < MAX_VISIT_HASH_SIZE; j++)
            {
                visit_info_t *p_info = node->visit_htable[j];
                while (p_info)
                {
                    int total_time = p_info->latest_time - p_info->first_time;
                    int interval_time = cur_time - p_info->first_time; 
                    if (interval_time > MAX_RECORD_TIME || interval_time < 0){
                        p_info->expire = 1;
                    }
                    else if (interval_time > RECORD_REMAIN_TIME){
                        if (total_time < INVALID_RECORD_TIME)
                            p_info->expire = 1;
                    }
                    p_info = p_info->next;
                }
            }
            node = node->next;
        }
    }
}

void flush_expire_visit_info(void)
{
    int i, j;
    int count = 0;
    visit_info_t *prev = NULL;
    for (i = 0; i < MAX_DEV_NODE_HASH_SIZE; i++)
    {
        dev_node_t *node = dev_hash_table[i];
        while (node)
        {
            for (j = 0; j < MAX_VISIT_HASH_SIZE; j++)
            {
                visit_info_t *p_info = node->visit_htable[j];
                prev = NULL;
                while (p_info)
                {
                    if (p_info->expire){
                        printf("del node %-20s %-20s %d\n",
                                                  node->mac, node->ip, p_info->appid
                                                  );
                        if (NULL == prev){
                            node->visit_htable[j] = p_info->next;
                            free(p_info);
                            p_info = node->visit_htable[j];
                            prev = NULL;
                        }
                        else{
                            prev->next = p_info->next;
                            free(p_info);
                            p_info = prev->next;
                        }
                    }
                    else{
                        prev = p_info;
                        p_info = p_info->next;
                    }
  
                }
            }
            node = node->next;
        }
    }
}




void dump_dev_visit_list(void)
{
    int i, j;
    int count = 0;
    FILE *fp = fopen(OAF_VISIT_LIST_FILE, "w");
    if (!fp)
    {
        return;
    }

    fprintf(fp, "%-4s %-20s %-20s %-8s %-32s %-32s %-32s %-8s\n", "Id", "Mac Addr",
            "Ip Addr", "Appid", "First Time", "Latest Time", "Total Time(s)", "Expire");
    for (i = 0; i < MAX_DEV_NODE_HASH_SIZE; i++)
    {
        dev_node_t *node = dev_hash_table[i];
        while (node)
        {
            for (j = 0; j < MAX_VISIT_HASH_SIZE; j++)
            {
                visit_info_t *p_info = node->visit_htable[j];
                while (p_info)
                {
                    char *first_time_str = format_time(p_info->first_time);
                    char *latest_time_str = format_time(p_info->latest_time);
                    int total_time = p_info->latest_time - p_info->first_time;
                    fprintf(fp, "%-4d %-20s %-20s %-8d %-32s %-32s %-32d %-4d\n",
                            count, node->mac, node->ip, p_info->appid, first_time_str,
                            latest_time_str, total_time, p_info->expire);
                    if (first_time_str)
                        free(first_time_str);
                    if (latest_time_str)
                        free(latest_time_str);
                    p_info = p_info->next;
                    count++;
                    if (count > 50)
                        goto EXIT;
                }
            }
            node = node->next;
        }
    }
EXIT:
    fclose(fp);
}
