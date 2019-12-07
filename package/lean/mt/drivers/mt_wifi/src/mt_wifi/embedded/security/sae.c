#ifdef DOT11_SAE_SUPPORT

#include "rt_config.h"
#include "security/ecc.h"

BUILD_TIMER_FUNCTION(sae_auth_retransmit);

const SAE_GROUP_OP ecc_group_op = {
	.sae_group_init = sae_group_init_ecc,
	.sae_group_deinit = sae_group_deinit_ecc,
	.sae_cn_confirm = sae_cn_confirm_ecc,
	.sae_parse_commit_element = sae_parse_commit_element_ecc,
	.sae_derive_commit_element = sae_derive_commit_element_ecc,
	.sae_derive_pwe = sae_derive_pwe_ecc,
	.sae_derive_k = sae_derive_k_ecc
};

const SAE_GROUP_OP ffc_group_op = {
	.sae_group_init = sae_group_init_ffc,
	.sae_group_deinit = sae_group_deinit_ffc,
	.sae_cn_confirm = sae_cn_confirm_ffc,
	.sae_parse_commit_element = sae_parse_commit_element_ffc,
	.sae_derive_commit_element = sae_derive_commit_element_ffc,
	.sae_derive_pwe = sae_derive_pwe_ffc,
	.sae_derive_k = sae_derive_k_ffc
};

static DH_GROUP_INFO dh_groups[] = {
	DH_GROUP(5, 1),
	DH_GROUP(1, 1),
	DH_GROUP(2, 1),
	DH_GROUP(14, 1),
	DH_GROUP(15, 1),
	DH_GROUP(16, 1),
	DH_GROUP(17, 1),
	DH_GROUP(18, 1),
	DH_GROUP(22, 0),
	DH_GROUP(23, 0),
	DH_GROUP(24, 0)
};



static DH_GROUP_INFO_BI dh_groups_bi[] = {
	DH_GROUP_BI(5, 1),
	DH_GROUP_BI(1, 1),
	DH_GROUP_BI(2, 1),
	DH_GROUP_BI(14, 1),
	DH_GROUP_BI(15, 1),
	DH_GROUP_BI(16, 1),
	DH_GROUP_BI(17, 1),
	DH_GROUP_BI(18, 1),
	DH_GROUP_BI(22, 0),
	DH_GROUP_BI(23, 0),
	DH_GROUP_BI(24, 0)
};

#ifdef BI_POOL_DBG
UINT32 sae_expected_cnt[20]; /* 0~12  is used */
#endif

int SAE_DEBUG_LEVEL = DBG_LVL_LOUD;
int SAE_DEBUG_LEVEL2 = DBG_LVL_TRACE;
int SAE_COST_TIME_DBG_LVL = DBG_LVL_INFO;


VOID sae_cfg_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg)
{
	UINT32 i = 0;

#ifdef BI_POOL
	big_integer_pool_init();
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	/* 11.3.8.5.1 When the parent SAE process starts up, Open is set to zero (0) */
	pSaeCfg->pAd = pAd;
	pSaeCfg->open = 0;
	pSaeCfg->dot11RSNASAERetransPeriod = 2;
	pSaeCfg->total_ins = 0;
	pSaeCfg->sae_anti_clogging_threshold = 10000;

	for (i = 0; i < MAX_SIZE_OF_ALLOWED_GROUP - 1; i++)
		pSaeCfg->support_group[i] = i + 1;

	pSaeCfg->support_group[MAX_SIZE_OF_ALLOWED_GROUP - 1] = 0;
}

VOID sae_cfg_deinit(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg)
{
#ifdef BI_POOL
	big_integer_pool_deinit();
#endif
}



SAE_INSTANCE *search_sae_instance(
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac)
{
	UINT32 i;
	SAE_INSTANCE *pSaeIns = NULL;
	UINT32 ins_cnt = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (!pSaeCfg || !own_mac || !peer_mac) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): search fail with null input\n", __func__));
		return NULL;
	}

	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		if (RTMPEqualMemory(pSaeCfg->sae_ins[i].own_mac, own_mac, MAC_ADDR_LEN) /* ellis */
			&& RTMPEqualMemory(pSaeCfg->sae_ins[i].peer_mac, peer_mac, MAC_ADDR_LEN)) {
			pSaeIns = &pSaeCfg->sae_ins[i];
			break;
		}

		ins_cnt++;

		if (ins_cnt == pSaeCfg->total_ins)
			break;
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);

	if (pSaeIns
		&& pSaeIns->state == SAE_ACCEPTED
		&& pSaeIns->same_mac_ins)
		pSaeIns = pSaeIns->same_mac_ins;

	return pSaeIns;
}

SAE_INSTANCE *create_sae_instance(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG * pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN UCHAR *psk)
{
	UINT32 i;
	SAE_INSTANCE *pSaeIns = NULL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	if (pSaeCfg->total_ins == MAX_LEN_OF_MAC_TABLE) {
		NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
		return NULL;
	}

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE) {
			pSaeIns = &pSaeCfg->sae_ins[i];
			sae_ins_init(pAd, pSaeCfg, pSaeIns,
						 own_mac, peer_mac, bssid, psk);
			pSaeIns->valid = TRUE;
			pSaeCfg->total_ins++;
			break;
		}
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
	return pSaeIns;
}


VOID delete_sae_instance(
	IN SAE_INSTANCE *pSaeIns)
{
	SAE_CFG *pSaeCfg = NULL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (pSaeIns == NULL)
		return;

	pSaeCfg = pSaeIns->pParentSaeCfg;
	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);
	pSaeIns->valid = FALSE;
	pSaeIns->group_op->sae_group_deinit(pSaeIns);

	if (pSaeIns->anti_clogging_token) {
		os_free_mem(pSaeIns->anti_clogging_token);
		pSaeIns->anti_clogging_token = NULL;
	}

	SAE_BN_FREE(&pSaeIns->sae_rand);

	if (pSaeIns->same_mac_ins) {
		pSaeIns->same_mac_ins->same_mac_ins = NULL;
		pSaeIns->same_mac_ins = NULL;
	}

	SAE_BN_FREE(&pSaeIns->peer_commit_scalar);
	NdisZeroMemory(pSaeIns, sizeof(SAE_INSTANCE));
	pSaeCfg->total_ins--;
	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
	return;
}

VOID sae_ins_init(
	IN RTMP_ADAPTER * pAd,
	IN SAE_CFG *pSaeCfg,
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN UCHAR *psk)
{
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	NdisZeroMemory(pSaeIns, sizeof(SAE_INSTANCE));
	COPY_MAC_ADDR(pSaeIns->own_mac,  own_mac);
	COPY_MAC_ADDR(pSaeIns->peer_mac,  peer_mac);
	COPY_MAC_ADDR(pSaeIns->bssid,  bssid);
	RTMPInitTimer(pAd, &pSaeIns->sae_retry_timer, GET_TIMER_FUNCTION(sae_auth_retransmit), pSaeIns,  FALSE);
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO, ("%s: timer valid = %d\n", __func__, pSaeIns->sae_retry_timer.Valid));
	pSaeIns->pParentSaeCfg = pSaeCfg;
	pSaeIns->psk = psk;
	SET_NOTHING_STATE(pSaeIns);
	pSaeIns->sync = 0;
	/* 11.3.8.5.2
	  * The number of Confirm messages that have been sent.
	  * This is the send-confirm counter used in the construction of Confirm messages
	  */
	pSaeIns->send_confirm = 0; /* ellis */
	pSaeIns->last_peer_sc = 0;
	pSaeIns->support_group_idx = 0;
	pSaeIns->same_mac_ins = NULL;
}

/* partial */
VOID sae_clear_data(
	IN SAE_INSTANCE *pSaeIns)
{
	BOOLEAN Cancelled;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (pSaeIns->group_op)
		pSaeIns->group_op->sae_group_deinit(pSaeIns);

	if (pSaeIns->anti_clogging_token) {
		os_free_mem(pSaeIns->anti_clogging_token);
		pSaeIns->anti_clogging_token = NULL;
	}

	SAE_BN_FREE(&pSaeIns->sae_rand);
	SAE_BN_FREE(&pSaeIns->peer_commit_scalar);
	SET_NOTHING_STATE(pSaeIns);
	NdisZeroMemory(pSaeIns, offsetof(SAE_INSTANCE, valid));
	RTMPCancelTimer(&pSaeIns->sae_retry_timer, &Cancelled);
	/* RTMPReleaseTimer(&pSaeIns->sae_retry_timer, &Cancelled); */
}

static VOID sae_record_time_begin(
	INOUT ULONG *time_interval)
{
	NdisGetSystemUpTime(time_interval);
}

static VOID sae_record_time_end(
	IN UCHAR *str,
	INOUT ULONG *time_interval)
{
	ULONG temp;
	NdisGetSystemUpTime(&temp);
	*time_interval = temp - *time_interval;
}


VOID sae_dump_time(
	IN SAE_TIME_INTERVAL * time_cost
)
{
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("cost time:"));
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("\npwe: %lu jiffies", time_cost->derive_pwe_time));
#ifdef LINUX
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, (", %u msec", jiffies_to_msecs(time_cost->derive_pwe_time)));
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("\nparse scalar: %lu jiffies", time_cost->parse_commit_scalar_time));
#ifdef LINUX
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, (", %u msec", jiffies_to_msecs(time_cost->parse_commit_scalar_time)));
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("\nparse element: %lu jiffies", time_cost->parse_commit_element_time));
#ifdef LINUX
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, (", %u msec", jiffies_to_msecs(time_cost->parse_commit_element_time)));
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("\nderive scalar: %lu jiffies", time_cost->derive_commit_scalar_time));
#ifdef LINUX
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, (", %u msec", jiffies_to_msecs(time_cost->derive_commit_scalar_time)));
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("\nderive element: %lu jiffies", time_cost->derive_commit_element_time));
#ifdef LINUX
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, (", %u msec", jiffies_to_msecs(time_cost->derive_commit_element_time)));
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("\nk: %lu jiffies", time_cost->derive_k_time));
#ifdef LINUX
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, (", %u msec", jiffies_to_msecs(time_cost->derive_k_time)));
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("\npmk: %lu jiffies", time_cost->derive_pmk_time));
#ifdef LINUX
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, (", %u msec", jiffies_to_msecs(time_cost->derive_pmk_time)));
#endif
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, SAE_COST_TIME_DBG_LVL, ("\n"));

	time_cost->derive_pwe_time = 0;
	time_cost->parse_commit_scalar_time = 0;
	time_cost->parse_commit_element_time = 0;
	time_cost->derive_commit_scalar_time = 0;
	time_cost->derive_commit_element_time = 0;
	time_cost->derive_k_time = 0;
	time_cost->derive_pmk_time = 0;

#ifdef BI_POOL
	sae_dump_pool_info_check(0, FALSE, TRUE);
#endif
}


UCHAR sae_using_anti_clogging(
	IN SAE_CFG *pSaeCfg)
{
	UINT32 i;
	UINT32 ins_cnt = 0;
	UINT32 open = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (pSaeCfg->total_ins < pSaeCfg->sae_anti_clogging_threshold)
		return FALSE;

	NdisAcquireSpinLock(&pSaeCfg->sae_cfg_lock);

	for (i = 0; i < MAX_LEN_OF_MAC_TABLE; i++) {
		if (pSaeCfg->sae_ins[i].valid == FALSE)
			continue;

		ins_cnt++;

		if ((pSaeCfg->sae_ins[i].state == SAE_COMMITTED)
			|| (pSaeCfg->sae_ins[i].state == SAE_CONFIRMED))
			open++;

		if (open >= pSaeCfg->sae_anti_clogging_threshold) {
			NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
			return TRUE;
		}

		if (ins_cnt == pSaeCfg->total_ins)
			break;
	}

	NdisReleaseSpinLock(&pSaeCfg->sae_cfg_lock);
	return FALSE;
}


VOID sae_set_retransmit_timer(
	IN SAE_INSTANCE *pSaeIns)
{
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	RTMPSetTimer(&pSaeIns->sae_retry_timer, pSaeIns->pParentSaeCfg->dot11RSNASAERetransPeriod * 1000);
}


VOID sae_clear_retransmit_timer(
	IN SAE_INSTANCE *pSaeIns)
{
	BOOLEAN Cancelled;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	RTMPCancelTimer(&pSaeIns->sae_retry_timer, &Cancelled);
}

VOID sae_auth_retransmit(
	IN VOID *SystemSpecific1,
	IN VOID *FunctionContext,
	IN VOID *SystemSpecific2,
	IN VOID *SystemSpecific3)
{
	SAE_INSTANCE *pSaeIns = (SAE_INSTANCE *) FunctionContext;
	RALINK_TIMER_STRUCT *pTimer = (RALINK_TIMER_STRUCT *) SystemSpecific3;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)pTimer->pAd;
	UCHAR ret;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (sae_check_big_sync(pSaeIns)) /* ellis */
		return;

	pSaeIns->sync++;

	switch (pSaeIns->state) {
	case SAE_COMMITTED:
		ret = sae_send_auth_commit(pAd, pSaeIns);
		sae_set_retransmit_timer(pSaeIns);
		break;

	case SAE_CONFIRMED:
		/* If Sync is not greater than dot11RSNASAESync, the Sync counter shall be incremented,
		  * Sc shall be incremented, and the protocol instance shall create a new Confirm (with the new Sc value) Message,
		  * transmit it to the peer, and set the t0 (retransmission) timer
		  */
		ret = sae_send_auth_confirm(pAd, pSaeIns);
		sae_set_retransmit_timer(pSaeIns);
		break;

	default:
		ret = FALSE;
		break;
	}

	if (ret == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): retransemit fail (state = %d, sync = %d)\n",
				  __func__, pSaeIns->state, pSaeIns->sync));
	}
}


UCHAR sae_auth_init(
	IN RTMP_ADAPTER *pAd,
	IN SAE_CFG *pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN UCHAR *psk,
	IN INT32 group)
{
	SAE_INSTANCE *pSaeIns = search_sae_instance(pSaeCfg, own_mac, peer_mac);
	SAE_INSTANCE *pPreSaeIns = pSaeIns;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("==>%s(): pSaeIns = %p, pSaeIns->state = %d\n", __func__, pSaeIns, (pSaeIns) ? pSaeIns->state : -1));

	/* 11.3.8.6.1 Upon receipt of an Initiate event, the parent process shall check whether there exists a protocol instance for
	  * the peer MAC address (from the Init event) in either Committed or Confirmed state. If there is, the Initiate
	  * event shall be ignored. Otherwise, a protocol instance shall be created, and an Init event shall be sent to the
	  * protocol instance.
	  */
	if (pSaeIns &&
		((pSaeIns->state == SAE_COMMITTED)
		 || (pSaeIns->state == SAE_CONFIRMED)))
		return FALSE;

	pSaeIns = create_sae_instance(pAd, pSaeCfg, own_mac, peer_mac, bssid, psk);

	if (!pSaeIns)
		return FALSE;

	pSaeIns->same_mac_ins = pPreSaeIns;

	if (pPreSaeIns)
		pPreSaeIns->same_mac_ins = pSaeIns;

	if (sae_group_allowed(pSaeIns, pSaeCfg->support_group, group) != MLME_SUCCESS)
		goto FAIL;

	if (sae_prepare_commit(pSaeIns) != MLME_SUCCESS)
		goto FAIL;

	if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
		goto FAIL;

	SET_COMMITTED_STATE(pSaeIns);
	sae_set_retransmit_timer(pSaeIns);
	return TRUE;
FAIL:
	delete_sae_instance(pSaeIns);
	return FALSE;
}


UCHAR *sae_handle_auth(
	IN RTMP_ADAPTER *pAd,
	IN SAE_CFG *pSaeCfg,
	IN VOID *msg,
	IN UINT32 msg_len,
	IN UCHAR *psk,
	IN USHORT auth_seq,
	IN USHORT auth_status)
{
	USHORT res = MLME_SUCCESS;
	FRAME_802_11 *Fr = (PFRAME_802_11)msg;
	SAE_INSTANCE *pSaeIns = search_sae_instance(pSaeCfg, Fr->Hdr.Addr1, Fr->Hdr.Addr2);
	UINT8 is_token_req = FALSE;
	UCHAR *token = NULL;
	UINT32 token_len = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("==>%s(): receive seq #%d with status code %d, instance %p, own mac addr = %02x:%02x:%02x:%02x:%02x:%02x, peer mac addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, auth_seq, auth_status, pSaeIns, PRINT_MAC(Fr->Hdr.Addr1), PRINT_MAC(Fr->Hdr.Addr2)));

	/* Upon receipt of a Com event, the t0 (retransmission) timer shall be cancelled in Committed/Confirmed state */
	/* Upon receipt of a Con event, the t0 (retransmission) timer shall be cancelled in Committed/Confirmed state */
	if (pSaeIns) {
		sae_clear_retransmit_timer(pSaeIns); /* ellis */
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
				 ("%s(): state = %d\n",
				  __func__, pSaeIns->state));
	}

	if (pSaeIns
		&& pSaeIns->state != SAE_COMMITTED
		&& auth_status != MLME_SUCCESS) {
		delete_sae_instance(pSaeIns);
		return NULL;
	}

	switch (auth_seq) {
	case SAE_COMMIT_SEQ:
		if (auth_status != MLME_SUCCESS) {
			if (!pSaeIns)
				return NULL;

			/* 11.3.8.6.3 Upon receipt of a Com event, the protocol instance shall check the Status of the Authentication frame. If the
			  * Status code is nonzero, the frame shall be silently discarded and a Del event shall be sent to the parent
			  * process.
			  */
			/* comments: it's not expected to receive the rejection commit msg in NOTHING/ACCEPTED state */
			if (pSaeIns->state == SAE_NOTHING
				&& pSaeIns->state == SAE_ACCEPTED) {
				delete_sae_instance(pSaeIns);
				return NULL;
			} else if (pSaeIns->state == SAE_CONFIRMED) {
				/* 11.3.8.6.5 Upon receipt of a Com event, if the Status is nonzero, the frame shall be silently discarded, the t0 (retransmission) timer set */
				sae_set_retransmit_timer(pSaeIns);
				return NULL;
			}

			if (auth_status == MLME_ANTI_CLOGGING_TOKEN_REQ) {
				;
			} else if (auth_status == MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED) {
				USHORT sae_group;
				USHORT new_sae_group;
				UCHAR *pos = &Fr->Octet[6];
				/* 11.3.8.6.4 If the Status code is 77, the protocol instance shall check the finite cyclic group field being rejected.*/
				/* Check Finite Cyclic Group */
				NdisMoveMemory(&sae_group, pos, 2); /* ellis bigendian */
				sae_group = cpu2le16(sae_group);

				/* If the rejected group does not match the last offered group the protocol instance shall silently discard the message and set the t0 (retransmission) timer */
				if (sae_group != pSaeIns->group) {
					sae_set_retransmit_timer(pSaeIns);
					return NULL;
				} else {
					/* If the rejected group matches the last offered group,
					  * the protocol instance shall choose a different group and generate the PWE and the secret
					  * values according to 11.3.5.2; it then generates and transmits a new Commit Message to the peer,
					  * zeros Sync, sets the t0 (retransmission) timer, and remains in Committed state.
					  */
					new_sae_group = pSaeCfg->support_group[++pSaeIns->support_group_idx];

					/*If there are no other groups to choose,
					the protocol instance shall send a Del event to the parent process and transitions back to Nothing state. */
					if ((new_sae_group != 0)
						&& (sae_group_allowed(pSaeIns, pSaeCfg->support_group, sae_group) != MLME_SUCCESS)
						&& (sae_prepare_commit(pSaeIns) != MLME_SUCCESS)) {
						delete_sae_instance(pSaeIns);
						return NULL;
					}

					sae_send_auth_commit(pAd, pSaeIns);
					return NULL;
				}
			} else {
				/* 11.3.8.6.4 If the Status is some other nonzero value, the frame shall be silently discarded and the t0 (retransmission) timer shall be set. */
				sae_set_retransmit_timer(pSaeIns);
				return NULL;
			}
		}

		if (!pSaeIns
			|| pSaeIns->state == SAE_ACCEPTED) {
			/* 11.3.8.6, the parent process checks the value of Open first.
			  * If Open is not greater than dot11RSNASAEAntiCloggingThreshold or Anti-Clogging Token exists and is correct,
			  * the parent process shall create a protocol instance.
			  * comment: But, parsing anti-clogging token needs group info, so always create instance first
			  */
			SAE_INSTANCE *pPreSaeIns = pSaeIns;
			pSaeIns = create_sae_instance(pAd, pSaeCfg, Fr->Hdr.Addr1, Fr->Hdr.Addr2, Fr->Hdr.Addr3, psk);

			if (!pSaeIns)
				res = MLME_UNSPECIFY_FAIL;

			pSaeIns->same_mac_ins = pPreSaeIns;

			if (pPreSaeIns)
				pPreSaeIns->same_mac_ins = pSaeIns;
		}

		if (sae_using_anti_clogging(pSaeCfg))
			;

		res = sae_parse_commit(pSaeCfg, pSaeIns, msg, msg_len, &token, &token_len, is_token_req);

		if (res == MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED) {
			/* 11.3.8.6.4  a Commit Message with Status code equal to 77 indicating rejection,
			  * and the Algorithm identifier set to the rejected algorithm, shall be sent to the peer
			  */
		} else if (res != MLME_SUCCESS)
			break;

		if (is_token_req) {
			/* 11.3.8.6.4 The protocol instance shall check the Status code of the Authentication frame.
			  * If the Status code is 76, a new Commit Message shall be constructed with the Anti-Clogging Token from the received
			  * Authentication frame, and the commit-scalar and COMMIT-ELEMENT previously sent
			  */
			/* The new Commit Message shall be transmitted to the peer,
			  *Sync shall be zeroed, and the t0 (retransmission) timer shall be set
			  */
			pSaeIns->sync = 0;
		}
		res = sae_sm_step(pAd, pSaeIns, auth_seq);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				 ("%s(): SAE_COMMIT_SEQ, res(sae_sm_step) = %d\n",
				  __func__, res));
		break;

	case SAE_CONFIRM_SEQ:
		if (!pSaeIns)
			return NULL;

		/* 11.3.8.6.5 Rejection frames received in Confirmed state shall be silently discarded */
		/* Comment: In Commited state, it's not expected to receive the Rejection confirm msg.*/
		if (auth_status != MLME_SUCCESS) {
			delete_sae_instance(pSaeIns);
			return NULL;
		}

		res = sae_parse_confirm(pSaeIns, msg, msg_len);

		if (res != MLME_SUCCESS) {
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
					 ("%s(): verify confirm fail\n",	__func__));
			break;
		}

		res = sae_sm_step(pAd, pSaeIns, auth_seq);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
				 ("%s(): SAE_CONFIRM_SEQ, res(sae_sm_step) = %d\n",
				  __func__, res));
		break;

	default:
		if (pSaeIns) {
			delete_sae_instance(pSaeIns);
			pSaeIns = NULL;
		}

		res = MLME_SEQ_NR_OUT_OF_SEQUENCE;
		break;
	}

	if (res != MLME_SUCCESS
		&& res != SAE_SILENTLY_DISCARDED)
		sae_send_auth(pAd, Fr->Hdr.Addr1, Fr->Hdr.Addr2, Fr->Hdr.Addr3, AUTH_MODE_SAE, auth_seq, res, "", 0);

	if (pSaeIns && pSaeIns->state == SAE_ACCEPTED) {
		sae_dump_time(&pSaeIns->sae_cost_time);
		SAE_LOG_TIME_DUMP();
		ecc_point_dump_time();
		return pSaeIns->pmk;
	} else
		return NULL;
}


USHORT sae_sm_step(
	IN RTMP_ADAPTER *pAd,
	IN SAE_INSTANCE *pSaeIns,
	IN USHORT auth_seq)
{
#define F(a, b) (a << 2 | b)
	USHORT res = MLME_SUCCESS;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	switch (F(pSaeIns->state, auth_seq)) {
	case F(SAE_NOTHING, SAE_COMMIT_SEQ):
		/* 11.3.8.6.3 If validation of the received
		  * Commit Message fails, the protocol instance shall send a Del event to the parent process; otherwise, it shall
		  * construct and transmit a Commit Message (see 11.3.5.3) followed by a Confirm Message (see 11.3.5.5). The
		  * Sync counter shall be set to zero and the t0 (retransmission) timer shall be set. The protocol instance
		  * transitions to Confirmed state.
		  */
		res = sae_prepare_commit(pSaeIns);

		if (res != MLME_SUCCESS)
			return res;

		if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;

		res = sae_process_commit(pSaeIns);

		if (res != MLME_SUCCESS)
			return res;

		if (sae_send_auth_confirm(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		SET_CONFIRMED_STATE(pSaeIns);
		pSaeIns->sync = 0;
		sae_set_retransmit_timer(pSaeIns);
		break;

	case F(SAE_COMMITTED, SAE_COMMIT_SEQ):
		/* 11.3.8.6.4 If the received element and scalar differ from the element and
		  * scalar offered, the received Commit Message shall be processed according to 11.3.5.4, the Sc
		  * counter shall be incremented (thereby setting its value to one), the protocol instance shall then
		  * construct a Confirm Message, transmit it to the peer, and set the t0 (retransmission) timer. It shall
		  * then transition to Confirmed state.
		  */
		res = sae_process_commit(pSaeIns);
		if (res != MLME_SUCCESS)
			return res;
		if (sae_send_auth_confirm(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		SET_CONFIRMED_STATE(pSaeIns);
		pSaeIns->sync = 0;
		sae_set_retransmit_timer(pSaeIns);
		break;

	case F(SAE_COMMITTED, SAE_CONFIRM_SEQ):
		/* 11.3.8.6.4 Upon receipt of a Con event, If Sync is not greater than
		  * dot11RSNASAESync, the protocol instance shall increment Sync, transmit the last Commit Message sent to
		  * the peer, and set the t0 (retransmission) timer.
		  * comments: In COMMITTED state, it's still awaiting for peer commit msg
		  */
		if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		sae_set_retransmit_timer(pSaeIns);
		break;

	case F(SAE_CONFIRMED, SAE_COMMIT_SEQ):
		if (sae_check_big_sync(pSaeIns))
			return MLME_SUCCESS;

		/* 11.3.8.6.5 the protocol instance shall increment Sync,
		  * increment Sc, and transmit its Commit and Confirm (with the new Sc value) messages.
		  * It then shall set the t0 (retransmission) timer.
		  */
		pSaeIns->sync++;
		res = sae_process_commit(pSaeIns);
		if (res != MLME_SUCCESS)
			return res;
		if (sae_send_auth_commit(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		if (sae_send_auth_confirm(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		sae_set_retransmit_timer(pSaeIns);
		break;

	case F(SAE_CONFIRMED, SAE_CONFIRM_SEQ):
		/* 11.3.8.6.5 If processing is successful and the Confirm Message has been verified,
		  * the Rc variable shall be set to the send-confirm portion of the frame, Sc shall be set to the value 2^16 ¡V 1, the
		  * t1 (key expiry) timer shall be set, and the protocol instance shall transition to Accepted state
		  */
		pSaeIns->last_peer_sc = pSaeIns->peer_send_confirm;
		pSaeIns->send_confirm = SAE_MAX_SEND_CONFIRM;

		/* If another protocol instance exists in the database indexed by the same peer identity as the protocol
		  * instance that sent the Auth event, the other protocol instance shall be destroyed.
		  */
		if (pSaeIns->same_mac_ins) {
			delete_sae_instance(pSaeIns->same_mac_ins);
			pSaeIns->same_mac_ins = NULL;
		}

		SET_ACCEPTED_STATE(pSaeIns);
		/* ellis todo: t1 (key expiry) timer shall be set, and the protocol instance shall transition to Accepted state */
		/* auth done */
		break;

	case F(SAE_ACCEPTED, SAE_CONFIRM_SEQ):
		if (sae_check_big_sync(pSaeIns))
			return MLME_SUCCESS;

		/* 11.3.8.6.6 If the verification succeeds, the Rc variable
		  * shall be set to the send-confirm portion of the frame, the Sync shall be incremented and a new Confirm
		  * Message shall be constructed (with Sc set to 216 ¡V 1) and sent to the peer
		  */
		pSaeIns->sync++;
		pSaeIns->last_peer_sc = pSaeIns->peer_send_confirm;
		if (sae_send_auth_confirm(pAd, pSaeIns) == FALSE)
			return SAE_SILENTLY_DISCARDED;
		break;

	default:
		break;
	}

	return res;
}

UCHAR sae_check_big_sync(
	IN SAE_INSTANCE *pSaeIns)
{
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (pSaeIns->sync > DOT11RSNASAESYNC) {
		delete_sae_instance(pSaeIns);
		return TRUE;
	}

	return FALSE;
}

UCHAR sae_get_pmk_cache(
	IN SAE_CFG *pSaeCfg,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	OUT UCHAR *pmkid,
	OUT UCHAR *pmk)
{
	/* PMKID = L((commit-scalar + peer-commit-scalar) mod r, 0, 128) */
	SAE_BN *tmp = NULL;
	UINT32 len = LEN_PMKID;
	SAE_INSTANCE *pSaeIns = search_sae_instance(pSaeCfg, own_mac, peer_mac);

	if (pSaeIns == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("%s(): pSaeIns not found\n", __func__));
		return FALSE;
	}

	if (pSaeIns->state != SAE_ACCEPTED
		|| !pSaeIns->own_commit_scalar
		|| !pSaeIns->peer_commit_scalar) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 ("%s(): get pmkid fail\n", __func__));
		return FALSE;
	}

	if (pmkid) {
		SAE_BN_INIT(&tmp);

		SAE_BN_MOD_ADD_QUICK(pSaeIns->own_commit_scalar, pSaeIns->peer_commit_scalar, pSaeIns->order, &tmp);
		SAE_BN_BI2BIN_WITH_PAD(tmp, pmkid, &len, LEN_PMKID);

		SAE_BN_FREE(&tmp);
	}

	if (pmk && pSaeIns->pmk)
		NdisMoveMemory(pmk, pSaeIns->pmk, LEN_PMK);
	else if (!pSaeIns->pmk)
		return FALSE;

	return TRUE;
}

USHORT sae_parse_commit(
	IN SAE_CFG *pSaeCfg,
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *msg,
	IN UINT32 msg_len,
	IN UCHAR **token,
	IN UINT32 *token_len,
	IN UCHAR is_token_req)
{
	USHORT sae_group;
	USHORT res;
	FRAME_802_11 *Fr = (PFRAME_802_11)msg;
	UCHAR *pos = &Fr->Octet[6];
	UCHAR *end = msg + msg_len;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	/* Check Finite Cyclic Group */
	NdisMoveMemory(&sae_group, pos, 2); /* ellis bigendian */
	sae_group = cpu2le16(sae_group);
	res = sae_group_allowed(pSaeIns, pSaeCfg->support_group, sae_group);

	if (res != MLME_SUCCESS)
		return res;

	pos = pos + 2;

	if (is_token_req == TRUE) {
		/* process the rejection with anti-clogging */
		return MLME_SUCCESS;
	}

	/* Optional Anti-Clogging Token */
	sae_parse_commit_token(pSaeIns, &pos, end, token, token_len);
	sae_record_time_begin(&pSaeIns->sae_cost_time.parse_commit_scalar_time);
	/* commit-scalar */
	res = sae_parse_commit_scalar(pSaeIns, &pos, end);

	if (res != MLME_SUCCESS)
		return res;

	sae_record_time_end("parse_commit_scalar_time", &pSaeIns->sae_cost_time.parse_commit_scalar_time);
	/* commit-element */
	res = sae_parse_commit_element(pSaeIns, pos, end);

	if (res != MLME_SUCCESS)
		return res;

	/* 11.3.8.6.4 the protocol instance checks the peer-commit-scalar and PEER-COMMIT-ELEMENT
	  * from the message. If they match those sent as part of the protocol instance¡¦s own Commit Message,
	  * the frame shall be silently discarded (because it is evidence of a reflection attack)
	  */
	if (!pSaeIns->own_commit_scalar
		|| SAE_BN_UCMP(pSaeIns->own_commit_scalar, pSaeIns->peer_commit_scalar) != 0
		|| !pSaeIns->own_commit_element
		|| SAE_BN_UCMP(pSaeIns->own_commit_element, pSaeIns->peer_commit_element) != 0)
		return MLME_SUCCESS;
	else
		return MLME_UNSPECIFY_FAIL;
}

VOID sae_parse_commit_token(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end,
	IN UCHAR **token,
	IN UINT32 *token_len)
{
	UINT32 non_token_len = (is_sae_group_ecc(pSaeIns->group) ? 3 : 2) * pSaeIns->prime_len; /* ellis */
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (*pos + non_token_len < end) {
		if (token)
			*token = *pos;

		if (token_len)
			*token_len = (UINT32)(end - *pos) - non_token_len;

		*pos += *token_len;
	} else {
		if (token)
			*token = NULL;

		if (token_len)
			*token_len = 0;
	}
}


USHORT sae_parse_commit_scalar(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR **pos,
	IN UCHAR *end)
{
	SAE_BN *peer_scalar = NULL;
	SAE_INSTANCE *pPreSaeIns = pSaeIns->same_mac_ins;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (*pos + pSaeIns->prime_len > end) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): not enough data for scalar\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}

	hex_dump_with_lvl("peer scalar:", (char *)*pos, pSaeIns->prime_len, SAE_DEBUG_LEVEL);
	SAE_BN_BIN2BI(*pos, pSaeIns->prime_len, &peer_scalar);

	/*
	 * IEEE Std 802.11-2012, 11.3.8.6.1: If there is a protocol instance for
	 * the peer and it is in Authenticated state, the new Commit Message
	 * shall be dropped if the peer-scalar is identical to the one used in
	 * the existing protocol instance.
	 */

	if (pPreSaeIns
		&& (pPreSaeIns->state == SAE_ACCEPTED)
		&& (pPreSaeIns->peer_commit_scalar)
		&& !SAE_BN_UCMP(peer_scalar, pPreSaeIns->peer_commit_scalar)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): do not accept re-use of previous peer-commit-scalar\n", __func__));
		SAE_BN_FREE(&peer_scalar);
		return MLME_UNSPECIFY_FAIL;
	}  else if (pPreSaeIns
		&& (pPreSaeIns->state == SAE_CONFIRMED)
		&& (pPreSaeIns->peer_commit_scalar)
		&& !SAE_BN_UCMP(peer_scalar, pPreSaeIns->peer_commit_scalar)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): this is retry msg, silient discard\n", __func__));
		SAE_BN_FREE(&peer_scalar);
		return SAE_SILENTLY_DISCARDED;
	}

	/* If the scalar value is greater than zero (0) and less than the order, r, of the negotiated group, scalar validation succeeds */
	/* 0 < scalar < r */
	if (SAE_BN_IS_ZERO(peer_scalar)
		|| SAE_BN_UCMP(peer_scalar, pSaeIns->order) >= 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): Invalid peer scalar\n", __func__));
		SAE_BN_FREE(&peer_scalar);
		return MLME_UNSPECIFY_FAIL;
	}

	SAE_BN_FREE(&pSaeIns->peer_commit_scalar);
	pSaeIns->peer_commit_scalar = peer_scalar;
	*pos += pSaeIns->prime_len;
	return MLME_SUCCESS;
}

USHORT sae_parse_commit_element(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end)
{
	USHORT res = MLME_UNSPECIFY_FAIL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	sae_record_time_begin(&pSaeIns->sae_cost_time.parse_commit_element_time);

	if (pSaeIns->group_op)
		res = pSaeIns->group_op->sae_parse_commit_element(pSaeIns, pos, end);

	sae_record_time_end("parse_commit_element_time", &pSaeIns->sae_cost_time.parse_commit_element_time);
	return res;
}




USHORT sae_prepare_commit(
	IN SAE_INSTANCE *pSaeIns)
{
	USHORT res;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	sae_record_time_begin(&pSaeIns->sae_cost_time.derive_pwe_time);

	if (pSaeIns->group_op)
		res = pSaeIns->group_op->sae_derive_pwe(pSaeIns);
	else
		return MLME_UNSPECIFY_FAIL;

	sae_record_time_end("derive_pwe_time", &pSaeIns->sae_cost_time.derive_pwe_time);

	if (res != MLME_SUCCESS)
		return res;

	return sae_derive_commit(pSaeIns);
}


USHORT sae_derive_commit(
	IN SAE_INSTANCE *pSaeIns)
{
	SAE_BN *mask = NULL;
	USHORT res = MLME_SUCCESS;
	UINT32 counter = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	sae_record_time_begin(&pSaeIns->sae_cost_time.derive_commit_scalar_time);


	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[0]);

	do {
		counter++;

		if (counter > 100) {
			res = MLME_UNSPECIFY_FAIL;
			goto end;
		}

		SAE_BN_FREE(&pSaeIns->sae_rand);
		pSaeIns->sae_rand = sae_gen_rand(pSaeIns);
		SAE_BN_FREE(&mask);
		mask = sae_gen_rand(pSaeIns);

		if (pSaeIns->own_commit_scalar == NULL) {
			SAE_BN_INIT(&pSaeIns->own_commit_scalar);

			if (pSaeIns->own_commit_scalar == NULL) {
				res = MLME_UNSPECIFY_FAIL;
				goto end;
			}
		}

		/* commit-scalar = (rand + mask) modulo r */
		SAE_BN_MOD_ADD(pSaeIns->sae_rand, mask, pSaeIns->order, &pSaeIns->own_commit_scalar);
	} while (SAE_BN_IS_ZERO(pSaeIns->own_commit_scalar)
			 /*|| SAE_BN_IS_ONE(pSaeIns->own_commit_scalar)*/);

	sae_record_time_end("derive_commit_scalar_time", &pSaeIns->sae_cost_time.derive_commit_scalar_time);
	sae_record_time_begin(&pSaeIns->sae_cost_time.derive_commit_element_time);

	if (pSaeIns->group_info == NULL
		|| pSaeIns->group_op == NULL
		|| pSaeIns->group_op->sae_derive_commit_element(pSaeIns, mask) == FALSE)
		res = MLME_UNSPECIFY_FAIL;

	sae_record_time_end("derive_commit_element_time", &pSaeIns->sae_cost_time.derive_commit_element_time);
end:
	SAE_BN_FREE(&mask);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[0]);
	return res;
}


USHORT sae_process_commit(
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR *k = NULL;
	USHORT res = MLME_SUCCESS;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	sae_record_time_begin(&pSaeIns->sae_cost_time.derive_k_time);

	os_alloc_mem(NULL, (UCHAR **)&k, SAE_MAX_PRIME_LEN);

	if (pSaeIns->group_op
		&& (pSaeIns->group_op->sae_derive_k(pSaeIns, k) == TRUE)) {
		sae_record_time_end("derive_k_time", &pSaeIns->sae_cost_time.derive_k_time);
		sae_record_time_begin(&pSaeIns->sae_cost_time.derive_pmk_time);

		if (sae_derive_key(pSaeIns, k) == TRUE)
			res = MLME_SUCCESS;
		else
			res = MLME_UNSPECIFY_FAIL;

		sae_record_time_end("derive_pmk_time", &pSaeIns->sae_cost_time.derive_pmk_time);
	} else
		res = MLME_UNSPECIFY_FAIL;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("%s() <==, res = %d\n", __func__, res));

	os_free_mem(k);
	return res;
}


UCHAR sae_derive_key(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *k)
{
	UCHAR null_key[SAE_KEYSEED_KEY_LEN];
	UCHAR keyseed[SHA256_DIGEST_SIZE];
	UCHAR *val = NULL;
	UINT32 val_len = SAE_MAX_PRIME_LEN;
	UCHAR keys[SAE_KCK_LEN + LEN_PMK];
	SAE_BN *tmp = NULL;
	UCHAR res = TRUE;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	/* keyseed = H(<0>32, k) */
	NdisZeroMemory(null_key, SAE_KEYSEED_KEY_LEN);
	RT_HMAC_SHA256(null_key, SAE_KEYSEED_KEY_LEN, k,
				   pSaeIns->prime_len, keyseed, SHA256_DIGEST_SIZE);

	hex_dump_with_lvl("keyseed:", (char *)keyseed, SHA256_DIGEST_SIZE, SAE_DEBUG_LEVEL);

	/* KCK || PMK = KDF-512(keyseed, "SAE KCK and PMK",
	  *                      (commit-scalar + peer-commit-scalar) modulo r)
	  */

	os_alloc_mem(NULL, (UCHAR **)&val, SAE_MAX_PRIME_LEN);
	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[1]);
	GET_BI_INS_FROM_POOL(tmp);
	SAE_BN_INIT(&tmp);
	SAE_BN_MOD_ADD(pSaeIns->own_commit_scalar, pSaeIns->peer_commit_scalar, pSaeIns->order, &tmp);
	SAE_BN_BI2BIN_WITH_PAD(tmp, val, &val_len, pSaeIns->prime_len);
	hex_dump_with_lvl("(commit-scalar + peer-commit-scalar) modulo r:", (char *)val, val_len, SAE_DEBUG_LEVEL);

	if (val_len < pSaeIns->prime_len) {
		SAE_BN_FREE(&tmp);
		res = FALSE;
		goto Free;
	}

	KDF(keyseed, sizeof(keyseed), (UINT8 *)"SAE KCK and PMK", 15, val, val_len, keys, sizeof(keys)); /* ellis KDF-512 */
	NdisCopyMemory(pSaeIns->kck, keys, SAE_KCK_LEN);
	NdisCopyMemory(pSaeIns->pmk, keys + SAE_KCK_LEN, LEN_PMK);
	hex_dump_with_lvl("kck:", (char *)pSaeIns->kck, SAE_KCK_LEN, SAE_DEBUG_LEVEL);
	hex_dump_with_lvl("pmk:", (char *)pSaeIns->pmk, LEN_PMK, SAE_DEBUG_LEVEL);
Free:
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[1]);
	os_free_mem(val);
	return res;
}

VOID sae_send_auth(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR *own_mac,
	IN UCHAR *peer_mac,
	IN UCHAR *bssid,
	IN USHORT alg,
	IN USHORT seq,
	IN USHORT status_code,
	IN UCHAR *buf,
	IN UINT32 bif_len)
{
	HEADER_802_11 AuthHdr;
	ULONG FrameLen = 0;
	PUCHAR pOutBuffer = NULL;
	NDIS_STATUS NStatus;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("==> %s(), seq = %d, statuscode = %d, own mac addr = %02x:%02x:%02x:%02x:%02x:%02x, peer mac addr = %02x:%02x:%02x:%02x:%02x:%02x\n",
			  __func__, seq, status_code, PRINT_MAC(own_mac), PRINT_MAC(peer_mac)));
	NStatus = MlmeAllocateMemory(pAd, &pOutBuffer);

	if (NStatus != NDIS_STATUS_SUCCESS)
		return;

	MgtMacHeaderInit(pAd, &AuthHdr, SUBTYPE_AUTH, 0, peer_mac,
					 own_mac,
					 bssid);
	MakeOutgoingFrame(pOutBuffer,	&FrameLen,
					  sizeof(HEADER_802_11), &AuthHdr,
					  2,			&alg,
					  2,			&seq,
					  2,			&status_code,
					  bif_len,		buf,
					  END_OF_ARGS);
	MiniportMMRequest(pAd, 0, pOutBuffer, FrameLen);
	MlmeFreeMemory(pOutBuffer);
}


UCHAR sae_send_auth_commit(
	IN RTMP_ADAPTER *pAd,
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR *buf = NULL;
	UCHAR *pos;
	UCHAR *end;
	UINT32 len;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
			 ("==> %s():\n", __func__));
	os_alloc_mem(pAd, &buf, SAE_COMMIT_MAX_LEN);

	if (buf == NULL)
		return FALSE;

	pos = buf;
	end = buf + SAE_COMMIT_MAX_LEN;
	NdisZeroMemory(pos, SAE_COMMIT_MAX_LEN);
	NdisMoveMemory(pos, &pSaeIns->group, 2);
	pos += 2;

	if (pSaeIns->anti_clogging_token) {
		NdisMoveMemory(pos, &pSaeIns->anti_clogging_token,
					   pSaeIns->anti_clogging_token_len);
		pos += pSaeIns->anti_clogging_token_len;
	}

	len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(pSaeIns->own_commit_scalar, pos, &len, pSaeIns->prime_len);
	pos += len;

	if (is_sae_group_ecc(pSaeIns->group)) {
		BIG_INTEGER_EC_POINT *element = (BIG_INTEGER_EC_POINT *) pSaeIns->own_commit_element;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element->x, pos, &len, pSaeIns->prime_len);
		pos += len;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element->y, pos, &len, pSaeIns->prime_len);
		pos += len;
	} else {
		SAE_BN *element = (SAE_BN *) pSaeIns->own_commit_element;
		len = pSaeIns->prime_len;
		SAE_BN_BI2BIN_WITH_PAD(element, pos, &len, pSaeIns->prime_len);
		pos += len;
	}

	sae_send_auth(pAd, pSaeIns->own_mac, pSaeIns->peer_mac, pSaeIns->bssid,
				  AUTH_MODE_SAE, SAE_COMMIT_SEQ, MLME_SUCCESS, buf, pos - buf);
	os_free_mem(buf);
	return TRUE;
}


UCHAR sae_send_auth_confirm(
	IN RTMP_ADAPTER *pAd,
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR *buf = NULL;
	UCHAR *pos;
	UCHAR *end;
	UCHAR confirm[SHA256_DIGEST_SIZE];
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	os_alloc_mem(pAd, &buf, SAE_CONFIRM_MAX_LEN);
	pos = buf;
	end = buf + SAE_CONFIRM_MAX_LEN;

	/* 11.3.8.6.4 the Sc counter shall be incremented (thereby setting its value to one), the protocol instance shall then
	  * construct a Confirm Message, transmit it to the peer
	  * 11.3.8.6.5 the protocol instance shall increment Sync,
	  * increment Sc, and transmit its Commit and Confirm (with the new Sc value) messages
	  * => increment send_confirm first and send comfirm msg with new sc value
	  */
	if (pSaeIns->send_confirm != SAE_MAX_SEND_CONFIRM)
		pSaeIns->send_confirm++;

	NdisMoveMemory(pos, &pSaeIns->send_confirm, 2);
	pos += 2;

	if (pSaeIns->group_op)
		pSaeIns->group_op->sae_cn_confirm(pSaeIns, TRUE, confirm);

	NdisMoveMemory(pos, confirm, SHA256_DIGEST_SIZE);
	hex_dump_with_lvl("confirm(pos):", (char *)pos, SHA256_DIGEST_SIZE, SAE_DEBUG_LEVEL);
	pos += SHA256_DIGEST_SIZE;
	sae_send_auth(pAd, pSaeIns->own_mac, pSaeIns->peer_mac, pSaeIns->bssid,
				  AUTH_MODE_SAE, SAE_CONFIRM_SEQ, MLME_SUCCESS, buf, pos - buf);
	os_free_mem(buf);
	return TRUE;
}

USHORT sae_parse_confirm(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *msg,
	IN UINT32 msg_len)
{
	UCHAR peer_confirm[SHA256_DIGEST_SIZE];
	FRAME_802_11 *Fr = (PFRAME_802_11)msg;
	UCHAR *pos = &Fr->Octet[6];
	UCHAR *end = msg + msg_len;
	USHORT peer_send_confirm;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	/* peer-send-confirm */
	if (end - pos < 2)
		return MLME_UNSPECIFY_FAIL;

	NdisMoveMemory(&peer_send_confirm, pos, 2);
	pos = pos + 2;

	/*  11.3.8.6.6 Upon receipt of a Con event, the value of send-confirm shall be checked.
	  * If the value is not greater than Rc or is equal to 2^16 ¡V 1, the received frame shall be silently discarded
	  */
	if (pSaeIns->state == SAE_ACCEPTED
		&& (peer_send_confirm <= pSaeIns->last_peer_sc
			|| peer_send_confirm == SAE_MAX_SEND_CONFIRM)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_WARN,
			("confirm fail: SAE_SILENTLY_DISCARDED due to  peer_send_confirm =%d, ast_peer_sc = %d\n",
			peer_send_confirm, pSaeIns->last_peer_sc));
		pSaeIns->peer_send_confirm = peer_send_confirm;
		/* return SAE_SILENTLY_DISCARDED; */
	} else
		pSaeIns->peer_send_confirm = peer_send_confirm;

	/* send-confirm */
	if (end - pos < SHA256_DIGEST_SIZE)
		return MLME_UNSPECIFY_FAIL;

	NdisMoveMemory(peer_confirm, pos, SHA256_DIGEST_SIZE);
	return sae_check_confirm(pSaeIns, peer_confirm);
}


USHORT sae_check_confirm(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *peer_confirm)
{
	UCHAR verifier[SHA256_DIGEST_SIZE];
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (pSaeIns->peer_commit_element == NULL
		|| pSaeIns->peer_commit_scalar == NULL
		|| pSaeIns->own_commit_element == NULL
		|| pSaeIns->own_commit_scalar == NULL)
		return MLME_UNSPECIFY_FAIL;

	if (pSaeIns->group_op)
		pSaeIns->group_op->sae_cn_confirm(pSaeIns, FALSE, verifier);
	else
		return MLME_UNSPECIFY_FAIL;

	if (is_sae_group_ecc(pSaeIns->group))
		return MLME_SUCCESS;

	if (RTMPEqualMemory(peer_confirm, verifier, SHA256_DIGEST_SIZE))
		return MLME_SUCCESS;
	else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
				 ("peer_send_confirm = %d\n", pSaeIns->peer_send_confirm));
		hex_dump_with_lvl("peer_confirm:", (char *)peer_confirm, SHA256_DIGEST_SIZE, SAE_DEBUG_LEVEL2);
		hex_dump_with_lvl("verifier:", (char *)verifier, SHA256_DIGEST_SIZE, SAE_DEBUG_LEVEL2);
		return MLME_UNSPECIFY_FAIL;
	}
}

SAE_BN *sae_gen_rand(
	IN SAE_INSTANCE *pSaeIns)
{
	UINT8 *rand = NULL;
	UINT32 i;
	SAE_BN *rand_bi = NULL;
	UINT32 iter = 0;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (!pSaeIns->order)
		return NULL;

	os_alloc_mem(NULL, (UCHAR **)&rand, SAE_MAX_PRIME_LEN);

	for (iter = 0; iter < 100; iter++) {
		for (i = 0; i < pSaeIns->order_len; i++)
			rand[i] = RandomByte(pSaeIns->pParentSaeCfg->pAd);

		hex_dump_with_lvl("rand:", (char *)rand, pSaeIns->order_len, SAE_DEBUG_LEVEL);
		SAE_BN_BIN2BI(rand, pSaeIns->order_len, &rand_bi);

		if (SAE_BN_IS_ZERO(rand_bi)
			|| SAE_BN_IS_ONE(rand_bi)
			|| SAE_BN_UCMP(rand_bi, pSaeIns->order) >= 0)
			continue;
		else {
			os_free_mem(rand);
			return rand_bi;
		}
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 ("%s(): gen rand fail\n", __func__));
	SAE_BN_FREE(&rand_bi);
	os_free_mem(rand);
	return NULL;
}


USHORT sae_group_allowed(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *allowed_groups,
	IN INT32 group)
{
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s():\n", __func__));

	if (allowed_groups) {
		UINT32 i;

		for (i = 0; allowed_groups[i] > 0; i++) {/* ellis */
			MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
					 ("%s(): i=%d, allowed_groups=%d, groups = %d\n", __func__, i, allowed_groups[i], group));

			if (allowed_groups[i] == group)
				break;
		}

		if (allowed_groups[i] != group)
			return MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED;
	}

	if (pSaeIns->group != group) {
		/*the protocol instance shall verify that the finite cyclic group is the same as the previously received Commit frame.
		If not, the frame shall be silently discarded. */
		if (pSaeIns->state == SAE_CONFIRMED) {
			delete_sae_instance(pSaeIns);
			return SAE_SILENTLY_DISCARDED;
		}

		sae_clear_data(pSaeIns);

		if (is_sae_group_ecc(group))
			pSaeIns->group_op = &ecc_group_op;
		else if (is_sae_group_ffc(group))
			pSaeIns->group_op = &ffc_group_op;
		else
			return MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED;

		pSaeIns->group_op->sae_group_init(pSaeIns, group); /* ellis exception */

		if (pSaeIns->group_info == NULL
			|| pSaeIns->group_op == NULL)
			return MLME_UNSPECIFY_FAIL;
	}

	return MLME_SUCCESS;
}


UCHAR is_sae_group_ecc(
	IN INT32 group)
{
	switch (group) {
	case 19:
	case 20:
	case 21:
	case 25:
	case 26:
	case 27:
	case 28:
	case 29:
	case 30:
		return TRUE;

	default:
		return FALSE;
	}
}


UCHAR is_sae_group_ffc(
	IN INT32 group)
{
	switch (group) {
	case 1:
	case 2:
	case 5:
	case 14:
	case 15:
	case 16:
	case 17:
	case 18:
	case 22:
	case 23:
	case 24:
		return TRUE;

	default:
		return FALSE;
	}
}

VOID sae_group_init_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN INT32 group)
{
	EC_GROUP_INFO *ec_group = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = NULL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	ec_group = get_ecc_group_info(group);
	ec_group_bi = get_ecc_group_info_bi(group);

	if (ec_group == NULL
		|| ec_group_bi == NULL)
		return;
	pSaeIns->group_info = (VOID *) ec_group;
	pSaeIns->group_info_bi = (VOID *) ec_group_bi;
	pSaeIns->prime = ec_group_bi->prime;
	pSaeIns->prime_len = ec_group->prime_len;
	pSaeIns->order = ec_group_bi->order;
	pSaeIns->order_len = ec_group->order_len;
	pSaeIns->group = group;
}


VOID sae_group_init_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN INT32 group)
{
	UINT32 i;
	DH_GROUP_INFO *dh_group = NULL;
	DH_GROUP_INFO_BI *dh_group_bi = NULL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	for (i = 0; i < DH_GROUP_NUM; i++) {
		if (dh_groups[i].group_id == group) {
			dh_group = &dh_groups[i];
			dh_group_bi = &dh_groups_bi[i];
		}
	}

	if (dh_group == NULL
		|| dh_group_bi == NULL)
		return;

	if (dh_group_bi->is_init == FALSE) {
		dh_group_bi->prime = NULL;
		dh_group_bi->order = NULL;
		dh_group_bi->generator = NULL;
		SAE_BN_BIN2BI((UINT8 *)dh_group->prime,
						  dh_group->prime_len,
						  &dh_group_bi->prime);
		SAE_BN_BIN2BI((UINT8 *)dh_group->order,
						  dh_group->order_len,
						  &dh_group_bi->order);
		SAE_BN_BIN2BI((UINT8 *)dh_group->generator,
						  dh_group->generator_len,
						  &dh_group_bi->generator);
	}

	pSaeIns->group_info = (VOID *) dh_group;
	pSaeIns->group_info_bi = (VOID *) dh_group_bi;
	pSaeIns->prime = dh_group_bi->prime;
	pSaeIns->prime_len = dh_group->prime_len;
	pSaeIns->order = dh_group_bi->order;
	pSaeIns->order_len = dh_group->order_len;
	pSaeIns->group = group;
}


VOID sae_group_deinit_ecc(
	IN SAE_INSTANCE *pSaeIns)
{
	BIG_INTEGER_EC_POINT *own_element = NULL;
	BIG_INTEGER_EC_POINT *peer_element = NULL;
	BIG_INTEGER_EC_POINT *pwe = NULL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	own_element = (BIG_INTEGER_EC_POINT *)pSaeIns->own_commit_element;
	peer_element = (BIG_INTEGER_EC_POINT *)pSaeIns->peer_commit_element;
	pwe = (BIG_INTEGER_EC_POINT *)pSaeIns->pwe;

	if (own_element) {
		SAE_BN_FREE(&own_element->x);
		SAE_BN_FREE(&own_element->y);
		pSaeIns->own_commit_element = NULL;
		os_free_mem(own_element);
	}

	if (peer_element) {
		SAE_BN_FREE(&peer_element->x);
		SAE_BN_FREE(&peer_element->y);
		pSaeIns->peer_commit_element = NULL;
		os_free_mem(peer_element);
	}

	if (pwe) {
		SAE_BN_FREE(&pwe->x);
		SAE_BN_FREE(&pwe->y);
		pSaeIns->pwe = NULL;
		os_free_mem(pwe);
	}
}


VOID sae_group_deinit_ffc(
	IN SAE_INSTANCE *pSaeIns)
{
	SAE_BN *own_element = NULL;
	SAE_BN *peer_element = NULL;
	SAE_BN *pwe = NULL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	own_element = (SAE_BN *)pSaeIns->own_commit_element;
	peer_element = (SAE_BN *)pSaeIns->peer_commit_element;
	pwe = (SAE_BN *)pSaeIns->pwe;

	if (own_element)
		SAE_BN_FREE(&own_element);

	pSaeIns->own_commit_element = NULL;

	if (peer_element)
		SAE_BN_FREE(&peer_element);

	pSaeIns->peer_commit_element = NULL;

	if (pwe)
		SAE_BN_FREE(&pwe);

	pSaeIns->pwe = NULL;
}



VOID sae_cn_confirm_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR is_send, /* otherwise, is verfication */
	OUT UCHAR *confirm)
{
	UCHAR own_element_bin[2 * SAE_MAX_ECC_PRIME_LEN];
	UCHAR peer_element_bin[2 * SAE_MAX_ECC_PRIME_LEN];
	UINT32 prime_len;
	BIG_INTEGER_EC_POINT *own_element = (BIG_INTEGER_EC_POINT *)pSaeIns->own_commit_element; /* ellis */
	BIG_INTEGER_EC_POINT *peer_element = (BIG_INTEGER_EC_POINT *)pSaeIns->peer_commit_element;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	prime_len = pSaeIns->prime_len; /* ellis */
	SAE_BN_BI2BIN_WITH_PAD(own_element->x, own_element_bin,
							   &prime_len, pSaeIns->prime_len);
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(own_element->y, own_element_bin + pSaeIns->prime_len,
							   &prime_len, pSaeIns->prime_len);
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(peer_element->x, peer_element_bin,
							   &prime_len, pSaeIns->prime_len);
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(peer_element->y, peer_element_bin + pSaeIns->prime_len,
							   &prime_len, pSaeIns->prime_len);

	if (is_send)
		sae_cn_confirm_cmm(pSaeIns, pSaeIns->own_commit_scalar,
						   pSaeIns->peer_commit_scalar,
						   own_element_bin, peer_element_bin,
						   2 * pSaeIns->prime_len,
						   pSaeIns->send_confirm,
						   confirm);
	else
		sae_cn_confirm_cmm(pSaeIns, pSaeIns->peer_commit_scalar,
						   pSaeIns->own_commit_scalar,
						   peer_element_bin, own_element_bin,
						   2 * pSaeIns->prime_len,
						   pSaeIns->peer_send_confirm,
						   confirm);
}



VOID sae_cn_confirm_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR is_send, /* otherwise, is verfication */
	OUT UCHAR *confirm)
{
	UCHAR *own_element_bin = NULL;
	UCHAR *peer_element_bin = NULL;
	UINT32 prime_len;
	SAE_BN *own_element = (SAE_BN *)pSaeIns->own_commit_element;
	SAE_BN *peer_element = (SAE_BN *)pSaeIns->peer_commit_element;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()is_send = %d\n", __func__, is_send));
	os_alloc_mem(NULL, (UCHAR **)&own_element_bin, SAE_MAX_PRIME_LEN);

	if (own_element_bin == NULL)
		return;

	os_alloc_mem(NULL, (UCHAR **)&peer_element_bin, SAE_MAX_PRIME_LEN);

	if (peer_element_bin == NULL) {
		os_free_mem(own_element_bin);
		return;
	}

	prime_len = pSaeIns->prime_len; /* ellis */
	SAE_BN_BI2BIN_WITH_PAD(own_element, own_element_bin,
							   &prime_len, pSaeIns->prime_len);
	prime_len = pSaeIns->prime_len; /* ellis */
	SAE_BN_BI2BIN_WITH_PAD(peer_element, peer_element_bin,
							   &prime_len, pSaeIns->prime_len);

	if (is_send)
		sae_cn_confirm_cmm(pSaeIns, pSaeIns->own_commit_scalar,
						   pSaeIns->peer_commit_scalar,
						   own_element_bin, peer_element_bin,
						   pSaeIns->prime_len,
						   pSaeIns->send_confirm,
						   confirm);
	else
		sae_cn_confirm_cmm(pSaeIns, pSaeIns->peer_commit_scalar,
						   pSaeIns->own_commit_scalar,
						   peer_element_bin, own_element_bin,
						   pSaeIns->prime_len,
						   pSaeIns->peer_send_confirm,
						   confirm);

	os_free_mem(own_element_bin);
	os_free_mem(peer_element_bin);
}

VOID sae_cn_confirm_cmm(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *scalar1,
	IN SAE_BN *scalar2,
	IN UCHAR *element_bin1,
	IN UCHAR *element_bin2,
	IN UINT32 element_len,
	IN USHORT send_confirm,
	OUT UCHAR *confirm)
{
	UCHAR *msg;
	UINT32 msg_len = sizeof(send_confirm) + 2 * element_len + 2 * pSaeIns->prime_len;
	UINT32 offset = 0;
	UINT32 prime_len;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s(), send_confirm = %d\n", __func__, send_confirm));
	os_alloc_mem(NULL, &msg, msg_len);

	if (msg == NULL)
		return;

	/*
	  * CN(key, X, Y, Z, ¡K) = HMAC-SHA256(key, D2OS(X) || D2OS(Y) || D2OS(Z) || ¡K)
	  * where D2OS() represents the data to octet string conversion functions in 11.3.7.2.
	  * confirm = CN(KCK, send-confirm, commit-scalar, COMMIT-ELEMENT,
	  *              peer-commit-scalar, PEER-COMMIT-ELEMENT)
	  * verifier = CN(KCK, peer-send-confirm, peer-commit-scalar,
	  *               PEER-COMMIT-ELEMENT, commit-scalar, COMMIT-ELEMENT)
	  */
	NdisMoveMemory(msg, &send_confirm, sizeof(send_confirm));
	offset += sizeof(send_confirm);
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(scalar1, msg + offset,
							   &prime_len, pSaeIns->prime_len);
	offset += pSaeIns->prime_len;
	NdisMoveMemory(msg + offset, element_bin1, element_len);
	offset += element_len;
	prime_len = pSaeIns->prime_len;
	SAE_BN_BI2BIN_WITH_PAD(scalar2, msg + offset,
							   &prime_len, pSaeIns->prime_len);
	offset += pSaeIns->prime_len;
	NdisMoveMemory(msg + offset, element_bin2, element_len);
	offset += element_len;

	hex_dump_with_lvl("element_bin1:", (char *)element_bin1, element_len, SAE_DEBUG_LEVEL);
	hex_dump_with_lvl("element_bin2:", (char *)element_bin2, element_len, SAE_DEBUG_LEVEL);
	RT_HMAC_SHA256(pSaeIns->kck, SAE_KCK_LEN, msg, msg_len, confirm, SHA256_DIGEST_SIZE);
	hex_dump_with_lvl("confirm:", (char *)confirm, SHA256_DIGEST_SIZE, SAE_DEBUG_LEVEL);

	os_free_mem(msg);
}

USHORT sae_parse_commit_element_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end)
{
	SAE_BN *peer_element_x = NULL;
	SAE_BN *peer_element_y = NULL;
	BIG_INTEGER_EC_POINT *peer_element = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *) pSaeIns->group_info_bi;
	USHORT res = MLME_SUCCESS;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (pos + 2 * pSaeIns->prime_len > end)
		goto fail;

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[2]);
	GET_BI_INS_FROM_POOL(peer_element_x);
	GET_BI_INS_FROM_POOL(peer_element_y);

	SAE_BN_BIN2BI(pos, pSaeIns->prime_len, &peer_element_x);
	SAE_BN_BIN2BI(pos + pSaeIns->prime_len, pSaeIns->prime_len, &peer_element_y);
	hex_dump_with_lvl("peer element x:", (char *)pos, pSaeIns->prime_len, SAE_DEBUG_LEVEL2);
	hex_dump_with_lvl("peer element y:", (char *)pos + pSaeIns->prime_len, pSaeIns->prime_len, SAE_DEBUG_LEVEL2);
	/*
	  * For ECC groups, both the x- and ycoordinates
	  * of the element shall be non-negative integers less than the prime number p, and the two
	  * coordinates shall produce a valid point on the curve satisfying the group¡¦s curve definition, not being equal
	  * to the ¡§point at the infinity.¡¨ If either of those conditions does not hold, element validation fails; otherwise,
	  * element validation succeeds.
	  */
	ecc_point_init(&peer_element);
	SAE_BN_COPY(peer_element_x, &peer_element->x);
	SAE_BN_COPY(peer_element_y, &peer_element->y);
	SAE_ECC_SET_Z_TO_1(peer_element);

	if (ecc_point_is_on_curve(ec_group_bi, peer_element) == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): is not on curve\n", __func__));
		ecc_point_free(&peer_element);
		res = MLME_UNSPECIFY_FAIL;
		goto fail;
	}

	pSaeIns->peer_commit_element = peer_element;
fail:
	SAE_BN_RELEASE_BACK_TO_POOL(&peer_element_x);
	SAE_BN_RELEASE_BACK_TO_POOL(&peer_element_y);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[2]);
	return res;
}

USHORT sae_parse_commit_element_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN UCHAR *pos,
	IN UCHAR *end)
{
	SAE_BN *scalar_op_res = NULL;
	SAE_BN *peer_commit_element = NULL;
	USHORT res = MLME_UNSPECIFY_FAIL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	if (pos + pSaeIns->prime_len > end)
		goto fail;

	SAE_BN_BIN2BI(pos, pSaeIns->prime_len, &peer_commit_element);
	hex_dump_with_lvl("peer element:", (char *)pos, pSaeIns->prime_len, SAE_DEBUG_LEVEL2);

	if (peer_commit_element == NULL)
		goto fail;

	/*
	  * For FFC groups, the element shall be an integer greater than zero (0) and less than the prime number p,
	  * and the scalar operation of the element and the order of the group, r, shall equal one (1) modulo the prime number p
	  */
	/* 0 < element < p */
	if (SAE_BN_IS_ZERO(peer_commit_element)
		|| SAE_BN_UCMP(peer_commit_element, pSaeIns->prime) >= 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): invalid peer element\n", __func__));
		goto fail;
	}

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[3]);
	/* GET_BI_INS_FROM_POOL(scalar_op_res); */

	/* scalar-op(r, ELEMENT) = 1 modulo p */
	SAE_BN_MOD_EXP_MONT(peer_commit_element, pSaeIns->order, pSaeIns->prime, &scalar_op_res);

	if (!SAE_BN_IS_ONE(scalar_op_res)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): invalid peer element (scalar-op)\n", __func__));
		goto fail;
	}

	pSaeIns->peer_commit_element = peer_commit_element;
	res = MLME_SUCCESS;
fail:
	if (res != MLME_SUCCESS)
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
			 ("%s(): fail\n", __func__));

	SAE_BN_RELEASE_BACK_TO_POOL(&scalar_op_res);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[3]);

	if (res == MLME_UNSPECIFY_FAIL)
		SAE_BN_FREE(&peer_commit_element);

	return res;
}

UCHAR sae_derive_commit_element_ecc(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *mask)
{
	BIG_INTEGER_EC_POINT *pwe = (BIG_INTEGER_EC_POINT *)pSaeIns->pwe;
	BIG_INTEGER_EC_POINT *commit_element = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *)pSaeIns->group_info_bi;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	/* COMMIT-ELEMENT = inverse(scalar-op(mask, PWE)) */
	if (pwe == NULL || mask == NULL)
		return FALSE;

	ECC_POINT_MUL(pwe, mask, ec_group_bi, &commit_element);

	SAE_ECC_3D_to_2D(ec_group_bi, commit_element);

	if (commit_element == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): ECC_POINT_MUL fail\n", __func__));
		return FALSE;
	}

	if (!ecc_point_is_on_curve(ec_group_bi, commit_element)) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR, ("ecc_point_mul_dblandadd fail!!!!!!\n"));
		return FALSE;
	}

	ecc_point_inverse(commit_element, ec_group_bi->prime, &commit_element);

	if (commit_element == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): inverse fail\n", __func__));
		return FALSE;
	}

	pSaeIns->own_commit_element = (VOID *) commit_element;
	return TRUE;
}

UCHAR sae_derive_commit_element_ffc(
	IN SAE_INSTANCE *pSaeIns,
	IN SAE_BN *mask)
{
	SAE_BN *commit_element = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *pwe = (SAE_BN *) pSaeIns->pwe;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	/* COMMIT-ELEMENT = inverse(scalar-op(mask, PWE)) */
	if (pwe == NULL || mask == NULL)
		return FALSE;

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[4]);
	/* GET_BI_INS_FROM_POOL(tmp); */

	SAE_BN_INIT(&tmp);
	SAE_BN_MOD_EXP_MONT(pwe, mask, pSaeIns->prime, &tmp);
	/* SAE_BN_MOD_EXP_MONT(pwe, mask, pSaeIns->prime, &commit_element); */
	SAE_BN_MOD_MUL_INV(tmp, pSaeIns->prime, &commit_element);
	/* SAE_BN_MOD_MUL_INV(commit_element, pSaeIns->prime, &tmp); */
	SAE_BN_FREE(&tmp);

	if (commit_element == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): inverse fail\n", __func__));
		return FALSE;
	}

	pSaeIns->own_commit_element = (VOID *) commit_element;
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[4]);
	return TRUE;
}

USHORT sae_derive_pwe_ecc(
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR counter = 0;
	/*UCHAR k = 50;*/
	UCHAR addrs[2 * MAC_ADDR_LEN];
	BIG_INTEGER_EC_POINT *res = NULL;
	UCHAR base[LEN_PSK + 1];
	UCHAR msg[LEN_PSK + 2]; /* sizeof(base)+sizeof(counter) */
	UINT32 base_len = strlen(pSaeIns->psk);
	UINT32 msg_len;
	UCHAR pwd_seed[SHA256_DIGEST_SIZE];
	UCHAR pwd_value[SAE_MAX_ECC_PRIME_LEN];
	EC_GROUP_INFO *ec_group;
	EC_GROUP_INFO_BI *ec_group_bi;
	SAE_BN *x = NULL;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	/* Get larger address first */
	if (RTMPCompareMemory(pSaeIns->own_mac, pSaeIns->peer_mac, MAC_ADDR_LEN) == 1) {
		COPY_MAC_ADDR(addrs, pSaeIns->own_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->peer_mac);
	} else {
		COPY_MAC_ADDR(addrs, pSaeIns->peer_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->own_mac);
	}

	NdisMoveMemory(base, pSaeIns->psk, base_len);

	hex_dump_with_lvl("base:", (char *)base, base_len, SAE_DEBUG_LEVEL2);
	ec_group = (EC_GROUP_INFO *)pSaeIns->group_info;
	ec_group_bi = (EC_GROUP_INFO_BI *)pSaeIns->group_info_bi;

	for (counter = 1; /*counter <= k ||*/ !res; counter++) {
		UCHAR shift_idx;
		SAE_BN *y = NULL;
		UINT32 i;
		UINT8 lsb_pwd_seed;
		UINT8 lsb_y;
		UCHAR has_y;

		if (counter == 0) {
			SAE_BN_FREE(&x);
			return MLME_UNSPECIFY_FAIL;
		}

		/* pwd-seed = H(MAX(STA-A-MAC, STA-B-MAC) || MIN(STA-A-MAC, STA-B-MAC),
				base || counter) */
		NdisMoveMemory(msg, base, base_len);
		NdisMoveMemory(msg + base_len, &counter, sizeof(counter));
		msg_len = base_len + sizeof(counter);

		RT_HMAC_SHA256(addrs, sizeof(addrs), msg, msg_len, pwd_seed, sizeof(pwd_seed));
		lsb_pwd_seed = pwd_seed[SHA256_DIGEST_SIZE - 1] & BIT0;

		hex_dump_with_lvl("pwd_seed:", (char *)pwd_seed, sizeof(pwd_seed), SAE_DEBUG_LEVEL);
		/*  z = len(p)
		     pwd-value = KDF-z(pwd-seed, ¡§SAE Hunting and Pecking¡¨, p) */
		KDF(pwd_seed, sizeof(pwd_seed), (UINT8 *)"SAE Hunting and Pecking", 23,
			(UINT8 *)ec_group->prime, ec_group->prime_len,
			pwd_value, pSaeIns->prime_len);

		hex_dump_with_lvl("pwd_value:", (char *)pwd_value, pSaeIns->prime_len, SAE_DEBUG_LEVEL);
		/* pwd-value should be less than prime */
		shift_idx = ec_group->prime_len - (pSaeIns->prime_len);

		if (NdisCmpMemory(pwd_value, ec_group->prime + shift_idx,
						  pSaeIns->prime_len) == 1)
			continue;

		/* x = pwd-value
		    y^2 = x^3 + ax + b */
		SAE_BN_BIN2BI(pwd_value, pSaeIns->prime_len, &x);
		has_y = ecc_point_find_by_x(ec_group_bi, x, &y, (res == NULL));

		if (has_y == FALSE)
			continue;

		if (!res) {
			ecc_point_init(&res);

			if (res == NULL) {
				SAE_BN_FREE(&x);
				SAE_BN_FREE(&y);
				return MLME_UNSPECIFY_FAIL;
			}

			/* if LSB(pwd-seed) = LSB(y)
			  * then PWE = (x, y)
			  * else PWE = (x, p - y)
			  */
			lsb_y = SAE_BN_IS_ODD(y);
			res->x = x;

			if (lsb_pwd_seed == lsb_y)
				res->y = y;
			else {
				res->y = NULL;
				SAE_BN_SUB(ec_group_bi->prime, y, &res->y);
				SAE_BN_FREE(&y);
			}

			SAE_ECC_SET_Z_TO_1(res);

			if (DebugLevel >= DBG_LVL_TRACE) {
				MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF, ("pwe->x\n"));
				SAE_BN_PRINT(res->x);
				MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_OFF, ("pwe->y\n"));
				SAE_BN_PRINT(res->y);
			}
			x = NULL;
		} else {
			SAE_BN_FREE(&y);

			/* base = new-random-number */
			for (i = 0; i < base_len; i++)
				base[i] = RandomByte(pSaeIns->pParentSaeCfg->pAd);
		}
	}

	SAE_BN_FREE(&x);
	pSaeIns->pwe = (VOID *)res;
	return MLME_SUCCESS;
}


USHORT sae_derive_pwe_ffc(
	IN SAE_INSTANCE *pSaeIns)
{
	UCHAR counter = 0;
	UCHAR found = FALSE;
	UCHAR addrs[2 * MAC_ADDR_LEN];
	SAE_BN *pwe = NULL;
	SAE_BN *exp = NULL;
	UCHAR msg[LEN_PSK + 2]; /* sizeof(base)+sizeof(counter) */
	UINT32 msg_len;
	UCHAR pwd_seed[SHA256_DIGEST_SIZE];
	UCHAR *pwd_value = NULL;
	SAE_BN *pwd_value_bi = NULL;
	SAE_BN *tmp_bi = NULL;
	DH_GROUP_INFO *dh_group;
	DH_GROUP_INFO_BI *dh_group_bi;
	UCHAR tmp[1];
	USHORT res;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[5]);
	/* GET_BI_INS_FROM_POOL(exp); */
	/* GET_BI_INS_FROM_POOL(pwd_value_bi); */

	/* Get larger address first */
	if (RTMPCompareMemory(pSaeIns->own_mac, pSaeIns->peer_mac, MAC_ADDR_LEN) == 1) {
		COPY_MAC_ADDR(addrs, pSaeIns->own_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->peer_mac);
	} else {
		COPY_MAC_ADDR(addrs, pSaeIns->peer_mac);
		COPY_MAC_ADDR(addrs + MAC_ADDR_LEN, pSaeIns->own_mac);
	}


	hex_dump_with_lvl("psk:", (char *)pSaeIns->psk, strlen(pSaeIns->psk), SAE_DEBUG_LEVEL);
	dh_group = (DH_GROUP_INFO *)pSaeIns->group_info;
	/* dh_group_bi->prime == pSaeIns->prime, dh_group_bi->order == pSaeIns->order */
	dh_group_bi = (DH_GROUP_INFO_BI *)pSaeIns->group_info_bi;
	SAE_BN_INIT(&exp);
	SAE_BN_INIT(&pwd_value_bi);
	SAE_BN_INIT(&pwe);
	SAE_BN_INIT(&tmp_bi);

	os_alloc_mem(NULL, (UCHAR **)&pwd_value, SAE_MAX_PRIME_LEN);

	for (counter = 1; counter <= 200; counter++) {
		UCHAR shift_idx;
		/* pwd-seed = H(MAX(STA-A-MAC, STA-B-MAC) || MIN(STA-A-MAC, STA-B-MAC),
				password || counter) */
		NdisMoveMemory(msg, pSaeIns->psk, strlen(pSaeIns->psk));
		hex_dump_with_lvl("msg:", (char *)msg, strlen(pSaeIns->psk), SAE_DEBUG_LEVEL);
		NdisMoveMemory(msg + strlen(pSaeIns->psk), &counter, sizeof(counter));
		msg_len = strlen(pSaeIns->psk) + sizeof(counter);
		hex_dump_with_lvl("addr:", (char *)addrs, 2 * MAC_ADDR_LEN, SAE_DEBUG_LEVEL);
		hex_dump_with_lvl("msg:", (char *)msg, msg_len, SAE_DEBUG_LEVEL);
		RT_HMAC_SHA256(addrs, sizeof(addrs), msg, msg_len, pwd_seed, sizeof(pwd_seed));
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_TRACE,
				 ("%s(): z = len(p) = %d\n", __func__, SAE_BN_GET_LEN(pSaeIns->prime)));

		hex_dump_with_lvl("pwd_seed:", (char *)pwd_seed, SHA256_DIGEST_SIZE, SAE_DEBUG_LEVEL);
		hex_dump_with_lvl("prime:", (char *)dh_group->prime, dh_group->prime_len, SAE_DEBUG_LEVEL);
		/*  z = len(p)
		     pwd-value = KDF-z(pwd-seed, ¡§SAE Hunting and Pecking¡¨, p) */
		KDF(pwd_seed, sizeof(pwd_seed), (UINT8 *)"SAE Hunting and Pecking", 23,
			(UINT8 *)dh_group->prime, dh_group->prime_len,
			pwd_value, pSaeIns->prime_len);
		hex_dump_with_lvl("pwd_value:", (char *)pwd_value, pSaeIns->prime_len, SAE_DEBUG_LEVEL);
		/* pwd-value should be less than prime */
		shift_idx = dh_group->prime_len - pSaeIns->prime_len;

		if (NdisCmpMemory(pwd_value, dh_group->prime + shift_idx,
						  dh_group->prime_len) == 1)
			continue;

		/* PWE = pwd-value^(p-1)/r modulo p */
		if (dh_group->safe_prime) {
			/*
			 * r = (p-1)/2 => (p-1)/r = 2
			 */
			tmp[0] = 2;
			SAE_BN_BIN2BI(tmp, sizeof(tmp), &exp);
		} else {
			/* GET_BI_INS_FROM_POOL(tmp_bi); */
			tmp[0] = 1;
			SAE_BN_BIN2BI(tmp, sizeof(tmp), &exp);
			SAE_BN_SUB(dh_group_bi->prime, exp, &tmp_bi); /* ellis: SAE_BN_SUB(A, B, A) need to be fix */
			SAE_BN_MOD(tmp_bi, dh_group_bi->order, &exp);
		}

		SAE_BN_BIN2BI(pwd_value, dh_group->prime_len, &pwd_value_bi);
		SAE_BN_MOD_EXP_MONT(pwd_value_bi, exp, dh_group_bi->prime, &pwe);

		/* if (PWE > 1) => found */
		if (!SAE_BN_IS_ZERO(pwe) && !SAE_BN_IS_ONE(pwe)) {
			found = TRUE;
			break;
		}
	}

	if (found) {
		pSaeIns->pwe = (VOID *) pwe;
		res = MLME_SUCCESS;
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_INFO,
				 ("%s(): Success to derive PWE\n", __func__));
	} else {
		SAE_BN_FREE(&pwe);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): Failed to derive PWE\n", __func__));
		res = MLME_UNSPECIFY_FAIL;
	}
	os_free_mem(pwd_value);
	SAE_BN_RELEASE_BACK_TO_POOL(&exp);
	SAE_BN_RELEASE_BACK_TO_POOL(&pwd_value_bi);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp_bi);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[5]);
	return res;
}



UCHAR sae_derive_k_ecc(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *k)
{
	BIG_INTEGER_EC_POINT *K = NULL;
	BIG_INTEGER_EC_POINT *pwe = (BIG_INTEGER_EC_POINT *) pSaeIns->pwe;
	BIG_INTEGER_EC_POINT *peer_commit_element =
		(BIG_INTEGER_EC_POINT *) pSaeIns->peer_commit_element;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *)pSaeIns->group_info_bi;
	UINT32 len = SAE_MAX_ECC_PRIME_LEN;
	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));
	/*
	 * K = scalar-op(rand, (elem-op(scalar-op(peer-commit-scalar, PWE),
	 *                                        PEER-COMMIT-ELEMENT)))
	 */
	ECC_POINT_MUL(pwe, pSaeIns->peer_commit_scalar, ec_group_bi, &K);
	ecc_point_add(K, peer_commit_element, ec_group_bi, &K);
	ECC_POINT_MUL(K, pSaeIns->sae_rand, ec_group_bi, &K);

	SAE_ECC_3D_to_2D(ec_group_bi, K);

	/* If K is point-at-infinity, reject. */
	if (K == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): K should not be point-at-infinity\n", __func__));
		return FALSE;
	}

	/* k = F(K) (= x coordinate) */
	SAE_BN_BI2BIN_WITH_PAD(K->x, k, &len, pSaeIns->prime_len);
	hex_dump_with_lvl("k:", (char *)k, len, SAE_DEBUG_LEVEL2);
	ecc_point_free(&K);
	return TRUE;
}

UCHAR sae_derive_k_ffc(
	IN SAE_INSTANCE *pSaeIns,
	OUT UCHAR *k)
{
	SAE_BN *K = NULL;
	SAE_BN *tmp = NULL;
	SAE_BN *tmp2 = NULL;
	SAE_BN *pwe = NULL;
	UINT32 len = SAE_MAX_PRIME_LEN;
	UCHAR res = TRUE;

	MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_LOUD,
			 ("==> %s()\n", __func__));

	POOL_COUNTER_CHECK_BEGIN(sae_expected_cnt[6]);
	/* GET_BI_INS_FROM_POOL(tmp); */
	/* GET_BI_INS_FROM_POOL(tmp2); */
	/* GET_BI_INS_FROM_POOL(K); */

	/*
	 * K = scalar-op(rand, (elem-op(scalar-op(peer-commit-scalar, PWE),
	 *                                        PEER-COMMIT-ELEMENT)))
	 * the K should be scalar-op((rand + peer-rand) modulo r, pwe)
	 */
	pwe = (SAE_BN *) pSaeIns->pwe;
	SAE_BN_INIT(&K);
	SAE_BN_INIT(&tmp);
	SAE_BN_INIT(&tmp2);
	SAE_BN_MOD_EXP_MONT(pwe, pSaeIns->peer_commit_scalar, pSaeIns->prime, &tmp);
	SAE_BN_MOD_MUL(tmp, (SAE_BN *)pSaeIns->peer_commit_element, pSaeIns->prime, &tmp2);
	SAE_BN_MOD_EXP_MONT(tmp2, pSaeIns->sae_rand, pSaeIns->prime, &K);

	/* If K is identity element (one), reject. */
	if (SAE_BN_IS_ONE(K)) {
		SAE_BN_FREE(&K);
		MTWF_LOG(DBG_CAT_SEC, CATSEC_SAE, DBG_LVL_ERROR,
				 ("%s(): K should not be one\n", __func__));
		res = FALSE;
		goto Free;
	}

	/* k = F(K) (= x coordinate) */
	SAE_BN_BI2BIN_WITH_PAD(K, k, &len, pSaeIns->prime_len);
	hex_dump_with_lvl("k:", (char *)k, len, SAE_DEBUG_LEVEL);
Free:
	SAE_BN_RELEASE_BACK_TO_POOL(&K);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp);
	SAE_BN_RELEASE_BACK_TO_POOL(&tmp2);
	POOL_COUNTER_CHECK_END(sae_expected_cnt[6]);
	return TRUE;
}
#endif /* DOT11_SAE_SUPPORT */
