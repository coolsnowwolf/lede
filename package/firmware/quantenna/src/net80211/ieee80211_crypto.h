/*-
 * Copyright (c) 2001 Atsushi Onoe
 * Copyright (c) 2002-2005 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * Alternatively, this software may be distributed under the terms of the
 * GNU General Public License ("GPL") version 2 as published by the Free
 * Software Foundation.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: ieee80211_crypto.h 1441 2006-02-06 16:03:21Z mrenzmann $
 */
#ifndef _NET80211_IEEE80211_CRYPTO_H_
#define _NET80211_IEEE80211_CRYPTO_H_

/*
 * 802.11 protocol crypto-related definitions.
 */
#define	IEEE80211_KEYBUF_SIZE		16
#define	IEEE80211_MICBUF_SIZE		(8 + 8)	/* space for both tx+rx keys */

#define IEEE80211_QOS_TID_MAX		16
#define IEEE80211_RSC_NON_QOS		(IEEE80211_QOS_TID_MAX)
#define IEEE80211_RSC_ROBUST_MGMT	(IEEE80211_QOS_TID_MAX + 1)
#define IEEE80211_RSC_MAX		(IEEE80211_QOS_TID_MAX + 2)

/*
 * Old WEP-style key.  Deprecated.
 */
struct ieee80211_wepkey {
	u_int wk_len;				/* key length in bytes */
	u_int8_t wk_key[IEEE80211_KEYBUF_SIZE];
};

struct ieee80211_cipher;

/*
 * Crypto key state.  There is sufficient room for all supported
 * ciphers (see below).  The underlying ciphers are handled
 * separately through loadable cipher modules that register with
 * the generic crypto support.  A key has a reference to an instance
 * of the cipher; any per-key state is hung off wk_private by the
 * cipher when it is attached.  Ciphers are automatically called
 * to detach and cleanup any such state when the key is deleted.
 *
 * The generic crypto support handles encap/decap of cipher-related
 * frame contents for both hardware- and software-based implementations.
 * A key requiring software crypto support is automatically flagged and
 * the cipher is expected to honor this and do the necessary work.
 * Ciphers such as TKIP may also support mixed hardware/software
 * encrypt/decrypt and MIC processing.
 *
 * Note: This definition must be the same as qtn_key.
 */
struct ieee80211_key {
	u_int8_t wk_keylen;		/* key length in bytes */
	u_int8_t wk_flags;
#define	IEEE80211_KEY_XMIT	0x01	/* key used for xmit */
#define	IEEE80211_KEY_RECV	0x02	/* key used for recv */
#define	IEEE80211_KEY_GROUP	0x04	/* key used for WPA group operation */
#define	IEEE80211_KEY_SWCRYPT	0x10	/* host-based encrypt/decrypt */
#define	IEEE80211_KEY_SWMIC	0x20	/* host-based enmic/demic */
#define IEEE80211_KEY_VLANGROUP	0x40	/* VLAN group key */
	u_int16_t wk_keyix;		/* key index */
	u_int8_t wk_key[IEEE80211_KEYBUF_SIZE+IEEE80211_MICBUF_SIZE];
#define	wk_txmic	wk_key + IEEE80211_KEYBUF_SIZE + 0
#define	wk_rxmic	wk_key + IEEE80211_KEYBUF_SIZE + 8
	u_int64_t wk_keyrsc[IEEE80211_RSC_MAX];	/* key receive sequence counter */
	u_int64_t wk_keytsc;			/* key transmit sequence counter */
	u_int32_t wk_ciphertype;
	const struct ieee80211_cipher *wk_cipher;
	void *wk_private;				/* private cipher state */
};
#define	IEEE80211_KEY_COMMON				/* common flags passed in by apps */\
	(IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV | IEEE80211_KEY_GROUP)

/*
 * NB: these values are ordered carefully; there are lots of
 * of implications in any reordering.  In particular beware
 * that 4 is not used to avoid conflicting with IEEE80211_F_PRIVACY.
 */
#define	IEEE80211_CIPHER_WEP		0
#define	IEEE80211_CIPHER_TKIP		1
#define	IEEE80211_CIPHER_AES_OCB	2
#define	IEEE80211_CIPHER_AES_CCM	3
#define IEEE80211_CIPHER_AES_CMAC	4
#define	IEEE80211_CIPHER_CKIP		5
#define	IEEE80211_CIPHER_NONE		6	/* pseudo value */

#define	IEEE80211_CIPHER_MAX		(IEEE80211_CIPHER_NONE+1)

#define	IEEE80211_KEYIX_NONE		((u_int8_t) - 1)

#if defined(__KERNEL__) || defined(_KERNEL)

struct ieee80211com;
struct ieee80211vap;
struct ieee80211_node;
struct sk_buff;

void ieee80211_crypto_attach(struct ieee80211com *);
void ieee80211_crypto_detach(struct ieee80211com *);
void ieee80211_crypto_vattach(struct ieee80211vap *);
void ieee80211_crypto_vdetach(struct ieee80211vap *);
int ieee80211_crypto_newkey(struct ieee80211vap *, int, int,
	struct ieee80211_key *);
int ieee80211_crypto_delkey(struct ieee80211vap *, struct ieee80211_key *,
	struct ieee80211_node *);
int ieee80211_crypto_setkey(struct ieee80211vap *, struct ieee80211_key *,
	const u_int8_t macaddr[IEEE80211_ADDR_LEN], struct ieee80211_node *);
void ieee80211_crypto_delglobalkeys(struct ieee80211vap *);

/*
 * Template for a supported cipher.  Ciphers register with the
 * crypto code and are typically loaded as separate modules
 * (the null cipher is always present).
 * XXX may need refcnts
 */
struct ieee80211_cipher {
	const char *ic_name;		/* printable name */
	u_int ic_cipher;			/* IEEE80211_CIPHER_* */
	u_int ic_header;			/* size of privacy header (bytes) */
	u_int ic_trailer;		/* size of privacy trailer (bytes) */
	u_int ic_miclen;			/* size of mic trailer (bytes) */
	void *(*ic_attach)(struct ieee80211vap *, struct ieee80211_key *);
	void (*ic_detach)(struct ieee80211_key *);
	int (*ic_setkey)(struct ieee80211_key *);
	int (*ic_encap)(struct ieee80211_key *, struct sk_buff *, u_int8_t);
	int (*ic_decap)(struct ieee80211_key *, struct sk_buff *, int);
	int (*ic_enmic)(struct ieee80211_key *, struct sk_buff *, int);
	int (*ic_demic)(struct ieee80211_key *, struct sk_buff *, int);
};
extern const struct ieee80211_cipher ieee80211_cipher_none;

void ieee80211_crypto_register(const struct ieee80211_cipher *);
void ieee80211_crypto_unregister(const struct ieee80211_cipher *);
int ieee80211_crypto_available(u_int);

struct ieee80211_key *ieee80211_crypto_encap(struct ieee80211_node *,
	struct sk_buff *);
struct ieee80211_key *ieee80211_crypto_decap(struct ieee80211_node *,
	struct sk_buff *, int);

/*
 * Check and remove any MIC.
 */
static __inline int
ieee80211_crypto_demic(struct ieee80211vap *vap, struct ieee80211_key *k,
	struct sk_buff *skb, int hdrlen)
{
	const struct ieee80211_cipher *cip = k->wk_cipher;
	return (cip->ic_miclen > 0 ? cip->ic_demic(k, skb, hdrlen) : 1);
}

/*
 * Add any MIC.
 */
static __inline int
ieee80211_crypto_enmic(struct ieee80211vap *vap, struct ieee80211_key *k,
	struct sk_buff *skb, int force)
{
	const struct ieee80211_cipher *cip = k->wk_cipher;
	return (cip->ic_miclen > 0 ? cip->ic_enmic(k, skb, force) : 1);
}

/* 
 * Reset key state to an unused state.  The crypto
 * key allocation mechanism ensures other state (e.g.
 * key data) is properly setup before a key is used.
 */
static __inline void
ieee80211_crypto_resetkey(struct ieee80211vap *vap, struct ieee80211_key *k,
	u_int16_t ix)
{
	k->wk_cipher = &ieee80211_cipher_none;;
	k->wk_private = k->wk_cipher->ic_attach(vap, k);
	k->wk_keyix = ix;
	k->wk_flags = IEEE80211_KEY_XMIT | IEEE80211_KEY_RECV;
}

/*
 * Crypto-related notification methods.
 */
void ieee80211_notify_replay_failure(struct ieee80211vap *,
	const struct ieee80211_frame *, const struct ieee80211_key *,
	u_int64_t rsc);
void ieee80211_notify_michael_failure(struct ieee80211vap *,
	const struct ieee80211_frame *, u_int keyix);
#endif /* defined(__KERNEL__) || defined(_KERNEL) */
#endif /* _NET80211_IEEE80211_CRYPTO_H_ */
