#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

define KernelPackage/nls-base
  SUBMENU:=Native Language Support
  TITLE:=Native Language Support
  KCONFIG:=CONFIG_NLS
  FILES:=$(LINUX_DIR)/fs/nls/nls_base.ko
endef

define KernelPackage/nls-base/description
 Kernel module for NLS (Native Language Support)
endef

$(eval $(call KernelPackage,nls-base))


define KernelPackage/nls-cp437
  SUBMENU:=Native Language Support
  TITLE:=Codepage 437 (United States, Canada)
  KCONFIG:=CONFIG_NLS_CODEPAGE_437
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp437.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp437)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp437/description
 Kernel module for NLS Codepage 437 (United States, Canada)
endef

$(eval $(call KernelPackage,nls-cp437))


define KernelPackage/nls-cp775
  SUBMENU:=Native Language Support
  TITLE:=Codepage 775 (Baltic Rim)
  KCONFIG:=CONFIG_NLS_CODEPAGE_775
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp775.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp775)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp775/description
 Kernel module for NLS Codepage 775 (Baltic Rim)
endef

$(eval $(call KernelPackage,nls-cp775))


define KernelPackage/nls-cp850
  SUBMENU:=Native Language Support
  TITLE:=Codepage 850 (Europe)
  KCONFIG:=CONFIG_NLS_CODEPAGE_850
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp850.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp850)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp850/description
 Kernel module for NLS Codepage 850 (Europe)
endef

$(eval $(call KernelPackage,nls-cp850))


define KernelPackage/nls-cp852
  SUBMENU:=Native Language Support
  TITLE:=Codepage 852 (Europe)
  KCONFIG:=CONFIG_NLS_CODEPAGE_852
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp852.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp852)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp852/description
 Kernel module for NLS Codepage 852 (Europe)
endef

$(eval $(call KernelPackage,nls-cp852))


define KernelPackage/nls-cp862
  SUBMENU:=Native Language Support
  TITLE:=Codepage 862 (Hebrew)
  KCONFIG:=CONFIG_NLS_CODEPAGE_862
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp862.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp862)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp862/description
  Kernel module for NLS Codepage 862 (Hebrew)
endef

$(eval $(call KernelPackage,nls-cp862))


define KernelPackage/nls-cp864
  SUBMENU:=Native Language Support
  TITLE:=Codepage 864 (Arabic)
  KCONFIG:=CONFIG_NLS_CODEPAGE_864
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp864.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp864)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp864/description
  Kernel module for NLS Codepage 864 (Arabic)
endef

$(eval $(call KernelPackage,nls-cp864))


define KernelPackage/nls-cp866
  SUBMENU:=Native Language Support
  TITLE:=Codepage 866 (Cyrillic)
  KCONFIG:=CONFIG_NLS_CODEPAGE_866
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp866.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp866)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp866/description
  Kernel module for NLS Codepage 866 (Cyrillic)
endef

$(eval $(call KernelPackage,nls-cp866))


define KernelPackage/nls-cp932
  SUBMENU:=Native Language Support
  TITLE:=Codepage 932 (Japanese)
  KCONFIG:=CONFIG_NLS_CODEPAGE_932
  FILES:= \
	$(LINUX_DIR)/fs/nls/nls_cp932.ko \
	$(LINUX_DIR)/fs/nls/nls_euc-jp.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp932 nls_euc-jp)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp932/description
  Kernel module for NLS Codepage 932 (Japanese)
endef

$(eval $(call KernelPackage,nls-cp932))


define KernelPackage/nls-cp936
  SUBMENU:=Native Language Support
  TITLE:=Codepage 936 (Simplified Chinese)
  KCONFIG:=CONFIG_NLS_CODEPAGE_936
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp936.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp936)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp936/description
  Kernel module for NLS Codepage 936 (Simplified Chinese)
endef

$(eval $(call KernelPackage,nls-cp936))


define KernelPackage/nls-cp950
  SUBMENU:=Native Language Support
  TITLE:=Codepage 950 (Traditional Chinese)
  KCONFIG:=CONFIG_NLS_CODEPAGE_950
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp950.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp950)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp950/description
  Kernel module for NLS Codepage 950 (Traditional Chinese)
endef

$(eval $(call KernelPackage,nls-cp950))


define KernelPackage/nls-cp1250
  SUBMENU:=Native Language Support
  TITLE:=Codepage 1250 (Eastern Europe)
  KCONFIG:=CONFIG_NLS_CODEPAGE_1250
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp1250.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp1250)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp1250/description
 Kernel module for NLS Codepage 1250 (Eastern Europe)
endef

$(eval $(call KernelPackage,nls-cp1250))


define KernelPackage/nls-cp1251
  SUBMENU:=Native Language Support
  TITLE:=Codepage 1251 (Russian)
  KCONFIG:=CONFIG_NLS_CODEPAGE_1251
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp1251.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp1251)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-cp1251/description
 Kernel module for NLS Codepage 1251 (Russian)
endef

$(eval $(call KernelPackage,nls-cp1251))


define KernelPackage/nls-iso8859-1
  SUBMENU:=Native Language Support
  TITLE:=ISO 8859-1 (Latin 1; Western European Languages)
  KCONFIG:=CONFIG_NLS_ISO8859_1
  FILES:=$(LINUX_DIR)/fs/nls/nls_iso8859-1.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_iso8859-1)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-iso8859-1/description
 Kernel module for NLS ISO 8859-1 (Latin 1)
endef

$(eval $(call KernelPackage,nls-iso8859-1))


define KernelPackage/nls-iso8859-2
  SUBMENU:=Native Language Support
  TITLE:=ISO 8859-2 (Latin 2; Central European Languages)
  KCONFIG:=CONFIG_NLS_ISO8859_2
  FILES:=$(LINUX_DIR)/fs/nls/nls_iso8859-2.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_iso8859-2)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-iso8859-2/description
 Kernel module for NLS ISO 8859-2 (Latin 2)
endef

$(eval $(call KernelPackage,nls-iso8859-2))


define KernelPackage/nls-iso8859-6
  SUBMENU:=Native Language Support
  TITLE:=ISO 8859-6 (Arabic)
  KCONFIG:=CONFIG_NLS_ISO8859_6
  FILES:=$(LINUX_DIR)/fs/nls/nls_iso8859-6.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_iso8859-6)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-iso8859-6/description
 Kernel module for NLS ISO 8859-6 (Arabic)
endef

$(eval $(call KernelPackage,nls-iso8859-6))


define KernelPackage/nls-iso8859-8
  SUBMENU:=Native Language Support
  TITLE:=ISO 8859-8, CP1255 (Hebrew)
  KCONFIG:=CONFIG_NLS_ISO8859_8
  FILES:=$(LINUX_DIR)/fs/nls/nls_cp1255.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_cp1255)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-iso8859-8/description
 Kernel module for Hebrew charsets (ISO-8859-8, CP1255)
endef

$(eval $(call KernelPackage,nls-iso8859-8))


define KernelPackage/nls-iso8859-13
  SUBMENU:=Native Language Support
  TITLE:=ISO 8859-13 (Latin 7; Baltic)
  KCONFIG:=CONFIG_NLS_ISO8859_13
  FILES:=$(LINUX_DIR)/fs/nls/nls_iso8859-13.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_iso8859-13)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-iso8859-13/description
 Kernel module for NLS ISO 8859-13 (Latin 7; Baltic)
endef

$(eval $(call KernelPackage,nls-iso8859-13))


define KernelPackage/nls-iso8859-15
  SUBMENU:=Native Language Support
  TITLE:=ISO 8859-15 (Latin 9; Western, with Euro symbol)
  KCONFIG:=CONFIG_NLS_ISO8859_15
  FILES:=$(LINUX_DIR)/fs/nls/nls_iso8859-15.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_iso8859-15)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-iso8859-15/description
 Kernel module for NLS ISO 8859-15 (Latin 9)
endef

$(eval $(call KernelPackage,nls-iso8859-15))


define KernelPackage/nls-koi8r
  SUBMENU:=Native Language Support
  TITLE:=KOI8-R (Russian)
  KCONFIG:=CONFIG_NLS_KOI8_R
  FILES:=$(LINUX_DIR)/fs/nls/nls_koi8-r.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_koi8-r)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-koi8r/description
 Kernel module for NLS KOI8-R (Russian)
endef

$(eval $(call KernelPackage,nls-koi8r))


define KernelPackage/nls-utf8
  SUBMENU:=Native Language Support
  TITLE:=UTF-8
  KCONFIG:=CONFIG_NLS_UTF8
  FILES:=$(LINUX_DIR)/fs/nls/nls_utf8.ko
  AUTOLOAD:=$(call AutoLoad,25,nls_utf8)
  $(call AddDepends/nls)
endef

define KernelPackage/nls-utf8/description
 Kernel module for NLS UTF-8
endef

$(eval $(call KernelPackage,nls-utf8))
