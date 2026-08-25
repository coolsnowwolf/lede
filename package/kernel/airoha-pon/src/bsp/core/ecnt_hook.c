/***************************************************************
Copyright Statement:

This software/firmware and related documentation (¡°EcoNet Software¡±) 
are protected under relevant copyright laws. The information contained herein 
is confidential and proprietary to EcoNet (HK) Limited (¡°EcoNet¡±) and/or 
its licensors. Without the prior written permission of EcoNet and/or its licensors, 
any reproduction, modification, use or disclosure of EcoNet Software, and 
information contained herein, in whole or in part, shall be strictly prohibited.

EcoNet (HK) Limited  EcoNet. ALL RIGHTS RESERVED.

BY OPENING OR USING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY 
ACKNOWLEDGES AND AGREES THAT THE SOFTWARE/FIRMWARE AND ITS 
DOCUMENTATIONS (¡°ECONET SOFTWARE¡±) RECEIVED FROM ECONET 
AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON AN ¡°AS IS¡± 
BASIS ONLY. ECONET EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, 
WHETHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, 
OR NON-INFRINGEMENT. NOR DOES ECONET PROVIDE ANY WARRANTY 
WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTIES WHICH 
MAY BE USED BY, INCORPORATED IN, OR SUPPLIED WITH THE ECONET SOFTWARE. 
RECEIVER AGREES TO LOOK ONLY TO SUCH THIRD PARTIES FOR ANY AND ALL 
WARRANTY CLAIMS RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES 
THAT IT IS RECEIVER¡¯S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD 
PARTY ALL PROPER LICENSES CONTAINED IN ECONET SOFTWARE.

ECONET SHALL NOT BE RESPONSIBLE FOR ANY ECONET SOFTWARE RELEASES 
MADE TO RECEIVER¡¯S SPECIFICATION OR CONFORMING TO A PARTICULAR 
STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND 
ECONET'S ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE ECONET 
SOFTWARE RELEASED HEREUNDER SHALL BE, AT ECONET'S SOLE OPTION, TO 
REVISE OR REPLACE THE ECONET SOFTWARE AT ISSUE OR REFUND ANY SOFTWARE 
LICENSE FEES OR SERVICE CHARGES PAID BY RECEIVER TO ECONET FOR SUCH 
ECONET SOFTWARE.
***************************************************************/

/************************************************************************
*                  I N C L U D E S
*************************************************************************
*/
#include <linux/kernel.h>
#include <linux/netfilter.h>
#include <net/protocol.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/wait.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/if.h>
#include <linux/netdevice.h>
#include <linux/inetdevice.h>
#include <linux/proc_fs.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <net/net_namespace.h>
#include <net/sock.h>
#include <ecnt_hook/ecnt_hook.h>

/************************************************************************
*                  D E F I N E S   &   C O N S T A N T S
*************************************************************************
*/

/************************************************************************
*                  M A C R O S
*************************************************************************
*/

/************************************************************************
*                  D A T A   T Y P E S
*************************************************************************
*/

/************************************************************************
*                  E X T E R N A L   D A T A   D E C L A R A T I O N S
*************************************************************************
*/

/************************************************************************
*                  F U N C T I O N   D E C L A R A T I O N S
*************************************************************************
*/


/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
struct list_head ecnt_hooks[ECNT_NUM_MAINTYPE][ECNT_MAX_SUBTYPE];
EXPORT_SYMBOL(ecnt_hooks);

/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/
static DEFINE_SPINLOCK(ecnt_hook_lock);
unsigned int hook_id = 0;

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/

/*****************************************************************************
//function :
//		ecnt_iterate
//description : 
//		this function is called by __ECNT_HOOK to iterate ecnt hook list
//input :	
//		struct list_head *head, struct list_head **i, struct ecnt_data *in_data
//return :
//		ECNT_RETURN_DROP 	0
//		ECNT_RETURN			2
//		ECNT_CONTINUE		1
******************************************************************************/

//static inline ecnt_ret_val ecnt_iterate(struct list_head *head, struct list_head **i, struct ecnt_data *in_data)
static inline ecnt_ret_val ecnt_iterate(struct list_head *head, struct ecnt_hook_ops **elemp, struct ecnt_data *in_data)
{
	ecnt_ret_val verdict = ECNT_CONTINUE;

	/*
	 * The caller must not block between calls to this
	 * function because of risk of continuing from deleted element.
	 */
	//list_for_each_continue_rcu(*i, head) {
	list_for_each_entry_continue_rcu((*elemp), head, list){
		if((*elemp)->is_execute){
			verdict = (*elemp)->hookfn(in_data);
			if (verdict == ECNT_RETURN_DROP) {
					(*elemp)->info.drop_num++;
					return verdict;
			}
			if(verdict == ECNT_RETURN)
				return verdict;
		}
	}

	return verdict;
}

/*****************************************************************************
//function :
//		__ECNT_HOOK
//description : 
//		kernel hook API
//input :	
//		unsigned int maintype, unsigned int subtype,struct ecnt_data *in_data
//return :
//		ECNT_HOOK_ERROR		-1
//		ECNT_RETURN_DROP	0
//		ECNT_RETURN			2
//		ECNT_CONTINUE		1
******************************************************************************/

__IMEM ecnt_ret_val __ECNT_HOOK(unsigned int maintype, unsigned int subtype,struct ecnt_data *in_data)
{
	ecnt_ret_val ret;
	struct ecnt_hook_ops *elem;
	struct list_head* ptr = &ecnt_hooks[maintype][subtype];
	
	if((maintype >= ECNT_NUM_MAINTYPE) || (subtype >= ECNT_MAX_SUBTYPE)){
		printk("__ECNT_HOOK fail, max maintype %d, max subtype %d\n", ECNT_NUM_MAINTYPE, ECNT_MAX_SUBTYPE);
		return ECNT_HOOK_ERROR;
	}
	if (list_empty(&ecnt_hooks[maintype][subtype])){
		return ECNT_HOOK_ERROR;
	}
	
	/* We may already have this, but read-locks nest anyway */
	rcu_read_lock();

	elem = list_entry_rcu(ptr, struct ecnt_hook_ops, list);	
	/* We may already have this, but read-locks nest anyway */
	ret = ecnt_iterate(&ecnt_hooks[maintype][subtype], &elem, in_data);
	rcu_read_unlock();
	
	return ret;
}
EXPORT_SYMBOL(__ECNT_HOOK);

/*****************************************************************************
//function :
//		set_ecnt_hookfn_execute_or_not
//description : 
//		used to control ecnt hook function execute or not
//input :	
//		unsigned int maintype, unsigned int subtype, unsigned int hook_id, unsigned int is_execute
//return :
//		0: fail
//		1: success
******************************************************************************/
int set_ecnt_hookfn_execute_or_not(unsigned int maintype, unsigned int subtype, unsigned int hook_id, unsigned int is_execute)
{
	struct list_head *pos;
	struct list_head* ptr = &ecnt_hooks[maintype][subtype];
	
	if((maintype >= ECNT_NUM_MAINTYPE) || (subtype >= ECNT_MAX_SUBTYPE)){
		printk("set_ecnt_hookfn_execute_or_not fail, max maintype %d, max subtype %d\n", ECNT_NUM_MAINTYPE, ECNT_MAX_SUBTYPE);
		return 0;
	}
	if(list_empty(&ecnt_hooks[maintype][subtype]))
		return 0;
	
	struct ecnt_hook_ops *elem;
	rcu_read_lock();
	elem = list_entry_rcu(ptr, struct ecnt_hook_ops, list);
	//list_for_each_continue_rcu(pos, &ecnt_hooks[maintype][subtype]){
	list_for_each_entry_continue_rcu(elem, &ecnt_hooks[maintype][subtype], list){
		if(elem->hook_id == hook_id){
			elem->is_execute = is_execute;
			rcu_read_unlock();
			return 1;
		}
	}
	rcu_read_unlock();

	return 0;
}
EXPORT_SYMBOL(set_ecnt_hookfn_execute_or_not);

/*****************************************************************************
//function :
//		get_ecnt_hookfn
//description : 
//		used to get hook
//input :	
//		unsigned int maintype, unsigned int subtype, unsigned int hook_id, unsigned int is_execute
//return :
//		0: fail
//		1: success
******************************************************************************/

int get_ecnt_hookfn(unsigned int maintype, unsigned int subtype){
	struct list_head *pos;
	struct ecnt_hook_ops *elem;
	int index = 1;
	
	if((maintype >= ECNT_NUM_MAINTYPE) || (subtype >= ECNT_MAX_SUBTYPE)){
		printk("get_ecnt_hookfn fail, max maintype %d, max subtype %d\n", ECNT_NUM_MAINTYPE, ECNT_MAX_SUBTYPE);
		return 0;
	}
	if(list_empty(&ecnt_hooks[maintype][subtype])){
		printk("maintype = %d, subtype=%d, 0 hook functions\n",maintype, subtype);
		return 0;
	}
	
	printk("maintype = %d, subtype=%d\n",maintype, subtype);
	printk("index\t[id]\tis_exe\tpri\tdropnum\tname\n");
	rcu_read_lock();
	
	list_for_each(pos, &ecnt_hooks[maintype][subtype]){
		elem = (struct ecnt_hook_ops *)pos;
		printk("%d.\t[%d]\t%d\t%d\t%d\t%s\n", 
			index++, elem->hook_id, elem->is_execute, elem->priority,elem->info.drop_num,elem->name);
			
	}
	rcu_read_unlock();
	
	return 1;
}
EXPORT_SYMBOL(get_ecnt_hookfn);

/*****************************************************************************
//function :
//		show_all_ecnt_hookfn
//description : 
//		used to show all of the ecnt hook functions
//input :	
//		void
//return :
//		1
******************************************************************************/
int show_all_ecnt_hookfn(void){
	int maintype, subtype;
	struct list_head *pos;
	struct ecnt_hook_ops *elem;
	int index = 1;

	printk("index\t[main-sub]\t[id]\tis_exe\tpri\tdropnum\tname\n");
	rcu_read_lock();
	for(maintype = 0; maintype < ECNT_NUM_MAINTYPE; maintype++){
		for(subtype = 0; subtype < ECNT_MAX_SUBTYPE; subtype++){
			list_for_each(pos, &ecnt_hooks[maintype][subtype]){
				elem = (struct ecnt_hook_ops *)pos;
				printk("%d.\t[%d-%d]\t\t[%d]\t%d\t%d\t%d\t%s\n", 
					index++, maintype, subtype,elem->hook_id, elem->is_execute, elem->priority,elem->info.drop_num,elem->name);
			}
		}
	}
	rcu_read_unlock();
	
	return 1;
}
EXPORT_SYMBOL(show_all_ecnt_hookfn);

/*****************************************************************************
//function :
//		ecnt_register_hook
//description : 
//		used to register hook function
//input :	
//		struct ecnt_hook_ops *reg
//return:
//		ECNT_REGISTER_FAIL -1
//		ECNT_REGISTER_SUCCESS   0
******************************************************************************/
int ecnt_register_hook(struct ecnt_hook_ops *reg)
{
	struct ecnt_hook_ops *elem;

	if(!reg){
		printk("ecnt_register_hook fail, reg is NULL\n");
		return ECNT_REGISTER_FAIL;
	}
	if((reg->maintype >= ECNT_NUM_MAINTYPE) || (reg->subtype >= ECNT_MAX_SUBTYPE)){
		printk("ecnt_register_hook fail, maintype = %d, subtype=%d, out of range\n", reg->maintype, reg->subtype);
		return ECNT_REGISTER_FAIL;
	}
	if(reg->list.next!= NULL){
		printk("ecnt_register_hook fail, %s already registered\n", reg->name);
		return ECNT_REGISTER_FAIL;
	}
	if(hook_id >= 0xFFFFFFFF){
		printk("ecnt_register_hook fail, hook_id out of range\n");
		return ECNT_REGISTER_FAIL;
	}
	spin_lock(&ecnt_hook_lock);
	list_for_each_entry(elem, &ecnt_hooks[reg->maintype][reg->subtype], list) {
		if (reg->priority < elem->priority)
			break;
	}
	reg->hook_id = ++hook_id;
	reg->info.drop_num = 0;
	list_add_rcu(&reg->list, elem->list.prev);
	spin_unlock(&ecnt_hook_lock);

	return ECNT_REGISTER_SUCCESS;
}
EXPORT_SYMBOL(ecnt_register_hook);

/*****************************************************************************
//function :
//		ecnt_unregister_hook
//description : 
//		used to unregister hook function
//input :	
//		struct ecnt_hook_ops *reg
//return :
//		void
******************************************************************************/
void ecnt_unregister_hook(struct ecnt_hook_ops *reg)
{
	if(!reg){
		printk("ecnt_unregister_hook fail, reg is NULL\n");
		return;
	}
	if(reg->list.prev == LIST_POISON2){
		printk("%s already unregistered\n", reg->name);
		return;
	}
	spin_lock(&ecnt_hook_lock);
	list_del_rcu(&reg->list);
	spin_unlock(&ecnt_hook_lock);
	synchronize_rcu();
}
EXPORT_SYMBOL(ecnt_unregister_hook);

/*****************************************************************************
//function :
//		ecnt_ops_unregister
//description : 
//		used to unregister hook function
//input :	
//		unsigned int maintype, unsigned int subtype, unsigned int hook_id
//return :
//		0: fail
//		1: success
******************************************************************************/

int ecnt_ops_unregister(unsigned int maintype, unsigned int subtype, unsigned int hook_id)
{
	struct ecnt_hook_ops *elem;
	struct list_head* ptr = &ecnt_hooks[maintype][subtype];
	if((maintype >= ECNT_NUM_MAINTYPE) || (subtype >= ECNT_MAX_SUBTYPE)){
		printk("set_ecnt_hookfn_execute_or_not fail, max maintype %d, max subtype %d\n", ECNT_NUM_MAINTYPE, ECNT_MAX_SUBTYPE);
		return 0;
	}
	if(list_empty(&ecnt_hooks[maintype][subtype]))
		return 0;
	
	rcu_read_lock();	
	elem = list_entry_rcu(ptr, struct ecnt_hook_ops, list);
	//list_for_each_continue_rcu(pos, &ecnt_hooks[maintype][subtype]){
	list_for_each_entry_continue_rcu(elem, &ecnt_hooks[maintype][subtype], list){
		if(elem->hook_id == hook_id){
			ecnt_unregister_hook(elem);
			rcu_read_unlock();
			return 1;
		}
	}
	rcu_read_unlock();

	return 0;
}
EXPORT_SYMBOL(ecnt_ops_unregister);

/*****************************************************************************
//function :
//		ecnt_register_hooks
//description : 
//		register ecnt ops
//input :	
//		struct ecnt_hook_ops *reg, unsigned int n
//return :
//		ECNT_REGISTER_FAIL -1
//		ECNT_REGISTER_SUCCESS   0
******************************************************************************/

int ecnt_register_hooks(struct ecnt_hook_ops *reg, unsigned int n)
{
	unsigned int i;
	int err = 0;

	for (i = 0; i < n; i++) {
		err = ecnt_register_hook(&reg[i]);
		if (err)
			goto err;
	}
	return err;

err:
	if (i > 0)
		ecnt_unregister_hooks(reg, i);
	return err;
}
EXPORT_SYMBOL(ecnt_register_hooks);

/*****************************************************************************
//function :
//		ecnt_unregister_hooks
//description : 
//		unregister ecnt ops
//input :	
//		struct ecnt_hook_ops *reg, unsigned int n
//return :
//		void
******************************************************************************/

void ecnt_unregister_hooks(struct ecnt_hook_ops *reg, unsigned int n)
{
	unsigned int i;

	for (i = 0; i < n; i++)
		ecnt_unregister_hook(&reg[i]);
}
EXPORT_SYMBOL(ecnt_unregister_hooks);

/*****************************************************************************
//function :
//		ecnt_hook_init
//description : 
//		used to init ecnt_hooks list head
//input :	
//		void
//return :
//		void
******************************************************************************/
void ecnt_hook_init(void)
{
	int i, h;
	for (i = 0; i < ARRAY_SIZE(ecnt_hooks); i++){
		for (h = 0; h < ECNT_MAX_SUBTYPE; h++)
			INIT_LIST_HEAD(&ecnt_hooks[i][h]);
	}

}

