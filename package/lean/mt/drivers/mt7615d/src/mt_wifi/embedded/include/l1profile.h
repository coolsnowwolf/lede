#ifdef DBDC_MODE
UCHAR l1profile_default[] = {
"Default\n\
INDEX0=MT7615\n\
INDEX0_profile_path=/tmp/profiles/mt_dbdc_2g.dat;/tmp/profiles/mt_dbdc_5g.dat\n\
INDEX0_EEPROM_offset=0x0\n\
INDEX0_EEPROM_size=0x4000\n\
INDEX0_EEPROM_name=e2p\n\
INDEX0_main_ifname=ra0;rax0\n\
INDEX0_ext_ifname=ra;rax\n\
INDEX0_wds_ifname=wds;wdsx\n\
INDEX0_apcli_ifname=apcli;apclix\n\
INDEX0_mesh_ifname=mesh;meshx\n\
INDEX0_nvram_zone=RT2860\n\
INDEX0_single_sku_path=/etc_ro/Wireless/RT2860AP/SingleSKU.dat\n\
INDEX0_bf_sku_path=/etc_ro/Wireless/RT2860AP/SingleSKU_BF.dat\n"
};

#else

UCHAR l1profile_default[] = {
"Default\n\
INDEX0=MT7615\n\
INDEX0_profile_path=/etc/wireless/mt7615/mt7615.1.dat\n\
INDEX0_EEPROM_offset=0x0\n\
INDEX0_EEPROM_size=0x4000\n\
INDEX0_EEPROM_name=e2p\n\
INDEX0_main_ifname=ra0\n\
INDEX0_ext_ifname=ra\n\
INDEX0_wds_ifname=wds\n\
INDEX0_apcli_ifname=apcli\n\
INDEX0_single_sku_path=/etc_ro/Wireless/RT2860AP/SingleSKU.dat\n\
INDEX0_bf_sku_path=/etc_ro/Wireless/RT2860AP/SingleSKU_BF.dat\n\
INDEX1=MT7615\n\
INDEX1_profile_path=/etc/wireless/mt7615/mt7615.2.dat\n\
INDEX1_EEPROM_offset=0x8000\n\
INDEX1_EEPROM_size=0x4000\n\
INDEX1_EEPROM_name=e2p\n\
INDEX1_main_ifname=rai0\n\
INDEX1_ext_ifname=rai\n\
INDEX1_wds_ifname=wdsi\n\
INDEX1_apcli_ifname=apclii\n\
INDEX1_single_sku_path=/etc_ro/Wireless/RT2860AP/SingleSKU.dat\n\
INDEX1_bf_sku_path=/etc_ro/Wireless/RT2860AP/SingleSKU_BF.dat\n"
};

#endif