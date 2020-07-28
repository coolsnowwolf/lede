*************************************************************************
*	Here is the files required for wireless configuration		*
*************************************************************************
l1profile.dat
	This file locate at SDK/users/luci.
	And it should be installed in root file system at /etc/wireless.

RT2860.dat
	For configuration of 7622 chip. Refer to /etc/wireless/l1profile.dat,
	it should be generated at /etc/Wireless/RT2860/RT2860.dat due to INDEX0_profile_path.

iNIC_ap.dat
	For configuration of 1st 7615 chip. Refer to /etc/wireless/l1profile.dat,
        it should be generated at /etc/Wireless/iNIC/iNIC_ap.dat due to INDEX1_profile_path.

mt7615_3.da
	For configuration of 2nd 7615 chip. Refer to /etc/wireless/l1profile.dat,
        it should be generated at /etc/wireless/mt7615/mt7615_3.dat due to INDEX2_profile_path.
