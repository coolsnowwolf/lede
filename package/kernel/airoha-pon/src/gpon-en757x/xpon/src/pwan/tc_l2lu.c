
#include "pwan/tc_l2lu.h"
#include "pwan/xpon_netif.h"

static L2LU_key_table_s hwnat_key_table[15];


static void PpeL2LUKeyTblInit(void)
{
	int i=0;
	
	for(i=0; i<L2LU_KEY_ENTRY_NUM; i++)
	{
		memset(&(hwnat_key_table[i]), 0, sizeof(L2LU_key_table_s));
	}

	return;
}

int getL2LUKeyMatchEntryByVid(uint16_t cvid)
{
	int i=0;
	
	for(i=0; i<L2LU_KEY_ENTRY_NUM; i++)
	{
		if(hwnat_key_table[i].occupy == K_OCCUPY)
		{
			if(hwnat_key_table[i].cvid != cvid)
			{
				//not match, continue
				continue;
			}		
			return i; 
		}
	}
	return -1;
}

int getL2LUKeyFreeEntry(void)
{
	int i=0;
	
	for(i=0; i<L2LU_KEY_ENTRY_NUM; i++)
	{
		if(hwnat_key_table[i].occupy == K_FREE)
			return i; 
	}

	return -1;
}

void PpeL2LUKeyTblSet(uint8_t index, uint16_t cvid, uint8_t occupy)
{
	uint32_t regAddr=0;
	uint32_t regValue=0;
	//set global table
	hwnat_key_table[index].cvid = cvid;
	hwnat_key_table[index].occupy = occupy;

	//set FE register
	regAddr = L2LU_KEY_BASE + index*8;
	regValue = (cvid << 20) |  (1<<7) ;

	printk("PpeL2LUKeyTblSet: set key0 table %d, cvid=0x%04x, \
	    mask= 0x%02x\n",index, cvid, occupy);
	printk("PpeL2LUKeyTblSet: set regAddr = 0x%08x, regValue = \
	    0x%08x\n", regAddr, regValue);
    fe_reg_write(regAddr,regValue);

	return;

}

void PpeL2LUKeyEntryDeleteByIndex(uint8_t index)
{
	PpeL2LUKeyTblSet(index, 0, K_FREE);
	return;
}

void PpeL2LUKeyEntryAdd(uint16_t cvid)
{
    int index;

    index = getL2LUKeyMatchEntryByVid(cvid);
    if(index == -1){
        index = getL2LUKeyFreeEntry();
        // get free entry 
        if(index != -1){
	        PpeL2LUKeyTblSet(index, cvid, K_OCCUPY);
	    }
	}
	return;
}

void PpeL2LUKeyEntryDelete(uint16_t cvid)
{
    int index;

    index = getL2LUKeyMatchEntryByVid(cvid);
    if(index != -1){
	    PpeL2LUKeyTblSet(index, 0, K_FREE);
	}
	return;
}


void PpeL2LUKeyTblClear(void)
{
	int i=0;

	for(i=0; i<L2LU_KEY_ENTRY_NUM ; i++)
	{
		PpeL2LUKeyEntryDeleteByIndex(i);
	}

	return;
}


