Package/ibt-firmware = $(call Package/firmware-default,Intel bluetooth firmware)
define Package/ibt-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/intel
	$(CP) \
		$(PKG_BUILD_DIR)/intel/*.bseq \
		$(PKG_BUILD_DIR)/intel/ibt*.sfi \
		$(PKG_BUILD_DIR)/intel/ibt*.ddc \
		$(1)/lib/firmware/intel
endef
$(eval $(call BuildPackage,ibt-firmware))

Package/iwl3945-firmware = $(call Package/firmware-default,Intel IWL3945 firmware)
define Package/iwl3945-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-3945-2.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwl3945-firmware))

Package/iwl4965-firmware = $(call Package/firmware-default,Intel IWL4965 firmware)
define Package/iwl4965-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-4965-2.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwl4965-firmware))

Package/iwlwifi-firmware-iwl100 = $(call Package/firmware-default,Intel Centrino Wireless-N 100 firmware)
define Package/iwlwifi-firmware-iwl100/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-100-5.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl100))

Package/iwlwifi-firmware-iwl1000 = $(call Package/firmware-default,Intel Centrino Wireless-N 1000 firmware)
define Package/iwlwifi-firmware-iwl1000/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-1000-5.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl1000))

Package/iwlwifi-firmware-iwl105 = $(call Package/firmware-default,Intel Centrino Wireless-N 105 firmware)
define Package/iwlwifi-firmware-iwl105/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-105-6.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl105))

Package/iwlwifi-firmware-iwl135 = $(call Package/firmware-default,Intel Centrino Wireless-N 135 firmware)
define Package/iwlwifi-firmware-iwl135/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-135-6.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl135))

Package/iwlwifi-firmware-iwl2000 = $(call Package/firmware-default,Intel Centrino Wireless-N 2200 firmware)
define Package/iwlwifi-firmware-iwl2000/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-2000-6.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl2000))

Package/iwlwifi-firmware-iwl2030 = $(call Package/firmware-default,Intel Centrino Wireless-N 2230 firmware)
define Package/iwlwifi-firmware-iwl2030/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-2030-6.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl2030))

Package/iwlwifi-firmware-iwl3160 = $(call Package/firmware-default,Intel Wireless 3160 firmware)
define Package/iwlwifi-firmware-iwl3160/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-3160-17.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl3160))

Package/iwlwifi-firmware-iwl3168 = $(call Package/firmware-default,Intel Wireless 3168 firmware)
define Package/iwlwifi-firmware-iwl3168/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-3168-29.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl3168))

Package/iwlwifi-firmware-iwl5000 = $(call Package/firmware-default,Intel Wireless 5100AGN 5300AGN and 5350AGN firmware)
define Package/iwlwifi-firmware-iwl5000/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-5000-5.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl5000))

Package/iwlwifi-firmware-iwl5150 = $(call Package/firmware-default,Intel Wireless Wi-Fi 5150AGN firmware)
define Package/iwlwifi-firmware-iwl5150/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-5150-2.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl5150))

Package/iwlwifi-firmware-iwl6000g2 = $(call Package/firmware-default,Intel Centrino 6300 and 6200 firmware)
define Package/iwlwifi-firmware-iwl6000g2/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-6000-4.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl6000g2))

Package/iwlwifi-firmware-iwl6000g2a = $(call Package/firmware-default,Intel Centrino 6205 firmware)
define Package/iwlwifi-firmware-iwl6000g2a/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-6000g2a-6.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl6000g2a))

Package/iwlwifi-firmware-iwl6000g2b = $(call Package/firmware-default,Intel Centrino 6230 1030 130 and 6235 firmware)
define Package/iwlwifi-firmware-iwl6000g2b/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-6000g2b-6.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl6000g2b))

Package/iwlwifi-firmware-iwl6050 = $(call Package/firmware-default,Intel Centrino 6150 and 6250 firmware)
define Package/iwlwifi-firmware-iwl6050/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-6050-5.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl6050))

Package/iwlwifi-firmware-iwl7260 = $(call Package/firmware-default,Intel Wireless 7260 firmware)
define Package/iwlwifi-firmware-iwl7260/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-7260-17.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl7260))

Package/iwlwifi-firmware-iwl7265 = $(call Package/firmware-default,Intel Wireless 7265 firmware)
define Package/iwlwifi-firmware-iwl7265/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-7265-17.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl7265))

Package/iwlwifi-firmware-iwl7265d = $(call Package/firmware-default,Intel Wireless 7265D and 3165 firmware)
define Package/iwlwifi-firmware-iwl7265d/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-7265D-29.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl7265d))

Package/iwlwifi-firmware-iwl8260c = $(call Package/firmware-default,Intel Wireless 8260 and 4165 firmware)
define Package/iwlwifi-firmware-iwl8260c/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-8000C-36.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl8260c))

Package/iwlwifi-firmware-iwl8265 = $(call Package/firmware-default,Intel Wireless 8265 firmware)
define Package/iwlwifi-firmware-iwl8265/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-8265-36.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl8265))

Package/iwlwifi-firmware-iwl9000 = $(call Package/firmware-default,Intel Wireless 9000 firmware)
define Package/iwlwifi-firmware-iwl9000/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-9000-pu-b0-jf-b0-46.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl9000))

Package/iwlwifi-firmware-iwl9260 = $(call Package/firmware-default,Intel Wireless 9260 firmware)
define Package/iwlwifi-firmware-iwl9260/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-9260-th-b0-jf-b0-46.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-iwl9260))

Package/iwlwifi-firmware-ax200 = $(call Package/firmware-default,Intel AX200 firmware)
define Package/iwlwifi-firmware-ax200/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-cc-a0-66.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-ax200))

Package/iwlwifi-firmware-ax201 = $(call Package/firmware-default,Intel AX201 firmware)
define Package/iwlwifi-firmware-ax201/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-so-a0-hr-b0-72.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-ax201))

Package/iwlwifi-firmware-ax210 = $(call Package/firmware-default,Intel AX210 firmware)
define Package/iwlwifi-firmware-ax210/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-ty-a0-gf-a0-66.ucode $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-ty-a0-gf-a0.pnvm $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-ax210))

Package/e100-firmware = $(call Package/firmware-default,Intel e100)
define Package/e100-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/e100
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/e100/d101m_ucode.bin $(1)/lib/firmware/e100/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/e100/d101s_ucode.bin $(1)/lib/firmware/e100/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/e100/d102e_ucode.bin $(1)/lib/firmware/e100/
endef
$(eval $(call BuildPackage,e100-firmware))

Package/i915-firmware = $(call Package/firmware-default,Intel GPU firmware)
define Package/i915-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/i915
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_dmc_ver1_09.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_dmc_ver2_09.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_dmc_ver2_10.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_dmc_ver2_12.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_dmc_ver2_14.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_dmc_ver2_16.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_guc_62.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_guc_69.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_guc_70.1.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adlp_guc_70.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/adls_dmc_ver2_01.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_dmc_ver1_07.bin $(1)/lib/firmware/i915/
	ln -s /lib/firmware/i915/bxt_dmc_ver1_07.bin $(1)/lib/firmware/i915/bxt_dmc_ver1.bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_guc_32.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_guc_33.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_guc_ver8_7.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_guc_ver9_29.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_huc_2.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_huc_ver01_07_1398.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/bxt_huc_ver01_8_2893.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/cml_guc_33.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/cml_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/cml_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/cml_huc_4.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/cnl_dmc_ver1_06.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/cnl_dmc_ver1_07.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/dg1_dmc_ver2_02.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/dg1_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/dg1_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/dg1_huc_7.7.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/dg1_huc_7.9.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/ehl_guc_33.0.4.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/ehl_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/ehl_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/ehl_guc_69.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/ehl_guc_70.1.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/ehl_huc_9.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/glk_dmc_ver1_04.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/glk_guc_32.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/glk_guc_33.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/glk_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/glk_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/glk_guc_70.1.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/glk_huc_4.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/glk_huc_ver03_01_2893.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_dmc_ver1_07.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_dmc_ver1_09.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_guc_32.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_guc_33.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_huc_9.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/icl_huc_ver8_4_3238.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_dmc_ver1_01.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_dmc_ver1_04.bin $(1)/lib/firmware/i915/
	ln -s /lib/firmware/i915/kbl_dmc_ver1_04.bin $(1)/lib/firmware/i915/kbl_dmc_ver1.bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_guc_32.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_guc_33.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_guc_70.1.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_guc_ver9_14.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_guc_ver9_39.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_huc_4.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/kbl_huc_ver02_00_1810.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/rkl_dmc_ver2_02.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/rkl_dmc_ver2_03.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_dmc_ver1_23.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_dmc_ver1_26.bin $(1)/lib/firmware/i915/
	ln -s /lib/firmware/i915/skl_dmc_ver1_26.bin $(1)/lib/firmware/i915/skl_dmc_ver1.bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_dmc_ver1_27.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_guc_32.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_guc_33.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_guc_ver1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_guc_ver4.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_guc_ver6_1.bin $(1)/lib/firmware/i915/
	ln -s /lib/firmware/i915/skl_guc_ver6_1.bin $(1)/lib/firmware/i915/skl_guc_ver6.bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_guc_ver9_33.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_huc_2.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/skl_huc_ver01_07_1398.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_dmc_ver2_04.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_dmc_ver2_06.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_dmc_ver2_08.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_dmc_ver2_12.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_guc_35.2.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_guc_49.0.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_guc_62.0.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_guc_69.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_guc_70.1.1.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_guc_70.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_huc.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_huc_7.0.12.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_huc_7.0.3.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_huc_7.5.0.bin $(1)/lib/firmware/i915/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/i915/tgl_huc_7.9.3.bin $(1)/lib/firmware/i915/
endef
$(eval $(call BuildPackage,i915-firmware))
