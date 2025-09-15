# SPDX-License-Identifier: GPL-2.0-only
#
# Copyright (C) 2015-2020 OpenWrt.org

PKG_CHECK_FORMAT_SECURITY ?= 1
PKG_ASLR_PIE ?= 1
PKG_ASLR_PIE_REGULAR ?= 0
PKG_SSP ?= 1
PKG_FORTIFY_SOURCE ?= 1
PKG_RELRO ?= 1

ifdef CONFIG_PKG_CHECK_FORMAT_SECURITY
  ifeq ($(strip $(PKG_CHECK_FORMAT_SECURITY)),1)
    TARGET_CFLAGS += -Wformat -Werror=format-security
  endif
endif
ifdef CONFIG_PKG_ASLR_PIE_ALL
  ifeq ($(strip $(PKG_ASLR_PIE)),1)
    TARGET_CFLAGS += $(FPIC)
    TARGET_LDFLAGS += $(FPIC) -specs=$(INCLUDE_DIR)/hardened-ld-pie.specs
  endif
endif
ifdef CONFIG_PKG_ASLR_PIE_REGULAR
  ifeq ($(strip $(PKG_ASLR_PIE_REGULAR)),1)
    TARGET_CFLAGS += $(FPIC)
    TARGET_LDFLAGS += $(FPIC) -specs=$(INCLUDE_DIR)/hardened-ld-pie.specs
  endif
endif
ifdef CONFIG_PKG_CC_STACKPROTECTOR_REGULAR
  ifeq ($(strip $(PKG_SSP)),1)
    TARGET_CFLAGS += -fstack-protector
  endif
endif
ifdef CONFIG_PKG_CC_STACKPROTECTOR_STRONG
  ifeq ($(strip $(PKG_SSP)),1)
    TARGET_CFLAGS += -fstack-protector-strong
  endif
endif
ifdef CONFIG_PKG_FORTIFY_SOURCE_1
  ifeq ($(strip $(PKG_FORTIFY_SOURCE)),1)
    TARGET_CFLAGS += -D_FORTIFY_SOURCE=1
  endif
endif
ifdef CONFIG_PKG_FORTIFY_SOURCE_2
  ifeq ($(strip $(PKG_FORTIFY_SOURCE)),1)
    TARGET_CFLAGS += -D_FORTIFY_SOURCE=2
  endif
endif
ifdef CONFIG_PKG_RELRO_PARTIAL
  ifeq ($(strip $(PKG_RELRO)),1)
    TARGET_CFLAGS += -Wl,-z,relro
    TARGET_LDFLAGS += -zrelro
  endif
endif
ifdef CONFIG_PKG_RELRO_FULL
  ifeq ($(strip $(PKG_RELRO)),1)
    TARGET_CFLAGS += -Wl,-z,now -Wl,-z,relro
    TARGET_LDFLAGS += -znow -zrelro
  endif
endif

