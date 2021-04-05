/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
    ap_mumimo.h

    Abstract:
    Miniport generic portion header file

    Revision History:
    Who         When          What
    --------    ----------    ----------------------------------------------
*/
#ifndef __AP_MUMIMO_H__
#define __AP_MUMIMO_H__

#define NUM_OF_USER                                4
#define NUM_OF_MODUL                               5

#define NUM_MUT_FEC             2
#define NUM_MUT_NR              3
#define NUM_MUT_MCS             10
#define NUM_MUT_INDEX           8
#define NUM_SUT_INDEX           3

enum {
	/* debug commands */
	MU_SET_ENABLE = 0,
	MU_GET_ENABLE,
	MU_SET_MUPROFILE_ENTRY,
	MU_GET_MUPROFILE_ENTRY,
	MU_SET_GROUP_TBL_ENTRY,
	MU_GET_GROUP_TBL_ENTRY,
	MU_SET_CLUSTER_TBL_ENTRY,
	MU_GET_CLUSTER_TBL_ENTRY,
	MU_SET_GROUP_USER_THRESHOLD,
	MU_GET_GROUP_USER_THRESHOLD,
	MU_SET_GROUP_NSS_THRESHOLD,
	MU_GET_GROUP_NSS_THRESHOLD,
	MU_SET_TXREQ_MIN_TIME,
	MU_GET_TXREQ_MIN_TIME,
	MU_SET_SU_NSS_CHECK,
	MU_GET_SU_NSS_CHECK,
	MU_SET_CALC_INIT_MCS,
	MU_GET_CALC_INIT_MCS,
	MU_SET_TXOP_DEFAULT,
	MU_GET_TXOP_DEFAULT,
	MU_SET_SU_LOSS_THRESHOLD,
	MU_GET_SU_LOSS_THRESHOLD,
	MU_SET_MU_GAIN_THRESHOLD,
	MU_GET_MU_GAIN_THRESHOLD,
	MU_SET_SECONDARY_AC_POLICY,
	MU_GET_SECONDARY_AC_POLICY,
	MU_SET_GROUP_TBL_DMCS_MASK,
	MU_GET_GROUP_TBL_DMCS_MASK,
	MU_SET_MAX_GROUP_SEARCH_CNT,
	MU_GET_MAX_GROUP_SEARCH_CNT,
	MU_GET_MU_PROFILE_TX_STATUS_CNT,
	MU_SET_TRIGGER_MU_TX,
	/* F/W flow test commands */
	MU_SET_TRIGGER_GROUP,
	MU_SET_TRIGGER_DEGROUP,
	MU_SET_TRIGGER_GID_MGMT_FRAME,
	MU_SET_TRIGGER_BBP,
	MU_SET_TRIGGER_SND,
	/* HQA AP commands */
	MU_HQA_SET_ENABLE = 70,
	MU_HQA_SET_SNR_OFFSET,
	MU_HQA_SET_ZERO_NSS,
	MU_HQA_SET_SPEED_UP_LQ,
	MU_HQA_SET_GROUP,
	MU_HQA_SET_MU_TABLE,
	MU_HQA_SET_SU_TABLE,
	MU_HQA_SET_CALC_LQ,
	MU_HQA_GET_CALC_LQ,
	MU_HQA_SET_CALC_INIT_MCS,
	MU_HQA_GET_CALC_INIT_MCS,
	MU_HQA_GET_QD,
	MU_HQA_GET_CALC_SU_LQ,
	MU_HQA_SET_CALC_SU_LQ,
};

enum {
	MU_EVENT_MU_ENABLE,
	MU_EVENT_MUPROFILE_ENTRY,
	MU_EVENT_GROUP_TBL_ENTRY,
	MU_EVENT_CLUSTER_TBL_ENTRY,
	MU_EVENT_GROUP_USER_THRESHOLD,
	MU_EVENT_GROUP_NSS_THRESHOLD,
	MU_EVENT_TXREQ_MIN_TIME,
	MU_EVENT_CALC_INIT_MCS,
	MU_EVENT_SU_NSS_CHECK,
	MU_EVENT_TXOP_DEFAULT,
	MU_EVENT_SU_LOSS_THRESHOLD,
	MU_EVENT_MU_GAIN_THRESHOLD,
	MU_EVENT_SECONDARY_AC_POLICY,
	MU_EVENT_GROUP_TBL_DMCS_MASK,
	MU_EVENT_MAX_GROUP_SEARCH_CNT,
	MU_EVENT_MUPROFILE_TX_STS_CNT,
	MU_EVENT_STATUS,
	/* HQA AP event */
	MU_EVENT_HQA_GET_INIT_MCS = 70,
	MU_EVENT_HQA_GET_LQ,
	MU_EVENT_HQA_GET_SU_LQ,
	MU_EVENT_HQA_GET_QD,
	MU_EVENT_HQA_STATUS,
};

typedef struct _CMD_MU_SET_ENABLE {
	UINT8 fgIsEnable;
} CMD_MU_SET_ENABLE, *P_CMD_MU_SET_ENABLE;

typedef struct _CMD_MU_SET_MUPROFILE_ENTRY {
	UINT8 index;
	BOOLEAN valid;
#ifdef RT_BIG_ENDIAN
	UINT8 reserved:4;
	UINT8 baMask:4;
#else
    UINT8 baMask:4;
	UINT8 reserved:4;
#endif
	UINT8 wlanIdx;
} CMD_MU_SET_MUPROFILE_ENTRY, *P_CMD_MU_SET_MUPROFILE_ENTRY;

typedef struct _CMD_MU_SET_GROUP_TBL_ENTRY {
	UINT32      index;
#ifdef RT_BIG_ENDIAN
	UINT8		NS3:1;
	UINT8		NS2:1;
	UINT8		NS1:1;
	UINT8		NS0:1;
	UINT8		BW:2;
	UINT8       numUser:2;
#else
    UINT8       numUser:2;
	UINT8       BW:2;
	UINT8       NS0:1;
	UINT8       NS1:1;
	UINT8       NS2:1;
	UINT8       NS3:1;
#endif
	UINT8       PFIDUser0;
	UINT8       PFIDUser1;
	UINT8       PFIDUser2;
	UINT8       PFIDUser3;
	BOOLEAN     fgIsShortGI;
	BOOLEAN     fgIsUsed;
	BOOLEAN     fgIsDisable;
#ifdef RT_BIG_ENDIAN
	UINT8		dMcsUser3:4;
	UINT8		dMcsUser2:4;
	UINT8		dMcsUser1:4;
	UINT8		dMcsUser0:4;
	UINT8		initMcsUser3:4;
	UINT8		initMcsUser2:4;
	UINT8		initMcsUser1:4;
	UINT8		initMcsUser0:4;
#else
	UINT8       initMcsUser0:4;
	UINT8       initMcsUser1:4;
	UINT8       initMcsUser2:4;
	UINT8       initMcsUser3:4;
	UINT8       dMcsUser0:4;
	UINT8       dMcsUser1:4;
	UINT8       dMcsUser2:4;
	UINT8       dMcsUser3:4;
#endif
} CMD_MU_SET_GROUP_TBL_ENTRY, *P_CMD_MU_SET_GROUP_TBL_ENTRY;

typedef struct _CMD_MU_SET_CLUSTER_TBL_ENTRY {
	UINT8   index;
	UINT32  gidUserMemberStatus[2];
	UINT32  gidUserPosition[4];
} CMD_MU_SET_CLUSTER_TBL_ENTRY, *P_CMD_MU_SET_CLUSTER_TBL_ENTRY;

typedef struct _CMD_SET_TXREQ_MIN_TIME {
	UINT16 value;
} CMD_SET_TXREQ_MIN_TIME, *P_CMD_SET_TXREQ_MIN_TIME;

typedef struct _CMD_SET_NSS_CHECK {
	UINT8 fgIsEnable;
} CMD_SET_NSS_CHECK, *P_CMD_SET_NSS_CHECK;

typedef struct _CMD_SET_SU_LOSS_THRESHOLD {
	UINT16 value;
} CMD_SET_SU_LOSS_THRESHOLD, *P_CMD_SET_SU_LOSS_THRESHOLD;

typedef struct _CMD_SET_MU_GAIN_THRESHOLD {
	UINT16 value;
} CMD_SET_MU_GAIN_THRESHOLD, *P_CMD_SET_MU_GAIN_THRESHOLD;

typedef struct _CMD_SET_TXOP_DEFAULT {
	UINT32 value;
} CMD_SET_TXOP_DEFAULT, *P_CMD_SET_TXOP_DEFAULT;

typedef struct _CMD_SET_GROUP_USER_THRESHOLD {
	UINT8 min;
	UINT8 max;
} CMD_SET_GROUP_USER_THRESHOLD, *P_CMD_SET_GROUP_USER_THRESHOLD;

typedef struct _CMD_SET_MU_SECONDARY_AC_POLICY {
	UINT16 value;
} CMD_SET_MU_SECONDARY_AC_POLICY, *P_CMD_SET_MU_SECONDARY_AC_POLICY;

typedef struct _CMD_SET_MU_MAX_GROUP_SEARCH_CNT {
	UINT32 value;
} CMD_SET_MU_MAX_GROUP_SEARCH_CNT, *P_CMD_SET_MU_MAX_GROUP_SEARCH_CNT;

typedef struct _CMD_SET_MU_GROUP_TBL_DMCS_MASK {
	UINT8 value;
} CMD_SET_MU_GROUP_TBL_DMCS_MASK, *P_CMD_SET_MU_GROUP_TBL_DMCS_MASK;

typedef struct _CMD_SET_GROUP_NSS_THRESHOLD {
	UINT8 min;
	UINT8 max;
} CMD_SET_GROUP_NSS_THRESHOLD, *P_CMD_SET_GROUP_NSS_THRESHOLD;

typedef struct _EVENT_SHOW_MU_ENABLE {
	UINT32 u4EventId;
	UINT8 fgIsEnable;
} EVENT_SHOW_MU_ENABLE, *P_EVENT_SHOW_MU_ENABLE;

typedef struct _EVENT_STATUS {
	UINT32 u4EventId;
	UINT16 status;
} EVENT_STATUS, *P_EVENT_STATUS;

typedef struct _EVENT_HQA_STATUS {
	UINT32 u4EventId;
	UINT16 status;
} EVENT_HQA_STATUS, *P_EVENT_HQA_STATUS;

typedef struct _EVENT_SHOW_NSS_CHECK {
	UINT32 u4EventId;
	UINT8 fgIsEnable;
} EVENT_SHOW_NSS_CHECK, *P_EVENT_SHOW_NSS_CHECK;

typedef struct _EVENT_SHOW_MUPROFILE_ENTRY {
	UINT32 u4EventId;
	UINT8 index;
	BOOLEAN valid;
#ifdef RT_BIG_ENDIAN
    UINT8 reserved:4;
	UINT8 baMask:4;
#else
    UINT8 baMask:4;
	UINT8 reserved:4;
#endif
	UINT8 wlanIdx;
} EVENT_SHOW_MUPROFILE_ENTRY, *P_EVENT_SHOW_MUPROFILE_ENTRY;

typedef struct _EVENT_GET_TXOP_DEFAULT {
	UINT32 u4EventId;
	UINT32 value;
} EVENT_GET_TXOP_DEFAULT, *P_EVENT_GET_TXOP_DEFAULT;

typedef struct _EVENT_SHOW_GROUP_TBL_ENTRY {
	UINT32 u4EventId;
	UINT32      index;
#ifdef RT_BIG_ENDIAN
	UINT8		NS3:1;
	UINT8		NS2:1;
	UINT8		NS1:1;
	UINT8		NS0:1;
	UINT8		BW:2;
	UINT8       numUser:2;
#else
    UINT8       numUser:2;
	UINT8       BW:2;
	UINT8       NS0:1;
	UINT8       NS1:1;
	UINT8       NS2:1;
	UINT8       NS3:1;
#endif
	UINT8       PFIDUser0;
	UINT8       PFIDUser1;
	UINT8       PFIDUser2;
	UINT8       PFIDUser3;
	BOOLEAN     fgIsShortGI;
	BOOLEAN     fgIsUsed;
	BOOLEAN     fgIsDisable;
#ifdef RT_BIG_ENDIAN
	UINT8		initMcsUser1:4;
	UINT8       initMcsUser0:4;
#else
    UINT8       initMcsUser0:4;
	UINT8       initMcsUser1:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT8		initMcsUser3:4;
	UINT8       initMcsUser2:4;
#else
    UINT8       initMcsUser2:4;
	UINT8       initMcsUser3:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT8		dMcsUser1:4;
	UINT8       dMcsUser0:4;
#else
    UINT8       dMcsUser0:4;
	UINT8       dMcsUser1:4;
#endif
#ifdef RT_BIG_ENDIAN
	UINT8		dMcsUser3:4;
	UINT8       dMcsUser2:4;
#else
    UINT8       dMcsUser2:4;
	UINT8       dMcsUser3:4;
#endif
} EVENT_SHOW_GROUP_TBL_ENTRY, *P_EVENT_SHOW_GROUP_TBL_ENTRY;

typedef struct _EVENT_MU_GET_ENABLE {
	UINT32 u4EventId;
	UINT8   fgIsEnable;
} EVENT_MU_GET_ENABLE, *P_EVENT_MU_GET_ENABLE;

typedef struct _EVENT_MU_GET_CLUSTER_TBL_ENTRY {
	UINT32 u4EventId;
	UINT8   index;
	UINT32  gidUserMemberStatus[2];
	UINT32  gidUserPosition[4];
} EVENT_MU_GET_CLUSTER_TBL_ENTRY, *P_EVENT_MU_GET_CLUSTER_TBL_ENTRY;

typedef struct _EVENT_MU_GET_GROUP_USER_THRESHOLD {
	UINT32 u4EventId;
	UINT8 min;
	UINT8 max;
} EVENT_MU_GET_GROUP_USER_THRESHOLD, *P_EVENT_MU_GET_GROUP_USER_THRESHOLD;

typedef struct _EVENT_MU_GET_SU_LOSS_THRESHOLD {
	UINT32 u4EventId;
	UINT32 value;
} EVENT_MU_GET_SU_LOSS_THRESHOLD, *P_EVENT_MU_GET_SU_LOSS_THRESHOLD;

typedef struct _EVENT_MU_GET_GROUP_NSS_THRESHOLD {
	UINT32 u4EventId;
	UINT8 min;
	UINT8 max;
} EVENT_MU_GET_GROUP_NSS_THRESHOLD, *P_EVENT_MU_GET_GROUP_NSS_THRESHOLD;

typedef struct _EVENT_MU_GET_TXOP_DEFAULT {
	UINT_32 u4EventId;
	UINT_8 value;
} EVENT_MU_GET_TXOP_DEFAULT, *P_EVENT_MU_GET_TXOP_DEFAULT;

typedef struct _EVENT_MU_GET_MU_GAIN_THRESHOLD {
	UINT32 u4EventId;
	UINT32 value;
} EVENT_MU_GET_MU_GAIN_THRESHOLD, *P_EVENT_MU_GET_MU_GAIN_THRESHOLD;

typedef struct _EVENT_MU_GET_SECONDARY_AC_POLICY {
	UINT32 u4EventId;
	UINT8 value;
} EVENT_MU_GET_SECONDARY_AC_POLICY, *P_EVENT_MU_GET_SECONDARY_AC_POLICY;

typedef struct _EVENT_MU_GET_GROUP_TBL_DMCS_MASK {
	UINT32 u4EventId;
	UINT8 value;
} EVENT_MU_GET_GROUP_TBL_DMCS_MASK, *P_EVENT_MU_GET_GROUP_TBL_DMCS_MASK;

typedef struct _EVENT_MU_GET_TXREQ_MIN_TIME {
	UINT32 u4EventId;
	UINT16 value;
} EVENT_MU_GET_TXREQ_MIN_TIME, *P_EVENT_MU_GET_TXREQ_MIN_TIME;

typedef struct _EVENT_MU_GET_MAX_GROUP_SEARCH_CNT {
	UINT32 u4EventId;
	UINT32 value;
} EVENT_MU_GET_MAX_GROUP_SEARCH_CNT, *P_EVENT_MU_GET_MAX_GROUP_SEARCH_CNT;


typedef struct _CMD_SET_TRIGGER_GID_MGMT_FRAME {
	UINT16 wlanIndex;
	UINT8 gid;
	UINT8 up;
} CMD_SET_TRIGGER_GID_MGMT_FRAME, *P_CMD_SET_TRIGGER_GID_MGMT_FRAME;

typedef struct _CMD_SET_TRIGGER_DEGROUP {
	UINT8 ucMuProfileIndex;
} CMD_SET_TRIGGER_DEGROUP, *P_CMD_SET_TRIGGER_DEGROUP;

typedef struct _CMD_SET_TRIGGER_GROUP {
	UINT8 ucNum;
} CMD_SET_TRIGGER_GROUP, *P_CMD_SET_TRIGGER_GROUP;

typedef struct _CMD_SET_TRIGGER_BBP {
	UINT16 u2GroupIndex;
} CMD_SET_TRIGGER_BBP, *P_CMD_SET_TRIGGER_BBP;

typedef struct _CMD_SET_TRIGGER_SND {
	UINT16 u2Reserved;
} CMD_SET_TRIGGER_SND, *P_CMD_SET_TRIGGER_SND;

typedef struct _CMD_SET_TRIGGER_MU_TX_FRAME {
	UINT8 ucAcIndex;
	BOOLEAN fgIsRandomPattern;
	UINT32 u4NumOfSTAs;
	UINT32 u4Round;
	UINT32  au4PayloadLength[4];
	UINT8   aucWlanIndexArray[4]; /* TODO: jeffrey, replaced by marco */
} CMD_SET_TRIGGER_MU_TX_FRAME, *P_CMD_SET_TRIGGER_MU_TX_FRAME;

typedef struct _CMD_GET_MU_PFID_TXS_CNT {
	UINT16 u2PfidIndex;
} CMD_GET_MU_PFID_TXS_CNT, *P_CMD_GET_MU_PFID_TXS_CNT;

typedef struct _EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT {
	UINT32 u4EventId;
	UINT16 pfIndex;
	UINT16 cn2used;
	UINT16 cn2rateDown;
	UINT16 cn2deltaMcs;
	UINT16 cn2TxFailCnt;
	UINT16 cn2TxSuccessCnt;
	UINT16 cn3used;
	UINT16 cn3rateDown;
	UINT16 cn3deltaMcs;
	UINT16 cn3TxFailCnt;
	UINT16 cn3TxSuccessCnt;
	UINT16 cn4used;
	UINT16 cn4rateDown;
	UINT16 cn4deltaMcs;
	UINT16 cn4TxFailCnt;
	UINT16 cn4TxSuccessCnt;
} EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT, *P_EVENT_MU_GET_MUPROFILE_TX_STATUS_CNT;

typedef struct _MU_STRUCT_LQ_REPORT {
	int lq_report[NUM_OF_USER][NUM_OF_MODUL];
} MU_STRUCT_LQ_REPORT, *P_MU_STRUCT_LQ_REPORT;

typedef struct _SU_STRUCT_LQ_REPORT {
	int lq_report[NUM_OF_MODUL];
} SU_STRUCT_LQ_REPORT, *P_SU_STRUCT_LQ_REPORT;

typedef struct _MU_STRUCT_MU_TABLE {
	UINT16 type;   /* 0: SU, 1:MU */
	UINT32 length;
	char *prTable; /* point to array of data */
} MU_STRUCT_MU_TABLE, *P_MU_STRUCT_MU_TABLE;

typedef struct _MU_STRUCT_MU_GROUP {
	UINT32 groupIndex;/* Group Table Idx */
	UINT32 numOfUser;
	UINT32 user0Ldpc;
	UINT32 user1Ldpc;
	UINT32 user2Ldpc;
	UINT32 user3Ldpc;
	UINT32 shortGI;
	UINT32 bw;
	UINT32 user0Nss;
	UINT32 user1Nss;
	UINT32 user2Nss;
	UINT32 user3Nss;
	UINT32 groupId;
	UINT32 user0UP;
	UINT32 user1UP;
	UINT32 user2UP;
	UINT32 user3UP;
	UINT32 user0MuPfId;
	UINT32 user1MuPfId;
	UINT32 user2MuPfId;
	UINT32 user3MuPfId;
	UINT32 user0InitMCS;
	UINT32 user1InitMCS;
	UINT32 user2InitMCS;
	UINT32 user3InitMCS;
	uint8_t aucUser0MacAddr[6];
	uint8_t aucUser1MacAddr[6];
	uint8_t aucUser2MacAddr[6];
	uint8_t aucUser3MacAddr[6];
} MU_STRUCT_MU_GROUP, *P_MU_STRUCT_MU_GROUP;


typedef struct _MU_STRUCT_MU_QD {
	UINT_32 qd_report[14];
} MU_STRUCT_MU_QD, *P_MU_STRUCT_MU_QD;


typedef struct _MU_STRUCT_SET_SU_CALC_LQ {
	UINT8  num_of_user;
	UINT8  bandwidth;

	UINT8  nss_of_user0;

	UINT8  pf_mu_id_of_user0;

	UINT8  num_of_txer; /* number of antenna */

	UINT8  spe_index;

	UINT8  reserved[2];

	UINT32 group_index;
} MU_STRUCT_SET_SU_CALC_LQ, *P_MU_STRUCT_SET_SU_CALC_LQ;



typedef struct _MU_STRUCT_SET_CALC_INIT_MCS {
	UINT8 num_of_user;
	UINT8 bandwidth;

	UINT8 nss_of_user0;
	UINT8 nss_of_user1;

	UINT8 nss_of_user2;
	UINT8 nss_of_user3;

	UINT8 pf_mu_id_of_user0;
	UINT8 pf_mu_id_of_user1;

	UINT8 pf_mu_id_of_user2;
	UINT8 pf_mu_id_of_user3;

	UINT8  num_of_txer; /* number of antenna */
	UINT8  spe_index;

	UINT32 group_index;

} MU_STRUCT_SET_CALC_INIT_MCS, *P_MU_STRUCT_SET_CALC_INIT_MCS;



typedef struct _MU_STRUCT_SET_CALC_LQ {
	UINT8 num_of_user;
	UINT8 bandwidth;

	UINT8 nss_of_user0;
	UINT8 nss_of_user1;

	UINT8 nss_of_user2;
	UINT8 nss_of_user3;

	UINT8 pf_mu_id_of_user0;
	UINT8 pf_mu_id_of_user1;

	UINT8 pf_mu_id_of_user2;
	UINT8 pf_mu_id_of_user3;

	UINT8  num_of_txer; /* number of antenna */
	UINT8  spe_index;

	UINT32 group_index;
} MU_STRUCT_SET_CALC_LQ, *P_MU_STRUCT_SET_CALC_LQ;

typedef struct _CMD_HQA_SET_SU_CALC_LQ {
	UINT8  num_of_user;
	UINT8  bandwidth;

	UINT8  nss_of_user0;

	UINT8  pf_mu_id_of_user0;

	UINT8  num_of_txer; /* number of antenna */

	UINT8  spe_index;

	UINT8  reserved[2];

	UINT32 group_index;
} CMD_HQA_SET_SU_CALC_LQ, *P_CMD_HQA_SET_SU_CALC_LQ;





typedef struct _CMD_HQA_SET_MU_CALC_LQ {
	UINT8 num_of_user;
	UINT8 bandwidth;

	UINT8 nss_of_user0;
	UINT8 nss_of_user1;

	UINT8 nss_of_user2;
	UINT8 nss_of_user3;

	UINT8 pf_mu_id_of_user0;
	UINT8 pf_mu_id_of_user1;

	UINT8 pf_mu_id_of_user2;
	UINT8 pf_mu_id_of_user3;

	UINT8  num_of_txer; /* number of antenna */
	UINT8  spe_index;

	UINT32 group_index;
} CMD_HQA_SET_MU_CALC_LQ, *P_CMD_HQA_SET_MU_CALC_LQ;


typedef struct _CMD_HQA_SET_MU_GROUP {
	UINT32 groupIndex;/* Group Table Idx */
	BOOLEAN fgUser0Ldpc;
	BOOLEAN fgUser1Ldpc;
	BOOLEAN fgUser2Ldpc;
	BOOLEAN fgUser3Ldpc;
	UINT8 user0Nss;
	UINT8 user1Nss;
	UINT8 user2Nss;
	UINT8 user3Nss;
	UINT8 user0InitMCS;
	UINT8 user1InitMCS;
	UINT8 user2InitMCS;
	UINT8 user3InitMCS;
	UINT8 user0MuPfId;
	UINT8 user1MuPfId;
	UINT8 user2MuPfId;
	UINT8 user3MuPfId;
	UINT8 numOfUser;
	UINT8 groupId;
	BOOLEAN fgIsShortGI;
	UINT8 bandwidth;
} CMD_HQA_SET_MU_GROUP, *P_CMD_HQA_SET_MU_GROUP;

typedef struct _CMD_HQA_CALC_GET_INIT_MCS_ENTRY {
	UINT32 groupIndex;
} CMD_HQA_CALC_GET_INIT_MCS_ENTRY, *P_CMD_HQA_CALC_GET_INIT_MCS_ENTRY;


typedef struct _CMD_HQA_SET_INIT_MCS {
	UINT8 num_of_user;     /* number of users */
	UINT8 bandwidth;

	UINT8 nss_of_user0;    /* the number of spatil streams which user0 used */
	UINT8 nss_of_user1;    /* the number of spatil streams which user1 used */
	UINT8 nss_of_user2;    /* the number of spatil streams which user2 used */
	UINT8 nss_of_user3;    /* the number of spatil streams which user3 used */

	UINT8 pf_mu_id_of_user0;   /* the PF MU ID of user0 */
	UINT8 pf_mu_id_of_user1;   /* the PF MU ID of user1 */
	UINT8 pf_mu_id_of_user2;   /* the PF MU ID of user2 */
	UINT8 pf_mu_id_of_user3;   /* the PF MU ID of user3 */

	UINT8  num_of_txer; /* number of antenna */
	UINT8  spe_index;

	UINT32 group_index;
} CMD_HQA_SET_INIT_MCS, *P_CMD_HQA_SET_INIT_MCS;


typedef struct _CMD_SHOW_CALC_INIT_MCS_ENTRY {
	UINT8 num_of_user;     /* number of users */
	UINT8 bandwidth;

	UINT8 nss_of_user0;    /* the number of spatil streams which user0 used */
	UINT8 nss_of_user1;    /* the number of spatil streams which user1 used */
	UINT8 nss_of_user2;    /* the number of spatil streams which user2 used */
	UINT8 nss_of_user3;    /* the number of spatil streams which user3 used */

	UINT8 pf_mu_id_of_user0;   /* the PF MU ID of user0 */
	UINT8 pf_mu_id_of_user1;   /* the PF MU ID of user1 */
	UINT8 pf_mu_id_of_user2;   /* the PF MU ID of user2 */
	UINT8 pf_mu_id_of_user3;   /* the PF MU ID of user3 */

	UINT8  num_of_txer; /* number of antenna */
	UINT8  reserved;

	UINT32 group_index;
} CMD_SHOW_CALC_INIT_MCS_ENTRY, *P_CMD_SHOW_CALC_INIT_MCS_ENTRY;



typedef struct _CMD_HQA_SET_MU_NSS_ZERO {
	UINT8 ucValue;
} CMD_HQA_SET_MU_NSS_ZERO, *P_CMD_HQA_SET_MU_NSS_ZERO;

typedef struct _CMD_HQA_SET_MU_METRIC_TABLE {
	UINT8 metric_table[NUM_MUT_NR * NUM_MUT_FEC * NUM_MUT_MCS * NUM_MUT_INDEX];
} CMD_HQA_SET_MU_METRIC_TABLE, *P_CMD_HQA_SET_MU_METRIC_TABLE;

typedef struct _CMD_HQA_SET_SU_METRIC_TABLE {
	UINT8 metric_table[NUM_MUT_FEC * NUM_MUT_MCS * NUM_SUT_INDEX];
} CMD_HQA_SET_SU_METRIC_TABLE, *P_CMD_HQA_SET_SU_METRIC_TABLE;

typedef struct _CMD_HQA_SET_MU_LQ_SPEED_UP {
	UINT8 ucValue;
} CMD_HQA_SET_MU_LQ_SPEED_UP, *P_CMD_HQA_SET_MU_LQ_SPEED_UP;

typedef struct _CMD_HQA_SET_MU_ENABLE {
	UINT8 fgIsEnable;
} CMD_HQA_SET_MU_ENABLE, *P_CMD_HQA_SET_MU_ENABLE;

typedef struct _CMD_HQA_GET_QD {
	INT8  scIdx;
} CMD_HQA_GET_QD, *P_CMD_HQA_GET_QD;

typedef struct _CMD_HQA_SET_MU_SNR_OFFSET {
	INT8 offset;
} CMD_HQA_SET_MU_SNR_OFFSET, *P_CMD_HQA_SET_MU_SNR_OFFSET;






typedef struct _EVENT_HQA_GET_MU_CALC_LQ {
	UINT32 u4EventId;
	/* P_MU_STRUCT_LQ_REPORT pOutput; */
	MU_STRUCT_LQ_REPORT rEntry;
} EVENT_HQA_GET_MU_CALC_LQ, *P_EVENT_HQA_GET_MU_CALC_LQ;

typedef struct _EVENT_HQA_GET_SU_CALC_LQ {
	UINT32 u4EventId;
	/* P_SU_STRUCT_LQ_REPORT pOutput; */
	SU_STRUCT_LQ_REPORT rEntry;
} EVENT_HQA_GET_SU_CALC_LQ, *P_EVENT_HQA_GET_SU_CALC_LQ;


typedef struct _EVENT_HQA_MU_QD {
	UINT32 u4EventId;
	/* P_MU_STRUCT_MU_QD pOutput; */
	MU_STRUCT_MU_QD rEntry;
} EVENT_HQA_MU_QD, *P_EVENT_HQA_MU_QD;

/*
typedef struct _EVENT_HQA_GET_QD {
    UINT32 u4EventId;
    INT_8 sCIdx;
    UINT_32 au4RawData[14];
} EVENT_HQA_GET_QD, *P_EVENT_HQA_GET_QD;
*/

typedef struct _EVENT_HQA_STATUS_CALCLQ {
	UINT32 u4EventId;
	UINT16 status;
} EVENT_HQA_STATUS_CALCLQ, *P_EVENT_HQA_STATUS_CALCLQ;


/*
typedef struct _MU_STRUCT_MU_GROUP_INIT_MCS {
    UINT32 u4EventId;
    P_MU_STRUCT_MU_GROUP_INIT_MCS pOutput;
    MU_STRUCT_MU_GROUP_INIT_MCS rEntry;
} MU_STRUCT_MU_GROUP_INIT_MCS, *P_MU_STRUCT_MU_GROUP_INIT_MCS;
*/
typedef struct _MU_STRUCT_MU_GROUP_INIT_MCS {
	UINT32 user0InitMCS;
	UINT32 user1InitMCS;
	UINT32 user2InitMCS;
	UINT32 user3InitMCS;
} MU_STRUCT_MU_GROUP_INIT_MCS, *P_MU_STRUCT_MU_GROUP_INIT_MCS;

typedef struct _EVENT_HQA_INIT_MCS {
	UINT32 u4EventId;
	/* P_MU_STRUCT_MU_GROUP_INIT_MCS pOutput; */
	MU_STRUCT_MU_GROUP_INIT_MCS rEntry;
} EVENT_HQA_INIT_MCS, *P_EVENT_HQA_INIT_MCS;

typedef struct _MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM {
	UINT8  fgIsRandomPattern;   /* is random pattern or not */
	UINT32 msduPayloadLength0;  /* payload length of the MSDU for user 0 */
	UINT32 msduPayloadLength1;  /* payload length of the MSDU for user 1 */
	UINT32 msduPayloadLength2;  /* payload length of the MSDU for user 2 */
	UINT32 msduPayloadLength3;  /* payload length of the MSDU for user 3 */
	UINT32 u4MuPacketCount;     /* MU TX count */
	UINT32 u4NumOfSTAs;         /* number of user in the MU TX */
	UINT8 macAddrs[4][6];       /* MAC address of users */
} MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM, *P_MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM;


/* these struct  here for temply, will be removed in feature */
typedef struct _MU_STRUCT_MU_GET_STA_PARAM {
	UINT32 gid[2];
	UINT32 up[4];
} MU_STRUCT_MU_GET_STA_PARAM, *P_MU_STRUCT_MU_GET_STA_PARAM;

typedef struct _MU_STRUCT_MU_STA_PARAM {
	UINT32 gid[2];
	UINT32 up[4];
} MU_STRUCT_MU_STA_PARAM, *P_MU_STRUCT_MU_STA_PARAM;


/* Prototype for HQA */
bool
hqa_wifi_test_mu_cal_init_mcs(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_CALC_INIT_MCS pParams
);

bool
hqa_wifi_test_mu_get_init_mcs(
	PRTMP_ADAPTER pAd,
	UINT32 groupIndex,
	P_MU_STRUCT_MU_GROUP_INIT_MCS poutput

);

bool
hqa_wifi_test_mu_cal_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_CALC_LQ pParams
);

bool
hqa_wifi_test_su_cal_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_SET_SU_CALC_LQ pParams
);

bool
hqa_wifi_test_mu_get_lq(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_LQ_REPORT pOutput
);

bool
hqa_wifi_test_su_get_lq(
	PRTMP_ADAPTER pAd,
	P_SU_STRUCT_LQ_REPORT pOutput
);

bool
hqa_wifi_test_snr_offset_set(
	PRTMP_ADAPTER pAd,
	char val
);

bool
hqa_wifi_test_mu_set_zero_nss(
	PRTMP_ADAPTER pAd,
	unsigned char val
);

bool
hqa_wifi_test_mu_speed_up_lq(
	PRTMP_ADAPTER pAd,
	int val
);

bool
hqa_wifi_test_mu_table_set(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_TABLE ptr
);

bool
hqa_wifi_test_mu_group_set(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_GROUP ptr
);

bool
hqa_wifi_test_mu_get_qd(
	PRTMP_ADAPTER pAd,
	INT8 subcarrierIndex,
	P_MU_STRUCT_MU_QD pOutput
);

bool
hqa_wifi_test_mu_set_enable(
	PRTMP_ADAPTER pAd,
	unsigned char val
);

/* new API for Mu TX */
bool
hqa_wifi_test_mu_trigger_mu_tx(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM pParam
);


/* STA API in here temply and will remove in feature */
bool
hqa_wifi_test_mu_get_sta_gid_and_up(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_GET_STA_PARAM pOutput
);

bool
hqa_wifi_test_mu_set_sta_gid_and_up(
	PRTMP_ADAPTER pAd,
	P_MU_STRUCT_MU_STA_PARAM ptr
);

INT SetMuEnableProc(
	RTMP_ADAPTER * pAd,
	RTMP_STRING * arg
);

#endif  /* __AP_MUMIMO_H__ */
