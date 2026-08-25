#include "pwan/tc_vlanfilter.h"
#include "pwan/tc_l2lu.h"

#include <linux/capability.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/spinlock.h>
#include <linux/times.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/if_vlan.h>
#include <linux/if_ether.h>
#include <linux/version.h>


static int vlan_filter_major =  VLAN_FILTER_MAJOR;
static DEFINE_SPINLOCK(vlan_filter_list_lock);
static struct list_head vlan_filter_white_list;
static int vlan_filter_enable = 0;

static int addToWhiteList(unsigned short vid)
{
	vlanfilter_whitelist_t* new_ptr = NULL;
	vlanfilter_whitelist_t* cur_ptr = NULL;

	new_ptr = (vlanfilter_whitelist_t*)kmalloc(sizeof(vlanfilter_whitelist_t),GFP_KERNEL);

	if(!new_ptr)
		return OP_FAIL;

	memset(new_ptr,0,sizeof(vlanfilter_whitelist_t));
	new_ptr->vid = vid;

	spin_lock(&vlan_filter_list_lock);
	list_for_each_entry(cur_ptr,&vlan_filter_white_list,list)
	{
		if(cur_ptr->vid == vid)
		{
			spin_unlock(&vlan_filter_list_lock);
			return ADD_ALREADY_EXIST;
		}
	}

	list_add(&new_ptr->list,&vlan_filter_white_list);
	spin_unlock(&vlan_filter_list_lock);
    PpeL2LUKeyEntryAdd(vid);
	return OP_SUCCESS;
}

static int existInWhiteList(unsigned short vid)
{
	vlanfilter_whitelist_t* cur_ptr = NULL;

	spin_lock(&vlan_filter_list_lock);
	list_for_each_entry(cur_ptr,&vlan_filter_white_list,list)
	{
		if(cur_ptr->vid == vid)
		{
			spin_unlock(&vlan_filter_list_lock);
			return 0;
		}
	}
	spin_unlock(&vlan_filter_list_lock);

	return -1;
}

static int deleteFromWhiteList(unsigned short vid)
{
	vlanfilter_whitelist_t* cur_ptr = NULL;
	vlanfilter_whitelist_t* tmp = NULL;
	
	spin_lock(&vlan_filter_list_lock);
	list_for_each_entry_safe(cur_ptr,tmp,&vlan_filter_white_list,list)
	{
		if(cur_ptr->vid == vid)
		{
			list_del(&cur_ptr->list);
			kfree(cur_ptr);
			spin_unlock(&vlan_filter_list_lock);
            PpeL2LUKeyEntryDelete(vid);
			return OP_SUCCESS;
		}
	}
	spin_unlock(&vlan_filter_list_lock);
	
	return DELETE_NOT_FOUND;
}

static int clearWhiteList(void)
{
	vlanfilter_whitelist_t* cur_ptr = NULL;
	vlanfilter_whitelist_t* tmp = NULL;
	
	spin_lock(&vlan_filter_list_lock);
	list_for_each_entry_safe(cur_ptr,tmp,&vlan_filter_white_list,list)
	{
		list_del(&cur_ptr->list);
		kfree(cur_ptr);
	}
	spin_unlock(&vlan_filter_list_lock);
    PpeL2LUKeyTblClear();

	return 0;
}

#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
static long vlanFilterIoctl(struct file *file, unsigned int cmd, unsigned long arg)
#else
static int
vlanFilterIoctl(struct inode *inode, struct file *filp,
	   unsigned int cmd, unsigned long arg)
#endif
{
	vlanfilter_args_t *opt = (vlanfilter_args_t *)arg;
	
	switch (cmd) {
	case VLAN_FILTER_LIST_ADD:
		opt->result = addToWhiteList(opt->vid);
		break;
	case VLAN_FILTER_LIST_DELETE:	
		opt->result = deleteFromWhiteList(opt->vid);
		break;
	default:
		break;
	}
	return 0;
}

struct file_operations vlan_filter_fops = {
#if LINUX_VERSION_CODE > KERNEL_VERSION(2,6,35)
    unlocked_ioctl:      vlanFilterIoctl,
#else
    ioctl:		 vlanFilterIoctl,
#endif
};


static int vlanFilterRegIoctlHandler(void)
{
    int result=0;
    result = register_chrdev(vlan_filter_major, VLAN_FILTER_DEVNAME, &vlan_filter_fops);
    if (result < 0) {
	printk("vlan_filter: can't get major %d\n",vlan_filter_major);
        return result;
    }

    if (vlan_filter_major == 0) {
	vlan_filter_major = result; 
    }

    return 0;
}

static void vlanFilterUnRegIoctlHandler(void)
{
	unregister_chrdev(vlan_filter_major, VLAN_FILTER_DEVNAME);
}

static int get_skb_vid(struct sk_buff* skb)
{
	unsigned short* tmp = (unsigned short *)skb_mac_header(skb);

	tmp += 6;

	if(*tmp == htons(ETH_P_8021Q) || *tmp == htons(ETH_P_QinQ_88a8) || *tmp == htons(ETH_P_QinQ_9100))
	{
		tmp += 1;
		return (*tmp)&0xFFF;
	}

	return -1;
}

static int get_skb_pbit(struct sk_buff* skb)
{
	unsigned short* tmp = (unsigned short *)skb_mac_header(skb);

	tmp += 6;

	if(*tmp == htons(ETH_P_8021Q) || *tmp == htons(ETH_P_QinQ_88a8) || *tmp == htons(ETH_P_QinQ_9100))
	{
		tmp += 1;
		return ((*tmp)&0xE000)>>13;
	}

	return -1;
}

int vlan_filter_handle(struct sk_buff* skb)
{
	int vid = get_skb_vid(skb);
	int pbit = get_skb_pbit(skb);

	if(!vlan_filter_enable)
		return 0;

	if(-1 == vid)/*untag,pass*/
		return 0;

	if(0 != existInWhiteList(vid))
		return -1;

	return 0;
}

EXPORT_SYMBOL(vlan_filter_handle);

static int vlan_filter_enable_read_proc(char *buf, char **start, off_t off, int count, int *eof, void *data)
{
	vlanfilter_whitelist_t* cur_ptr = NULL;
	vlanfilter_whitelist_t* tmp = NULL;
	int i = 0;
	
	printk("%d\n",vlan_filter_enable);
	printk("Vlan filter white list:\n");
	
	spin_lock(&vlan_filter_list_lock);
	list_for_each_entry_safe(cur_ptr,tmp,&vlan_filter_white_list,list)
	{
		printk("%2d vid=%4d\n", i++, cur_ptr->vid);
	}
	spin_unlock(&vlan_filter_list_lock);
	
	return 0;

}

static int vlan_filter_enable_write_proc(struct file *file, const char *buffer,unsigned long count, void *data)
{
	char val_string[64];
	char* endpo; 
	int ret = 0;
		
	if (count > sizeof(val_string) - 1)
	{
		ret = -EFAULT;
		return ret;
	}
	memset(val_string,0,64);
	if (copy_from_user(val_string, buffer, count))
	{
		ret = -EFAULT;
		return ret;
	}

	vlan_filter_enable = simple_strtol(val_string,&endpo,10);
	
	return count;

}

int vlan_filter_init(void)
{
	 struct proc_dir_entry *vf_proc;
	 
	vlanFilterRegIoctlHandler();
	INIT_LIST_HEAD(&vlan_filter_white_list);

	vf_proc = create_proc_entry("tc3162/vlan_filter_enable", 0, NULL);
	if(NULL == vf_proc)
	{
		printk("ERROR!Create proc entry vlan_filter_enable fail!");
		return -ENOMEM;
	}
	vf_proc->read_proc = vlan_filter_enable_read_proc;
	vf_proc->write_proc = vlan_filter_enable_write_proc;

	return 0;
}

int vlan_filter_exit(void)
{
	vlanFilterUnRegIoctlHandler();
	clearWhiteList();
	remove_proc_entry("tc3162/vlan_filter_enable",NULL);
	
	return 0;
}

