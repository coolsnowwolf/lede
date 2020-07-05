/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	sae.h

	Abstract:

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
	Name		Date			Modification logs
*/

#ifndef	__SAE_H__
#define	__SAE_H__
#ifdef DOT11_SAE_SUPPORT

INT show_sae_info_proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

VOID sae_cfg_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg);

VOID sae_cfg_deinit(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg);


SAE_INSTANCE *search_sae_instance(
	IN SAE_CFG * pSaeCfg,
	IN UCHAR * own_mac,
	IN UCHAR * peer_mac);

SAE_INSTANCE *create_sae_instance(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg,
	IN UCHAR * own_mac,
	IN UCHAR * peer_mac,
	IN UCHAR * bssid,
	IN UCHAR * psk);


VOID delete_sae_instance(
	IN SAE_INSTANCE *pSaeIns);

UCHAR set_sae_instance_removable(
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac);

VOID sae_ins_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg,
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR * own_mac,
	IN UCHAR * peer_mac,
	IN UCHAR * bssid,
	IN UCHAR * psk);

/* partial */
VOID sae_clear_data(
	IN SAE_INSTANCE *pSaeIns);

VOID sae_dump_time(
	IN SAE_TIME_INTERVAL * time_cost
);

UCHAR sae_using_anti_clogging(
	IN SAE_CFG * pSaeCfg);


VOID sae_set_retransmit_timer(
	IN SAE_INSTANCE *pSaeIns);


VOID sae_clear_retransmit_timer(
	IN SAE_INSTANCE *pSaeIns);


DECLARE_TIMER_FUNCTION(sae_auth_retransmit);


VOID sae_auth_retransmit(
	IN VOID *SystemSpecific1,
	IN VOID *FunctionContext,
	IN VOID *SystemSpecific2,
	IN VOID *SystemSpecific3);


UCHAR sae_auth_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN UCHAR *psk,
	IN INT32 group);



UCHAR sae_handle_auth(
	IN RTMP_ADAPTER *pAd,
	IN SAE_CFG *pSaeCfg,
	IN VOID *msg,
	IN UINT32 msg_len,
	IN UCHAR *psk,
	IN USHORT auth_seq,
	IN USHORT auth_status,
	OUT UCHAR** pmk);


USHORT sae_sm_step(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE *pSaeIns,
	IN USHORT auth_seq);


UCHAR sae_check_big_sync(
	IN SAE_INSTANCE *pSaeIns);

UCHAR sae_get_pmk_cache(
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	OUT UCHAR *pmkid,
	OUT UCHAR *pmk);

UCHAR sae_build_token_req(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE * pSaeIns,
	OUT UCHAR *token_req,
	OUT UINT32 * token_req_len);

UCHAR sae_check_token(
	IN SAE_INSTANCE * pSaeIns,
	IN UCHAR *peer_token,
	IN UINT32 peer_token_len);


USHORT sae_parse_commit(
	IN SAE_CFG * pSaeCfg,
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *msg,
	IN UINT32 msg_len,
	IN UCHAR **token,
	IN UINT32 * token_len,
	IN UCHAR is_token_req);


VOID sae_parse_commit_token(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end,
	IN UCHAR **token,
	IN UINT32 * token_len);


USHORT sae_parse_commit_scalar(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end);


USHORT sae_parse_commit_element(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end);


USHORT sae_prepare_commit(
	IN SAE_INSTANCE *pSaeIns);


USHORT sae_derive_commit(
	IN SAE_INSTANCE *pSaeIns);


USHORT sae_process_commit(
	IN SAE_INSTANCE *pSaeIns);


UCHAR sae_derive_key(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *k);


VOID sae_send_auth(
	IN RTMP_ADAPTER * pAd,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN USHORT alg,
	IN USHORT seq,
	IN USHORT status_code,
	IN UCHAR *buf,
	IN UINT32 buf_len);


UCHAR sae_send_auth_commit(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE *pSaeIns);


UCHAR sae_send_auth_confirm(
	IN RTMP_ADAPTER * pAd,
	IN SAE_INSTANCE *pSaeIns);


USHORT sae_parse_confirm(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *msg,
	IN UINT32 msg_len);


USHORT sae_check_confirm(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *peer_confirm);


SAE_BN *sae_gen_rand(
	IN SAE_INSTANCE *pSaeIns);


USHORT sae_group_allowed(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *allowed_groups,
	IN INT32 group);


UCHAR is_sae_group_ecc(
	IN INT32 group);


UCHAR is_sae_group_ffc(
	IN INT32 group);


#ifdef group_related
/*
 =====================================
	group related
 =====================================
*/
#endif
VOID sae_group_init_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN INT32 group);

VOID sae_group_init_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN INT32 group);


VOID sae_group_deinit_ecc(
	IN SAE_INSTANCE *pSaeIns);


VOID sae_group_deinit_ffc(
	IN SAE_INSTANCE *pSaeIns);


VOID sae_cn_confirm_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR is_send, /* otherwise, is verfication */
	OUT UCHAR *confirm);


VOID sae_cn_confirm_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR is_send, /* otherwise, is verfication */
	OUT UCHAR *confirm);


VOID sae_cn_confirm_cmm(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *scalar1,
	IN SAE_BN *scalar2,
	IN UCHAR *element_bin1,
	IN UCHAR *element_bin2,
	IN UINT32 element_len,
	IN USHORT send_confirm,
	OUT UCHAR *confirm);


USHORT sae_parse_commit_element_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end);


USHORT sae_parse_commit_element_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end);


UCHAR sae_derive_commit_element_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *mask);


UCHAR sae_derive_commit_element_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *mask);

USHORT sae_derive_pwe_ecc(
	IN SAE_INSTANCE *pSaeIns);


USHORT sae_derive_pwe_ffc(
	IN SAE_INSTANCE *pSaeIns);



UCHAR sae_derive_k_ecc(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *k);


UCHAR sae_derive_k_ffc(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *k);

USHORT sae_reflection_check_ecc(
	IN SAE_INSTANCE *pSaeIns);

USHORT sae_reflection_check_ffc(
	IN SAE_INSTANCE *pSaeIns);

#endif /* DOT11_SAE_SUPPORT */
#endif /* __SAE_H__ */

