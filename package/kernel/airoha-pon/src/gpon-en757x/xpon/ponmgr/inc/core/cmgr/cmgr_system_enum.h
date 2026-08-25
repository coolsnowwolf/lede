#ifndef _CMGR_SYSTEM_ENUM_H_
#define _CMGR_SYSTEM_ENUM_H_

#include <xpon_global/private/xmcs_if.h>

typedef enum {
    ENUM_SYSTEM_WAN_DETECTION_AUTO      = XMCS_IF_WAN_DETECT_MODE_AUTO,
    ENUM_SYSTEM_WAN_DETECTION_GPON      = XMCS_IF_WAN_DETECT_MODE_GPON,
    ENUM_SYSTEM_WAN_DETECTION_EPON      = XMCS_IF_WAN_DETECT_MODE_EPON
} MGR_SysWanDetectMode_t ;

typedef enum {
    ENUM_SYSTEM_LINK_OFF                = XMCS_IF_WAN_LINK_OFF,
    ENUM_SYSTEM_LINK_GPON               = XMCS_IF_WAN_LINK_GPON,
    ENUM_SYSTEM_LINK_EPON               = XMCS_IF_WAN_LINK_EPON
} MGR_SysLinkStatus_t ;

typedef enum {
    ENUM_SYSTEM_ONU_TYPE_UNKNOWN        = XMCS_IF_ONU_TYPE_UNKNOWN,
    ENUM_SYSTEM_ONU_TYPE_SFU            = XMCS_IF_ONU_TYPE_SFU,
    ENUM_SYSTEM_ONU_TYPE_HGU            = XMCS_IF_ONU_TYPE_HGU
} MGR_SysOnuType_t ;

typedef enum {
    ENUM_SYSTEM_CLEAR_XPON              = XMCS_IF_CLEAR_TYPE_XPON,
} MGR_SysClearType_t ;

#endif /* _CMGR_SYSTEM_ENUM_H_ */

