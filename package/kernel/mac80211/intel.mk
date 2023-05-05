PKG_DRIVERS += iwlwifi

config-$(call config_package,iwlwifi) += IWLWIFI IWLDVM IWLMVM
config-$(CONFIG_PACKAGE_IWLWIFI_DEBUG)+= IWLWIFI_DEBUG
config-$(CONFIG_PACKAGE_IWLWIFI_DEBUGFS)+= IWLWIFI_DEBUGFS

define KernelPackage/iwlwifi
  $(call KernelPackage/mac80211/Default)
  DEPENDS:= +kmod-mac80211 @PCI_SUPPORT +@DRIVER_11AC_SUPPORT +@DRIVER_11AX_SUPPORT
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

