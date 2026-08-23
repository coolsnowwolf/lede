#ifndef AVS_H
#define AVS_H

typedef unsigned int        u32;

typedef enum
{
	AVS_OK = 0, 		// OK
	OUT_RANGE_FAIL, 	 //input out of range FAIL
	AVS_FAIL			//HW function fail
} AVS_STATUS_T; // AVS status type

typedef enum
{
	NV = 0, 
	LV, 	 
	HV,
	OFF,
} V_SEL_T; // AVS status type

extern u32 AVS_Get_Vcore(void);
extern AVS_STATUS_T AVS_Set(u32 V_SEL, u32 iddq);
extern AVS_STATUS_T SET_AVS(u32 V_SEL);
extern u32 AVS_Get_Affinity(u32 pid, u32 *mask);
extern u32 AVS_Set_Affinity(u32 pid, u32 *mask);
#endif /* AVS_H */
