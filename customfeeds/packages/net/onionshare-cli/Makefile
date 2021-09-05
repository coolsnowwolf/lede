#
# Copyright (C) 2021 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=onionshare-cli
PKG_VERSION:=2.3.1
PKG_RELEASE:=1

PYPI_NAME:=onionshare-cli
PYPI_SOURCE_NAME:=onionshare_cli
PKG_HASH:=47320a5f270b3629586c249fb2ae1c2f67682cb53c5013a8af9702d0d6e50193

PKG_MAINTAINER:=Jan Pavlinec <jan.pavlinec@nic.cz>
PKG_LICENSE:=GPL-3.0-or-later
PKG_LICENSE_FILES:=LICENSE

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm toml

include ../../lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include ../../lang/python/python3-package.mk

define Package/onionshare-cli
  SECTION:=net
  CATEGORY:=Network
  TITLE:=Secure chat, web and file sharing
  URL:=https://onionshare.org/
  DEPENDS:= \
    +python3-light \
    +python3-psutil \
    +python3-click \
    +python3-flask \
    +python3-flask-httpauth \
    +python3-flask-socketio \
    +python3-pysocks \
    +python3-requests \
    +python3-stem \
    +python3-eventlet \
    +python3-unidecode \
    +python3-cryptodome \
    +python3-urllib3 \
    +tor
endef

define Package/onionshare-cli/description
  OnionShare is an open source tool that lets you securely and
  anonymously share files, host websites, and chat with friends using the Tor network.
endef

$(eval $(call Py3Package,onionshare-cli))
$(eval $(call BuildPackage,onionshare-cli))
$(eval $(call BuildPackage,onionshare-cli-src))
