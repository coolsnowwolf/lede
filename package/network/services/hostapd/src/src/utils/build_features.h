#ifndef BUILD_FEATURES_H
#define BUILD_FEATURES_H

static inline int has_feature(const char *feat)
{
#if defined(IEEE8021X_EAPOL) || (defined(HOSTAPD) && !defined(CONFIG_NO_RADIUS))
	if (!strcmp(feat, "eap"))
		return 1;
#endif
#ifdef CONFIG_IEEE80211N
	if (!strcmp(feat, "11n"))
		return 1;
#endif
#ifdef CONFIG_IEEE80211AC
	if (!strcmp(feat, "11ac"))
		return 1;
#endif
#ifdef CONFIG_IEEE80211AX
	if (!strcmp(feat, "11ax"))
		return 1;
#endif
#ifdef CONFIG_IEEE80211R
	if (!strcmp(feat, "11r"))
		return 1;
#endif
#ifdef CONFIG_IEEE80211W
	if (!strcmp(feat, "11w"))
		return 1;
#endif
#ifdef CONFIG_ACS
	if (!strcmp(feat, "acs"))
		return 1;
#endif
#ifdef CONFIG_SAE
	if (!strcmp(feat, "sae"))
		return 1;
#endif
#ifdef CONFIG_OWE
	if (!strcmp(feat, "owe"))
		return 1;
#endif
#ifdef CONFIG_SUITEB192
	if (!strcmp(feat, "suiteb192"))
		return 1;
#endif
#ifdef CONFIG_WEP
	if (!strcmp(feat, "wep"))
		return 1;
#endif
	return 0;
}

#endif /* BUILD_FEATURES_H */
