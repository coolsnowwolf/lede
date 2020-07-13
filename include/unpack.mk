#
# Copyright (C) 2006-2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

HOST_TAR:=$(TAR)
TAR_CMD=$(HOST_TAR) -C $(1)/.. $(TAR_OPTIONS)
UNZIP_CMD=unzip -q -d $(1)/.. $(DL_DIR)/$(PKG_SOURCE)

ifeq ($(PKG_SOURCE),)
  PKG_UNPACK ?= true
else

ifeq ($(strip $(UNPACK_CMD)),)
  ifeq ($(strip $(PKG_CAT)),)
    # try to autodetect file type
    EXT:=$(call ext,$(PKG_SOURCE))
    EXT1:=$(EXT)

    ifeq ($(filter gz tgz,$(EXT)),$(EXT))
      EXT:=$(call ext,$(PKG_SOURCE:.$(EXT)=))
      DECOMPRESS_CMD:=gzip -dc $(DL_DIR)/$(PKG_SOURCE) |
    endif
    ifeq ($(filter bzip2 bz2 bz tbz2 tbz,$(EXT)),$(EXT))
      EXT:=$(call ext,$(PKG_SOURCE:.$(EXT)=))
      DECOMPRESS_CMD:=bzcat $(DL_DIR)/$(PKG_SOURCE) |
    endif
    ifeq ($(filter xz txz,$(EXT)),$(EXT))
      EXT:=$(call ext,$(PKG_SOURCE:.$(EXT)=))
      DECOMPRESS_CMD:=xzcat $(DL_DIR)/$(PKG_SOURCE) |
    endif
    ifeq (zst,$(EXT))
      EXT:=$(call ext,$(PKG_SOURCE:.$(EXT)=))
      DECOMPRESS_CMD:=zstdcat $(DL_DIR)/$(PKG_SOURCE) |
    endif
    ifeq ($(filter tgz tbz tbz2 txz,$(EXT1)),$(EXT1))
      EXT:=tar
    endif
    DECOMPRESS_CMD ?= cat $(DL_DIR)/$(PKG_SOURCE) |
    ifeq ($(EXT),tar)
      UNPACK_CMD=$(DECOMPRESS_CMD) $(TAR_CMD)
    endif
    ifeq ($(EXT),cpio)
      UNPACK_CMD=$(DECOMPRESS_CMD) (cd $(1)/..; cpio -i -d)
    endif
    ifeq ($(EXT),zip)
      UNPACK_CMD=$(UNZIP_CMD)
    endif
  endif

  # compatibility code for packages that set PKG_CAT
  ifeq ($(strip $(UNPACK_CMD)),)
    # use existing PKG_CAT
    UNPACK_CMD=$(PKG_CAT) $(DL_DIR)/$(PKG_SOURCE) | $(TAR_CMD)
    ifeq ($(PKG_CAT),unzip)
      UNPACK_CMD=$(UNZIP_CMD)
    endif
    # replace zcat with $(ZCAT), because some system don't support it properly
    ifeq ($(PKG_CAT),zcat)
      UNPACK_CMD=gzip -dc $(DL_DIR)/$(PKG_SOURCE) | $(TAR_CMD)
    endif
  endif
endif

ifdef PKG_BUILD_DIR
  PKG_UNPACK ?= $(SH_FUNC) $(call UNPACK_CMD,$(PKG_BUILD_DIR))
endif
ifdef HOST_BUILD_DIR
  HOST_UNPACK ?= $(SH_FUNC) $(call UNPACK_CMD,$(HOST_BUILD_DIR))
endif

endif # PKG_SOURCE

