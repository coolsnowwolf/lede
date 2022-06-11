PKG_DRIVERS += \
	iwl-legacy iwl3945 iwl4965 iwlwifi \
	libipw ipw2100 ipw2200 \

config-$(call config_package,iwl-legacy) += IWLEGACY
config-$(call config_package,iwl3945) += IWL3945
config-$(call config_package,iwl4965) += IWL4965
config-$(call config_package,iwlwifi) += IWLWIFI IWLDVM IWLMVM
config-$(CONFIG_PACKAGE_IWLWIFI_DEBUG)+= IWLWIFI_DEBUG
config-$(CONFIG_PACKAGE_IWLWIFI_DEBUGFS)+= IWLWIFI_DEBUGFS

config-$(call config_package,libipw) += LIBIPW
config-$(call config_package,ipw2100) += IPW2100
config-$(call config_package,ipw2200) += IPW2200

define KernelPackage/iwlwifi
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 @PCI_SUPPORT +@DRIVER_11N_SUPPORT +@DRIVER_11AC_SUPPORT
  TITLE:=Intel AGN Wireless support
  FILES:= \
	$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlwifi/iwlwifi.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlwifi/dvm/iwldvm.ko \
	$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlwifi/mvm/iwlmvm.ko
  AUTOLOAD:=$(call AutoProbe,iwlwifi iwldvm iwlmvm)
  MENU:=1
endef

define KernelPackage/iwlwifi/description
 iwlwifi kernel module for
 Intel Wireless WiFi Link 6250AGN Adapter
 Intel 6000 Series Wi-Fi Adapters (6200AGN and 6300AGN)
 Intel WiFi Link 1000BGN
 Intel Wireless WiFi 5150AGN
 Intel Wireless WiFi 5100AGN, 5300AGN, and 5350AGN
 Intel 6005 Series Wi-Fi Adapters
 Intel 6030 Series Wi-Fi Adapters
 Intel Wireless WiFi Link 6150BGN 2 Adapter
 Intel 100 Series Wi-Fi Adapters (100BGN and 130BGN)
 Intel 2000 Series Wi-Fi Adapters
 Intel 7260 Wi-Fi Adapter
 Intel 3160 Wi-Fi Adapter
 Intel 7265 Wi-Fi Adapter
 Intel 8260 Wi-Fi Adapter
 Intel 3165 Wi-Fi Adapter
endef

define KernelPackage/iwlwifi/config
  if PACKAGE_kmod-iwlwifi

	config PACKAGE_IWLWIFI_DEBUG
		bool "Enable full debugging output in the iwlwifi driver"
		default n
		help
		  This option will enable debug tracing output for the iwlwifi drivers

		  This will result in the kernel module being ~100k larger.  You can
		  control which debug output is sent to the kernel log by setting the
		  value in

			/sys/module/iwlwifi/parameters/debug

		  This entry will only exist if this option is enabled.

		  To set a value, simply echo an 8-byte hex value to the same file:

			  % echo 0x43fff > /sys/module/iwlwifi/parameters/debug

		  You can find the list of debug mask values in:
			  drivers/net/wireless/intel/iwlwifi/iwl-debug.h

		  If this is your first time using this driver, you should say Y here
		  as the debug information can assist others in helping you resolve
		  any problems you may encounter.

	config PACKAGE_IWLWIFI_DEBUGFS
	        bool "iwlwifi debugfs support"
		depends on PACKAGE_MAC80211_DEBUGFS
		default n
		help
		  Enable creation of debugfs files for the iwlwifi drivers. This
		  is a low-impact option that allows getting insight into the
		  driver's state at runtime.

  endif
endef

define KernelPackage/iwl-legacy
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 @PCI_SUPPORT
  TITLE:=Intel legacy Wireless support
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlegacy/iwlegacy.ko
  AUTOLOAD:=$(call AutoProbe,iwlegacy)
endef

define KernelPackage/iwl-legacy/description
 iwl-legacy kernel module for legacy Intel wireless support
endef

define KernelPackage/iwl3945
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 +kmod-iwl-legacy +iwl3945-firmware
  TITLE:=Intel iwl3945 Wireless support
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlegacy/iwl3945.ko
  AUTOLOAD:=$(call AutoProbe,iwl3945)
endef

define KernelPackage/iwl3945/description
 iwl3945 kernel module for Intel 3945 support
endef

define KernelPackage/iwl4965
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 +kmod-iwl-legacy +@DRIVER_11N_SUPPORT +iwl4965-firmware
  TITLE:=Intel iwl4965 Wireless support
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/iwlegacy/iwl4965.ko
  AUTOLOAD:=$(call AutoProbe,iwl4965)
endef

define KernelPackage/iwl4965/description
 iwl4965 kernel module for Intel 4965 support
endef


define KernelPackage/libipw
  $(call KernelPackage/mac80211/Default)
  TITLE:=libipw for ipw2100 and ipw2200
  DEPENDS:=@PCI_SUPPORT +kmod-crypto-michael-mic +kmod-crypto-ecb +kmod-lib80211 +kmod-cfg80211 +@DRIVER_WEXT_SUPPORT @!BIG_ENDIAN
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/ipw2x00/libipw.ko
  AUTOLOAD:=$(call AutoProbe,libipw)
endef

define KernelPackage/libipw/description
 Hardware independent IEEE 802.11 networking stack for ipw2100 and ipw2200.
endef

IPW2100_NAME:=ipw2100-fw
IPW2100_VERSION:=1.3

define Download/ipw2100
  URL:= \
	https://src.fedoraproject.org/repo/pkgs/ipw2100-firmware/ipw2100-fw-1.3.tgz/46aa75bcda1a00efa841f9707bbbd113/ \
	https://archlinux.mirror.pkern.at/other/packages/ipw2100-fw/ \
	http://mirror.ox.ac.uk/sites/ftp.openbsd.org/pub/OpenBSD/distfiles/firmware/ \
	http://firmware.openbsd.org/firmware-dist/
  FILE:=$(IPW2100_NAME)-$(IPW2100_VERSION).tgz
  HASH:=e1107c455e48d324a616b47a622593bc8413dcce72026f72731c0b03dae3a7a2
endef
$(eval $(call Download,ipw2100))

define KernelPackage/ipw2100
  $(call KernelPackage/mac80211/Default)
  TITLE:=Intel IPW2100 driver
  DEPENDS:=@PCI_SUPPORT +kmod-libipw
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/ipw2x00/ipw2100.ko
  AUTOLOAD:=$(call AutoProbe,ipw2100)
endef

define KernelPackage/ipw2100/description
 Kernel support for Intel IPW2100
 Includes:
 - ipw2100
endef

IPW2200_NAME:=ipw2200-fw
IPW2200_VERSION:=3.1

define Download/ipw2200
  URL:= \
	https://src.fedoraproject.org/repo/pkgs/ipw2200-firmware/ipw2200-fw-3.1.tgz/eaba788643c7cc7483dd67ace70f6e99/ \
	https://archlinux.mirror.pkern.at/other/packages/ipw2200-fw/ \
	http://mirror.ox.ac.uk/sites/ftp.openbsd.org/pub/OpenBSD/distfiles/firmware/ \
	http://firmware.openbsd.org/firmware-dist/
  FILE:=$(IPW2200_NAME)-$(IPW2200_VERSION).tgz
  HASH:=c6818c11c18cc030d55ff83f64b2bad8feef485e7742f84f94a61d811a6258bd
endef
$(eval $(call Download,ipw2200))

define KernelPackage/ipw2200
  $(call KernelPackage/mac80211/Default)
  TITLE:=Intel IPW2200 driver
  DEPENDS:=@PCI_SUPPORT +kmod-libipw
  FILES:=$(PKG_BUILD_DIR)/drivers/net/wireless/intel/ipw2x00/ipw2200.ko
  AUTOLOAD:=$(call AutoProbe,ipw2200)
endef

define KernelPackage/ipw2200/description
 Kernel support for Intel IPW2200
 Includes:
 - ipw2200
endef

define KernelPackage/ipw2100/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/ipw2100-$(IPW2100_VERSION)*.fw $(1)/lib/firmware
endef

define KernelPackage/ipw2200/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/$(IPW2200_NAME)-$(IPW2200_VERSION)/ipw2200*.fw $(1)/lib/firmware
endef
