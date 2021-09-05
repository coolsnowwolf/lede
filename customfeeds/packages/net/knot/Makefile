#
# Copyright (C) 2014-2019 CZ.NIC, z.s.p.o. <knot-dns@labs.nic.cz>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=knot
PKG_VERSION:=3.0.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://secure.nic.cz/files/knot-dns/
PKG_HASH:=63756ac5a00c3e4a066ed231a287faef5963a9183d77326e30bf0644cdf74f86

PKG_MAINTAINER:=Daniel Salzman <daniel.salzman@nic.cz>
PKG_LICENSE:=GPL-3.0 LGPL-2.0 0BSD BSD-3-Clause OLDAP-2.8
PKG_CPE_ID:=cpe:/a:knot-dns:knot_dns

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk

define Package/knot-lib/Default
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Knot DNS
	DEPENDS=+libpthread
	URL:=https://www.knot-dns.cz
endef

define Package/knot/Default
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=IP Addresses and Names
	TITLE:=Knot DNS
	DEPENDS=+libatomic
	URL:=https://www.knot-dns.cz
endef

define Package/knot-libs
	$(call Package/knot-lib/Default)
	TITLE+= common DNS and DNSSEC libraries
	DEPENDS+=+libgnutls +lmdb @!arc
endef

define Package/knot-libzscanner
	$(call Package/knot-lib/Default)
	TITLE+= zone parser library
	DEPENDS+=@!arc
endef

define Package/knot
	$(call Package/knot/Default)
	TITLE+= server with control utility
	DEPENDS+=+libedit +liburcu +knot-libs +knot-libzscanner
	EXTRA_DEPENDS:=knot-libs (=$(PKG_VERSION)-$(PKG_RELEASE)), \
	               knot-libzscanner (=$(PKG_VERSION)-$(PKG_RELEASE))
	USERID:=knot=5353:knot=5353
endef

define Package/knot-dig
	$(call Package/knot/Default)
	TITLE+= advanced DNS lookup utility
	DEPENDS+=+libedit +knot-libs
	EXTRA_DEPENDS:=knot-libs (=$(PKG_VERSION)-$(PKG_RELEASE))
endef

define Package/knot-host
	$(call Package/knot/Default)
	TITLE+= simple DNS lookup utility
	DEPENDS+=+libedit +knot-libs
	EXTRA_DEPENDS:=knot-libs (=$(PKG_VERSION)-$(PKG_RELEASE))
endef

define Package/knot-nsupdate
	$(call Package/knot/Default)
	TITLE+= dynamic DNS update utility
	DEPENDS+=+libedit +knot-libs +knot-libzscanner
	EXTRA_DEPENDS:=knot-libs (=$(PKG_VERSION)-$(PKG_RELEASE)), \
	               knot-libzscanner (=$(PKG_VERSION)-$(PKG_RELEASE))
endef

define Package/knot-zonecheck
	$(call Package/knot/Default)
	TITLE+= zonefile check utility
	DEPENDS+=+libedit +liburcu +knot-libs +knot-libzscanner
	EXTRA_DEPENDS:=knot-libs (=$(PKG_VERSION)-$(PKG_RELEASE)), \
	               knot-libzscanner (=$(PKG_VERSION)-$(PKG_RELEASE))
endef

define Package/knot-keymgr
	$(call Package/knot/Default)
	TITLE+= DNSSEC key management utility
	DEPENDS+=+libedit +liburcu +knot-libs +knot-libzscanner
	EXTRA_DEPENDS:=knot-libs (=$(PKG_VERSION)-$(PKG_RELEASE)), \
	               knot-libzscanner (=$(PKG_VERSION)-$(PKG_RELEASE))
endef

define Package/knot-tests
	$(call Package/knot/Default)
	TITLE+= tests
	DEPENDS+=+libedit +liburcu +knot-libs +knot-libzscanner
	EXTRA_DEPENDS:=knot-libs (=$(PKG_VERSION)-$(PKG_RELEASE)), \
	               knot-libzscanner (=$(PKG_VERSION)-$(PKG_RELEASE))
endef

define Package/knot-libs/description
	Knot DNS common DNS and DNSSEC libraries.
endef

define Package/knot-libzscanner/description
	Knot DNS zone parser library.
endef

define Package/knot/description
	High-performance authoritative-only DNS server.
endef

define Package/knot-dig/description
	Knot DNS advanced DNS lookup utility.
endef

define Package/knot-host/description
	Knot DNS simple DNS lookup utility.
endef

define Package/knot-nsupdate/description
	Knot DNS dynamic DNS update utility.
endef

define Package/knot-zonecheck/description
	Knot DNS zonefile check utility.
endef

define Package/knot-keymgr/description
	Knot DNS DNSSEC key management utility.
endef

define Package/knot-tests/description
	Unit tests for the Knot DNS server and libraries.
	Usage: /usr/share/knot/runtests.sh
endef

export KNOT_VERSION_FORMAT=release

CONFIGURE_ARGS += 			\
	--enable-recvmmsg=no		\
	--enable-cap-ng=no            	\
	--enable-xdp=no                 \
	--disable-fastparser		\
	--without-libidn		\
	--with-libnghttp2=no		\
	--with-rundir=/var/run/knot	\
	--with-storage=/var/lib/knot	\
	--with-configdir=/etc/knot	\
	--with-conf-mapsize=20

TARGET_CFLAGS += -DNDEBUG

define Package/knot/conffiles
/etc/knot/knot.conf
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/tests check-compile
endef

ifneq ($(CONFIG_arc),y)
define Build/InstallDev
	$(INSTALL_DIR)					$(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.{a,so*}	$(1)/usr/lib/

	$(INSTALL_DIR)						$(1)/usr/include/libknot
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libknot/*		$(1)/usr/include/libknot/

	$(INSTALL_DIR)						$(1)/usr/include/libdnssec
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libdnssec/*	$(1)/usr/include/libdnssec/

	$(INSTALL_DIR)						$(1)/usr/include/libzscanner
	$(CP) $(PKG_INSTALL_DIR)/usr/include/libzscanner/*	$(1)/usr/include/libzscanner/

	$(INSTALL_DIR)							$(1)/usr/lib/pkgconfig
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc	$(1)/usr/lib/pkgconfig/
endef
endif

define Package/knot-libs/install
	$(INSTALL_DIR) 						$(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libdnssec.so.*		$(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libknot.so.*		$(1)/usr/lib/
endef

define Package/knot-libzscanner/install
	$(INSTALL_DIR) 						$(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libzscanner.so.*	$(1)/usr/lib/
endef

define Package/knot/install
	$(INSTALL_DIR) 						$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/knotc	$(1)/usr/bin/
	$(INSTALL_DIR) 						$(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/knotd	$(1)/usr/sbin/

	$(INSTALL_DIR) 							$(1)/etc/knot
	$(INSTALL_CONF) $(PKG_INSTALL_DIR)/etc/knot/knot.sample.conf	$(1)/etc/knot/knot.conf

	$(INSTALL_DIR)				$(1)/etc/init.d
	$(INSTALL_BIN) ./files/knotd.init	$(1)/etc/init.d/knotd
endef

define Package/knot-dig/install
	$(INSTALL_DIR)						$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kdig		$(1)/usr/bin/
endef

define Package/knot-host/install
	$(INSTALL_DIR)						$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/khost		$(1)/usr/bin/
endef

define Package/knot-nsupdate/install
	$(INSTALL_DIR)						$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/knsupdate	$(1)/usr/bin/
endef

define Package/knot-zonecheck/install
	$(INSTALL_DIR)						$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/kzonecheck	$(1)/usr/bin/
endef

define Package/knot-keymgr/install
	$(INSTALL_DIR)						$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/keymgr	$(1)/usr/bin/
endef

define Package/knot-tests/install
	$(INSTALL_DIR) 						$(1)/usr/share/knot
	$(INSTALL_BIN) ./files/runtests.sh			$(1)/usr/share/knot/

	$(INSTALL_DIR) 							$(1)/usr/share/knot/tap
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tests/tap/.libs/runtests	$(1)/usr/share/knot/tap/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tests/tap/libtap.sh		$(1)/usr/share/knot/tap/

	$(INSTALL_DIR)								$(1)/usr/share/knot/tests

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tests/libzscanner/.libs/zscanner-tool	$(1)/usr/share/knot/tests/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tests/libzscanner/test_zscanner		$(1)/usr/share/knot/tests/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/tests/libzscanner/TESTS		$(1)/usr/share/knot/tests/
	cp -a $(PKG_BUILD_DIR)/tests/libzscanner/data				$(1)/usr/share/knot/tests/

	for module in contrib knot libdnssec libknot modules utils; do \
		find $(PKG_BUILD_DIR)/tests/$$$${module}/.libs -maxdepth 1 -executable -type f | \
			xargs -I{} basename {} | \
			xargs -I{} $(INSTALL_BIN) -T $(PKG_BUILD_DIR)/tests/$$$${module}/.libs/{} \
										$(1)/usr/share/knot/tests/$$$${module}_{}; \
	done
endef

$(eval $(call BuildPackage,knot-libs))
$(eval $(call BuildPackage,knot-libzscanner))
$(eval $(call BuildPackage,knot))
$(eval $(call BuildPackage,knot-dig))
$(eval $(call BuildPackage,knot-host))
$(eval $(call BuildPackage,knot-nsupdate))
$(eval $(call BuildPackage,knot-zonecheck))
$(eval $(call BuildPackage,knot-keymgr))
$(eval $(call BuildPackage,knot-tests))
