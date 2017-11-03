Package/ibt-firmware = $(call Package/firmware-default,Intel bluetooth firmware)
define Package/ibt-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/intel
	$(CP) \
		$(PKG_BUILD_DIR)/intel/*.bseq \
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

Package/iwlwifi-firmware = $(call Package/firmware-default,Intel wireless firmware)
define Package/iwlwifi-firmware/config
  if PACKAGE_iwlwifi-firmware
	config IWL100_FW
		bool "Intel 100 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Wireless-N 100

	config IWL1000_FW
		bool "Intel 1000 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Wireless-N 1000

	config IWL105_FW
		bool "Intel 105 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Wireless-N 105

	config IWL135_FW
		bool "Intel 135 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Wireless-N 135

	config IWL2000_FW
		bool "Intel 2000 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Wireless-N 2200

	config IWL2030_FW
		bool "Intel 2030 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Wireless-N 2230

	config IWL3160_FW
		bool "Intel 3160 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Wireless WiFi 3160

	config IWL5000_FW
		bool "Intel 5000 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Wireless WiFi 5100AGN, 5300AGN, and 5350AGN

	config IWL5150_FW
		bool "Intel 5150 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Wireless WiFi 5150AGN

	config IWL6000_FW
		bool "Intel 6000 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Ultimate-N 6300 and Advanced-N 6200

	config IWL6005_FW
		bool "Intel 6005 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Advanced-N 6205

	config IWL6030_FW
		bool "Intel 6030 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Advanced-N 6230, Wireless-N 1030, Wireless-N 130 and Advanced-N 6235

	config IWL6050_FW
		bool "Intel 6050 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Centrino Advanced-N + WiMAX 6250 and Wireless-N + WiMAX 6150

	config IWL7260_FW
		bool "Intel 7260 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Dual Band Wireless-N 7260 and Intel Dual Band Wireless-AC 7260

	config IWL7265_FW
		bool "Intel 7265 Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Wireless 7265, 7265D, 3165

	config IWL8000_FW
		bool "Intel 8000 Series Firmware"
		default y
		help
		  Download and install firmware for:
		    Intel Wireless Series 8260, 4165
  endif
endef
define Package/iwlwifi-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
ifneq ($(CONFIG_IWL100_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-100-5.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL1000_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-1000-5.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL105_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-105-6.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL135_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-135-6.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL2000_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-2000-6.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL2030_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-2030-6.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL3160_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-3160-16.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL5000_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-5000-5.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL5150_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-5150-2.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL6000_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-6000-4.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL6005_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-6000g2a-6.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL6030_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-6000g2b-6.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL6050_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-6050-5.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL7260_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-7260-16.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL7265_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-7265-16.ucode $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-7265D-16.ucode $(1)/lib/firmware
endif
ifneq ($(CONFIG_IWL8000_FW),)
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/iwlwifi-8000C-16.ucode $(1)/lib/firmware
endif
endef
$(eval $(call BuildPackage,iwlwifi-firmware))
