#ifndef TC_L2LU_H_
#define TC_L2LU_H_

#include <linux/list.h>


#define L2LU_KEY_ENTRY_NUM 15
#define RALINK_FRAME_ENGINE_BASE 	0xBFB50000
#define L2LU_KEY_BASE		        0x200

enum L2LUKeyStatus
{
	K_FREE,
	K_OCCUPY,
};

typedef struct {
	uint16_t	cvid;	
	uint8_t     occupy;
}L2LU_key_table_s;


void PpeL2LUKeyEntryAdd(uint16_t cvid);
void PpeL2LUKeyEntryDelete(uint16_t cvid);
void PpeL2LUKeyTblClear(void);

#endif
