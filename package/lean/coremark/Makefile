# Copyright 2018 Embedded Microprocessor Benchmark Consortium (EEMBC)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=coremark
PKG_RELEASE:=12

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/eembc/coremark.git
PKG_SOURCE_VERSION:=2e9d6165f362f7071d29b7f88c6d32655e5d4a3c
PKG_SOURCE_DATE:=2019-01-31

include $(INCLUDE_DIR)/package.mk

define Package/coremark
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CoreMark Embedded Microprocessor Benchmark
  URL:=https://github.com/eembc/coremark
endef

define Package/coremark/description
  Embedded Microprocessor Benchmark
endef

DIR_ARCH:=linux

ifeq ($(ARCH),x86_64)
	DIR_ARCH:=linux64
endif
ifeq ($(ARCH),aarch64)
	DIR_ARCH:=linux64
endif

define Build/Compile
	$(SED) 's|EXE = .exe|EXE =|' $(PKG_BUILD_DIR)/$(DIR_ARCH)/core_portme.mak
	mkdir $(PKG_BUILD_DIR)/$(ARCH)
	cp -r $(PKG_BUILD_DIR)/$(DIR_ARCH)/* $(PKG_BUILD_DIR)/$(ARCH)
	$(MAKE) XCFLAGS="-DMULTITHREAD=8 -DUSE_PTHREAD" -C $(PKG_BUILD_DIR) PORT_DIR=$(ARCH) $(MAKE_FLAGS) \
		compile 
endef

define Package/coremark/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/coremark $(1)/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./coremark.sh $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./coremark $(1)/etc/uci-defaults/xxx-coremark
endef

define Package/coremark/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || sed -i '/coremark/d' /etc/crontabs/root
[ -n "$${IPKG_INSTROOT}" ] || echo "0 4 * * 0 /etc/coremark.sh" >> /etc/crontabs/root
[ -n "$${IPKG_INSTROOT}" ] || crontab /etc/crontabs/root
endef

$(eval $(call BuildPackage,coremark))
