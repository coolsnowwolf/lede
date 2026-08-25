#ifndef _NPU_MBOX_API_H_
#define _NPU_MBOX_API_H_

//#define MBOX_DBG_ON (1)
//#define MBOX_API_TEST   (1)

#define MAX_MBOX_FUNC   (8)
#define MBOX2HOST_IDX   (8)

#define MASK_CORE_ID    (0xf)
#define MASK_RET_STATUS (0x7)
#define MASK_FUNC_ID    (0xf)

typedef union
{
    struct {
        unsigned short block_mode   : 1;
        unsigned short done_bit     : 1;
        unsigned short ret_status   : 3;
        unsigned short static_buff  : 1;
        unsigned short reserved     : 5;
        unsigned short func_id      : 4;
    } bits;
    unsigned short hWord;
    
} mboxArg_t;

typedef enum npuCoreId
{
    CORE0=0,
    CORE1,
    CORE2,
    CORE3,
    CORE4,
    CORE5,
    CORE6,
    CORE7
    
} npuCoreId_t;

typedef enum npuMboxFuncId
{
    MFUNC_WIFI=0,
    MFUNC_TUNNEL,
    MFUNC_NOTIFY,
    MFUNC_DBA,
    MFUNC_TR471,
    MFUNC_MAX_CASE
    
} npuMboxFuncId_t;

typedef int (*h_mbox_cb_t) (unsigned long virtAddr, unsigned short info);

typedef struct npuMboxFlags
{
    unsigned int isBlockingMode     : 1;
    unsigned int isStaticBuffInit   : 1;
    unsigned int isStaticBuffDel    : 1;
    unsigned int resv               :29;

} mboxFlags_t;

typedef struct npuMboxInfo
{
    npuCoreId_t core_id;
    npuMboxFuncId_t func_id;
    unsigned long virtAddr;
    unsigned int physAddr;
    unsigned short len;
    mboxFlags_t flags;
    unsigned int blockTimeout;  /*unit: 100us*/
    h_mbox_cb_t cb;

} npuMboxInfo_t;

#endif
