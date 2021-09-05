#
# Copyright (C) 2019 Jeffery To <jeffery.to@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifndef DUMP
  ifdef __package_mk
    $(warning pypi.mk should be included before package.mk)
  endif
endif

ifneq ($(strip $(PYPI_NAME)),)
  PYPI_SOURCE_NAME?=$(PYPI_NAME)
  PYPI_SOURCE_EXT?=tar.gz
  PYPI_NAME_FIRST_LETTER?=$(strip $(foreach a,$(chars_lower) $(chars_upper) 0 1 2 3 4 5 6 7 8 9 _ -,$(if $(PYPI_NAME:$a%=),,$a)))

  PKG_SOURCE?=$(PYPI_SOURCE_NAME)-$(PKG_VERSION).$(PYPI_SOURCE_EXT)
  PKG_SOURCE_URL?=https://files.pythonhosted.org/packages/source/$(PYPI_NAME_FIRST_LETTER)/$(PYPI_NAME)

  PKG_BUILD_DIR:=$(BUILD_DIR)/pypi/$(if $(BUILD_VARIANT),$(PKG_NAME)-$(BUILD_VARIANT)/)$(PYPI_SOURCE_NAME)$(if $(PKG_VERSION),-$(PKG_VERSION))
  HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/pypi/$(PYPI_SOURCE_NAME)$(if $(PKG_VERSION),-$(PKG_VERSION))
endif
