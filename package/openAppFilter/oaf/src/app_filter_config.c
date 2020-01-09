
/*
	author: destan19@126.com
	Î¢ÐÅ¹«ÖÚºÅ: OpenWrt
	date:2019/1/10
*/
#include <linux/init.h>
#include <linux/module.h>
#include <net/tcp.h>
#include <linux/netfilter.h>
#include <net/netfilter/nf_conntrack.h>
#include <linux/skbuff.h>
#include <net/ip.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/etherdevice.h>
#include <linux/cdev.h>
#include <linux/vmalloc.h>
#include <linux/device.h>
#include "cJSON.h"
#include "app_filter.h"
#include "af_utils.h"
#include "af_log.h"
#define AF_MAX_APP_TYPE_NUM 16
#define AF_MAX_APP_NUM 256
#define AF_DEV_NAME "appfilter"

DEFINE_RWLOCK(af_rule_lock);

#define af_rule_read_lock() 		read_lock_bh(&af_rule_lock);
#define af_rule_read_unlock() 		read_unlock_bh(&af_rule_lock);
#define af_rule_write_lock() 		write_lock_bh(&af_rule_lock);
#define af_rule_write_unlock()		write_unlock_bh(&af_rule_lock);


static struct mutex af_cdev_mutex;
struct af_config_dev {
    dev_t id;
    struct cdev char_dev;
    struct class *c;
};
struct af_config_dev g_af_dev;

struct af_cdev_file {
    size_t size;
    char buf[256 << 10];
};
	
enum AF_CONFIG_CMD{
	AF_CMD_ADD_APPID = 1,
	AF_CMD_DEL_APPID,
	AF_CMD_CLEAN_APPID,
};

char g_app_id_array[AF_MAX_APP_TYPE_NUM][AF_MAX_APP_NUM] = {0};


void af_show_app_status(void)
{
	int i, j;
	printk("#########show app status##########\n");
	for (i = 0; i < AF_MAX_APP_TYPE_NUM; i++) {
		for (j = 0; j < AF_MAX_APP_NUM; j++) {
			
			af_rule_read_lock();
			if (g_app_id_array[i][j] == AF_TRUE) {
				printk("%d, %d\n", i, j);
			}
			af_rule_read_unlock();
		}
	}
	
	printk("\n\n\n");
}

int af_change_app_status(cJSON * data_obj, int status)
{
	int i;
	int id;
	int type;
	if (!data_obj) {
		AF_ERROR("data obj is null\n");
		return -1;
	}
	cJSON *appid_arr = cJSON_GetObjectItem(data_obj, "apps");
	if (!appid_arr){
		AF_ERROR("apps obj is null\n");
		return -1;
	}
	for (i = 0; i < cJSON_GetArraySize(appid_arr); i++) {
		cJSON *appid_obj = cJSON_GetArrayItem(appid_arr, i);
		if (!appid_obj){
			AF_ERROR("appid obj is null\n");
			return -1;
		}
		id = AF_APP_ID(appid_obj->valueint);
		type = AF_APP_TYPE(appid_obj->valueint);
		AF_DEBUG("appid:%d, type = %d, id = %d\n", appid_obj->valueint, type, id);
		
		af_rule_write_lock();
		g_app_id_array[type][id] = status;
		af_rule_write_unlock();
	}
	
	return 0;
}


void af_init_app_status(void)
{
	int i, j;
	
	for (i = 0; i < AF_MAX_APP_TYPE_NUM; i++) {
		for (j = 0; j < AF_MAX_APP_NUM; j++) {
			af_rule_write_lock();
			g_app_id_array[i][j] = AF_FALSE;
			af_rule_write_unlock();
		}
	}
}
int af_get_app_status(int appid)
{
	int status = 0;
	int id = AF_APP_ID(appid);
	int type = AF_APP_TYPE(appid);
	af_rule_read_lock();
	status = g_app_id_array[type][id];
	af_rule_read_unlock();
	return status;
}
/*
add:
{
	"op":1,
	"data"{
		"apps":[]
	}
}
clean
{
	"op":3,
}

*/
int af_config_handle(char *config, unsigned int len)
{
	cJSON * config_obj = NULL;
	cJSON * cmd_obj = NULL;
	cJSON * data_obj = NULL;
	if (!config || len == 0) {
		AF_ERROR("config or len is invalid\n");
		return -1;
	}
	config_obj = cJSON_Parse(config);
	if (!config_obj){
		AF_ERROR("config_obj is NULL\n");
		return -1;
	}
	cmd_obj = cJSON_GetObjectItem(config_obj, "op");
	if (!cmd_obj){
		AF_ERROR("not find op object\n");
		return -1;
	}
	data_obj = cJSON_GetObjectItem(config_obj, "data");

	switch(cmd_obj->valueint) {
	case AF_CMD_ADD_APPID:
		if (!data_obj)
			break;
		af_change_app_status(data_obj, AF_TRUE);
		break;
	case AF_CMD_DEL_APPID:
		if (!data_obj)
			break;
		af_change_app_status(data_obj, AF_FALSE);
		break;
	case AF_CMD_CLEAN_APPID:
		af_init_app_status();
		break;
	default:
		AF_ERROR("invalid cmd %d\n", cmd_obj->valueint);
		return -1;
	}
	af_show_app_status();
	return 0;
	
}


static int af_cdev_open(struct inode *inode, struct file *filp)
{
    struct af_cdev_file *file;
    file = vzalloc(sizeof(*file));
    if (!file)
        return -EINVAL;

    mutex_lock(&af_cdev_mutex);
    filp->private_data = file;
    return 0;
}

static ssize_t af_cdev_read(struct file *filp, char *buf, size_t count, loff_t *off)
{
    return 0;
}

static int af_cdev_release(struct inode *inode, struct file *filp)
{
    struct af_cdev_file *file = filp->private_data;
    printk("config size: %d,data = %s\n", (int)file->size, file->buf);
	af_config_handle(file->buf, file->size);
    filp->private_data = NULL;
    mutex_unlock(&af_cdev_mutex);
    vfree(file);
    return 0;
}

static ssize_t af_cdev_write(struct file *filp, const char *buffer, size_t count, loff_t *off)
{
    struct af_cdev_file *file = filp->private_data;
    int ret;
    if (file->size + count > sizeof(file->buf)) {
        printk("config overflow, cur_size: %d, block_size: %d, max_size: %d",
            (int)file->size, (int)count, (int)sizeof(file->buf));
        return -EINVAL;
    }

    ret = copy_from_user(file->buf + file->size, buffer, count);
    if (ret != 0)
        return -EINVAL;

    file->size += count;
    return count;
}

static struct file_operations af_cdev_ops = {
	owner:   THIS_MODULE,
	release: af_cdev_release,
	open:    af_cdev_open,
	write:   af_cdev_write,
	read:    af_cdev_read,
};

int af_register_dev(void)
{
    struct device *dev;
    int res;
    mutex_init(&af_cdev_mutex);

    res = alloc_chrdev_region(&g_af_dev.id, 0, 1, AF_DEV_NAME);
    if (res != 0) {
        return -EINVAL;
    }

    cdev_init(&g_af_dev.char_dev, &af_cdev_ops);
    res = cdev_add(&g_af_dev.char_dev, g_af_dev.id, 1);
    if (res < 0) {
        goto REGION_OUT;
    }

    g_af_dev.c= class_create(THIS_MODULE, AF_DEV_NAME);
    if (IS_ERR_OR_NULL(g_af_dev.c)) {
        goto CDEV_OUT;
    }

    dev = device_create(g_af_dev.c, NULL, g_af_dev.id, NULL, AF_DEV_NAME);
    if (IS_ERR_OR_NULL(dev)) {
        goto CLASS_OUT;
    }
	printk("register char dev....ok\n");

    return 0;

CLASS_OUT:
    class_destroy(g_af_dev.c);
CDEV_OUT:
    cdev_del(&g_af_dev.char_dev);
REGION_OUT:
    unregister_chrdev_region(g_af_dev.id, 1);
	
	printk("register char dev....fail\n");
    return -EINVAL;
}


void af_unregister_dev(void)
{
	device_destroy(g_af_dev.c, g_af_dev.id);
    class_destroy(g_af_dev.c);
    cdev_del(&g_af_dev.char_dev);
    unregister_chrdev_region(g_af_dev.id, 1);
	printk("unregister char dev....ok\n");
}

