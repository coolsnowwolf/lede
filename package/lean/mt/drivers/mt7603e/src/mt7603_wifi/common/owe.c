#ifdef CONFIG_OWE_SUPPORT

#include "rt_config.h"
#include "ecc.h"
#include "crypt_sha2.h"

UCHAR OWE_TRANS_OUI[] = {0x50, 0x6f, 0x9a, 0x1c};

#ifdef APCLI_OWE_SUPPORT
UCHAR apcli_owe_supp_groups[APCLI_MAX_SUPPORTED_OWE_GROUPS] = {19, 20};
#endif

static inline UINT16 GET_LE16(const UCHAR *a)
{
	return (a[1] << 8) | a[0];
}

static UINT owe_process_peer_pubkey(OWE_INFO *owe, UCHAR *peer_pub_key, UCHAR pubkey_len)
{
	SAE_BN *peer_pubkey_bn = NULL;
	BIG_INTEGER_EC_POINT *peer_pub_point = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *)owe->group_info_bi;
	UINT ret = 0;

	if (owe->peer_pub_key != NULL)
		ecc_point_free((BIG_INTEGER_EC_POINT **)&owe->peer_pub_key);

	ecc_point_init(&peer_pub_point);
	SAE_BN_BIN2BI((UINT8 *)peer_pub_key,
				pubkey_len,
				&peer_pubkey_bn);

	SAE_BN_COPY(peer_pubkey_bn, &peer_pub_point->x);

	while (peer_pub_point->y == NULL)
		ecc_point_find_by_x(ec_group_bi, peer_pub_point->x, &peer_pub_point->y, TRUE);

	if (ecc_point_is_on_curve(ec_group_bi, peer_pub_point) == FALSE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_OFF, ("%s, point is not on curve\n", __func__));
		goto err;
	}
	SAE_ECC_SET_Z_TO_1(peer_pub_point);
	owe->peer_pub_key = (VOID *)peer_pub_point;

	ret = 1;

err:
	if (peer_pubkey_bn)
		SAE_BN_FREE(&peer_pubkey_bn);

	return ret;
}

INT owe_calculate_secret(OWE_INFO *owe, SAE_BN **secret)
{
	BIG_INTEGER_EC_POINT *peer_pub = (BIG_INTEGER_EC_POINT *)owe->peer_pub_key;
	SAE_BN *priv_key = owe->priv_key;
	BIG_INTEGER_EC_POINT *secret_point = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = (EC_GROUP_INFO_BI *)owe->group_info_bi;
	INT ret = 0;

	ECC_POINT_MUL(peer_pub, priv_key, ec_group_bi, &secret_point);
	SAE_ECC_3D_to_2D(ec_group_bi, secret_point);

	if (secret_point == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
			 ("==> %s(), can't output the common secret point\n", __func__));
		return ret;
	}

	SAE_BN_COPY(secret_point->x, secret);

	if (secret_point)
		ecc_point_free(&secret_point);
	ret = 1;

	return ret;
}

static BIG_INTEGER_EC_POINT *get_owe_pub_key(OWE_INFO *owe)
{
	return (BIG_INTEGER_EC_POINT *)owe->pub_key;
}

static VOID owe_calculate_pmkid(OWE_INFO *owe,
				SAE_BN * my_pubkey,
				UCHAR *peer_pub,
				UCHAR type,
				UCHAR peer_pub_length,
				UINT16 group,
				UCHAR *pmkid)
{
	EC_GROUP_INFO *ec_group = NULL;
	UCHAR *material = NULL;
	UINT material_length = 0;
	const UCHAR *_addr[2];
	INT _len[2];

	ec_group = (EC_GROUP_INFO *)owe->group_info;

	if (os_alloc_mem(NULL, (UCHAR **)&material, ec_group->prime_len) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), alloc buf for material failed...\n", __func__));
		goto err;
	}

	/* Truncate-128(hash(C | A)) */
	if (type == SUBTYPE_ASSOC_REQ) {
		/*we are processing ecdh from (re)assoc req*/
		_addr[0] = peer_pub;
		_len[0] = peer_pub_length;
		SAE_BN_BI2BIN_WITH_PAD(my_pubkey, material, &material_length, ec_group->prime_len);
		_addr[1] = material;
		_len[1] = material_length;
	} else {
		SAE_BN_BI2BIN_WITH_PAD(my_pubkey, material, &material_length, ec_group->prime_len);
		_addr[0] = material;
		_len[0] = material_length;
		_addr[1] = peer_pub;
		_len[1] = peer_pub_length;
	}

	switch (group) {
	case ECDH_GROUP_521:
		break;
	case ECDH_GROUP_384:
		rt_sha384_vector(2, _addr, _len, pmkid);
		break;
	case ECDH_GROUP_256:
	default:
		rt_sha256_vector(2, _addr, _len, pmkid);
	}

	hex_dump("OWE PMKID:", pmkid, LEN_PMKID);

	if (owe->pmkid)
		os_free_mem(NULL, owe->pmkid);

	if (os_alloc_mem(NULL, (UCHAR **)&owe->pmkid, LEN_PMKID) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), alloc pmkid failed...\n", __func__));
		goto err;
	}
	NdisMoveMemory(owe->pmkid, pmkid, LEN_PMKID);

err:
	if (material)
		os_free_mem(NULL, material);
}

INT process_ecdh_element(
	struct _RTMP_ADAPTER *ad,
	VOID *pEntry_v,
	EXT_ECDH_PARAMETER_IE *ext_ie_ptr,
	UCHAR ie_len,
	UCHAR type,
	BOOLEAN update_only_grp_info)
{
    MAC_TABLE_ENTRY *entry = (MAC_TABLE_ENTRY *)pEntry_v;
	OWE_INFO *owe = &entry->owe;
	UINT16 peer_group = 0;
	UCHAR remain_len = 0;
	UCHAR *pos = NULL;
	UCHAR *peer_pub = NULL;
	SAE_BN *secret = NULL;
	UCHAR *hkey = NULL;
	BIG_INTEGER_EC_POINT *my_pubkey = NULL;
	EC_GROUP_INFO *ec_group = NULL;
	UINT hkey_length = 0;
	UCHAR *sec_buf = NULL;
	UINT sec_length = 0;
	UCHAR prk[SHA512_DIGEST_SIZE], pmkid[SHA512_DIGEST_SIZE];
	UCHAR hash_len = 0;

	if ((ext_ie_ptr->ext_ie_id == 0) && (ext_ie_ptr->length == 0))
		return MLME_SUCCESS;

	remain_len = ie_len - 1;/*the length which starts from group field directly.*/
	pos = (UCHAR *)&ext_ie_ptr->group;/*start from group field directly.*/

	peer_group = GET_LE16((UCHAR *)&ext_ie_ptr->group);
	if ((peer_group != ECDH_GROUP_256) && (peer_group != ECDH_GROUP_384) && (peer_group != ECDH_GROUP_521))
		owe->last_try_group = 0;
	else {
		owe->last_try_group = peer_group;

		switch (peer_group) {
		case ECDH_GROUP_521:
			entry->key_deri_alg = SEC_KEY_DERI_SHA512;
			return MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED;
		case ECDH_GROUP_384:
			entry->key_deri_alg = SEC_KEY_DERI_SHA384;
			return MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED;
		case ECDH_GROUP_256:
		default:
			entry->key_deri_alg = SEC_KEY_DERI_SHA256;
		}
	}

	if (update_only_grp_info == TRUE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_TRACE,
				 ("==> %s(), Update group info :%d\n", __func__, entry->key_deri_alg));
		return MLME_SUCCESS;
	}

	MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_TRACE,
			 ("==> %s(), peer_group:%d\n", __func__, peer_group));

	/*if we cannot support the group, skip the further steps.*/
	if (owe->last_try_group == 0)
		return MLME_FINITE_CYCLIC_GROUP_NOT_SUPPORTED;

	pos =  pos + sizeof(peer_group);
	remain_len = remain_len - sizeof(peer_group);

	if (init_owe_group(owe, peer_group) == 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
			 ("==> %s(), init_owe_group failed. shall not happen!\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}

	if (os_alloc_mem(NULL, (UCHAR **)&peer_pub, remain_len) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), alloc buf for peer_pub failed...\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}
	NdisMoveMemory(peer_pub, pos, remain_len);
	if (owe_process_peer_pubkey(owe, peer_pub, remain_len) == 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), owe_process_peer_pubkey failed...\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}

	SAE_BN_INIT(&secret);
	if (owe_calculate_secret(owe, &secret) == 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), owe_calculate_secret failed...\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}

	ec_group = (EC_GROUP_INFO *)owe->group_info;
	if (os_alloc_mem(NULL, (UCHAR **)&sec_buf, ec_group->prime_len) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), alloc buf for hkey failed...\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}
	SAE_BN_BI2BIN_WITH_PAD(secret, sec_buf, &sec_length, ec_group->prime_len);

	if (os_alloc_mem(NULL, (UCHAR **)&hkey, ec_group->prime_len + remain_len + 2) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), alloc buf for hkey failed...\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}

	my_pubkey = get_owe_pub_key(owe);
	if (my_pubkey == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), get own pub failed, shall not happen...\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}

	owe_calculate_pmkid(owe, my_pubkey->x, peer_pub, type, remain_len, peer_group, pmkid);

	/* C | A | group */
	if (type == SUBTYPE_ASSOC_REQ) {
		/*we are processing ecdh from (re)assoc req*/
		NdisMoveMemory(hkey, peer_pub, remain_len);
		SAE_BN_BI2BIN_WITH_PAD(my_pubkey->x, hkey + remain_len, &hkey_length, ec_group->prime_len);
		hkey_length += remain_len;
		NdisMoveMemory(hkey+hkey_length, &peer_group, sizeof(peer_group));
		hkey_length += sizeof(peer_group);
	} else if (type == SUBTYPE_ASSOC_RSP) {
		SAE_BN_BI2BIN_WITH_PAD(my_pubkey->x, hkey + hkey_length, &hkey_length, ec_group->prime_len);
		NdisMoveMemory(hkey + hkey_length, peer_pub, remain_len);
		hkey_length += remain_len;
		NdisMoveMemory(hkey + hkey_length, &peer_group, sizeof(peer_group));
		hkey_length += sizeof(peer_group);
	} else {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), parsing wrong type, shall not happen\n", __func__));
		return MLME_UNSPECIFY_FAIL;
	}
	if (peer_group == ECDH_GROUP_256) {
		hash_len = SHA256_DIGEST_SIZE;
		RT_HMAC_SHA256(hkey, hkey_length, sec_buf, sec_length, prk, hash_len);
		/*FIXME: is the length of PMK that to use LEN_PMK here is correct?*/
		HKDF_expand_sha256(prk,
				   hash_len,
				   "OWE Key Generation",
				   18,
				   entry->PMK,
				   LEN_PMK);
	} else if (peer_group == ECDH_GROUP_384) {
		hash_len = SHA384_DIGEST_SIZE;
		RT_HMAC_SHA384(hkey, hkey_length, sec_buf, sec_length, prk, hash_len);
		HKDF_expand_sha384(prk,
				   hash_len,
				   "OWE Key Generation",
				   18,
				   entry->PMK,
				   LEN_PMK_SHA384);
	}
	hex_dump("OWE PRK:", prk, hash_len);
	hex_dump("OWE PMK:", entry->PMK, hash_len);

	if (sec_buf)
		os_free_mem(NULL, sec_buf);
	if (hkey)
		os_free_mem(NULL, hkey);

	if (secret)
		SAE_BN_FREE(&secret);
	if (peer_pub)
		os_free_mem(NULL, peer_pub);

	return MLME_SUCCESS;
}

INT init_owe_group(OWE_INFO *owe, UCHAR group)
{
	BIG_INTEGER_EC_POINT *generator = NULL;
	EC_GROUP_INFO *ec_group = get_ecc_group_info(group);
	EC_GROUP_INFO_BI *ec_group_bi = get_ecc_group_info_bi(group);
	INT ret = 0;

	if (owe->inited == TRUE)
		return 1;

	if (ec_group == NULL || ec_group_bi == NULL) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
			 ("==> %s(), get ec_group failed. shall not happen!\n", __func__));
		goto err;
	}

	if (owe->group_info == NULL)
		owe->group_info = (VOID *)ec_group;
	if (owe->group_info_bi == NULL)
		owe->group_info_bi = (VOID *)ec_group_bi;

	ecc_point_init(&generator);
	SAE_BN_COPY(ec_group_bi->gx, &generator->x);
	SAE_BN_COPY(ec_group_bi->gy, &generator->y);
	SAE_ECC_SET_Z_TO_1(generator);

	owe->generator = (VOID *)generator;
	if (ecc_gen_key(ec_group, ec_group_bi, &owe->priv_key, generator, &owe->pub_key) == 0) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
			 ("==> %s(), ecc_gen_key failed...\n", __func__));
		goto err;
	}

	owe->inited = TRUE;

	ret = 1;
err:
	return ret;
}

INT build_owe_dh_ie(struct _RTMP_ADAPTER *ad,
		  VOID *pEntry_v,
		  UCHAR *buf,
		  UCHAR group)
{
    MAC_TABLE_ENTRY *pentry = (MAC_TABLE_ENTRY *)pEntry_v;
	OWE_INFO *owe = &pentry->owe;

	EC_GROUP_INFO *ec_group = NULL;
	EC_GROUP_INFO_BI *ec_group_bi = NULL;
	BIG_INTEGER_EC_POINT *generator = NULL;
	BIG_INTEGER_EC_POINT *my_pub_key = NULL;
	UCHAR *pub_buf = NULL;
	INT extend_length = 0;
	UCHAR ie = IE_WLAN_EXTENSION;
	UCHAR ie_len = 0;
	UCHAR ext_ie = IE_EXTENSION_ID_ECDH;
	UINT16 g = group;
	UINT pub_buf_length = 0;

	ec_group = (EC_GROUP_INFO *)owe->group_info;
	ec_group_bi = (EC_GROUP_INFO_BI *)owe->group_info_bi;

	generator = (BIG_INTEGER_EC_POINT *)owe->generator;
	my_pub_key = (BIG_INTEGER_EC_POINT *)owe->pub_key;

	if (os_alloc_mem(NULL, (UCHAR **)&pub_buf, ec_group->prime_len) == NDIS_STATUS_FAILURE) {
		MTWF_LOG(DBG_CAT_SEC, CATSEC_OWE, DBG_LVL_ERROR,
				 ("==> %s(), ecc_gen_key failed...\n", __func__));
		goto err;
	}
	SAE_BN_BI2BIN_WITH_PAD(my_pub_key->x, pub_buf, &pub_buf_length, ec_group->prime_len);

	NdisMoveMemory(buf + extend_length, &ie, sizeof(ie));
	extend_length += 1;

	ie_len = 1 + 2 + ec_group->prime_len;
	NdisMoveMemory(buf + extend_length, &ie_len, sizeof(ie_len));
	extend_length += 1;

	NdisMoveMemory(buf + extend_length, &ext_ie, sizeof(ext_ie));
	extend_length += 1;

	NdisMoveMemory(buf + extend_length, &g, sizeof(g));
	extend_length += 2;

	NdisMoveMemory(buf + extend_length, pub_buf, ec_group->prime_len);
	extend_length += ec_group->prime_len;

	hex_dump("ECDH parameter:", (UCHAR *)buf, extend_length);

err:
	if (pub_buf)
		os_free_mem(NULL, pub_buf);

	return extend_length;
}

INT deinit_owe_group(OWE_INFO *owe)
{
	if (owe->peer_pub_key != NULL) {
		ecc_point_free((BIG_INTEGER_EC_POINT **)&owe->peer_pub_key);
		owe->peer_pub_key = NULL;
	}
	if (owe->pub_key != NULL) {
		ecc_point_free((BIG_INTEGER_EC_POINT **)&owe->pub_key);
		owe->pub_key = NULL;
	}
	if (owe->generator != NULL) {
		ecc_point_free((BIG_INTEGER_EC_POINT **)&owe->generator);
		owe->generator = NULL;
	}
	if (owe->priv_key != NULL) {
		SAE_BN_FREE(&owe->priv_key);
		owe->priv_key = NULL;
	}

	if (owe->pmkid != NULL) {
		os_free_mem(NULL, owe->pmkid);
		owe->pmkid = NULL;
	}

	/*TODO: deinit group_info  itself. need to discuss with E*/
	owe->group_info = NULL;
	owe->group_info_bi = NULL;

	owe->last_try_group = 0;
	owe->inited = FALSE;

	return 0;
}

#ifdef CONFIG_AP_SUPPORT
USHORT owe_pmkid_ecdh_process(RTMP_ADAPTER *pAd,
			      VOID *pEntry_v,
			      UCHAR *rsn_ie,
			      UCHAR rsn_ie_len,
			      EXT_ECDH_PARAMETER_IE *ecdh_ie,
			      UCHAR ecdh_ie_length,
			      PUINT8 pPmkid,
			      UINT8 *pmkid_count,
			      UCHAR type)
{
	BOOLEAN need_update_grp_info = FALSE;
	BOOLEAN need_process_ecdh_ie = FALSE;
	INT CacheIdx;/* Key cache */
	USHORT ret = MLME_SUCCESS;
    MAC_TABLE_ENTRY *pEntry = (MAC_TABLE_ENTRY *)pEntry_v;

	pPmkid = WPA_ExtractSuiteFromRSNIE(rsn_ie,
					   rsn_ie_len,
					   PMKID_LIST,
					   pmkid_count);
	if (pPmkid != NULL) {
		CacheIdx = RTMPSearchPMKIDCache(pAd,
						pEntry->func_tb_idx,
						pEntry->Addr);
		if ((CacheIdx == -1) ||
		   ((RTMPEqualMemory(pPmkid,
				     &pAd->ApCfg.MBSSID[pEntry->func_tb_idx].PMKIDCache.BSSIDInfo[CacheIdx].PMKID,
				LEN_PMKID)) == 0)) {
			MTWF_LOG(DBG_CAT_AP, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s: no OWE PMKID, do normal ECDH procedure\n",
					__func__));
			need_process_ecdh_ie = TRUE;
		} else {
				store_pmkid_cache_in_entry(pAd, pEntry, CacheIdx);
			need_update_grp_info = TRUE;
		}
	} else
		need_process_ecdh_ie = TRUE;

	if (need_process_ecdh_ie == TRUE || (need_update_grp_info == TRUE)) {
		ret = process_ecdh_element(pAd,
						pEntry,
						ecdh_ie,
						ecdh_ie_length,
						type,
						need_update_grp_info);
	}
	return ret;
}
#endif

/*parse from bssid field directly*/
BOOLEAN extract_pair_owe_bss_info(UCHAR *owe_vendor_ie,
				  UCHAR owe_vendor_ie_len,
				  UCHAR *pair_bssid,
				  UCHAR *pair_ssid,
				  UCHAR *pair_ssid_len,
				  UCHAR *pair_band,
				  UCHAR *pair_ch)
{
	BOOLEAN ret = TRUE;
	BOOLEAN has_band_ch_info = FALSE;
	UCHAR local_ssid_len = *(owe_vendor_ie + MAC_ADDR_LEN);
	UCHAR ssid_field_len = sizeof(UCHAR);
	UCHAR at_least_length = local_ssid_len + MAC_ADDR_LEN + ssid_field_len;
	UCHAR *pos = owe_vendor_ie;

	/*Sanity check length information*/
	if (owe_vendor_ie_len < (local_ssid_len + MAC_ADDR_LEN)) {
		ret = FALSE;
		goto end;
	} else if (owe_vendor_ie_len > at_least_length)
		has_band_ch_info = TRUE;/*remain length */

	NdisMoveMemory(pair_bssid, pos, MAC_ADDR_LEN);
	pos = pos + MAC_ADDR_LEN + ssid_field_len;
	NdisMoveMemory(pair_ssid, pos, local_ssid_len);

	*pair_ssid_len = local_ssid_len;

	if (has_band_ch_info == TRUE) {
		pos = pos + local_ssid_len;/*parse ch info field*/
		*pair_band = *pos;
		/*TODO: sanity check of Operating Class and CH???? */
		pos = pos + sizeof(UCHAR);
		*pair_ch = *pos;
	}

end:
	return ret;
}

#ifdef APCLI_OWE_SUPPORT
void wext_send_owe_trans_chan_event(PNET_DEV net_dev,
											UCHAR event_id,
											UCHAR *pair_bssid,
											UCHAR *pair_ssid,
											UCHAR *pair_ssid_len,
											UCHAR *pair_band,
											UCHAR *pair_ch)
{
	struct owe_trans_channel_change_info *owe_tran_ch_data = NULL;
	struct owe_event *event_data = NULL;
	UINT16 buflen = 0;
	char *buf = NULL;


	buflen = sizeof(struct owe_event) + sizeof(struct owe_trans_channel_change_info);
	os_alloc_mem(NULL, (UCHAR **)&buf, buflen);
	NdisZeroMemory(buf, buflen);

	event_data = (struct owe_event *)buf;
	event_data->event_id = event_id;

	event_data->event_len = sizeof(*owe_tran_ch_data);
	owe_tran_ch_data = (struct owe_trans_channel_change_info *)event_data->event_body;

	NdisCopyMemory(owe_tran_ch_data->ifname, RtmpOsGetNetDevName(net_dev), IFNAMSIZ);

	if (pair_bssid)
		memcpy(owe_tran_ch_data->pair_bssid, pair_bssid, 6);
	if (pair_ssid)
		memcpy(owe_tran_ch_data->pair_ssid, pair_ssid, *pair_ssid_len);

	if (pair_ssid_len)
		owe_tran_ch_data->pair_ssid_len = *pair_ssid_len;

	if (pair_band && (*pair_band != 0))
		owe_tran_ch_data->pair_band = *pair_band;

	if (pair_ch && (*pair_ch != 0))
		owe_tran_ch_data->pair_ch = *pair_ch;


	RtmpOSWrielessEventSend(net_dev, RT_WLAN_EVENT_CUSTOM,
							OID_802_11_OWE_TRANS_EVENT, NULL, (PUCHAR)buf, buflen);
	os_free_mem(NULL, buf);
}

#endif



#endif /*CONFIG_OWE_SUPPORT*/
