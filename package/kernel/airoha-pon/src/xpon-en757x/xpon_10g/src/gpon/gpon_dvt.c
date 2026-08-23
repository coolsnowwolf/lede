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
#include <linux/string.h>
#include <linux/skbuff.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/types.h>
#include <linux/random.h>


#include "common/xpon_global.h"
#include "gpon/gpon_dvt.h"
#include <ecnt_hook/ecnt_hook_pon_mac.h>
#include "gpon/gpon_dev.h"

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
extern u32 random32(void);

#ifdef TCSUPPORT_CPU_ARMV8_64
extern struct device* get_xpon_dev(void);
#endif

/************************************************************************
*                  P U B L I C   D A T A
*************************************************************************
*/
ploam_queue_t *ploam_test_queue = NULL;
/************************************************************************
*                  P R I V A T E   D A T A
*************************************************************************
*/
static struct task_struct *gpon_test_task;
static struct task_struct *gpon_cmac0_test_task;
static struct task_struct *gpon_cmac1_test_task;

REG_INT_STATUS intStatus;
REG_INT_ENABLE intEnable;

__u32 xgponRegTestPattern[] = {
    0xffffffff,
    0x00000000,
    0x55555555,
    0xAAAAAAAA,
};

/************************************************************************
*                  F U N C T I O N   D E F I N I T I O N S
*************************************************************************
*/
ploam_queue_t *createEmptyPloamQueue(void)
{
	ploam_queue_t *queue;
	
	queue = (ploam_queue_t *) kmalloc(sizeof(ploam_queue_t),GFP_KERNEL);
	if(NULL == queue)
		return NULL;
	queue->front = queue->rear =0;

	return queue;
}

void destoryPloamQueue(ploam_queue_t *queue){
	if(NULL != queue){
		kfree(queue);
	}
}

int enQueue(ploam_queue_t *queue, uint data){
	
	if(NULL == queue)
		return -1;
	if(((queue->rear +1) % PLOAM_TEST_QUEUE_LEN) == queue->front)
		return -1;
	
	queue->rear = (queue->rear +1) % PLOAM_TEST_QUEUE_LEN;	
	queue->data[queue->rear] = data;	

	return 0;
}

int deQueue(ploam_queue_t *queue, uint *data){
	
	if(NULL == queue)
		return -1;
	if(queue->front == queue->rear)
		return -1;
	
	queue->front = (queue->front +1) % PLOAM_TEST_QUEUE_LEN;

	if(NULL !=data)
		*data = queue->data[queue->front];

	return 0;	
}

int gpon_act_test(void *pdata){

	const int schedTime = HZ;
	
	while(!kthread_should_stop()){

		if(ploam_test_queue->front != ploam_test_queue->rear)
			gpon_isr();		
		
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(schedTime);
		set_current_state(TASK_RUNNING);
	}

	return 0;
}
/******************************************************************************
******************************************************************************/

int gpon_dvt_gemport(int times)
{
	GPONDEV_GemPortInfo_T gemPort[CONFIG_GPON_10G_MAX_GEMPORT] ;
	unchar valid=0; 
    unchar encrypt=0;
    unchar type=0 ;
	int n=0;
    int i=0;
    int ret =0;
	ushort tmp = 0 ;
    ushort tmpx = 0 ;
	
	gponDevResetCtrl(XPON_RESET_RELEASE);
	gponDevResetGemInfo() ;
	
	for(n=0 ; n<times ; n++) {
		tmp = 0;
		for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
			memset(&gemPort[i], 0x0, sizeof(GPONDEV_GemPortInfo_T)) ;
            tmp += (i==0) ? ((ushort)random32()%256) : (((ushort)random32()%256)+1) ;
			gemPort[i].gemPortId = tmp ;
			gemPort[i].isValid = (random32()%2) ? 1 : 0 ;
			gemPort[i].isType = (random32()%2) ? 1 : 0 ;
			gemPort[i].isEncrypted = (random32()%2) ? 1 : 0 ;			
			ret = gponDevSetGemInfo(gemPort[i].gemPortId, gemPort[i].isValid, gemPort[i].isType,gemPort[i].isEncrypted);			
			if(ret != 0) {
				printk("Write the GEM port index :%d, tmpx:%d ID:%d valid:%d type:%d Encrypt:%d failed\n", i,tmpx, tmp, gemPort[i].isValid,gemPort[i].isType,gemPort[i].isEncrypted) ;	
				return -EFAULT ;
			}
		}

		for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
			ret = gponDevGetGemInfo(gemPort[i].gemPortId, &valid, &type,&encrypt);
			if(ret != 0) {
				printk("Get GEM port ID:%d failed\n", i) ;
				return -EFAULT ;
			} else {
				if((gemPort[i].isValid != valid) || (gemPort[i].isEncrypted!=encrypt) || (gemPort[i].isType !=type)) {
					printk(" The GEM port index:%d, field mismatch. ID:%d  write_valid:%d valid:%d ;write_type:%d type:%d ;write_Encrypt:%d Encrypt:%d\n", 
						i,gemPort[i].gemPortId,gemPort[i].isValid,valid,gemPort[i].isType,type,gemPort[i].isEncrypted,encrypt) ;	
					return -EFAULT ;
				}
			}
		}
		msleep(10);
		if((n%1000) == 999) {
			printk("Repeat to verification Gem Port CSR (%d times)\n", i) ;
		}

		gponDevResetGemInfo() ;
	}

	printk("Gem Port CSR verification successful (%d times)\n", times) ;
	return 0 ;
}

/*****************************************************************************
******************************************************************************/

void gpon_index_table(int times)
{
	GPONDEV_GPIDIndexInfo_T gpidx[CONFIG_GPON_10G_MAX_GEMPORT] ;
	uint readData =0 ;
    uint i=0;
	int n=0; 
	uint tmp = 0 ;	

	memset(gpidx, 0, sizeof(gpidx));

	gponDevResetCtrl(XPON_RESET_RELEASE);
    gponDevGemMibTablesInit() ; 
    gponDevUpdateGemMibIdxTable(0, 0) ;
    
	for(n=0 ; n<times ; n++) {
        for(i=1 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {            
            memset(&gpidx[i], 0x0, sizeof(GPONDEV_GPIDIndexInfo_T)) ;
            tmp += (((ushort)random32()%7)+1) ;
			if((tmp&0x7ff) == 0){
				tmp = (((ushort)random32()%7)+1);
			}
            gpidx[i].tableAddr = (tmp & 0x7ff);
            gpidx[i].writeData= (random32() & 0x1ff);
            gpidx[i].readData= 0x0;
            gponDevUpdateGemMibIdxTable(gpidx[i].tableAddr, gpidx[i].writeData) ; 
        }
        for(i=0 ; i<CONFIG_GPON_10G_MAX_GEMPORT ; i++) {
            gponDevGetGemMibIdxTable(gpidx[i].tableAddr, &readData) ; 
            if(readData != gpidx[i].writeData){
                printk("round:%d failed i:%d addr:%d readData:%d writeData:%d \n",n,i,gpidx[i].tableAddr,readData,gpidx[i].writeData);
                return;
            }               
        }
        msleep(10);
    	if((n%1000) == 999) {
    		printk("Repeat to verification Gem Port index CSR (%d times)\n", i) ;
        }    
    }    
   
	printk("Gem Port index table verification successful (%d times)\n", times) ;
	return;
}
/*****************************************************************************
Descriptor:    start cmac calculate downstrem ploam mic with the golden form spec IV.7
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponGoldenDsPloamMicCheck(unchar cmacIndex){

    int ret = 0;
    unchar ploam_ik_index=0;
    unchar i=0;
    unchar golden_ploam_content[40] ={0x00,0x13,0x0a,0x03,0x04,0x45,0x1,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
	unchar ploam_IK[16]={0xe2,0x56,0xce,0x76,0x78,0x5c,0x78,0x71,0x7c,0x7b,0x30,0x44,0xab,0x28,0xe2,0xcd};    
    unchar golden_ploam_mic[8]={0x46,0x39,0x87,0x56,0x28,0x08,0x14,0xe6};
    
    unchar *golden_ploam_data = NULL ;		
	unchar *golden_ploam_reslt = NULL ;

#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_addr_t dram_glodenPloamData_phy;
	dma_addr_t dram_glodenPloamReslt_phy;	
	int count = 0;

	struct device *xpon_dev = NULL;

	if((xpon_dev = get_xpon_dev()) == NULL)
	{
		printk("\nget xpon dev fail\n");
		return;
	}

	PON_MSG(MSG_DBG, "Ds ARM architecture start test gponGoldenDsPloamMicCheck\n") ;
	for (count = 0; (golden_ploam_data==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_ploam_data = (unchar *)dma_alloc_coherent(xpon_dev, 40, &dram_glodenPloamData_phy, GFP_ATOMIC);
	}

	if(golden_ploam_data == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: DS ARM dma_alloc_coherent for golden_ploam_data fail\n");
		return ;
	}
	
	for (count = 0; (golden_ploam_reslt==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_ploam_reslt = (unchar *)dma_alloc_coherent(xpon_dev, 9, &dram_glodenPloamReslt_phy, GFP_ATOMIC);
	}

	if(golden_ploam_reslt == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: DS ARM dma_alloc_coherent for golden_ploam_reslt fail\n");
		dma_free_coherent(xpon_dev, 40, golden_ploam_data, dram_glodenPloamData_phy);
		return ;
	}
#else
	golden_ploam_data = CACHE_TO_NONCACHE((unchar *)kmalloc(40, GFP_KERNEL)) ;
	if(golden_ploam_data == NULL){
		return;
	}
	golden_ploam_reslt = CACHE_TO_NONCACHE((unchar *)kmalloc(9, GFP_KERNEL));
	if(golden_ploam_reslt == NULL){
		kfree(NONCACHE_TO_CACHE(golden_ploam_data));
		return;
	}
#endif
    
    memcpy(golden_ploam_data,golden_ploam_content, 40);
    XPON_ARR_PRINT(MSG_DBG,"\ngolden_ploam_data: ","%x ",golden_ploam_data,40);
    XPON_ARR_PRINT(MSG_DBG,"ploam_IK: ","%x ",ploam_IK,16);
    
    for(ploam_ik_index = 0;ploam_ik_index < 2;ploam_ik_index++){      
        
        if(ploam_ik_index == 0)
            gponDevSetPloamIk0(ploam_IK);
        else
            gponDevSetPloamIk1(ploam_IK);
        
        memset(golden_ploam_reslt, 0x0, 9);
        PON_MSG(MSG_DBG, "cmac%d with  ploam ik%d:\n",cmacIndex,ploam_ik_index) ;
        if(cmacIndex == GPON_CMAC_IDX0)
        {
        	#ifdef TCSUPPORT_CPU_ARMV8_64
				ret = gponDevSetCmac0Start(GPON_CMAC_PLOAM_IDX0+ploam_ik_index,
					  GPON_CMAC_DOWNSTREAM,NULL,(unchar *)dram_glodenPloamData_phy,40,(unchar *)dram_glodenPloamReslt_phy,golden_ploam_reslt,9);
			#else
            ret =gponDevSetCmac0Start(GPON_CMAC_PLOAM_IDX0+ploam_ik_index,
               		 GPON_CMAC_DOWNSTREAM,NULL,golden_ploam_data,40,golden_ploam_reslt,NULL,9);
			#endif
        }
        else                
        {
           	#ifdef TCSUPPORT_CPU_ARMV8_64
            ret =gponDevSetCmac1Start(GPON_CMAC_PLOAM_IDX0+ploam_ik_index,
				  	  GPON_CMAC_DOWNSTREAM,NULL,(unchar *)dram_glodenPloamData_phy,40,(unchar *)dram_glodenPloamReslt_phy,golden_ploam_reslt,9);
			#else
            	ret =gponDevSetCmac1Start(GPON_CMAC_PLOAM_IDX0+ploam_ik_index,
                	 GPON_CMAC_DOWNSTREAM,NULL,golden_ploam_data,40,golden_ploam_reslt,NULL,9);
			#endif
        }
		
        if(ret != 0)
            printk("cmac%d calculate ploam mic with ploam IK%d fail\n",cmacIndex,ploam_ik_index);

        for(i=0;i<8;i++){
            if(*(golden_ploam_reslt+i) != golden_ploam_mic[i])
                printk("cmac%d calculte with ploam key index:%d mic value wrong\n",cmacIndex,ploam_ik_index) ;
        }
        XPON_ARR_PRINT(MSG_DBG,"cmac0 golden ploam reslt noncache data: ","%x ",golden_ploam_reslt,9);            
   }

#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_free_coherent(xpon_dev, 40, golden_ploam_data, dram_glodenPloamData_phy);
	dma_free_coherent(xpon_dev, 9, golden_ploam_reslt, dram_glodenPloamReslt_phy);
#else
    kfree(NONCACHE_TO_CACHE(golden_ploam_data));
    kfree(NONCACHE_TO_CACHE(golden_ploam_reslt));
#endif
}
/*****************************************************************************
Descriptor:    start cmac calculate upstrem ploam mic with the golden form spec IV.8
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponGoldenUsPloamMicCheck(unchar cmacIndex){

    int ret =0;
    unchar ploam_ik_index = 0;
    unchar i = 0;
    unchar golden_ploam_content[40] ={0x0,0x13,0x10,0x0,0x02,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0};
	unchar ploam_IK[16]={0xe2,0x56,0xce,0x76,0x78,0x5c,0x78,0x71,0x7c,0x7b,0x30,0x44,0xab,0x28,0xe2,0xcd};    
    unchar golden_ploam_mic[8]={0x68,0xae,0x4d,0xd7,0x75,0x55,0x0a,0xcb};
    
    unchar *golden_ploam_data = NULL;	
	unchar *golden_ploam_reslt = NULL ;

#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_addr_t dram_glodenPloamData_phy;
	dma_addr_t dram_glodenPloamReslt_phy;
	int count = 0;

	struct device *xpon_dev = NULL;

	if((xpon_dev = get_xpon_dev()) == NULL)
	{
		printk("\nget xpon dev fail\n");
		return;
	}

	PON_MSG(MSG_DBG, "Us ARM architecture start test gponGoldenUsPloamMicCheck\n") ;
	for (count = 0; (golden_ploam_data==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_ploam_data = (unchar *)dma_alloc_coherent(xpon_dev, 40, &dram_glodenPloamData_phy, GFP_ATOMIC);
	}

	if(golden_ploam_data == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: US ARM dma_alloc_coherent for golden_ploam_data fail\n");
		return ;
	}
	
	for (count = 0; (golden_ploam_reslt==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_ploam_reslt = (unchar *)dma_alloc_coherent(xpon_dev, 9, &dram_glodenPloamReslt_phy, GFP_ATOMIC);
	}

	if(golden_ploam_reslt == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: US ARM dma_alloc_coherent for golden_ploam_reslt fail\n");
		dma_free_coherent(xpon_dev, 40, golden_ploam_data, dram_glodenPloamData_phy);
		return ;
	}
#else
	golden_ploam_data = CACHE_TO_NONCACHE((unchar *)kmalloc(40, GFP_KERNEL)) ;
    if(golden_ploam_data == NULL){
        return;
    }
    golden_ploam_reslt = CACHE_TO_NONCACHE((unchar *)kmalloc(9, GFP_KERNEL));
	if(golden_ploam_reslt == NULL){
		kfree(NONCACHE_TO_CACHE(golden_ploam_data));
		return;
	}
 #endif
    
    memcpy(golden_ploam_data,golden_ploam_content, 40);
    XPON_ARR_PRINT(MSG_DBG,"\ngolden_ploam_data: ","%x ",golden_ploam_data,40);
    XPON_ARR_PRINT(MSG_DBG,"ploam_IK: ","%x ",ploam_IK,16);
    
    for(ploam_ik_index = 0;ploam_ik_index < 2;ploam_ik_index++){
             
        if(ploam_ik_index == 0)
            gponDevSetPloamIk0(ploam_IK);
        else
            gponDevSetPloamIk1(ploam_IK);
        
        memset(golden_ploam_reslt, 0x0, 9);
        PON_MSG(MSG_DBG, "cmac%d with  ploam ik%d:\n",cmacIndex,ploam_ik_index) ;
        if(cmacIndex == GPON_CMAC_IDX0)
        {
        	#ifdef TCSUPPORT_CPU_ARMV8_64
				ret = gponDevSetCmac0Start(GPON_CMAC_PLOAM_IDX0+ploam_ik_index,
					  GPON_CMAC_UPSTREAM,NULL,(unchar *)dram_glodenPloamData_phy,40,(unchar *)dram_glodenPloamReslt_phy,golden_ploam_reslt,9);
			#else
            ret = gponDevSetCmac0Start(GPON_CMAC_PLOAM_IDX0+ploam_ik_index,
                	GPON_CMAC_UPSTREAM,NULL,golden_ploam_data,40,golden_ploam_reslt,NULL,9);
			#endif
        }
        else                
        {
        	#ifdef TCSUPPORT_CPU_ARMV8_64
				ret = gponDevSetCmac1Start(GPON_CMAC_PLOAM_IDX0+ploam_ik_index,
					  GPON_CMAC_UPSTREAM,NULL,(unchar *)dram_glodenPloamData_phy,40,(unchar *)dram_glodenPloamReslt_phy,golden_ploam_reslt,9);
			#else
            ret =gponDevSetCmac1Start(GPON_CMAC_PLOAM_IDX0+ploam_ik_index,
               		 GPON_CMAC_UPSTREAM,NULL,golden_ploam_data,40,golden_ploam_reslt,NULL,9);
			#endif
        }
		
        if(ret != 0)
            printk("cmac%d calculate ploam mic with ploam IK%d fail\n",cmacIndex,ploam_ik_index);

        for(i=0;i<8;i++){
            if(*(golden_ploam_reslt+i) != golden_ploam_mic[i])
                printk("cmac%d calculte with ploam key index:%d mic value wrong\n",cmacIndex,ploam_ik_index) ;
        }
        XPON_ARR_PRINT(MSG_DBG,"cmac0 golden ploam reslt noncache data: ","%x ",golden_ploam_reslt,9);            
   }

#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_free_coherent(xpon_dev, 40, golden_ploam_data, dram_glodenPloamData_phy);
	dma_free_coherent(xpon_dev, 9, golden_ploam_reslt, dram_glodenPloamReslt_phy);
#else
    kfree(NONCACHE_TO_CACHE(golden_ploam_data));
    kfree(NONCACHE_TO_CACHE(golden_ploam_reslt));
#endif
}
/*****************************************************************************
Descriptor:    start cmac calculate data key hash with kek with the golden form spec IV.9
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponGoldenKeyReportingCheck(unchar cmacIndex){

    int ret = 0;
    unchar kek_index = 0;
    unchar i = 0;
    unchar datakey[GPON_DATA_ENCRYPT_KEY_LENS] ={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00};
	unchar keyName[16] ={0x33,0x31,0x34,0x31,0x35,0x39,0x32,0x36,0x35,0x33,0x35,0x38,0x39,0x37,0x39,0x33};
	unchar kek[GPON_KEK_LENS] ={0x6f,0x9c,0x99,0xb8,0x36,0x17,0x68,0x93,0x7e,0x45,0x3b,0x16,0x5f,0x60,0x97,0x10};
    unchar golden_key_hash[16]={0x3c,0xc5,0x07,0xbb,0x17,0x31,0xc5,0x69,0xed,0x7b,0x79,0xf8,0xbd,0xc3,0x76,0xbe};
    
    unchar *golden_data_key_noncache = NULL;
	unchar *golden_reslt_noncache =NULL;

#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_addr_t dram_glodenDataKey_phy;
	dma_addr_t dram_glodenReslt_phy;
	int count = 0;

	struct device *xpon_dev = NULL;

	if((xpon_dev = get_xpon_dev()) == NULL)
	{
		printk("\nget xpon dev fail\n");
		return;
	}

	PON_MSG(MSG_DBG, "ARM architecture start test gponGoldenKeyReportingCheck\n") ;
	for (count = 0; (golden_data_key_noncache==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_data_key_noncache = (unchar *)dma_alloc_coherent(xpon_dev, 32, &dram_glodenDataKey_phy, GFP_ATOMIC);
	}

	if(golden_data_key_noncache == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: ARM dma_alloc_coherent for golden_data_key_noncache fail\n");
		return ;
	}
	
	for (count = 0; (golden_reslt_noncache==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_reslt_noncache = (unchar *)dma_alloc_coherent(xpon_dev, 17, &dram_glodenReslt_phy, GFP_ATOMIC);
	}

	if(golden_reslt_noncache == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: ARM dma_alloc_coherent for golden_reslt_noncache fail\n");
		dma_free_coherent(xpon_dev, 32, golden_data_key_noncache, dram_glodenDataKey_phy);
		return ;
	}
#else
    golden_data_key_noncache =CACHE_TO_NONCACHE((unchar *)kmalloc(32, GFP_KERNEL));
	if(golden_data_key_noncache == NULL){
		return;
	}
	golden_reslt_noncache =CACHE_TO_NONCACHE((unchar *)kmalloc(17, GFP_KERNEL));
	if(golden_reslt_noncache == NULL){
		kfree(NONCACHE_TO_CACHE(golden_data_key_noncache));
		return;
	}
 #endif
    
	memcpy(golden_data_key_noncache,datakey, GPON_DATA_ENCRYPT_KEY_LENS);
	memcpy(golden_data_key_noncache+16,keyName, GPON_DATA_ENCRYPT_KEY_LENS);
	XPON_ARR_PRINT(MSG_DBG,"\ngolden_data_key_noncache data: ","%x ",golden_data_key_noncache,32);

    for(kek_index = 0;kek_index < 2;kek_index++){
        if(kek_index == 0)
            gponDevSetKEK0(kek);
        else
            gponDevSetKEK1(kek);
        memset(golden_reslt_noncache, 0, 17);            
        PON_MSG(MSG_DBG, "cmac%d with  kek ik%d:\n",cmacIndex,kek_index) ;
        if(cmacIndex == GPON_CMAC_IDX0)
        {
        	#ifdef TCSUPPORT_CPU_ARMV8_64
				ret = gponDevSetCmac0Start(GPON_CMAC_KEK_IDX0+kek_index,
					GPON_CMAC_NORMAL,NULL,(unchar *)dram_glodenDataKey_phy,32,(unchar *)dram_glodenReslt_phy,golden_reslt_noncache,17);

			#else
    	    ret = gponDevSetCmac0Start(GPON_CMAC_KEK_IDX0+kek_index,
    	        	GPON_CMAC_NORMAL,NULL,golden_data_key_noncache,32,golden_reslt_noncache,NULL,17);
			#endif
        }
        else 
        {
        	#ifdef TCSUPPORT_CPU_ARMV8_64
				ret = gponDevSetCmac1Start(GPON_CMAC_KEK_IDX0+kek_index,
					GPON_CMAC_NORMAL,NULL,(unchar *)dram_glodenDataKey_phy,32,(unchar *)dram_glodenReslt_phy,golden_reslt_noncache,17);

			#else
            ret = gponDevSetCmac1Start(GPON_CMAC_KEK_IDX0+kek_index,
                	GPON_CMAC_NORMAL,NULL,golden_data_key_noncache,32,golden_reslt_noncache,NULL,17);
			#endif
        }

        if(ret != 0)
            printk("cmac%d kek hash fail\n",cmacIndex);
        
    	for(i=0;i<16;i++){
            if(*(golden_reslt_noncache+i) != golden_key_hash[i])
                printk("cmac%d calculte with kek index:%d mic value wrong\n",cmacIndex,kek_index) ;
        }
    	XPON_ARR_PRINT(MSG_DBG,"cmac golden reslt noncache data: ","%x ",golden_reslt_noncache,17);  

    }
	
#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_free_coherent(xpon_dev, 32, golden_data_key_noncache, dram_glodenDataKey_phy);
	dma_free_coherent(xpon_dev, 17, golden_reslt_noncache, dram_glodenReslt_phy);
#else
    kfree(NONCACHE_TO_CACHE(golden_data_key_noncache));
    kfree(NONCACHE_TO_CACHE(golden_reslt_noncache));
#endif
}

/*****************************************************************************
Descriptor:    start cmac calculate omci mic with the golden form spec IV.10
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponGoldenDsOmciMicCheck(unchar cmacIndex){

    int ret = 0;
    unchar omci_ik_index = 0;
    unchar i=0;
    unchar golden_omci_content[44] ={0x80,0x0,0x49,0x0a,0x01,0x0,0x0,0x0,
                                         0x0,0x80,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
                                         0x0,0x0,0x0,0x28};
	unchar omci_IK[16]={0x18,0x4b,0x8a,0xd4,0xd1,0xac,0x4a,0xf4,0xdd,0x4b,0x33,0x9e,0xcc,0x0d,0x33,0x70};
    unchar golden_omci_mic[4]={0x78,0xdc,0xa5,0x3d};
    unchar *golden_omci_data = NULL;	
	unchar *golden_omci_reslt = NULL ;

#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_addr_t dram_glodenOmciData_phy;
	dma_addr_t dram_glodenOmciReslt_phy;
	int count = 0;

	struct device *xpon_dev = NULL;

	if((xpon_dev = get_xpon_dev()) == NULL)
	{
		printk("\nget xpon dev fail\n");
		return;
	}

	PON_MSG(MSG_DBG, "ARM architecture start test gponGoldenDsOmciMicCheck\n") ;
	for (count = 0; (golden_omci_data==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_omci_data = (unchar *)dma_alloc_coherent(xpon_dev, 44, &dram_glodenOmciData_phy, GFP_ATOMIC);
	}
	
	if(golden_omci_data == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: DS ARM dma_alloc_coherent for golden_omci_data fail\n");
		return ;
	}
	
	for (count = 0; (golden_omci_reslt==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_omci_reslt = (unchar *)dma_alloc_coherent(xpon_dev, 5, &dram_glodenOmciReslt_phy, GFP_ATOMIC);
	}

	if(golden_omci_reslt == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM: DS ARM dma_alloc_coherent for golden_omci_reslt fail\n");
		dma_free_coherent(xpon_dev, 5, golden_omci_reslt, dram_glodenOmciReslt_phy);
		return ;
	}
#else
    golden_omci_data = CACHE_TO_NONCACHE((unchar *)kmalloc(44, GFP_KERNEL)) ;	
	if(golden_omci_data == NULL){
		return;
	}
	golden_omci_reslt = CACHE_TO_NONCACHE((unchar *)kmalloc(5, GFP_KERNEL)) ;
	if(golden_omci_reslt == NULL){
		kfree(NONCACHE_TO_CACHE(golden_omci_data));
		return;
	}
#endif
	
    memcpy(golden_omci_data,golden_omci_content, 44);
    XPON_ARR_PRINT(MSG_DBG,"\ngolden_omci_data: ","%x ",golden_omci_data,44);
    XPON_ARR_PRINT(MSG_DBG,"omci_IK: ","%x ",omci_IK,16);
    
    for(omci_ik_index = 0;omci_ik_index < 2;omci_ik_index++){
        if(omci_ik_index == 0)
            gponDevSetOmciIk0(omci_IK);
        else
            gponDevSetOmciIk1(omci_IK);
        
        PON_MSG(MSG_DBG, "cmac%d with  omci ik%d:\n",cmacIndex,omci_ik_index) ;
        memset(golden_omci_reslt, 0x0,5);
        if(cmacIndex == GPON_CMAC_IDX0)
        {
        	#ifdef TCSUPPORT_CPU_ARMV8_64
            ret =gponDevSetCmac0Start(GPON_CMAC_OMCI_IDX0+omci_ik_index,
					GPON_CMAC_DOWNSTREAM,NULL,(unchar *)dram_glodenOmciData_phy,44,(unchar *)dram_glodenOmciReslt_phy,golden_omci_reslt,5);
			#else
            	ret =gponDevSetCmac0Start(GPON_CMAC_OMCI_IDX0+omci_ik_index,
                	GPON_CMAC_DOWNSTREAM,NULL,golden_omci_data,44,golden_omci_reslt,NULL,5);
			#endif
        }
        else
        {
        	#ifdef TCSUPPORT_CPU_ARMV8_64
            ret =gponDevSetCmac1Start(GPON_CMAC_OMCI_IDX0+omci_ik_index,
               	 	GPON_CMAC_DOWNSTREAM,NULL,(unchar *)dram_glodenOmciData_phy,44,(unchar *)dram_glodenOmciReslt_phy,golden_omci_reslt,5);
			#else
            	ret =gponDevSetCmac1Start(GPON_CMAC_OMCI_IDX0+omci_ik_index,
               	 	GPON_CMAC_DOWNSTREAM,NULL,golden_omci_data,44,golden_omci_reslt,NULL,5);
			#endif
        }
        
        if(ret != 0)
            PON_MSG(MSG_DBG, "cmac%d calculate omci mic fail with key index:%d\n",cmacIndex,omci_ik_index) ;
        for(i=0;i<4;i++){
            if(*(golden_omci_reslt+i) != golden_omci_mic[i])
                printk("cmac%d calculte with omci key index:%d mic value wrong\n",cmacIndex,omci_ik_index) ;
        }
        XPON_ARR_PRINT(MSG_DBG,"cmac golden Downstream omci mic reslt noncache data: ","%x ",golden_omci_reslt,5);
    }    
    
#ifdef TCSUPPORT_CPU_ARMV8_64
  	dma_free_coherent(xpon_dev, 44, golden_omci_data, dram_glodenOmciData_phy);
	dma_free_coherent(xpon_dev, 5, golden_omci_reslt, dram_glodenOmciReslt_phy);
#else
    kfree(NONCACHE_TO_CACHE(golden_omci_data));
    kfree(NONCACHE_TO_CACHE(golden_omci_reslt));
#endif
}
/*****************************************************************************
Descriptor:    start cmac normal encryption with the golden form spec IV.10
Input Args:    none
Ret Value:     none
******************************************************************************/
void gponGoldenNormalEncryptCheck(unchar cmacIndex){

    int ret =0;
    unchar i=0;
    int j=0;
    ulong flags = 0;    
    unchar data[GPON_DATA_ENCRYPT_KEY_LENS] ={0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff,0x00};
    unchar keyName[16] ={0x33,0x31,0x34,0x31,0x35,0x39,0x32,0x36,0x35,0x33,0x35,0x38,0x39,0x37,0x39,0x33};
    unchar key[GPON_KEK_LENS] ={0x6f,0x9c,0x99,0xb8,0x36,0x17,0x68,0x93,0x7e,0x45,0x3b,0x16,0x5f,0x60,0x97,0x10};
    unchar golden_key_hash[16]={0x3c,0xc5,0x07,0xbb,0x17,0x31,0xc5,0x69,0xed,0x7b,0x79,0xf8,0xbd,0xc3,0x76,0xbe};
    
    unchar *golden_data_noncache = NULL;
	unchar *golden_reslt_noncache = NULL;
	unchar *golden_data_key_noncache = NULL;
	
#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_addr_t dram_glodenDataKey_phy;
	dma_addr_t dram_glodenReslt_phy;
	dma_addr_t dram_glodenDataKEK_phy;
	int count = 0;

	struct device *xpon_dev = NULL;

	if((xpon_dev = get_xpon_dev()) == NULL)
	{
		printk("\nget xpon dev fail\n");
		return;
	}

	PON_MSG(MSG_DBG, "ARM architecture start test gponGoldenNormalEncryptCheck\n") ;
	for (count = 0; (golden_data_noncache==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_data_noncache = (unchar *)dma_alloc_coherent(xpon_dev, 32, &dram_glodenDataKey_phy, GFP_ATOMIC);
	}

	if(golden_data_noncache == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM:ARM dma_alloc_coherent for Normal golden_data_noncache fail\n");
		return ;
	}
	
	for (count = 0; (golden_reslt_noncache==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_reslt_noncache = (unchar *)dma_alloc_coherent(xpon_dev, 17, &dram_glodenReslt_phy, GFP_ATOMIC);
	}

	if(golden_reslt_noncache == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM:ARM dma_alloc_coherent for Normal golden_reslt_noncache fail\n");
		dma_free_coherent(xpon_dev, 32, golden_data_noncache, dram_glodenDataKey_phy);
		return ;
	}
	
	for (count = 0; (golden_data_key_noncache==NULL)&&(count<DMA_ALLOC_MAX_NUM); count++)
	{
		golden_data_key_noncache = (unchar *)dma_alloc_coherent(xpon_dev, 16, &dram_glodenDataKEK_phy, GFP_ATOMIC);
	}

	if(golden_data_key_noncache == NULL)
	{
		PON_MSG(MSG_DBG,"PLOAM:ARM dma_alloc_coherent for Normal golden_data_key_noncache fail\n");
		dma_free_coherent(xpon_dev, 32, golden_data_noncache, dram_glodenDataKey_phy);
		dma_free_coherent(xpon_dev, 17, golden_reslt_noncache, dram_glodenReslt_phy);
		return ;
	}
#else
	golden_data_noncache =CACHE_TO_NONCACHE((unchar *)kmalloc(32, GFP_KERNEL));
	if(golden_data_noncache == NULL){
		return;
	}
	golden_reslt_noncache =CACHE_TO_NONCACHE((unchar *)kmalloc(17, GFP_KERNEL));
	if(golden_reslt_noncache == NULL){
		kfree(NONCACHE_TO_CACHE(golden_data_noncache));
		return;
	}		
	golden_data_key_noncache =CACHE_TO_NONCACHE((unchar *)kmalloc(16, GFP_KERNEL));
    if(golden_data_key_noncache == NULL){
		kfree(NONCACHE_TO_CACHE(golden_reslt_noncache));
		kfree(NONCACHE_TO_CACHE(golden_data_noncache));
		return;
	}
	
	dma_cache_inv((long unsigned int)NONCACHE_TO_CACHE(golden_data_noncache), GPON_DATA_ENCRYPT_KEY_LENS);
    dma_cache_inv((long unsigned int)NONCACHE_TO_CACHE(golden_data_noncache+16), GPON_DATA_ENCRYPT_KEY_LENS);
    dma_cache_inv((long unsigned int)NONCACHE_TO_CACHE(golden_data_key_noncache), GPON_DATA_ENCRYPT_KEY_LENS);
#endif

	spin_lock_irqsave(&gpGponPriv->cmac_test, flags) ;
	
    memcpy(golden_data_noncache,data, GPON_DATA_ENCRYPT_KEY_LENS);
    memcpy(golden_data_noncache+16,keyName, GPON_DATA_ENCRYPT_KEY_LENS);
    memcpy(golden_data_key_noncache,key, GPON_DATA_ENCRYPT_KEY_LENS);
    XPON_ARR_PRINT(MSG_DBG,"\ngolden_data_key_noncache data: ","%x ",golden_data_noncache,32);
    
    memset(golden_reslt_noncache, 0x0, 17);

    for(i=0; i<16; i++){
        if(*(golden_data_noncache+i) != data[i]){            
            printk("cmac%d data: %p write wrong0~15 >>",cmacIndex,golden_data_noncache) ;            
            XPON_ARR_PRINT_TEST(" data: ","%x ",golden_data_noncache,32);
            for(j=0;j<16;j++){
                if(*(golden_data_noncache+i) != data[i]){
                    printk("cmac%d data: %p write wrong0~15 >>",cmacIndex,golden_data_noncache) ;                    
                    XPON_ARR_PRINT_TEST(" data: ","%x ",golden_data_noncache,32);
                }
            }
            break;
        }
    }

    for(i=0; i<16; i++){
        if(*(golden_data_noncache+16+i) != keyName[i]){
            printk("cmac%d data: %p write wrong16~31 >>",cmacIndex,golden_data_noncache) ;
            XPON_ARR_PRINT_TEST(" data: ","%x ",golden_data_noncache,32);
            for(j=0;j<16;j++){
                if(*(golden_data_noncache+16+i) != keyName[i]){
                    printk("cmac%d data: %p write wrong16~31 >>",cmacIndex,golden_data_noncache) ;                    
                    XPON_ARR_PRINT_TEST(" data: ","%x ",golden_data_noncache,32);
                }
            }
            break;
        }
    }
    
    for(i=0; i<16; i++){
        if(*(golden_data_key_noncache+i) != key[i]){
            printk("cmac%d key: %p write wrong >>",cmacIndex,golden_data_key_noncache) ;
            XPON_ARR_PRINT_TEST(" data: ","%x ",golden_data_key_noncache,16);
            for(j=0;j<16;j++){
                if(*(golden_data_key_noncache+i) != key[i]){
                    printk("cmac%d key: %p write wrong >>",cmacIndex,golden_data_key_noncache) ;
                    XPON_ARR_PRINT_TEST(" data: ","%x ",golden_data_key_noncache,16);
                }
            }
            break;
        }
    }
    
    for(i=0; i<16; i++){
        if(*(golden_reslt_noncache+i) != 0){
            printk("cmac%d result: %p set 0 wrong >>",cmacIndex,golden_reslt_noncache) ;
            XPON_ARR_PRINT_TEST(" data: ","%x ",golden_reslt_noncache,16);
            for(j=0;j<16;j++){
                if(*(golden_reslt_noncache+i) != 0){
                    printk("cmac%d result: %p set 0 wrong >>",cmacIndex,golden_reslt_noncache) ;
                    XPON_ARR_PRINT_TEST(" data: ","%x ",golden_reslt_noncache,16);
                }
            }
            break;
        }
    }

    #if 1
    PON_MSG(MSG_DBG, "cmac%d normal encryption \n",cmacIndex) ;
    if(cmacIndex == GPON_CMAC_IDX0)
    {
    	#ifdef TCSUPPORT_CPU_ARMV8_64
        ret = gponDevSetCmac0Start(GPON_CMAC_NORMAL_ENCRYPT,GPON_CMAC_NORMAL,
				  (unchar *)dram_glodenDataKEK_phy,(unchar *)dram_glodenDataKey_phy,32,(unchar *)dram_glodenReslt_phy,golden_reslt_noncache,17);
		#else
       		ret = gponDevSetCmac0Start(GPON_CMAC_NORMAL_ENCRYPT,GPON_CMAC_NORMAL,
            	golden_data_key_noncache,golden_data_noncache,32,golden_reslt_noncache,NULL,17);
		#endif
    }
    else 
    {
      	#ifdef TCSUPPORT_CPU_ARMV8_64
			ret = gponDevSetCmac1Start(GPON_CMAC_NORMAL_ENCRYPT,GPON_CMAC_NORMAL,
				(unchar *)dram_glodenDataKEK_phy,(unchar *)dram_glodenDataKey_phy,32,(unchar *)dram_glodenReslt_phy,golden_reslt_noncache,17);
		#else
        ret = gponDevSetCmac1Start(GPON_CMAC_NORMAL_ENCRYPT,GPON_CMAC_NORMAL,
            	golden_data_key_noncache,golden_data_noncache,32,golden_reslt_noncache,NULL,17);
		#endif
    }

    if(ret == -2){
        printk("cmac%d normal encryption fail, no interrupt; msg:%p key:%p result:%p\n",
            cmacIndex,golden_data_noncache,golden_data_key_noncache,golden_reslt_noncache); 
    }
    if(ret == -1){
        printk("cmac%d normal encryption fail, no done bit; msg:%p key:%p result:%p\n",
            cmacIndex,golden_data_noncache,golden_data_key_noncache,golden_reslt_noncache); 
    }
    
    for(i=0;i<16;i++){
        if(*(golden_reslt_noncache+i) != golden_key_hash[i]){
            printk(">>cmac%d normal encryption value wrong >> msg:%p key:%p result:%p\n",
                cmacIndex,golden_data_noncache,golden_data_key_noncache,golden_reslt_noncache) ;
            XPON_ARR_PRINT_TEST(" data: ","%x ",golden_reslt_noncache,17);
            break;
        }
    }
    XPON_ARR_PRINT(MSG_DBG,"cmac golden reslt noncache data: ","%x ",golden_reslt_noncache,17);  
    #endif 

    spin_unlock_irqrestore(&gpGponPriv->cmac_test, flags) ;

#ifdef TCSUPPORT_CPU_ARMV8_64
	dma_free_coherent(xpon_dev, 32, golden_data_noncache, dram_glodenDataKey_phy);
	dma_free_coherent(xpon_dev, 17, golden_reslt_noncache, dram_glodenReslt_phy);
	dma_free_coherent(xpon_dev, 16, golden_data_key_noncache, dram_glodenDataKEK_phy);
#else
    kfree(NONCACHE_TO_CACHE(golden_data_noncache));
    kfree(NONCACHE_TO_CACHE(golden_data_key_noncache));
    kfree(NONCACHE_TO_CACHE(golden_reslt_noncache));
#endif
}
/*****************************************************************************
******************************************************************************/
int gpon_cmac0_task(void *pdata){

    const int schedTime = HZ/100;
    uint j=0;
    
	while(!kthread_should_stop()){       

        gponGoldenNormalEncryptCheck(GPON_CMAC_IDX0);

        j++;
        if(j%1000==0)
            printk("gpon_cmac0_task :%d\n",j/1000);
        
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(schedTime);
		set_current_state(TASK_RUNNING);
	}

	return 0;
}
/*****************************************************************************
******************************************************************************/
int gpon_cmac1_task(void *pdata){

	const int schedTime = HZ/100;
    uint j = 0;
    
	while(!kthread_should_stop()){        

        gponGoldenNormalEncryptCheck(GPON_CMAC_IDX1);
		j++;
        if(j%1000==0)
            printk("gpon_cmac1_task :%d\n",j/1000);		
        
		set_current_state(TASK_INTERRUPTIBLE);
		schedule_timeout(schedTime);
		set_current_state(TASK_RUNNING);
	}

	return 0;
}
/*****************************************************************************
******************************************************************************/
void gpon_aec_cmac_test(void){

    gpon_cmac0_test_task = kthread_run(&gpon_cmac0_task, NULL, "cmac0_test");
    if(IS_ERR(gpon_cmac0_test_task)){
        panic("@%s>>%d--gpon cmac0 test task init failed\n", __FUNCTION__, __LINE__);
    }

    gpon_cmac1_test_task = kthread_run(&gpon_cmac1_task, NULL, "cmac1_test");
    if(IS_ERR(gpon_cmac1_test_task)){
       panic("@%s>>%d--gpon cmac1 test task init failed\n", __FUNCTION__, __LINE__);
    }
}
/*****************************************************************************
******************************************************************************/
void gpon_rxhec3err_statistic_test(void)
{
	REG_RX_HLEND_HEC_CNT	   rx_hlend_hec_cnt;
	REG_RX_ALLOC_HEC_CNT	   rx_alloc_hec_cnt;
	rx_hlend_hec_cnt.Raw =		IO_GREG(RX_HLEND_HEC_CNT) ;
	rx_alloc_hec_cnt.Raw =		IO_GREG(RX_ALLOC_HEC_CNT) ;
	gpGponPriv->gponCfg.hec3errCtrl.hec_3err_hlend_new = rx_hlend_hec_cnt.Bits.rx_hlend_hec_3err_cnt;
	gpGponPriv->gponCfg.hec3errCtrl.hec_3err_alloc_new = rx_alloc_hec_cnt.Bits.rx_alloc_hec_3err_cnt;
	if((gpGponPriv->gponCfg.hec3errCtrl.hec_3err_hlend_new > gpGponPriv->gponCfg.hec3errCtrl.hec_3err_hlend_old) 
		|| (gpGponPriv->gponCfg.hec3errCtrl.hec_3err_alloc_new > gpGponPriv->gponCfg.hec3errCtrl.hec_3err_alloc_old)) {
		PON_MSG((MSG_INT|MSG_ERR), "Successful to get hec_3err_hlend = %d, hec_3err_alloc = %d\n",gpGponPriv->gponCfg.hec3errCtrl.hec_3err_hlend_new
			, gpGponPriv->gponCfg.hec3errCtrl.hec_3err_alloc_new);
		PON_MSG((MSG_INT|MSG_ERR), "Successful to get DS_SPF_CNT_H %.8x\n", IO_GREG(0xbfb65838)) ;
		PON_MSG((MSG_INT|MSG_ERR), "Successful to get DS_SPF_CNT_L %.8x\n", IO_GREG(0xbfb65834)) ;
		gpGponPriv->gponCfg.hec3errCtrl.hec_3err_hlend_old = gpGponPriv->gponCfg.hec3errCtrl.hec_3err_hlend_new;
		gpGponPriv->gponCfg.hec3errCtrl.hec_3err_alloc_old = gpGponPriv->gponCfg.hec3errCtrl.hec_3err_alloc_new;
	}
}
/*****************************************************************************
******************************************************************************/
int gpon_dvt_init(void){

	gpon_test_task= kthread_run(gpon_act_test, NULL, "gpon_act_test");
	if(IS_ERR(gpon_test_task)){
		panic("@%s>>%d--xpon_daemon init failed\n", __FUNCTION__, __LINE__);
	}

	ploam_test_queue = createEmptyPloamQueue();
	if(ploam_test_queue == NULL){
		printk("ploam test queue init failed\n");
		return -1;
	}
	return 0;
}

/*****************************************************************************
******************************************************************************/
int xgpon_register_test(uint testNum)
{
    regAddr_t xgponRegInfoTable[][3] = {
        /*addr,                         def_value,      rwmask*/
        {(regAddr_t)SW_RST,                        0x1,            0x1},
        {(regAddr_t)MBI_MPI_STOP,                  0x0,            0x101},
        {(regAddr_t)VENDOR_ID,                     0x0,            0xffffffff},
        {(regAddr_t)VS_SN,                         0x0,            0xffffffff},
        {(regAddr_t)ONU_ID,                        0x3ff,          0x83ff},
        {(regAddr_t)RGS_ID3_0,                     0x0,            0xffffffff},
        {(regAddr_t)RGS_ID7_4,                     0x0,            0xffffffff},
        {(regAddr_t)RGS_ID11_8,                    0x0,            0xffffffff},
        {(regAddr_t)RGS_ID15_12,                   0x0,            0xffffffff},
        {(regAddr_t)RGS_ID19_16,                   0x0,            0xffffffff},
        {(regAddr_t)RGS_ID23_20,                   0x0,            0xffffffff},
        {(regAddr_t)RGS_ID27_24,                   0x0,            0xffffffff},
        {(regAddr_t)RGS_ID31_28,                   0x0,            0xffffffff},
        {(regAddr_t)RGS_ID35_32,                   0x0,            0xffffffff},
        {(regAddr_t)INT_ENABLE,                    0x0,            0x3ffffff},
        {(regAddr_t)COR_TAG,                       0x0,            0xffff},
        {(regAddr_t)CUR_DS_PON_ID,                 0x0,            0xffffffff},
        {(regAddr_t)CUR_US_PON_ID,                 0x0,            0xffffffff},
        {(regAddr_t)CALIB_STS_L32,                 0x0,            0xffffffff},
        {(regAddr_t)CALIB_STS_H32,                 0x0,            0xffffffff},
        {(regAddr_t)TUNING_GRAN,                   0x0,            0xff},
        {(regAddr_t)STEP_TUNING_TIME,              0x0,            0xff},
        {(regAddr_t)US_RATE_CAP,                   0x3,            0x3},
        {(regAddr_t)ATTENUATION,                   0x0,            0xff},
        {(regAddr_t)POWER_LEVEL_CAP,               0x0,            0xff},
#if defined(TCSUPPORT_CPU_EN7581)
		{(regAddr_t)ACT_DEBUG_INFO, 			   0x0, 		   0xff},
#endif
        {(regAddr_t)O23_O4_PLOAMU_CTRL,            0x0,            0x1},
        {(regAddr_t)ACTIVATION_ST,                 0x1,            0xf},
        {(regAddr_t)RSP_TIME,                      0x551,          0x3fff},
        {(regAddr_t)RDM_DLY,                       0x1d20000,      0xfff0000},
        {(regAddr_t)INT_DLY,                       0x4480347,      0xffffffff},
        {(regAddr_t)EQD,                           0x0,            0xffffffff},
        {(regAddr_t)SYN_MRG,                       0x3ff,          0x3ff},
        {(regAddr_t)US_PROF_VLD,                   0x0,            0xf1f1f1f1},
        {(regAddr_t)US_PROF_PSBu_LEN_0_1,          0x00180018,       0xffffffff},
        {(regAddr_t)US_PROF_PSBu_LEN_2_3,          0x00180018,       0xffffffff},
#if defined(TCSUPPORT_CPU_EN7581)
		{(regAddr_t)EXT_RDM_DLY,				   0x9d200000,		0xfff00000},
#endif
        {(regAddr_t)US_AES_KEY_CTRL,               0x0,            0x80000001},
        {(regAddr_t)DS_AES_KEY_VLD,                0x0,            0xf},
        {(regAddr_t)AES_UC_IDX0_KEY0,              0x0,            0xffffffff},
        {(regAddr_t)AES_UC_IDX0_KEY1,              0x0,            0xffffffff},
        {(regAddr_t)AES_UC_IDX0_KEY2,              0x0,            0xffffffff},
        {(regAddr_t)AES_UC_IDX0_KEY3,              0x0,            0xffffffff},
        {(regAddr_t)AES_UC_IDX1_KEY0,              0x0,            0xffffffff},
        {(regAddr_t)AES_UC_IDX1_KEY1,              0x0,            0xffffffff},
        {(regAddr_t)AES_UC_IDX1_KEY2,              0x0,            0xffffffff},
        {(regAddr_t)AES_UC_IDX1_KEY3,              0x0,            0xffffffff},
        {(regAddr_t)AES_BC_IDX0_KEY0,              0x0,            0xffffffff},
        {(regAddr_t)AES_BC_IDX0_KEY1,              0x0,            0xffffffff},
        {(regAddr_t)AES_BC_IDX0_KEY2,              0x0,            0xffffffff},
        {(regAddr_t)AES_BC_IDX0_KEY3,              0x0,            0xffffffff},
        {(regAddr_t)AES_BC_IDX1_KEY0,              0x0,            0xffffffff},
        {(regAddr_t)AES_BC_IDX1_KEY1,              0x0,            0xffffffff},
        {(regAddr_t)AES_BC_IDX1_KEY2,              0x0,            0xffffffff},
        {(regAddr_t)AES_BC_IDX1_KEY3,              0x0,            0xffffffff},
        {(regAddr_t)TCONT_ID_CFG,                  0x0,            0x81f13fff},
        {(regAddr_t)GPIDX_TBL_CTRL,                0x0,            0x87ff01ff},
        {(regAddr_t)GEM_PORT_CFG,                  0x80000000,     0x8007ffff},
        {(regAddr_t)IDLE_GEM_CTRL,                 0x100200,       0xffffff},  
#if defined(TCSUPPORT_CPU_EN7581)
		{(regAddr_t)US_DYING_GASP_CTRL, 		   0xffffe001,	   0xfffff001},
#else
        {(regAddr_t)US_DYING_GASP_CTRL,            0x3000001,      0xff000001},
#endif
        {(regAddr_t)TX_XGEM_IDLE_DATA,             0x55555555,     0xffffffff},
        {(regAddr_t)TX_OMCI_PRE_GET,               0x300101,       0xffff0101},
        {(regAddr_t)RX_OMCI_PRE_GET,               0x0,            0x1},
        {(regAddr_t)EPDROP_EN,                     0x1,            0x1},
        {(regAddr_t)MSK_0,                         0xb5d432eb,     0xffffffff},
        {(regAddr_t)MSK_1,                         0x538bb1b4,     0xffffffff},
        {(regAddr_t)MSK_2,                         0xe95e6ee3,     0xffffffff},
        {(regAddr_t)MSK_3,                         0x2437be54,     0xffffffff},
        {(regAddr_t)PIK0_0,                        0x55555555,     0xffffffff},
        {(regAddr_t)PIK0_1,                        0x55555555,     0xffffffff},
        {(regAddr_t)PIK0_2,                        0x55555555,     0xffffffff},
        {(regAddr_t)PIK0_3,                        0x55555555,     0xffffffff},
        {(regAddr_t)PIK1_0,                        0x55555555,     0xffffffff},
        {(regAddr_t)PIK1_1,                        0x55555555,     0xffffffff},
        {(regAddr_t)PIK1_2,                        0x55555555,     0xffffffff},
        {(regAddr_t)PIK1_3,                        0x55555555,     0xffffffff},
        {(regAddr_t)OIK0_0,                        0x0,            0xffffffff},
        {(regAddr_t)OIK0_1,                        0x0,            0xffffffff},
        {(regAddr_t)OIK0_2,                        0x0,            0xffffffff},
        {(regAddr_t)OIK0_3,                        0x0,            0xffffffff},
        {(regAddr_t)OIK1_0,                        0x0,            0xffffffff},
        {(regAddr_t)OIK1_1,                        0x0,            0xffffffff},
        {(regAddr_t)OIK1_2,                        0x0,            0xffffffff},
        {(regAddr_t)OIK1_3,                        0x0,            0xffffffff},
        {(regAddr_t)KEK0_0,                        0x0,            0xffffffff},
        {(regAddr_t)KEK0_1,                        0x0,            0xffffffff},
        {(regAddr_t)KEK0_2,                        0x0,            0xffffffff},
        {(regAddr_t)KEK0_3,                        0x0,            0xffffffff},
        {(regAddr_t)KEK1_0,                        0x0,            0xffffffff},
        {(regAddr_t)KEK1_1,                        0x0,            0xffffffff},
        {(regAddr_t)KEK1_2,                        0x0,            0xffffffff},
        {(regAddr_t)KEK1_3,                        0x0,            0xffffffff},
        {(regAddr_t)PON_TAG_0,                     0x0,            0xffffffff},
        {(regAddr_t)PON_TAG_1,                     0x0,            0xffffffff},
        {(regAddr_t)SW_SET_KIDX,                   0x101,          0x00000101},
        {(regAddr_t)SW0_MADDR,                     0x0,            0xffffffff},
        {(regAddr_t)SW0_RADDR,                     0x0,            0xffffffff},
        {(regAddr_t)SW0_KADDR,                     0x0,            0xffffffff},
        {(regAddr_t)SW0_ENCLEN,                    0x0,            0xffffffff},
        {(regAddr_t)SW0_ENCINFO,                   0x0,            0x70003},
        {(regAddr_t)SW1_MADDR,                     0x0,            0xffffffff},
        {(regAddr_t)SW1_RADDR,                     0x0,            0xffffffff},
        {(regAddr_t)SW1_KADDR,                     0x0,            0xffffffff},
        {(regAddr_t)SW1_ENCLEN,                    0x0,            0xffffffff},
        {(regAddr_t)SW1_ENCINFO,                   0x0,            0x70003},
        {(regAddr_t)MIB_CTRL,                      0x1,            0x101},
        {(regAddr_t)MIB_CTRL_STS,                  0x0,            0x13ffff},
        {(regAddr_t)MIB_WDATA_L32,                 0x0,            0xffffffff},
        {(regAddr_t)MIB_WDATA_H32,                 0x0,            0xffffffff},
        {(regAddr_t)TOD_CLK_PERIOD,                0x32,           0xff},
        {(regAddr_t)TOD_1PPS_WD_CTRL,              0x1312d00,      0xffffffff},
        {(regAddr_t)TOD_SFC_L,                     0x0,            0xffffffff},
        {(regAddr_t)TOD_SFC_H,                     0x0,            0x3ffff},
        {(regAddr_t)NEW_TOD_SEC,                   0x0,            0xffffffff},
        {(regAddr_t)NEW_TOD_NANO_SEC,              0x0,            0xffffffff},
        {(regAddr_t)SLEEP_CNT,                     0xffff,         0xffffffff},
        {(regAddr_t)TURNING_SFC_L,                 0x0,            0xffffffff},
        {(regAddr_t)TURNING_SFC_H,                 0x0,            0x3ffff},
        {(regAddr_t)DBG_CAP_SETTING,               0x1000299,      0xff0003ff},
        {(regAddr_t)DBG_BWM_CHK_CTRL,              0x820000,       0xffff001f},
        {(regAddr_t)DBG_DBA_BACK_DOOR,             0x0,            0x3},
        {(regAddr_t)DBG_DBA_MODIFY,                0x0,            0xffffffff},
        {(regAddr_t)DBG_RESYNC,                    0x0,            0x100},
        {(regAddr_t)DBG_PHY_DLY_CTRL,              0x101,          0x101},
        {(regAddr_t)RX_GEM_ENC_CHK,                0x0,            0x11ffff},
        {(regAddr_t)RX_MBI_FIFO_DROP_EN,           0x0,            0x1},
        {(regAddr_t)RX_MBI_HDR_FIFO_DROP_CTRL,     0x180004,       0xffffffff},
        {(regAddr_t)RX_MBI_PL_FIFO_DROP_CTRL,      0x340008,       0xffffffff},
#if defined(TCSUPPORT_CPU_EN7581)
		{(regAddr_t)DBG_CAP_SETTING1,			   0x3800,		   0x3ffb},
		{(regAddr_t)BRO_ALLOC_SPECIAL,			   0x7ff,		   0xffff},
		{(regAddr_t)CAL_GNT_SIZE_CTRL_STS,         0x1,			   0x2fffffff},
		{(regAddr_t)CAL_GNT_SIZE_TCONT_EN,		   0x0,			   0xffffffff},
		{(regAddr_t)CAL_GNT_SIZE_SUM_TCONT_SEL,    0x0,			   0x1f},
		{(regAddr_t)MONI_CTRL_FOR_NOT_GNT,		   0x13880,		   0x8fffffff},
		{(regAddr_t)US_BIP_ERR_CTRL,			   0x0,			   0xfffffff},
		{(regAddr_t)SRAM_PWR_DOWN,         		   0x0,			   0x1},
		{(regAddr_t)MBIST_DELSEL_G0,	   		   0x22222222,	   0xffffffff},
		{(regAddr_t)MBIST_DELSEL_G1,	   		   0x22222222,	   0xffffffff},
		{(regAddr_t)MBIST_DELSEL_G2,	   		   0x22222222,	   0xffffffff},
		{(regAddr_t)MBIST_DELSEL_G3,	   		   0x22222222,	   0xffffffff},
		{(regAddr_t)MBIST_DELSEL_G4,	   		   0x22222222,	   0xffffffff},
		{(regAddr_t)MBIST_DELSEL_G5,	   		   0x22222222,	   0xffffffff},
		{(regAddr_t)MBIST_DELSEL_G6,	   		   0x22222222,	   0xffffffff},
#endif
        {(regAddr_t)SNF_CTRL,                      0x0,            0x301},
        {(regAddr_t)SNF_GPID,                      0xffffffff,     0xffffffff},
        {(regAddr_t)DS_SNF_ETH_DASA_H16,           0x0,            0xffffffff},
        {(regAddr_t)SNF_ETH_TAG,                   0x70000,        0xffff0000},
        {(regAddr_t)SNF_ETH_TYPE,                  0x88b6,         0xffff},
        {(regAddr_t)US_SNF_ETH_DASA_H16,           0x10001,        0xffffffff},
        {(regAddr_t)DBG_PROBE_CTRL,                0x403f0000,     0xc03f3f1f},
        {(regAddr_t)NULL,                          0x0,            0x0},
    };
    
    uint index = 0;
    uint patternIndex = 0;
    uint value = 0;

#ifdef TCSUPPORT_CPU_ARMV8_64
	regAddr_t reg_addr = 0;
#else
    __u32 * reg_addr = NULL;
#endif
    
    while(testNum > 0)
    {
        for(patternIndex = 0; patternIndex< sizeof(xgponRegTestPattern)/sizeof(__u32); patternIndex++)
        {
            index = 0;

#ifdef TCSUPPORT_CPU_ARMV8_64
            reg_addr = xgponRegInfoTable[index][0];
#else
            reg_addr = (__u32 *)xgponRegInfoTable[index][0];
#endif
            while(NULL != (regAddr_t *)reg_addr)
            {		
                IO_SREG(reg_addr, (xgponRegTestPattern[patternIndex] & xgponRegInfoTable[index][2]));
                value = IO_GREG(reg_addr);
        		if((xgponRegTestPattern[patternIndex] & xgponRegInfoTable[index][2]) != (value & xgponRegInfoTable[index][2])){
        			printk("[%s %d]ERROR: XG-PON register RW different.\n", __FUNCTION__, __LINE__);
#ifdef TCSUPPORT_CPU_ARMV8_64
                    printk("addr = 0x%llx, writeValue = 0x%llx, readValue = 0x%x, pattern = 0x%x\n", reg_addr, (xgponRegTestPattern[patternIndex] & xgponRegInfoTable[index][2]), value, xgponRegTestPattern[patternIndex]);
#else
                    printk("addr = 0x%p, writeValue = 0x%x, readValue = 0x%x, pattern = 0x%x\n", reg_addr, (xgponRegTestPattern[patternIndex] & xgponRegInfoTable[index][2]), value, xgponRegTestPattern[patternIndex]);
#endif
        		}
				index++;
#ifdef TCSUPPORT_CPU_ARMV8_64
                reg_addr = xgponRegInfoTable[index][0];
#else
				reg_addr = (__u32 *)xgponRegInfoTable[index][0];
#endif
            }            
        }
        testNum--;
    }

	UNION_IC_FUNCTION_HOOK(REGISTER_ACTION_PON_MAC_SCU_RESET,NULL,NULL);

    index = 0;

#ifdef TCSUPPORT_CPU_ARMV8_64
    reg_addr = xgponRegInfoTable[index][0];
#else
    reg_addr = (__u32 *)xgponRegInfoTable[index][0];
#endif

    while(NULL != (regAddr_t *)reg_addr)
    {
        value = IO_GREG(reg_addr);
		if((xgponRegInfoTable[index][1]) != (value & xgponRegInfoTable[index][2])){
			printk("[%s %d]ERROR: XG-PON register default value different.\n", __FUNCTION__, __LINE__);
#ifdef TCSUPPORT_CPU_ARMV8_64
			printk("addr = 0x%llx, defaultValue = 0x%llx, readValue = 0x%x value:0x%llx\n", reg_addr, xgponRegInfoTable[index][1], value,(value & xgponRegInfoTable[index][2]));
#else
            printk("addr = 0x%p, defaultValue = 0x%x, readValue = 0x%x value:0x%x\n", reg_addr, xgponRegInfoTable[index][1], value,(value & xgponRegInfoTable[index][2]));
#endif
		}
		index++;
#ifdef TCSUPPORT_CPU_ARMV8_64
        reg_addr = xgponRegInfoTable[index][0];
#else
		reg_addr = (__u32 *)xgponRegInfoTable[index][0];
#endif
    }
    
    return 0;
}

int xgpon_power_saving_test(void)
{
    uint value = 0;

    IO_SREG(SLEEP_CNT, 0x1f40); /*sleep 1s*/
    value = IO_GREG(INT_ENABLE);
    value = value | (1<<19);
    IO_SREG(INT_ENABLE, value);
    IO_SREG(SLEEP_CFG, 1);
    
    return 0;
}
/******************************************************************************
 Descriptor:	Test GPON MAC HW gemport table.
 Input Args:	none.
 Ret Value:	  0: Success.
                  -1: set gem table fail.
******************************************************************************/
int test_gem_info(void)
{
	unchar valid =0;
    unchar encrypt =0;
    unchar type =0;
	//ushort id =0;
	ushort i =0;	

    for(i= 1 ; i < (GPON_10G_MAX_GEM_ID -1); i++) {
		if(i%100==0){
            msleep(5);
        }
		encrypt = 1;//random32() & 0x01;
		valid = 1;//random32() & 0x01;
        type = 1;//random32() & 0x01;
		//id = random32() & 0xFFFF;        
		//if(gponDevSetGemInfo(id, valid, type,encrypt) != 0) {
		if(gponDevSetGemInfo(i, valid, type,encrypt) != 0) {
			printk(">>test_gem_info i:%d\n",i);
			return -1 ;
		}
	}

	return 0 ;
}
/******************************************************************************
 Descriptor:	Test GPON MAC HW tcont table.
 Input Args:	none.
 Ret Value:	  0: Success.
                  -1: set tcont table fail.
******************************************************************************/
int test_tcont_info(void)
{
	int i=0;
	GPON_TCONT_t valid;
	ushort id;

	for(i = 0 ; i < CONFIG_GPON_10G_MAX_TCONT ; i++) {
		/* set tcont status */
		valid = random32() & 0x01;
		id = random32() & 0xFFF;       
		if(gponDevSetTCont(valid, i, id) != GPON_TCONT_CMD_SUCCESS) {            
			return -1;
		}
	}

	return 0 ;
}

/******************************************************************************
 Descriptor:	Test GPON MAC software reset(register bfb643a0 bit0) for EN7521.
 Input Args:	none.
 Ret Value:	  0: Success.
******************************************************************************/
int test_gpon_mac_reg(void)
{
	if(test_gem_info() != 0) {
		return -1;
	}

	if(test_tcont_info() != 0) {
		return -1;
	}
	
	IO_SREG(MBI_MPI_STOP,						random32()) ;
	IO_SREG(VENDOR_ID,						random32()) ;
	IO_SREG(VS_SN,					random32()) ;
	IO_SREG(ONU_ID,					random32()) ;
	IO_SREG(RGS_ID3_0,						random32()) ;
	IO_SREG(RGS_ID7_4,						random32()) ;
	IO_SREG(RGS_ID11_8,				random32()) ;
	IO_SREG(RGS_ID15_12,				random32()) ;
	IO_SREG(RGS_ID19_16,				random32()) ;
	IO_SREG(RGS_ID23_20,				random32()) ;
	IO_SREG(RGS_ID27_24,				random32()) ;
	IO_SREG(RGS_ID31_28,				random32()) ;
	IO_SREG(RGS_ID35_32,				random32()) ;
//	IO_SREG(INT_ENABLE,				random32()) ;
//	IO_SREG(INT_STATUS,			random32()) ;
//	IO_SREG(FIFO_ERR_STS,				random32()) ;
//	IO_SREG(TX_ERR_STS,				random32()) ;
//	IO_SREG(RX_ERR_STS,			random32()) ;
	IO_SREG(COR_TAG,				random32()) ;
	IO_SREG(CUR_DS_PON_ID,				random32()) ;
	IO_SREG(CUR_US_PON_ID,				random32()) ;
	IO_SREG(CALIB_STS_L32,							random32()) ;
	IO_SREG(CALIB_STS_H32,						random32()) ;
	IO_SREG(TUNING_GRAN,					random32()) ;
	IO_SREG(STEP_TUNING_TIME,						random32()) ;
	IO_SREG(US_RATE_CAP,					random32()) ;
	IO_SREG(ATTENUATION,				random32()) ;
	IO_SREG(POWER_LEVEL_CAP,						random32()) ;
    
	IO_SREG(O23_O4_PLOAMU_CTRL,				random32()) ;
    IO_SREG(ACTIVATION_ST,				random32()) ;
    IO_SREG(RSP_TIME,				random32()) ;
    IO_SREG(RDM_DLY,				random32()) ;
    IO_SREG(INT_DLY,				random32()) ;
    IO_SREG(EQD,				random32()) ;
    IO_SREG(SYN_MRG,				random32()) ;
    IO_SREG(US_PROF_VLD,				random32()) ;
    IO_SREG(US_PROF_PSBu_LEN_0_1,				random32()) ;
    IO_SREG(US_PROF_PSBu_LEN_2_3,				random32()) ;
    IO_SREG(US_AES_KEY_CTRL,				random32()) ;
    IO_SREG(DS_AES_KEY_VLD,				random32()) ;
    IO_SREG(DEFT_GPID_ENC,				random32()) ;
    IO_SREG(AES_UC_IDX0_KEY0,				random32()) ;
    IO_SREG(AES_UC_IDX0_KEY1,				random32()) ;
    IO_SREG(AES_UC_IDX0_KEY2,				random32()) ;
    IO_SREG(AES_UC_IDX0_KEY3,				random32()) ;
    IO_SREG(AES_UC_IDX1_KEY0,				random32()) ;
    IO_SREG(AES_UC_IDX1_KEY1,				random32()) ;
    IO_SREG(AES_UC_IDX1_KEY2,				random32()) ;
    IO_SREG(AES_UC_IDX1_KEY3,				random32()) ;
    IO_SREG(AES_BC_IDX0_KEY0,				random32()) ;
    IO_SREG(AES_BC_IDX0_KEY1,				random32()) ;
    IO_SREG(AES_BC_IDX0_KEY2,				random32()) ;
    IO_SREG(AES_BC_IDX0_KEY3,				random32()) ;
    IO_SREG(AES_BC_IDX1_KEY0,				random32()) ;
    IO_SREG(AES_BC_IDX1_KEY1,				random32()) ;    
    IO_SREG(AES_BC_IDX1_KEY2,				random32()) ;
    IO_SREG(AES_BC_IDX1_KEY3,				random32()) ;
    IO_SREG(GPIDX_TBL_INIT,				random32()) ;
    IO_SREG(GPIDX_TBL_CTRL,				random32()) ;
    IO_SREG(GPIDX_TBL_STS,				random32()) ;
    IO_SREG(G_TX_FCS_TBL_INIT,				random32()) ;    
    IO_SREG(IDLE_GEM_CTRL,				random32()) ;
    IO_SREG(US_DYING_GASP_CTRL,				random32()) ;
    IO_SREG(TX_XGEM_IDLE_DATA,				random32()) ;
    IO_SREG(TX_OMCI_PRE_GET,				random32()) ;
    IO_SREG(RX_OMCI_PRE_GET,				random32()) ;
    IO_SREG(EPDROP_EN,				random32()) ;    
    IO_SREG(PLOAMu_FIFO_STS,				random32()) ;
    IO_SREG(PLOAMu_WDATA,				random32()) ;
    IO_SREG(PLOAMd_FIFO_STS,				random32()) ;
    IO_SREG(PLOAMd_RDATA,				random32()) ;
    IO_SREG(KEY_GEN,				random32()) ;
    IO_SREG(CUR_KIDX,				random32()) ;    
    IO_SREG(MSK_0,				random32()) ;
    IO_SREG(MSK_1,				random32()) ;
    IO_SREG(MSK_2,				random32()) ;
    IO_SREG(MSK_3,				random32()) ;
    IO_SREG(REGMSK_0,				random32()) ;
    IO_SREG(REGMSK_1,				random32()) ;    
    IO_SREG(REGMSK_2,				random32()) ;
    IO_SREG(REGMSK_3,				random32()) ;
    IO_SREG(HW_GENK_0,				random32()) ;
    IO_SREG(HW_GENK_1,				random32()) ;
    IO_SREG(HW_GENK_2,				random32()) ;
    IO_SREG(HW_GENK_3,				random32()) ;
    IO_SREG(PIK0_0,				random32()) ;    
    IO_SREG(PIK0_1,				random32()) ;
    IO_SREG(PIK0_2,				random32()) ;
    IO_SREG(PIK0_3,				random32()) ;
    IO_SREG(PIK1_0,				random32()) ;
    IO_SREG(PIK1_1,				random32()) ;
    IO_SREG(PIK1_2,				random32()) ;
    IO_SREG(PIK1_3,				random32()) ;    
    IO_SREG(OIK0_0,				random32()) ;
    IO_SREG(OIK0_1,				random32()) ;
    IO_SREG(OIK0_2,				random32()) ;
    IO_SREG(OIK0_3,				random32()) ;
    IO_SREG(OIK1_0,				random32()) ;
    IO_SREG(OIK1_1,				random32()) ;    
    IO_SREG(OIK1_2,				random32()) ;
    IO_SREG(OIK1_3,				random32()) ;
    IO_SREG(KEK0_0,				random32()) ;
    IO_SREG(KEK0_1,				random32()) ;
    IO_SREG(KEK0_2,				random32()) ;
    IO_SREG(KEK0_3,				random32()) ;
    IO_SREG(KEK1_0,				random32()) ;    
    IO_SREG(KEK1_1,				random32()) ;
    IO_SREG(KEK1_2,				random32()) ;
    IO_SREG(KEK1_3,				random32()) ;
    IO_SREG(PON_TAG_0,				random32()) ;
    IO_SREG(PON_TAG_1,				random32()) ;
    IO_SREG(SW_SET_KIDX,				random32()) ;    
    IO_SREG(SW0_ENCSTART,				random32()) ;
    IO_SREG(SW0_MADDR,				random32()) ;
    IO_SREG(SW0_RADDR,				random32()) ;
    IO_SREG(SW0_KADDR,				random32()) ;
    IO_SREG(SW0_ENCLEN,				random32()) ;
    IO_SREG(SW0_ENCINFO,				random32()) ;
    IO_SREG(SW1_ENCSTART,				random32()) ;    
    IO_SREG(SW1_MADDR,				random32()) ;
    IO_SREG(SW1_RADDR,				random32()) ;
    IO_SREG(SW1_KADDR,				random32()) ;
    IO_SREG(SW1_ENCLEN,				random32()) ;
    IO_SREG(SW1_ENCINFO,				random32()) ;
    IO_SREG(MIB_CTRL,				random32()) ;
    IO_SREG(MIB_TBL_CONFIG,				random32()) ;
    IO_SREG(MIB_CTRL_STS,				random32()) ;
    IO_SREG(MIB_RDATA_L32,				random32()) ;
    IO_SREG(MIB_RDATA_H32,				random32()) ;
    IO_SREG(MIB_WDATA_L32,				     random32()) ;
    IO_SREG(MIB_WDATA_H32,				     random32()) ;
    IO_SREG(TOD_CLK_PERIOD,				     random32()) ;
    IO_SREG(TOD_1PPS_WD_CTRL,				 random32()) ;
    IO_SREG(CUR_TOD_SEC,				     random32()) ;
    IO_SREG(CUR_TOD_NANO_SEC,				 random32()) ;
    IO_SREG(TOD_SFC_L,				         random32()) ;
    IO_SREG(TOD_SFC_H,				         random32()) ;
    IO_SREG(TOD_UPD_CTRL,				     random32()) ;
    IO_SREG(NEW_TOD_SEC,				     random32()) ;
    IO_SREG(NEW_TOD_NANO_SEC,				 random32()) ;
    IO_SREG(SLEEP_CFG,				         random32()) ;
    IO_SREG(SLEEP_CNT,				         random32()) ;
    IO_SREG(TURNING_SFC_L,				     random32()) ;
    IO_SREG(TURNING_SFC_H,				     random32()) ;
    IO_SREG(DBG_CAP_SETTING ,				 random32()) ;
    IO_SREG(DBG_BWM_CHK_CTRL,				 random32()) ;
    IO_SREG(DBG_BWM_CKH_STS,				 random32()) ;
    IO_SREG(DBG_BWM_SFIFO_STS,				 random32()) ;
    IO_SREG(DBG_BWM_BFIFO_STS,				 random32()) ;
    IO_SREG(DBG_TX_ALIGN_FIFO_STS,			 random32()) ;
    IO_SREG(DBG_TX_AES_MAX_USED,			 random32()) ;
    IO_SREG(DBG_DBA_BACK_DOOR,				 random32()) ;
    IO_SREG(DBG_DBA_MODIFY,				     random32()) ;	
    IO_SREG(DBG_RESYNC,				         random32()) ;
    IO_SREG(DBG_PHY_DLY_CTRL,				 random32()) ;
    IO_SREG(DBG_DS_SPF_CNT_L,				 random32()) ;
    IO_SREG(DBG_DS_SPF_CNT_H ,				 random32()) ;
    IO_SREG(DBG_PON_ID_L,				     random32()) ;
    IO_SREG(DBG_PON_ID_H,				     random32()) ;
    IO_SREG(DBG_TX_SYNC_OFFSET,				 random32()) ;
    IO_SREG(DBG_RX_PHY_DLY_OFS,				 random32()) ;
    IO_SREG(DBG_RX_HEC_ERR,				     random32()) ;
    IO_SREG(RX_GEM_ENC_CHK,				     random32()) ;
    IO_SREG(RX_MBI_FIFO_DROP_EN ,			 random32()) ;
    IO_SREG(RX_MBI_HDR_FIFO_DROP_CTRL ,		 random32()) ;
    IO_SREG(RX_MBI_PL_FIFO_DROP_CTRL,		 random32()) ;
    IO_SREG(CNT_CLR,				         random32()) ;
    IO_SREG(RX_HLEND_HEC_CNT ,				 random32()) ;
    IO_SREG(RX_ALLOC_HEC_CNT,				 random32()) ;
    IO_SREG(RX_HDR_HEC_CNT,				     random32()) ;
    IO_SREG(RX_PHY_HEC_ERR_CNT,				 random32()) ;
    IO_SREG(RX_MIC_ERR_CNT,				     random32()) ;
    IO_SREG(RX_ETH_FCS_ERR_CNT,				 random32()) ;
    IO_SREG(RX_BIP_ERR_CNT ,				 random32()) ;
    IO_SREG(RX_KEY_ERR_CNT,				     random32()) ;
    IO_SREG(RX_LOST_WCNT,				     random32()) ;
    IO_SREG(INVLD_PROF_BST_GNT_CNT,			 random32()) ;
    IO_SREG(RX_MBI_XGEM_DROP_CNT,			 random32()) ;
    IO_SREG(RX_XGTC_CNT ,				     random32()) ;
    IO_SREG(TX_BST_CNT,				         random32()) ;
    IO_SREG(RX_PLOAMD_CNT,				     random32()) ;
    IO_SREG(TX_PLOAMU_CNT,				     random32()) ;
    IO_SREG(RX_OMCI_CNT,				     random32()) ;
    IO_SREG(TX_OMCI_CNT,				     random32()) ;
    IO_SREG(RX_XGEM_CNT,				     random32()) ;
    IO_SREG(TX_XGEM_CNT,				     random32()) ;
    IO_SREG(RX_MBI_CNT,				         random32()) ;
    IO_SREG(TX_MBI_CNT,				         random32()) ;
    IO_SREG(RX_NON_IDLE_BCNT,				 random32()) ;
    IO_SREG(TX_NON_IDLE_BCNT,				 random32()) ;
    IO_SREG(TX_NLF_XGEM_CNT ,				 random32()) ;
    IO_SREG(TX_ACK_PLOAMU_CNT,				 random32()) ;
    IO_SREG(RX_BIP_PROTECT_WCNT,			 random32()) ;
    IO_SREG(SNF_CTRL,				         random32()) ;
    IO_SREG(SNF_GPID,				         random32()) ;
    IO_SREG(DS_SNF_ETH_DASA_H16,			 random32()) ;
    IO_SREG(SNF_ETH_TAG,				     random32()) ;
    IO_SREG(SNF_ETH_TYPE,				     random32()) ;
    IO_SREG(US_SNF_ETH_DASA_H16 ,			 random32()) ;
    IO_SREG(DBG_PROBE_CTRL,				     random32()) ;
   // IO_SREG(DBG_PROBE_HIGH32,				 random32()) ;
    IO_SREG(DBG_PROBE_LOW32,				 random32()) ;

	return 0;
}

/******************************************************************************
 Descriptor:	dump GPON MAC control status register.
 Input Args:	none.
 Ret Value:		0: Success.
             -EFAULT: Get gemInfo error.
******************************************************************************/
int gponDevDumpCsr(void)
{	
	printk("0x5000: SW RST                                         :0x%.8x\n", IO_GREG(SW_RST)) ;    
	printk("0x5004: MBI MPI STOP                                   :0x%.8x\n", IO_GREG(MBI_MPI_STOP)) ;
    printk("0x500c: VENDOR ID                                      :0x%.8x\n", IO_GREG(VENDOR_ID)) ;
    printk("0x5010: VS SN                                          :0x%.8x\n", IO_GREG(VS_SN)) ;
#if 1
	printk("0x5014: ONU ID                                         :0x%.8x\n", IO_GREG(ONU_ID)) ;
    printk("0x5018: RGS ID3_0                                      :0x%.8x\n", IO_GREG(RGS_ID3_0)) ;
    printk("0x501c: RGS ID7_4                                      :0x%.8x\n", IO_GREG(RGS_ID7_4)) ;
    printk("0x5020: RGS ID11_8                                     :0x%.8x\n", IO_GREG(RGS_ID11_8)) ;
    printk("0x5024: RGS ID15_12                                    :0x%.8x\n", IO_GREG(RGS_ID15_12)) ;
    printk("0x5028: RGS ID19_16                                    :0x%.8x\n", IO_GREG(RGS_ID19_16)) ;
    printk("0x502c: RGS ID23_20                                    :0x%.8x\n", IO_GREG(RGS_ID23_20)) ;
    printk("0x5030: RGS ID27_24                                    :0x%.8x\n", IO_GREG(RGS_ID27_24)) ;
    printk("0x5034: RGS ID31_28                                    :0x%.8x\n", IO_GREG(RGS_ID31_28)) ;
    printk("0x5038: RGS ID35_32                                    :0x%.8x\n", IO_GREG(RGS_ID35_32)) ;
    printk("0x5040: INT ENABLE                                     :0x%.8x\n", IO_GREG(INT_ENABLE)) ;
    printk("0x5080: COR TAG(NGPON2 USED ONLY)                      :0x%.8x\n", IO_GREG(COR_TAG)) ;
    printk("0x5084: CUR DS PON ID                                  :0x%.8x\n", IO_GREG(CUR_DS_PON_ID)) ;
    printk("0x5088: CUR US PON ID                                  :0x%.8x\n", IO_GREG(CUR_US_PON_ID)) ;
    printk("0x508C: CALIB STS L32                                  :0x%.8x\n", IO_GREG(CALIB_STS_L32)) ;
    printk("0x5090: CALIB STS H32                                  :0x%.8x\n", IO_GREG(CALIB_STS_H32)) ;
    printk("0x5094: TUNING GRAN(NGPON2 USED ONLY)                  :0x%.8x\n", IO_GREG(TUNING_GRAN)) ;
    printk("0x5098: STEP TUNING TIME(NGPON2 USED ONLY)             :0x%.8x\n", IO_GREG(STEP_TUNING_TIME)) ;
    printk("0x509C: US RATE CAP(NGPON2& XGSPON USED)               :0x%.8x\n", IO_GREG(US_RATE_CAP)) ;
    printk("0x50A0: ATTENUATION(NGPON2 USED ONLY)                  :0x%.8x\n", IO_GREG(ATTENUATION)) ;
    printk("0x50A4: POWER LEVEL CAP(NGPON2 USED ONLY)              :0x%.8x\n", IO_GREG(POWER_LEVEL_CAP)) ;
    printk("0x5100: O23 O4 PLOAM CTRL                              :0x%.8x\n", IO_GREG(O23_O4_PLOAMU_CTRL)) ;
    printk("0x5104: ACTIVATION ST                                  :0x%.8x\n", IO_GREG(ACTIVATION_ST)) ;
    printk("0x5108: RSP TIME                                       :0x%.8x\n", IO_GREG(RSP_TIME)) ;
    printk("0x510c: RDM DLY                                        :0x%.8x\n", IO_GREG(RDM_DLY)) ;
    printk("0x5110: INT DLY                                        :0x%.8x\n", IO_GREG(INT_DLY)) ;
    printk("0x5114: EQD                                            :0x%.8x\n", IO_GREG(EQD)) ;
    printk("0x5118: SYN MRG                                        :0x%.8x\n", IO_GREG(SYN_MRG)) ;
    printk("0x511C: US PROF VLD                                    :0x%.8x\n", IO_GREG(US_PROF_VLD)) ;
    printk("0x5120: US PROF PSBu LEN 0 1                           :0x%.8x\n", IO_GREG(US_PROF_PSBu_LEN_0_1)) ;
    printk("0x5124: US PROF PSBu LEN 2 3                           :0x%.8x\n", IO_GREG(US_PROF_PSBu_LEN_2_3)) ;
    printk("0x5200: US AES KEY CTRL                                :0x%.8x\n", IO_GREG(US_AES_KEY_CTRL)) ;
    printk("0x5204: DS AES KEY VLD                                 :0x%.8x\n", IO_GREG(DS_AES_KEY_VLD)) ;
    printk("0x5210: AES US IDX0 KEY0                               :0x%.8x\n", IO_GREG(AES_UC_IDX0_KEY0)) ;
    printk("0x5214: AES US IDX0 KEY1                               :0x%.8x\n", IO_GREG(AES_UC_IDX0_KEY1)) ;
    printk("0x5218: AES US IDX0 KEY2                               :0x%.8x\n", IO_GREG(AES_UC_IDX0_KEY2)) ;
    printk("0x521C: AES US IDX0 KEY3                               :0x%.8x\n", IO_GREG(AES_UC_IDX0_KEY3)) ;
    printk("0x5220: AES US IDX1 KEY0                               :0x%.8x\n", IO_GREG(AES_UC_IDX1_KEY0)) ;
    printk("0x5224: AES US IDX1 KEY1                               :0x%.8x\n", IO_GREG(AES_UC_IDX1_KEY1)) ;
    printk("0x5228: AES US IDX1 KEY2                               :0x%.8x\n", IO_GREG(AES_UC_IDX1_KEY2)) ;
    printk("0x522c: AES US IDX1 KEY3                               :0x%.8x\n", IO_GREG(AES_UC_IDX1_KEY3)) ;
    printk("0x5230: AES BC IDX0 KEY0                               :0x%.8x\n", IO_GREG(AES_BC_IDX0_KEY0)) ;
    printk("0x5234: AES BC IDX0 KEY1                               :0x%.8x\n", IO_GREG(AES_BC_IDX0_KEY1)) ;
    printk("0x5238: AES BC IDX0 KEY2                               :0x%.8x\n", IO_GREG(AES_BC_IDX0_KEY2)) ;
    printk("0x523c: AES BC IDX0 KEY3                               :0x%.8x\n", IO_GREG(AES_BC_IDX0_KEY3)) ;
    printk("0x5240: AES BC IDX1 KEY0                               :0x%.8x\n", IO_GREG(AES_BC_IDX1_KEY0)) ;
    printk("0x5244: AES BC IDX1 KEY1                               :0x%.8x\n", IO_GREG(AES_BC_IDX1_KEY1)) ;
    printk("0x5248: AES BC IDX1 KEY2                               :0x%.8x\n", IO_GREG(AES_BC_IDX1_KEY2)) ;
    printk("0x524c: AES BC IDX1 KEY3                               :0x%.8x\n", IO_GREG(AES_BC_IDX1_KEY3)) ;
    printk("0x5260: GPIDX TBL INIT                                 :0x%.8x\n", IO_GREG(GPIDX_TBL_INIT)) ;
    printk("0x5264: GPIDX TBL CTRL                                 :0x%.8x\n", IO_GREG(GPIDX_TBL_CTRL)) ;
    printk("0x5280: IDLE GEM CTRL                                  :0x%.8x\n", IO_GREG(IDLE_GEM_CTRL)) ;
    printk("0x5284: US DYING GASP CRTL                             :0x%.8x\n", IO_GREG(US_DYING_GASP_CTRL)) ;
    printk("0x5288: TX XGEM IDLE DATA                              :0x%.8x\n", IO_GREG(TX_XGEM_IDLE_DATA)) ;
    printk("0x528c: TX OMCI PRE GET                                :0x%.8x\n", IO_GREG(TX_OMCI_PRE_GET)) ;
    printk("0x5290: RX OMCI PRE GET                                :0x%.8x\n", IO_GREG(RX_OMCI_PRE_GET)) ;
    printk("0x52f0: EPDROP EN                                      :0x%.8x\n", IO_GREG(EPDROP_EN)) ;
    printk("0x5304: PLOAMD WDATA                                   :0x%.8x\n", IO_GREG(PLOAMu_WDATA)) ;
    printk("0x5320: MASK 0                                         :0x%.8x\n", IO_GREG(MSK_0)) ;
    printk("0x5324: MASK 1                                         :0x%.8x\n", IO_GREG(MSK_1)) ;
    printk("0x5328: MASK 2                                         :0x%.8x\n", IO_GREG(MSK_2)) ;
    printk("0x532C: MASK 3                                         :0x%.8x\n", IO_GREG(MSK_3)) ;    
    printk("0x5360: PIK0 0                                         :0x%.8x\n", IO_GREG(PIK0_0)) ;
    printk("0x5364: PIK0 1                                         :0x%.8x\n", IO_GREG(PIK0_1)) ;
    printk("0x5368: PIK0 2                                         :0x%.8x\n", IO_GREG(PIK0_2)) ;
    printk("0x536C: PIK0 3                                         :0x%.8x\n", IO_GREG(PIK0_3)) ;
    printk("0x5370: PIK1 0                                         :0x%.8x\n", IO_GREG(PIK1_0)) ;
    printk("0x5374: PIK1 1                                         :0x%.8x\n", IO_GREG(PIK1_1)) ;
    printk("0x5378: PIK1 2                                         :0x%.8x\n", IO_GREG(PIK1_2)) ;
    printk("0x537C: PIK1 3                                         :0x%.8x\n", IO_GREG(PIK1_3)) ;
    printk("0x5380: OIK0 0                                         :0x%.8x\n", IO_GREG(OIK0_0)) ;
    printk("0x5384: OIK0 1                                         :0x%.8x\n", IO_GREG(OIK0_1)) ;
    printk("0x5388: OIK0 2                                         :0x%.8x\n", IO_GREG(OIK0_2)) ;
    printk("0x538C: OIK0 3                                         :0x%.8x\n", IO_GREG(OIK0_3)) ;
    printk("0x5390: OIK1 0                                         :0x%.8x\n", IO_GREG(OIK1_0)) ;
    printk("0x5394: OIK1 1                                         :0x%.8x\n", IO_GREG(OIK1_1)) ;
    printk("0x5398: OIK1 2                                         :0x%.8x\n", IO_GREG(OIK1_2)) ;
    printk("0x539C: OIK1 3                                         :0x%.8x\n", IO_GREG(OIK1_3)) ;
    printk("0x53A0: KEK0 0                                         :0x%.8x\n", IO_GREG(KEK0_0)) ;
    printk("0x53A4: KEK0 1                                         :0x%.8x\n", IO_GREG(KEK0_1)) ;
    printk("0x53A8: KEK0 2                                         :0x%.8x\n", IO_GREG(KEK0_2)) ;
    printk("0x53AC: KEK0 3                                         :0x%.8x\n", IO_GREG(KEK0_3)) ;
    printk("0x53B0: KEK1 0                                         :0x%.8x\n", IO_GREG(KEK1_0)) ;
    printk("0x53B4: KEK1 1                                         :0x%.8x\n", IO_GREG(KEK1_1)) ;
    printk("0x53B8: KEK1 2                                         :0x%.8x\n", IO_GREG(KEK1_2)) ;
    printk("0x53BC: KEK1 3                                         :0x%.8x\n", IO_GREG(KEK1_3)) ;
    printk("0x53C0: PON TAG 0                                      :0x%.8x\n", IO_GREG(PON_TAG_0)) ;
    printk("0x53C4: PON TAG 1                                      :0x%.8x\n", IO_GREG(PON_TAG_1)) ;
    printk("0x53E8: SW SET KIDX                                    :0x%.8x\n", IO_GREG(SW_SET_KIDX)) ;
    printk("0x5404: SW0 MADDR                                      :0x%.8x\n", IO_GREG(SW0_MADDR)) ;
    printk("0x5408: SW0 RADDR                                      :0x%.8x\n", IO_GREG(SW0_RADDR)) ;
    printk("0x540C: SW0 KADDR                                      :0x%.8x\n", IO_GREG(SW0_KADDR)) ;
    printk("0x5410: SW0 ENCLEN                                     :0x%.8x\n", IO_GREG(SW0_ENCLEN)) ;
    printk("0x5414: SW0 ENCINFO                                    :0x%.8x\n", IO_GREG(SW0_ENCINFO)) ;
    printk("0x5424: SW1 MADDR                                      :0x%.8x\n", IO_GREG(SW1_MADDR)) ;
    printk("0x5428: SW1 RADDR                                      :0x%.8x\n", IO_GREG(SW1_RADDR)) ;
    printk("0x542C: SW1 KADDR                                      :0x%.8x\n", IO_GREG(SW1_KADDR)) ;
    printk("0x5430: SW1 ENCLEN                                     :0x%.8x\n", IO_GREG(SW1_ENCLEN)) ;
    printk("0x5434: SW1 ENCINFO                                    :0x%.8x\n", IO_GREG(SW1_ENCINFO)) ;
    printk("0x5500: MIB CRTL                                       :0x%.8x\n", IO_GREG(MIB_CTRL)) ;
    printk("0x5504: MIB TBL CONFIG                                 :0x%.8x\n", IO_GREG(MIB_TBL_CONFIG)) ;
    printk("0x5508: MIB CTRL STS                                   :0x%.8x\n", IO_GREG(MIB_CTRL_STS)) ;
    printk("0x5518: MIB WDATA L32                                  :0x%.8x\n", IO_GREG(MIB_WDATA_L32)) ;
    printk("0x551C: MIB WDATA H32                                  :0x%.8x\n", IO_GREG(MIB_WDATA_H32)) ;
    printk("0x5550: TOD CLK PERIOD                                 :0x%.8x\n", IO_GREG(TOD_CLK_PERIOD)) ;
    printk("0x5554: TOD 1PPS WD CTRL                               :0x%.8x\n", IO_GREG(TOD_1PPS_WD_CTRL)) ;
    printk("0x5560: TOD SFC L                                      :0x%.8x\n", IO_GREG(TOD_SFC_L)) ; 
    printk("0x5564: TOD SFC H                                      :0x%.8x\n", IO_GREG(TOD_SFC_H)) ;
    printk("0x556C: NEW TOD SEC                                    :0x%.8x\n", IO_GREG(NEW_TOD_SEC)) ;
    printk("0x5570: NEW TOD NANO SEC                               :0x%.8x\n", IO_GREG(NEW_TOD_NANO_SEC)) ;
    printk("0x5584: SLEEP CNT                                      :0x%.8x\n", IO_GREG(SLEEP_CNT)) ;    
    printk("0x55A0: TURNING SFC L                                  :0x%.8x\n", IO_GREG(TURNING_SFC_L)) ;
    printk("0x55A4: TURNING SFC H                                  :0x%.8x\n", IO_GREG(TURNING_SFC_H)) ;    
    printk("0x5800: DBF CAP SETTING                                :0x%.8x\n", IO_GREG(DBG_CAP_SETTING)) ;    
    printk("0x5804: DBG BWM CHK CRTL                               :0x%.8x\n", IO_GREG(DBG_BWM_CHK_CTRL)) ;
    printk("0x5820: DBG DBA BACK DOOR                              :0x%.8x\n", IO_GREG(DBG_DBA_BACK_DOOR)) ;
    printk("0x5824: DBG DBA MODIFY                                 :0x%.8x\n", IO_GREG(DBG_DBA_MODIFY)) ;
    printk("0x582C: DBG RESYNC                                     :0x%.8x\n", IO_GREG(DBG_RESYNC)) ;
    printk("0x5830: DBG PHY DLY CTRL                               :0x%.8x\n", IO_GREG(DBG_PHY_DLY_CTRL)) ;
    printk("0x5850: RX GEM ENC CHK                                 :0x%.8x\n", IO_GREG(RX_GEM_ENC_CHK)) ;
    printk("0x5854: RX MBI FIFO DROP EN                            :0x%.8x\n", IO_GREG(RX_MBI_FIFO_DROP_EN)) ;
    printk("0x5858: RX MBI HDR FIFO DROP CRTL                      :0x%.8x\n", IO_GREG(RX_MBI_HDR_FIFO_DROP_CTRL)) ;
    printk("0x585C: RX MBI PL FIFO DROP CRTL                       :0x%.8x\n", IO_GREG(RX_MBI_PL_FIFO_DROP_CTRL)) ;
    printk("0x5FD0: SNF CTRL                                       :0x%.8x\n", IO_GREG(SNF_CTRL)) ;    
    printk("0x5FD4: SNF GPID                                       :0x%.8x\n", IO_GREG(SNF_GPID)) ;
    printk("0x5FE0: DS SNF ETH DATA H16                            :0x%.8x\n", IO_GREG(DS_SNF_ETH_DASA_H16)) ;    
    printk("0x5FE4: SNF ETH TAG                                    :0x%.8x\n", IO_GREG(SNF_ETH_TAG)) ;
    printk("0x5FE8: SNF ETH TYPE                                   :0x%.8x\n", IO_GREG(SNF_ETH_TYPE)) ;
    printk("0x5FEC: US SNF ETH DATA H16                            :0x%.8x\n", IO_GREG(US_SNF_ETH_DASA_H16)) ;
    printk("0x5FF0: DBG PROBE CTRL                                 :0x%.8x\n", IO_GREG(DBG_PROBE_CTRL)) ;
    printk("0x5FF4: DBG PROBE HIGH32                               :0x%.8x\n", IO_GREG(DBG_PROBE_HIGH32)) ;
    printk("0x5FF8: DBG PROBE LOW32                                :0x%.8x\n", IO_GREG(DBG_PROBE_LOW32)) ; 
 #endif   
    return 0 ;
}


/******************************************************************************
 **      function name: gpon_dvt_sw_reset
 **      descriptions:
 **           It's used to test the sw reset function
 .**      parameters:
 **            None
 **      global:
 **             None
 **      return:
 **             success: 0
 **      call:
 **            gponDevDumpCsr
 **            gponDevDumpGemInfo
 **            gponDevDumpTcontInfo
 **            gponDevSwReset
 **      revision:
 **
******************************************************************************/
int gpon_dvt_sw_reset(void)
{   
    gponDevMacReset(XPON_RESET_RELEASE); 

	/* write GPON MAC register */
	if(test_gpon_mac_reg() != 0) {
		return -1;
	}

	/* dump CSR */
	printk("before sw reset, dump CSR\n");
	if(gponDevDumpCsr() != 0) {
		return -1;
	}

	/* dump hw tcont table info */
	printk("before sw reset, dump hw tcont table info\n");
	if(gponDevDumpTcontInfo() != 0) {
		return -1;
	}

	/* dump hw gemport table info */
	printk("before sw reset, dump hw gemport table info\n");
	if(gponDevDumpGemInfo() != 0) {
		return -1;
	}
	/* GPON MAC SW reset */
    gponDevMacReset(XPON_RESET_HOLD_ON);
    udelay(1);
    gponDevMacReset(XPON_RESET_RELEASE);
    msleep(10);
	/* dump CSR */
	printk("after sw reset, dump CSR\n");
	if(gponDevDumpCsr() != 0) {
		return -1;
	}

	/* dump hw tcont table info */
	printk("after sw reset, dump hw tcont table info\n");
	if(gponDevDumpTcontInfo() != 0) {
		return -1;
	}

	/* dump hw gemport table info */
	printk("after sw reset, dump hw gemport table info  num:%d \n",(GPON_10G_MAX_GEM_ID -1));
	if(gponDevDumpGemInfo() != 0) {
		return -1;
	}

	return 0;
}


