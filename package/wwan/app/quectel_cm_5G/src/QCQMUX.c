/******************************************************************************
  @file    MPQMUX.c
  @brief   QMI mux.

  DESCRIPTION
  Connectivity Management Tool for USB network adapter of Quectel wireless cellular modules.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  None.

  ---------------------------------------------------------------------------
  Copyright (c) 2016 - 2023 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "QMIThread.h"
static char line[1024];
static pthread_mutex_t dumpQMIMutex = PTHREAD_MUTEX_INITIALIZER;
#undef dbg
#define dbg( format, arg... ) do {if (strlen(line) < sizeof(line)) snprintf(&line[strlen(line)], sizeof(line) - strlen(line), format, ## arg);} while (0)

PQMI_TLV_HDR GetTLV (PQCQMUX_MSG_HDR pQMUXMsgHdr, int TLVType);

typedef struct {
    UINT type;
    const char *name;
} QMI_NAME_T;

#define qmi_name_item(type) {type, #type}

#if 0
static const QMI_NAME_T qmi_IFType[] = {
{USB_CTL_MSG_TYPE_QMI, "USB_CTL_MSG_TYPE_QMI"},
};

static const QMI_NAME_T qmi_CtlFlags[] = {
qmi_name_item(QMICTL_CTL_FLAG_CMD),
qmi_name_item(QCQMI_CTL_FLAG_SERVICE),
};

static const QMI_NAME_T qmi_QMIType[] = {
qmi_name_item(QMUX_TYPE_CTL),
qmi_name_item(QMUX_TYPE_WDS),
qmi_name_item(QMUX_TYPE_DMS),
qmi_name_item(QMUX_TYPE_NAS),
qmi_name_item(QMUX_TYPE_QOS),
qmi_name_item(QMUX_TYPE_WMS),
qmi_name_item(QMUX_TYPE_PDS),
qmi_name_item(QMUX_TYPE_WDS_ADMIN),
qmi_name_item(QMUX_TYPE_COEX),
};

static const QMI_NAME_T qmi_ctl_CtlFlags[] = {
qmi_name_item(QMICTL_FLAG_REQUEST),
qmi_name_item(QMICTL_FLAG_RESPONSE),
qmi_name_item(QMICTL_FLAG_INDICATION),
};
#endif

static const QMI_NAME_T qmux_ctl_QMICTLType[] = {
// QMICTL Type
qmi_name_item(QMICTL_SET_INSTANCE_ID_REQ), //    0x0020
qmi_name_item(QMICTL_SET_INSTANCE_ID_RESP), //   0x0020
qmi_name_item(QMICTL_GET_VERSION_REQ), //        0x0021
qmi_name_item(QMICTL_GET_VERSION_RESP), //       0x0021
qmi_name_item(QMICTL_GET_CLIENT_ID_REQ), //      0x0022
qmi_name_item(QMICTL_GET_CLIENT_ID_RESP), //     0x0022
qmi_name_item(QMICTL_RELEASE_CLIENT_ID_REQ), //  0x0023
qmi_name_item(QMICTL_RELEASE_CLIENT_ID_RESP), // 0x0023
qmi_name_item(QMICTL_REVOKE_CLIENT_ID_IND), //   0x0024
qmi_name_item(QMICTL_INVALID_CLIENT_ID_IND), //  0x0025
qmi_name_item(QMICTL_SET_DATA_FORMAT_REQ), //    0x0026
qmi_name_item(QMICTL_SET_DATA_FORMAT_RESP), //   0x0026
qmi_name_item(QMICTL_SYNC_REQ), //               0x0027
qmi_name_item(QMICTL_SYNC_RESP), //              0x0027
qmi_name_item(QMICTL_SYNC_IND), //               0x0027
};

static const QMI_NAME_T qmux_CtlFlags[] = {
qmi_name_item(QMUX_CTL_FLAG_TYPE_CMD),
qmi_name_item(QMUX_CTL_FLAG_TYPE_RSP),
qmi_name_item(QMUX_CTL_FLAG_TYPE_IND),
};


static const QMI_NAME_T qmux_wds_Type[] = {
qmi_name_item(QMIWDS_SET_EVENT_REPORT_REQ), //           0x0001
qmi_name_item(QMIWDS_SET_EVENT_REPORT_RESP), //          0x0001
qmi_name_item(QMIWDS_EVENT_REPORT_IND), //               0x0001
qmi_name_item(QMIWDS_START_NETWORK_INTERFACE_REQ), //    0x0020
qmi_name_item(QMIWDS_START_NETWORK_INTERFACE_RESP), //   0x0020
qmi_name_item(QMIWDS_STOP_NETWORK_INTERFACE_REQ), //     0x0021
qmi_name_item(QMIWDS_STOP_NETWORK_INTERFACE_RESP), //    0x0021
qmi_name_item(QMIWDS_GET_PKT_SRVC_STATUS_REQ), //        0x0022
qmi_name_item(QMIWDS_GET_PKT_SRVC_STATUS_RESP), //       0x0022
qmi_name_item(QMIWDS_GET_PKT_SRVC_STATUS_IND), //        0x0022  
qmi_name_item(QMIWDS_GET_CURRENT_CHANNEL_RATE_REQ), //   0x0023  
qmi_name_item(QMIWDS_GET_CURRENT_CHANNEL_RATE_RESP), //  0x0023  
qmi_name_item(QMIWDS_GET_PKT_STATISTICS_REQ), //         0x0024  
qmi_name_item(QMIWDS_GET_PKT_STATISTICS_RESP), //        0x0024  
qmi_name_item(QMIWDS_MODIFY_PROFILE_SETTINGS_REQ), //    0x0028
qmi_name_item(QMIWDS_MODIFY_PROFILE_SETTINGS_RESP), //   0x0028
qmi_name_item(QMIWDS_GET_PROFILE_SETTINGS_REQ), //    0x002B
qmi_name_item(QMIWDS_GET_PROFILE_SETTINGS_RESP), //   0x002BD
qmi_name_item(QMIWDS_GET_DEFAULT_SETTINGS_REQ), //       0x002C
qmi_name_item(QMIWDS_GET_DEFAULT_SETTINGS_RESP), //      0x002C
qmi_name_item(QMIWDS_GET_RUNTIME_SETTINGS_REQ), //       0x002D
qmi_name_item(QMIWDS_GET_RUNTIME_SETTINGS_RESP), //      0x002D
qmi_name_item(QMIWDS_GET_MIP_MODE_REQ), //               0x002F
qmi_name_item(QMIWDS_GET_MIP_MODE_RESP), //              0x002F
qmi_name_item(QMIWDS_GET_DATA_BEARER_REQ), //            0x0037
qmi_name_item(QMIWDS_GET_DATA_BEARER_RESP), //           0x0037
qmi_name_item(QMIWDS_DUN_CALL_INFO_REQ), //              0x0038
qmi_name_item(QMIWDS_DUN_CALL_INFO_RESP), //             0x0038
qmi_name_item(QMIWDS_DUN_CALL_INFO_IND), //              0x0038
qmi_name_item(QMIWDS_SET_CLIENT_IP_FAMILY_PREF_REQ), //  0x004D  
qmi_name_item(QMIWDS_SET_CLIENT_IP_FAMILY_PREF_RESP), // 0x004D  
qmi_name_item(QMIWDS_SET_AUTO_CONNECT_REQ), //  0x0051  
qmi_name_item(QMIWDS_SET_AUTO_CONNECT_RESP), // 0x0051
qmi_name_item(QMIWDS_BIND_MUX_DATA_PORT_REQ), //         0x00A2  
qmi_name_item(QMIWDS_BIND_MUX_DATA_PORT_RESP), //        0x00A2  
};

static const QMI_NAME_T qmux_dms_Type[] = {
// ======================= DMS ==============================
qmi_name_item(QMIDMS_SET_EVENT_REPORT_REQ), //           0x0001
qmi_name_item(QMIDMS_SET_EVENT_REPORT_RESP), //          0x0001
qmi_name_item(QMIDMS_EVENT_REPORT_IND), //               0x0001
qmi_name_item(QMIDMS_GET_DEVICE_CAP_REQ), //             0x0020
qmi_name_item(QMIDMS_GET_DEVICE_CAP_RESP), //            0x0020
qmi_name_item(QMIDMS_GET_DEVICE_MFR_REQ), //             0x0021
qmi_name_item(QMIDMS_GET_DEVICE_MFR_RESP), //            0x0021
qmi_name_item(QMIDMS_GET_DEVICE_MODEL_ID_REQ), //        0x0022
qmi_name_item(QMIDMS_GET_DEVICE_MODEL_ID_RESP), //       0x0022
qmi_name_item(QMIDMS_GET_DEVICE_REV_ID_REQ), //          0x0023
qmi_name_item(QMIDMS_GET_DEVICE_REV_ID_RESP), //         0x0023
qmi_name_item(QMIDMS_GET_MSISDN_REQ), //                 0x0024
qmi_name_item(QMIDMS_GET_MSISDN_RESP), //                0x0024
qmi_name_item(QMIDMS_GET_DEVICE_SERIAL_NUMBERS_REQ), //  0x0025
qmi_name_item(QMIDMS_GET_DEVICE_SERIAL_NUMBERS_RESP), // 0x0025
qmi_name_item(QMIDMS_UIM_SET_PIN_PROTECTION_REQ), //     0x0027
qmi_name_item(QMIDMS_UIM_SET_PIN_PROTECTION_RESP), //    0x0027
qmi_name_item(QMIDMS_UIM_VERIFY_PIN_REQ), //             0x0028
qmi_name_item(QMIDMS_UIM_VERIFY_PIN_RESP), //            0x0028
qmi_name_item(QMIDMS_UIM_UNBLOCK_PIN_REQ), //            0x0029
qmi_name_item(QMIDMS_UIM_UNBLOCK_PIN_RESP), //           0x0029
qmi_name_item(QMIDMS_UIM_CHANGE_PIN_REQ), //             0x002A
qmi_name_item(QMIDMS_UIM_CHANGE_PIN_RESP), //            0x002A
qmi_name_item(QMIDMS_UIM_GET_PIN_STATUS_REQ), //         0x002B
qmi_name_item(QMIDMS_UIM_GET_PIN_STATUS_RESP), //        0x002B
qmi_name_item(QMIDMS_GET_DEVICE_HARDWARE_REV_REQ), //    0x002C
qmi_name_item(QMIDMS_GET_DEVICE_HARDWARE_REV_RESP), //   0x002C
qmi_name_item(QMIDMS_GET_OPERATING_MODE_REQ), //         0x002D 
qmi_name_item(QMIDMS_GET_OPERATING_MODE_RESP), //        0x002D 
qmi_name_item(QMIDMS_SET_OPERATING_MODE_REQ), //         0x002E 
qmi_name_item(QMIDMS_SET_OPERATING_MODE_RESP), //        0x002E 
qmi_name_item(QMIDMS_GET_ACTIVATED_STATUS_REQ), //       0x0031 
qmi_name_item(QMIDMS_GET_ACTIVATED_STATUS_RESP), //      0x0031 
qmi_name_item(QMIDMS_ACTIVATE_AUTOMATIC_REQ), //         0x0032
qmi_name_item(QMIDMS_ACTIVATE_AUTOMATIC_RESP), //        0x0032
qmi_name_item(QMIDMS_ACTIVATE_MANUAL_REQ), //            0x0033
qmi_name_item(QMIDMS_ACTIVATE_MANUAL_RESP), //           0x0033
qmi_name_item(QMIDMS_UIM_GET_ICCID_REQ), //              0x003C 
qmi_name_item(QMIDMS_UIM_GET_ICCID_RESP), //             0x003C 
qmi_name_item(QMIDMS_UIM_GET_CK_STATUS_REQ), //          0x0040
qmi_name_item(QMIDMS_UIM_GET_CK_STATUS_RESP), //         0x0040
qmi_name_item(QMIDMS_UIM_SET_CK_PROTECTION_REQ), //      0x0041
qmi_name_item(QMIDMS_UIM_SET_CK_PROTECTION_RESP), //     0x0041
qmi_name_item(QMIDMS_UIM_UNBLOCK_CK_REQ), //             0x0042
qmi_name_item(QMIDMS_UIM_UNBLOCK_CK_RESP), //            0x0042
qmi_name_item(QMIDMS_UIM_GET_IMSI_REQ), //               0x0043 
qmi_name_item(QMIDMS_UIM_GET_IMSI_RESP), //              0x0043 
qmi_name_item(QMIDMS_UIM_GET_STATE_REQ), //              0x0044 
qmi_name_item(QMIDMS_UIM_GET_STATE_RESP), //             0x0044 
qmi_name_item(QMIDMS_GET_BAND_CAP_REQ), //               0x0045 
qmi_name_item(QMIDMS_GET_BAND_CAP_RESP), //              0x0045 
};

static const QMI_NAME_T qmux_qos_Type[] = {
qmi_name_item( QMI_QOS_SET_EVENT_REPORT_REQ), //        0x0001
qmi_name_item( QMI_QOS_SET_EVENT_REPORT_RESP), //       0x0001
qmi_name_item( QMI_QOS_SET_EVENT_REPORT_IND), //        0x0001
qmi_name_item( QMI_QOS_BIND_DATA_PORT_REQ), //          0x002B
qmi_name_item( QMI_QOS_BIND_DATA_PORT_RESP), //         0x002B
qmi_name_item( QMI_QOS_INDICATION_REGISTER_REQ), //     0x002F
qmi_name_item( QMI_QOS_INDICATION_REGISTER_RESP), //    0x002F
qmi_name_item( QMI_QOS_GLOBAL_QOS_FLOW_IND), //         0x0031
qmi_name_item( QMI_QOS_GET_QOS_INFO_REQ), //            0x0033
qmi_name_item( QMI_QOS_GET_QOS_INFO_RESP), //           0x0033
};

static const QMI_NAME_T qmux_nas_Type[] = {
// ======================= NAS ==============================
qmi_name_item(QMINAS_SET_EVENT_REPORT_REQ), //             0x0002
qmi_name_item(QMINAS_SET_EVENT_REPORT_RESP), //            0x0002
qmi_name_item(QMINAS_EVENT_REPORT_IND), //                 0x0002
qmi_name_item(QMINAS_GET_SIGNAL_STRENGTH_REQ), //          0x0020
qmi_name_item(QMINAS_GET_SIGNAL_STRENGTH_RESP), //         0x0020
qmi_name_item(QMINAS_PERFORM_NETWORK_SCAN_REQ), //         0x0021
qmi_name_item(QMINAS_PERFORM_NETWORK_SCAN_RESP), //        0x0021
qmi_name_item(QMINAS_INITIATE_NW_REGISTER_REQ), //         0x0022
qmi_name_item(QMINAS_INITIATE_NW_REGISTER_RESP), //        0x0022
qmi_name_item(QMINAS_INITIATE_ATTACH_REQ), //              0x0023
qmi_name_item(QMINAS_INITIATE_ATTACH_RESP), //             0x0023
qmi_name_item(QMINAS_GET_SERVING_SYSTEM_REQ), //           0x0024
qmi_name_item(QMINAS_GET_SERVING_SYSTEM_RESP), //          0x0024
qmi_name_item(QMINAS_SERVING_SYSTEM_IND), //               0x0024
qmi_name_item(QMINAS_GET_HOME_NETWORK_REQ), //             0x0025
qmi_name_item(QMINAS_GET_HOME_NETWORK_RESP), //            0x0025
qmi_name_item(QMINAS_GET_PREFERRED_NETWORK_REQ), //        0x0026
qmi_name_item(QMINAS_GET_PREFERRED_NETWORK_RESP), //       0x0026
qmi_name_item(QMINAS_SET_PREFERRED_NETWORK_REQ), //        0x0027
qmi_name_item(QMINAS_SET_PREFERRED_NETWORK_RESP), //       0x0027
qmi_name_item(QMINAS_GET_FORBIDDEN_NETWORK_REQ), //        0x0028
qmi_name_item(QMINAS_GET_FORBIDDEN_NETWORK_RESP), //       0x0028
qmi_name_item(QMINAS_SET_FORBIDDEN_NETWORK_REQ), //        0x0029
qmi_name_item(QMINAS_SET_FORBIDDEN_NETWORK_RESP), //       0x0029
qmi_name_item(QMINAS_SET_TECHNOLOGY_PREF_REQ), //          0x002A
qmi_name_item(QMINAS_SET_TECHNOLOGY_PREF_RESP), //         0x002A
qmi_name_item(QMINAS_GET_RF_BAND_INFO_REQ), //             0x0031
qmi_name_item(QMINAS_GET_RF_BAND_INFO_RESP), //            0x0031
qmi_name_item(QMINAS_GET_CELL_LOCATION_INFO_REQ),
qmi_name_item(QMINAS_GET_CELL_LOCATION_INFO_RESP),
qmi_name_item(QMINAS_GET_PLMN_NAME_REQ), //                0x0044
qmi_name_item(QMINAS_GET_PLMN_NAME_RESP), //               0x0044
qmi_name_item(QUECTEL_PACKET_TRANSFER_START_IND), //                0X100
qmi_name_item(QUECTEL_PACKET_TRANSFER_END_IND), //               0X101
qmi_name_item(QMINAS_GET_SYS_INFO_REQ), //                 0x004D
qmi_name_item(QMINAS_GET_SYS_INFO_RESP), //                0x004D
qmi_name_item(QMINAS_SYS_INFO_IND), //                     0x004D
qmi_name_item(QMINAS_GET_SIG_INFO_REQ),
qmi_name_item(QMINAS_GET_SIG_INFO_RESP),

};

static const QMI_NAME_T qmux_wms_Type[] = {
// ======================= WMS ==============================
qmi_name_item(QMIWMS_SET_EVENT_REPORT_REQ), //           0x0001
qmi_name_item(QMIWMS_SET_EVENT_REPORT_RESP), //          0x0001
qmi_name_item(QMIWMS_EVENT_REPORT_IND), //               0x0001
qmi_name_item(QMIWMS_RAW_SEND_REQ), //                   0x0020
qmi_name_item(QMIWMS_RAW_SEND_RESP), //                  0x0020
qmi_name_item(QMIWMS_RAW_WRITE_REQ), //                  0x0021
qmi_name_item(QMIWMS_RAW_WRITE_RESP), //                 0x0021
qmi_name_item(QMIWMS_RAW_READ_REQ), //                   0x0022
qmi_name_item(QMIWMS_RAW_READ_RESP), //                  0x0022
qmi_name_item(QMIWMS_MODIFY_TAG_REQ), //                 0x0023
qmi_name_item(QMIWMS_MODIFY_TAG_RESP), //                0x0023
qmi_name_item(QMIWMS_DELETE_REQ), //                     0x0024
qmi_name_item(QMIWMS_DELETE_RESP), //                    0x0024
qmi_name_item(QMIWMS_GET_MESSAGE_PROTOCOL_REQ), //       0x0030
qmi_name_item(QMIWMS_GET_MESSAGE_PROTOCOL_RESP), //      0x0030
qmi_name_item(QMIWMS_LIST_MESSAGES_REQ), //              0x0031
qmi_name_item(QMIWMS_LIST_MESSAGES_RESP), //             0x0031
qmi_name_item(QMIWMS_GET_SMSC_ADDRESS_REQ), //           0x0034
qmi_name_item(QMIWMS_GET_SMSC_ADDRESS_RESP), //          0x0034
qmi_name_item(QMIWMS_SET_SMSC_ADDRESS_REQ), //           0x0035
qmi_name_item(QMIWMS_SET_SMSC_ADDRESS_RESP), //          0x0035
qmi_name_item(QMIWMS_GET_STORE_MAX_SIZE_REQ), //         0x0036
qmi_name_item(QMIWMS_GET_STORE_MAX_SIZE_RESP), //        0x0036
};

static const QMI_NAME_T qmux_wds_admin_Type[] = {
qmi_name_item(QMIWDS_ADMIN_SET_DATA_FORMAT_REQ), //      0x0020
qmi_name_item(QMIWDS_ADMIN_SET_DATA_FORMAT_RESP), //     0x0020
qmi_name_item(QMIWDS_ADMIN_GET_DATA_FORMAT_REQ), //      0x0021
qmi_name_item(QMIWDS_ADMIN_GET_DATA_FORMAT_RESP), //     0x0021
qmi_name_item(QMIWDS_ADMIN_SET_QMAP_SETTINGS_REQ), //    0x002B
qmi_name_item(QMIWDS_ADMIN_SET_QMAP_SETTINGS_RESP), //   0x002B
qmi_name_item(QMIWDS_ADMIN_GET_QMAP_SETTINGS_REQ), //    0x002C
qmi_name_item(QMIWDS_ADMIN_GET_QMAP_SETTINGS_RESP), //   0x002C
qmi_name_item(QMI_WDA_SET_LOOPBACK_CONFIG_REQ), //	 0x002F
qmi_name_item(QMI_WDA_SET_LOOPBACK_CONFIG_RESP), //	 0x002F
qmi_name_item(QMI_WDA_SET_LOOPBACK_CONFIG_IND), //	 0x002F
};

static const QMI_NAME_T qmux_uim_Type[] = {
qmi_name_item( QMIUIM_READ_TRANSPARENT_REQ), //      0x0020
qmi_name_item( QMIUIM_READ_TRANSPARENT_RESP), //     0x0020
qmi_name_item( QMIUIM_READ_TRANSPARENT_IND), //      0x0020
qmi_name_item( QMIUIM_READ_RECORD_REQ), //           0x0021
qmi_name_item( QMIUIM_READ_RECORD_RESP), //          0x0021
qmi_name_item( QMIUIM_READ_RECORD_IND), //           0x0021
qmi_name_item( QMIUIM_WRITE_TRANSPARENT_REQ), //     0x0022
qmi_name_item( QMIUIM_WRITE_TRANSPARENT_RESP), //    0x0022
qmi_name_item( QMIUIM_WRITE_TRANSPARENT_IND), //     0x0022
qmi_name_item( QMIUIM_WRITE_RECORD_REQ), //          0x0023
qmi_name_item( QMIUIM_WRITE_RECORD_RESP), //         0x0023
qmi_name_item( QMIUIM_WRITE_RECORD_IND), //          0x0023
qmi_name_item( QMIUIM_SET_PIN_PROTECTION_REQ), //    0x0025
qmi_name_item( QMIUIM_SET_PIN_PROTECTION_RESP), //   0x0025
qmi_name_item( QMIUIM_SET_PIN_PROTECTION_IND), //    0x0025
qmi_name_item( QMIUIM_VERIFY_PIN_REQ), //            0x0026
qmi_name_item( QMIUIM_VERIFY_PIN_RESP), //           0x0026
qmi_name_item( QMIUIM_VERIFY_PIN_IND), //            0x0026
qmi_name_item( QMIUIM_UNBLOCK_PIN_REQ), //           0x0027
qmi_name_item( QMIUIM_UNBLOCK_PIN_RESP), //          0x0027
qmi_name_item( QMIUIM_UNBLOCK_PIN_IND), //           0x0027
qmi_name_item( QMIUIM_CHANGE_PIN_REQ), //            0x0028
qmi_name_item( QMIUIM_CHANGE_PIN_RESP), //           0x0028
qmi_name_item( QMIUIM_CHANGE_PIN_IND), //            0x0028
qmi_name_item( QMIUIM_DEPERSONALIZATION_REQ), //     0x0029
qmi_name_item( QMIUIM_DEPERSONALIZATION_RESP), //    0x0029
qmi_name_item( QMIUIM_EVENT_REG_REQ), //             0x002E
qmi_name_item( QMIUIM_EVENT_REG_RESP), //            0x002E
qmi_name_item( QMIUIM_GET_CARD_STATUS_REQ), //       0x002F
qmi_name_item( QMIUIM_GET_CARD_STATUS_RESP), //      0x002F
qmi_name_item( QMIUIM_STATUS_CHANGE_IND), //         0x0032
};

static const QMI_NAME_T qmux_coex_Type[] = {
qmi_name_item(QMI_COEX_GET_WWAN_STATE_REQ), //    0x0022
qmi_name_item(QMI_COEX_GET_WWAN_STATE_RESP), //    0x0022
};

static const char * qmi_name_get(const QMI_NAME_T *table, size_t size, int type, const char *tag) {
    static char unknow[40];
    size_t i;

    if (qmux_CtlFlags == table) {
        if (!strcmp(tag, "_REQ"))
            tag = "_CMD";
        else  if (!strcmp(tag, "_RESP"))
            tag = "_RSP";
    }
    
    for (i = 0; i < size; i++) {
        if (table[i].type == (UINT)type) {
            if (!tag || (strstr(table[i].name, tag)))
                return table[i].name;
        }
    }
    sprintf(unknow, "unknow_%x", type);
    return unknow;
}

#define QMI_NAME(table, type) qmi_name_get(table, sizeof(table) / sizeof(table[0]), type, 0)
#define QMUX_NAME(table, type, tag) qmi_name_get(table, sizeof(table) / sizeof(table[0]), type, tag)

void dump_tlv(PQCQMUX_MSG_HDR pQMUXMsgHdr) {
    int TLVFind = 0;
    int i;
    //dbg("QCQMUX_TLV-----------------------------------\n");
    //dbg("{Type,\tLength,\tValue}\n");

    while (1) {
        PQMI_TLV_HDR TLVHdr = GetTLV(pQMUXMsgHdr, 0x1000 + (++TLVFind));
        if (TLVHdr == NULL)
            break;

        //if ((TLVHdr->TLVType == 0x02) && ((USHORT *)(TLVHdr+1))[0])
        {        
            dbg("{%02x,\t%04x,\t", TLVHdr->TLVType, le16_to_cpu(TLVHdr->TLVLength));
            for (i = 0; i < le16_to_cpu(TLVHdr->TLVLength); i++) {
                dbg("%02x ", ((UCHAR *)(TLVHdr+1))[i]);
            }
            dbg("}\n");
        }
    }  // while
}

void dump_ctl(PQCQMICTL_MSG_HDR CTLHdr) {
    const char *tag;
    
    //dbg("QCQMICTL_MSG--------------------------------------------\n");
    //dbg("CtlFlags:           %02x\t\t%s\n", CTLHdr->CtlFlags, QMI_NAME(qmi_ctl_CtlFlags, CTLHdr->CtlFlags));
   dbg("TransactionId:      %02x\n", CTLHdr->TransactionId);
        switch (CTLHdr->CtlFlags) {
            case QMICTL_FLAG_REQUEST: tag = "_REQ"; break;
            case QMICTL_FLAG_RESPONSE: tag = "_RESP"; break;
            case QMICTL_FLAG_INDICATION: tag = "_IND"; break;
            default: tag = 0; break;
       }
        dbg("QMICTLType:         %04x\t%s\n", le16_to_cpu(CTLHdr->QMICTLType),
        QMUX_NAME(qmux_ctl_QMICTLType, le16_to_cpu(CTLHdr->QMICTLType), tag));     
        dbg("Length:             %04x\n", le16_to_cpu(CTLHdr->Length));

     dump_tlv((PQCQMUX_MSG_HDR)(&CTLHdr->QMICTLType));
}

int dump_qmux(QMI_SERVICE_TYPE serviceType, PQCQMUX_HDR QMUXHdr) {
    PQCQMUX_MSG_HDR QMUXMsgHdr = (PQCQMUX_MSG_HDR) (QMUXHdr + 1);
    const char *tag;

    //dbg("QCQMUX--------------------------------------------\n");
    switch (QMUXHdr->CtlFlags&QMUX_CTL_FLAG_MASK_TYPE) {
        case QMUX_CTL_FLAG_TYPE_CMD: tag = "_REQ"; break;
        case QMUX_CTL_FLAG_TYPE_RSP: tag = "_RESP"; break;
        case QMUX_CTL_FLAG_TYPE_IND: tag = "_IND"; break;
        default: tag = 0; break;
    }
    //dbg("CtlFlags:           %02x\t\t%s\n", QMUXHdr->CtlFlags, QMUX_NAME(qmux_CtlFlags, QMUXHdr->CtlFlags, tag));
    dbg("TransactionId:    %04x\n", le16_to_cpu(QMUXHdr->TransactionId));

    //dbg("QCQMUX_MSG_HDR-----------------------------------\n");
    switch (serviceType) {
        case QMUX_TYPE_DMS:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type),
            QMUX_NAME(qmux_dms_Type, le16_to_cpu(QMUXMsgHdr->Type), tag));
        break;
        case QMUX_TYPE_NAS:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type),
            QMUX_NAME(qmux_nas_Type, le16_to_cpu(QMUXMsgHdr->Type), tag));
        break;
        case QMUX_TYPE_WDS:
        case QMUX_TYPE_WDS_IPV6:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type),
            QMUX_NAME(qmux_wds_Type, le16_to_cpu(QMUXMsgHdr->Type), tag));
        break;
        case QMUX_TYPE_WMS:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type),
            QMUX_NAME(qmux_wms_Type, le16_to_cpu(QMUXMsgHdr->Type), tag));
        break;
        case QMUX_TYPE_WDS_ADMIN:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type),
            QMUX_NAME(qmux_wds_admin_Type, le16_to_cpu(QMUXMsgHdr->Type), tag));
        break;
        case QMUX_TYPE_UIM:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type),
            QMUX_NAME(qmux_uim_Type, le16_to_cpu(QMUXMsgHdr->Type), tag));
        break;
        case QMUX_TYPE_PDS:
        case QMUX_TYPE_QOS:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type),
            QMUX_NAME(qmux_qos_Type, le16_to_cpu(QMUXMsgHdr->Type), tag));
        break;
        case QMUX_TYPE_COEX:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type),
            QMUX_NAME(qmux_coex_Type, le16_to_cpu(QMUXMsgHdr->Type), tag));
        break;
        case QMUX_TYPE_CTL:                
        default:
            dbg("Type:               %04x\t%s\n", le16_to_cpu(QMUXMsgHdr->Type), "PDS/QOS/CTL/unknown!");
        break;    
    }
    dbg("Length:             %04x\n", le16_to_cpu(QMUXMsgHdr->Length));

    dump_tlv(QMUXMsgHdr);
    
    return 0;
}

void dump_qmi(void *dataBuffer, int dataLen) 
{
    PQCQMI_HDR QMIHdr = (PQCQMI_HDR)dataBuffer;
    PQCQMUX_HDR QMUXHdr = (PQCQMUX_HDR) (QMIHdr + 1);
    PQCQMICTL_MSG_HDR CTLHdr =  (PQCQMICTL_MSG_HDR) (QMIHdr + 1);

    int i;

    if (!debug_qmi)
        return;

    pthread_mutex_lock(&dumpQMIMutex);
    line[0] = 0;
    for (i = 0; i < dataLen; i++) {
        dbg("%02x ", ((unsigned char *)dataBuffer)[i]);
    }
    dbg_time("%s", line);
    line[0] = 0;
    
    //dbg("QCQMI_HDR-----------------------------------------");
    //dbg("IFType:             %02x\t\t%s", QMIHdr->IFType, QMI_NAME(qmi_IFType, QMIHdr->IFType));
    //dbg("Length:             %04x", le16_to_cpu(QMIHdr->Length));
    //dbg("CtlFlags:           %02x\t\t%s", QMIHdr->CtlFlags, QMI_NAME(qmi_CtlFlags, QMIHdr->CtlFlags));
    //dbg("QMIType:            %02x\t\t%s", QMIHdr->QMIType, QMI_NAME(qmi_QMIType, QMIHdr->QMIType));
    //dbg("ClientId:           %02x", QMIHdr->ClientId);

    if (QMIHdr->QMIType == QMUX_TYPE_CTL) {
        dump_ctl(CTLHdr);
    } else {
        dump_qmux(QMIHdr->QMIType, QMUXHdr);
    }
    dbg_time("%s", line);
    pthread_mutex_unlock(&dumpQMIMutex);
}
