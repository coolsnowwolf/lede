/*
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Copyright (c) 2023 Realtek Semiconductor Corp. All rights reserved.
 */

#ifndef __COMMON_ERROR_H__
#define __COMMON_ERROR_H__

/*
 * Include Files
 */
#if defined(RTK_PHYDRV_IN_LINUX)
  #include "type.h"
#else
  #include <common/type.h>
#endif
/*
 * Data Type Declaration
 */
typedef enum rt_error_common_e
{
    RT_ERR_FAILED = -1,                             /* General Error                                                                    */

    /* 0x0000xxxx for common error code */
    RT_ERR_OK = 0,                                  /* 0x00000000, OK                                                                   */
    RT_ERR_INPUT = 0xF001,                          /* 0x0000F001, invalid input parameter                                              */
    RT_ERR_UNIT_ID,                                 /* 0x0000F002, invalid unit id                                                      */
    RT_ERR_PORT_ID,                                 /* 0x0000F003, invalid port id                                                      */
    RT_ERR_PORT_MASK,                               /* 0x0000F004, invalid port mask                                                    */
    RT_ERR_PORT_LINKDOWN,                           /* 0x0000F005, link down port status                                                */
    RT_ERR_ENTRY_INDEX,                             /* 0x0000F006, invalid entry index                                                  */
    RT_ERR_NULL_POINTER,                            /* 0x0000F007, input parameter is null pointer                                      */
    RT_ERR_QUEUE_ID,                                /* 0x0000F008, invalid queue id                                                     */
    RT_ERR_QUEUE_NUM,                               /* 0x0000F009, invalid queue number                                                 */
    RT_ERR_BUSYWAIT_TIMEOUT,                        /* 0x0000F00a, busy watting time out                                                */
    RT_ERR_MAC,                                     /* 0x0000F00b, invalid mac address                                                  */
    RT_ERR_OUT_OF_RANGE,                            /* 0x0000F00c, input parameter out of range                                         */
    RT_ERR_CHIP_NOT_SUPPORTED,                      /* 0x0000F00d, functions not supported by this chip model                           */
    RT_ERR_SMI,                                     /* 0x0000F00e, SMI error                                                            */
    RT_ERR_NOT_INIT,                                /* 0x0000F00f, The module is not initial                                            */
    RT_ERR_CHIP_NOT_FOUND,                          /* 0x0000F010, The chip can not found                                               */
    RT_ERR_NOT_ALLOWED,                             /* 0x0000F011, actions not allowed by the function                                  */
    RT_ERR_DRIVER_NOT_FOUND,                        /* 0x0000F012, The driver can not found                                             */
    RT_ERR_SEM_LOCK_FAILED,                         /* 0x0000F013, Failed to lock semaphore                                             */
    RT_ERR_SEM_UNLOCK_FAILED,                       /* 0x0000F014, Failed to unlock semaphore                                           */
    RT_ERR_THREAD_EXIST,                            /* 0x0000F015, Thread exist                                                         */
    RT_ERR_THREAD_CREATE_FAILED,                    /* 0x0000F016, Thread create fail                                                   */
    RT_ERR_FWD_ACTION,                              /* 0x0000F017, Invalid forwarding Action                                            */
    RT_ERR_IPV4_ADDRESS,                            /* 0x0000F018, Invalid IPv4 address                                                 */
    RT_ERR_IPV6_ADDRESS,                            /* 0x0000F019, Invalid IPv6 address                                                 */
    RT_ERR_PRIORITY,                                /* 0x0000F01a, Invalid Priority value                                               */
    RT_ERR_FID,                                     /* 0x0000F01b, invalid fid                                                          */
    RT_ERR_ENTRY_NOTFOUND,                          /* 0x0000F01c, specified entry not found                                            */
    RT_ERR_DROP_PRECEDENCE,                         /* 0x0000F01d, invalid drop precedence                                              */
    RT_ERR_NOT_FINISH,                              /* 0x0000F01e, Action not finish, still need to wait                                */
    RT_ERR_TIMEOUT,                                 /* 0x0000F01f, Time out                                                             */
    RT_ERR_REG_ARRAY_INDEX_1,                       /* 0x0000F020, invalid index 1 of register array                                    */
    RT_ERR_REG_ARRAY_INDEX_2,                       /* 0x0000F021, invalid index 2 of register array                                    */
    RT_ERR_ETHER_TYPE,                              /* 0x0000F022, invalid ether type                                                   */
    RT_ERR_MBUF_PKT_NOT_AVAILABLE,                  /* 0x0000F023, mbuf->packet is not available                                        */
    RT_ERR_QOS_INVLD_RSN,                           /* 0x0000F024, invalid pkt to CPU reason                                            */
    RT_ERR_CB_FUNCTION_EXIST,                       /* 0x0000F025, Callback function exist                                              */
    RT_ERR_CB_FUNCTION_FULL,                        /* 0x0000F026, Callback function number is full                                     */
    RT_ERR_CB_FUNCTION_NOT_FOUND,                   /* 0x0000F027, Callback function can not found                                      */
    RT_ERR_TBL_FULL,                                /* 0x0000F028, The table is full                                                    */
    RT_ERR_TRUNK_ID,                                /* 0x0000F029, invalid trunk id                                                     */
    RT_ERR_TYPE,                                    /* 0x0000F02a, invalid type                                                         */
    RT_ERR_ENTRY_EXIST,                             /* 0x0000F02b, entry exists                                                         */
    RT_ERR_CHIP_UNDEFINED_VALUE,                    /* 0x0000F02c, chip returned an undefined value                                     */
    RT_ERR_EXCEEDS_CAPACITY,                        /* 0x0000F02d, exceeds the capacity of hardware                                     */
    RT_ERR_ENTRY_REFERRED,                          /* 0x0000F02e, entry is still being referred                                        */
    RT_ERR_OPER_DENIED,                             /* 0x0000F02f, operation denied                                                     */
    RT_ERR_PORT_NOT_SUPPORTED,                      /* 0x0000F030, functions not supported by this port                                 */
    RT_ERR_SOCKET,                                  /* 0x0000F031, socket error                                                         */
    RT_ERR_MEM_ALLOC,                               /* 0x0000F032, insufficient memory resource                                         */
    RT_ERR_ABORT,                                   /* 0x0000F033, operation aborted                                                    */
    RT_ERR_DEV_ID,                                  /* 0x0000F034, invalid device id                                                    */
    RT_ERR_DRIVER_NOT_SUPPORTED,                    /* 0x0000F035, functions not supported by this driver                               */
    RT_ERR_NOT_SUPPORTED,                           /* 0x0000F036, functions not supported                                              */
    RT_ERR_SER,                                     /* 0x0000F037, ECC or parity error                                                  */
    RT_ERR_MEM_NOT_ALIGN,                           /* 0x0000F038, memory address is not aligned                                        */
    RT_ERR_SEM_FAKELOCK_OK,                         /* 0x0000F039, attach thread lock a semaphore which was already locked              */
    RT_ERR_CHECK_FAILED,                            /* 0x0000F03a, check result is failed                                               */

    RT_ERR_COMMON_END = 0xFFFF                      /* The symbol is the latest symbol of common error                                  */
} rt_error_common_t;

/*
 * Macro Definition
 */
#define RT_PARAM_CHK(expr, errCode)\
do {\
    if ((int32)(expr)) {\
        return errCode; \
    }\
} while (0)

#define RT_PARAM_CHK_EHDL(expr, errCode, err_hdl)\
do {\
    if ((int32)(expr)) {\
        {err_hdl}\
        return errCode; \
    }\
} while (0)

#define RT_INIT_CHK(state)\
do {\
    if (INIT_COMPLETED != (state)) {\
        return RT_ERR_NOT_INIT;\
    }\
} while (0)

#define RT_INIT_REENTRY_CHK(state)\
do {\
    if (INIT_COMPLETED == (state)) {\
        osal_printf(" %s had already been initialized!\n", __FUNCTION__);\
        return RT_ERR_OK;\
    }\
} while (0)

#define RT_INIT_REENTRY_CHK_NO_WARNING(state)\
    do {\
        if (INIT_COMPLETED == (state)) {\
            return RT_ERR_OK;\
        }\
    } while (0)

#define RT_ERR_CHK(op, ret)\
do {\
    if ((ret = (op)) != RT_ERR_OK)\
        return ret;\
} while(0)

#define RT_ERR_HDL(op, errHandle, ret)\
do {\
    if ((ret = (op)) != RT_ERR_OK)\
        goto errHandle;\
} while(0)

#define RT_ERR_CHK_EHDL(op, ret, err_hdl)\
do {\
    if ((ret = (op)) != RT_ERR_OK)\
    {\
        {err_hdl}\
        return ret;\
    }\
} while(0)

#define RT_NULL_HDL(pointer, err_label)\
do {\
    if (NULL == (pointer)) {\
        goto err_label;\
    }\
} while (0)

#define RT_ERR_VOID_CHK(op, ret)\
do {\
    if ((ret = (op)) != RT_ERR_OK) {\
        osal_printf("Fail in %s %d, ret %x!\n", __FUNCTION__, __LINE__, ret);\
        return ;}\
} while(0)

#endif /* __COMMON_ERROR_H__ */

