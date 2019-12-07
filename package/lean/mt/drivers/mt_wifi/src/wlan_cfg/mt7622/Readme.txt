*************************************************************************
*	Here is the files required for wireless configuration		*
*************************************************************************
Step 1: Select product configuration
	- make menuconfig
	- MTK Properties--->Drivers--->wifi-l1profile
	- Reference l1profile_MT7622_MT7615_MT7615.dat or l1profile_MT7622D_MT7615A.dat

Step 2: Select wifi default configuration
	[MT7622+MT7615+MT7615]
	- Copy MT7622_ap.dat, MT7615_1_ap.dat, and MT7615_2_ap.dat to openwrt/lede/package/mtk/drivers/mt_wifi/files
	- Replace mt7622.1.dat, mt7615.1.dat, and mt7615.2.dat
		
	[MT7622D+MT7615A]
	- Copy MT7622D_ap.dat, mt7615a.b0.5g.dat, and mt7615a.b1.5g.dat  to openwrt/lede/package/mtk/drivers/mt_wifi/files
	- Replace mt7622.1.dat, mt7615a.b0.5g.dat, and mt7615a.b1.5g.dat
