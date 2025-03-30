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

Package/ice-firmware = $(call Package/firmware-default,Intel ICE firmware)
define Package/ice-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/intel/ice/ddp
	$(CP) \
		$(PKG_BUILD_DIR)/intel/ice/ddp/*.pkg \
		$(1)/lib/firmware/intel/ice/ddp/ice.pkg
endef
$(eval $(call BuildPackage,ice-firmware))

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

Package/iwlwifi-firmware-ax101 = $(call Package/firmware-default,Intel AX101 firmware)
define Package/iwlwifi-firmware-ax101/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-so-a0-hr-b0-89.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-ax101))

Package/iwlwifi-firmware-ax200 = $(call Package/firmware-default,Intel AX200 firmware)
define Package/iwlwifi-firmware-ax200/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-cc-a0-77.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-ax200))

Package/iwlwifi-firmware-ax201 = $(call Package/firmware-default,Intel AX201 firmware)
define Package/iwlwifi-firmware-ax201/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-QuZ-a0-hr-b0-77.ucode $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-ax201))

Package/iwlwifi-firmware-ax210 = $(call Package/firmware-default,Intel AX210 firmware)
define Package/iwlwifi-firmware-ax210/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-ty-a0-gf-a0-89.ucode $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-ty-a0-gf-a0.pnvm $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-ax210))

Package/iwlwifi-firmware-ax411 = $(call Package/firmware-default,Intel AX411 firmware)
define Package/iwlwifi-firmware-ax411/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-so-a0-gf4-a0-89.ucode $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-so-a0-gf4-a0.pnvm $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-ax411))

Package/iwlwifi-firmware-be200 = $(call Package/firmware-default,Intel BE200 firmware)
define Package/iwlwifi-firmware-be200/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-gl-c0-fm-c0-92.ucode $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-gl-c0-fm-c0.pnvm $(1)/lib/firmware
endef
$(eval $(call BuildPackage,iwlwifi-firmware-be200))

Package/e100-firmware = $(call Package/firmware-default,Intel e100)
define Package/e100-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/e100
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/e100/d101m_ucode.bin $(1)/lib/firmware/e100/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/e100/d101s_ucode.bin $(1)/lib/firmware/e100/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/e100/d102e_ucode.bin $(1)/lib/firmware/e100/
endef
$(eval $(call BuildPackage,e100-firmware))

i915_deps:=+i915-firmware-dmc +i915-firmware-guc +i915-firmware-huc +i915-firmware-gsc
Package/i915-firmware = $(call Package/firmware-default,Intel I915 firmware \(meta package\),$(i915_deps))
define Package/i915-firmware/install
	true
endef
$(eval $(call BuildPackage,i915-firmware))

Package/i915-firmware-dmc = $(call Package/firmware-default,Intel I915 DMC firmware)
define Package/i915-firmware-dmc/install
	$(INSTALL_DIR) $(1)/lib/firmware/i915
	for f in $(PKG_BUILD_DIR)/i915/*_dmc*.bin; do                        \
	  t=`echo $$$${f##*/} | cut -d_ -f2 | cut -d. -f1`;                  \
	  if [ "$$$$t" = dmc ]; then $(CP) $$$$f $(1)/lib/firmware/i915/; fi \
	done
endef
$(eval $(call BuildPackage,i915-firmware-dmc))

Package/i915-firmware-guc = $(call Package/firmware-default,Intel I915 GUC firmware)
define Package/i915-firmware-guc/install
	$(INSTALL_DIR) $(1)/lib/firmware/i915
	for f in $(PKG_BUILD_DIR)/i915/*_guc*.bin; do                        \
	  t=`echo $$$${f##*/} | cut -d_ -f2 | cut -d. -f1`;                  \
	  if [ "$$$$t" = guc ]; then $(CP) $$$$f $(1)/lib/firmware/i915/; fi \
	done
endef
$(eval $(call BuildPackage,i915-firmware-guc))

Package/i915-firmware-huc = $(call Package/firmware-default,Intel I915 HUC firmware)
define Package/i915-firmware-huc/install
	$(INSTALL_DIR) $(1)/lib/firmware/i915
	for f in $(PKG_BUILD_DIR)/i915/*_huc*.bin; do                        \
	  t=`echo $$$${f##*/} | cut -d_ -f2 | cut -d. -f1`;                  \
	  if [ "$$$$t" = huc ]; then $(CP) $$$$f $(1)/lib/firmware/i915/; fi \
	done
endef
$(eval $(call BuildPackage,i915-firmware-huc))

Package/i915-firmware-gsc = $(call Package/firmware-default,Intel I915 GSC firmware)
define Package/i915-firmware-gsc/install
	$(INSTALL_DIR) $(1)/lib/firmware/i915
	for f in $(PKG_BUILD_DIR)/i915/*_gsc*.bin; do                        \
	  t=`echo $$$${f##*/} | cut -d_ -f2 | cut -d. -f1`;                  \
	  if [ "$$$$t" = gsc ]; then $(CP) $$$$f $(1)/lib/firmware/i915/; fi \
	done
endef
$(eval $(call BuildPackage,i915-firmware-gsc))
