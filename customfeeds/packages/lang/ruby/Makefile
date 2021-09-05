#
# Copyright (C) 2006-2016 OpenWrt.org
# Copyright (C) 2017-2020 Luiz Angelo Daros de Luca <luizluca@gmail.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# To Do:
#  - dirs not removed when uninstalling! opkg bug?
#
include $(TOPDIR)/rules.mk

PKG_NAME:=ruby
PKG_VERSION:=3.0.1
PKG_RELEASE:=1

# First two numbes
PKG_ABI_VERSION:=$(subst $(space),.,$(wordlist 1, 2, $(subst .,$(space),$(PKG_VERSION))))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://cache.ruby-lang.org/pub/ruby/$(PKG_ABI_VERSION)/
PKG_HASH:=d06bccd382d03724b69f674bc46cd6957ba08ed07522694ce44b9e8ffc9c48e2
PKG_MAINTAINER:=Luiz Angelo Daros de Luca <luizluca@gmail.com>
PKG_LICENSE:=BSD-2-Clause
PKG_LICENSE_FILES:=COPYING
PKG_CPE_ID:=cpe:/a:ruby-lang:ruby

PKG_BUILD_DEPENDS:=ruby/host
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

HOST_CONFIGURE_ARGS += \
	--disable-install-doc \
	--disable-install-rdoc \
	--disable-install-capi \
	--without-gmp \
	--with-out-ext=-test-/array/resize,-test-/bignum,-test-/bug-3571,-test-/bug-5832,-test-/bug_reporter,-test-/class,-test-/debug,-test-/dln/empty,-test-/exception,-test-/fatal,-test-/file,-test-/float,-test-/funcall,-test-/gvl/call_without_gvl,-test-/hash,-test-/integer,-test-/iseq_load,-test-/iter,-test-/load/dot.dot,-test-/marshal/compat,-test-/marshal/internal_ivar,-test-/marshal/usr,-test-/memory_status,-test-/method,-test-/notimplement,-test-/num2int,-test-/path_to_class,-test-/popen_deadlock,-test-/postponed_job,-test-/printf,-test-/proc,-test-/rational,-test-/recursion,-test-/st/foreach,-test-/st/numhash,-test-/st/update,-test-/string,-test-/struct,-test-/symbol,-test-/time,-test-/tracepoint,-test-/typeddata,-test-/vm,-test-/wait_for_single_fd,-test-/win32/console,-test-/win32/dln,-test-/win32/fd_setsize,bigdecimal,cgi/escape,continuation,coverage,dbm,etc,fcntl,fiber,fiddle,gdbm,io/console,json,json/generator,json/parser,mathn/complex,mathn/rational,nkf,objspace,pty,racc/cparse,rbconfig/sizeof,readline,rubyvm,sdbm,syslog,win32,win32ole,win32/resolv

# Does not compile with this. Workaround is --without-gmp
# https://bugs.ruby-lang.org/issues/11940
	#--with-static-linked-ext \

# even not used, host build with restricted exts results in gems not being
# compiled for target (probably some cross compiling problem like checking
# host for selecting target features)
#	--with-out-ext \
#	--with-ext=thread,stringio \

CONFIGURE_ARGS += \
	--enable-shared \
	--enable-static \
	--disable-rpath \
	$(call autoconf_bool,CONFIG_IPV6,ipv6) \
	--disable-install-doc \
	--disable-install-capi \
	--with-ruby-version=minor \
	--with-iconv-dir=$(ICONV_PREFIX) \
	--with-out-ext=win32,win32ole

ifndef CONFIG_RUBY_DIGEST_USE_OPENSSL
CONFIGURE_ARGS += \
	--with-bundled-sha1\
	--with-bundled-md5\
	--with-bundled-rmd160\
	--with-bundled-sha2 \

endif

# JIT requires a local cc installed and it is currently pointing to a wrong
# compiler (ccache) used during build, without a way to set it without a patch.
# Disabling it saves dozens of kbytes in libruby
CONFIGURE_ARGS += --disable-jit-support
# Host JIT does work but it is not worth it
HOST_CONFIGURE_ARGS += --disable-jit-support

TARGET_LDFLAGS += -L$(PKG_BUILD_DIR)

# Ruby uses DLDFLAGS and not LDFLAGS for libraries. LDFLAGS is only for execs.
# However, DLDFLAGS from configure is not passed to Makefile when target is linux.
# XLDFLAGS is used by both libraries and execs. This is somehow brute force but
# it will fix when some LD_FLAGS is needed for libraries. As side effect, it will
# duplicate ld args for binaries.
CONFIGURE_VARS += XLDFLAGS="$(TARGET_LDFLAGS)"

MAKE_FLAGS += \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	SHELL="/bin/bash"

define Build/InstallDev
	( cd $(PKG_INSTALL_DIR); $(TAR) -cf - \
		. \
	) | ( cd $(1); $(TAR) -xf - )
endef

define Host/Install
	# When ruby version is updated, make install asks in some cases before replace
	# an existing different file. Remove them before install and avoid the interaction
	rm -f $(STAGING_DIR_HOSTPKG)/bin/rake
	$(call Host/Install/Default)
endef

define Package/ruby/Default
  SUBMENU:=Ruby
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Ruby scripting language
  URL:=http://www.ruby-lang.org/
endef

define Package/ruby/Default/description
 Ruby is the interpreted scripting language for quick and easy
 object-oriented programming.  It has many features to process text files
 and to do system management tasks (as in perl).  It is simple,
 straight-forward, and extensible.

endef

define Package/ruby
$(call Package/ruby/Default)
  TITLE+= (interpreter)
  DEPENDS:=+libruby
endef

define Package/ruby/description
$(call Package/ruby/Default/description)
endef

define RubyDependency
    $(eval \
        $(call Package/Default)
        $(call Package/ruby-$(1))
        FILTER_CONFIG:=$$(strip \
            $$(foreach config_dep, \
                $$(filter @%, \
                    $$(foreach v, \
                        $$(DEPENDS), \
                        $$(if $$(findstring :,$$v),,$$v) \
                    ) \
                ), \
                $$(subst @,,$$(config_dep)) \
            ) \
        )
        ifneq (,$$(FILTER_CONFIG))
           FILTER_CONFIG:=($$(subst $$(space),&&,$$(FILTER_CONFIG))):
        endif
    ) \
    +$(FILTER_CONFIG)ruby-$(1)
endef

define Package/ruby/config
    comment "Standard Library"
      depends on PACKAGE_ruby

    config PACKAGE_ruby-stdlib
      depends on PACKAGE_ruby
      default m if ALL
      prompt "Select Ruby Complete Standard Library (ruby-stdlib)"

endef

define Package/ruby/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib/ruby/$(PKG_ABI_VERSION)
	$(INSTALL_DIR) $(1)/usr/lib/ruby/vendor_ruby/$(PKG_ABI_VERSION)
	$(INSTALL_DIR) $(1)/usr/lib/ruby/site_ruby/$(PKG_ABI_VERSION)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ruby $(1)/usr/lib/ruby/ruby$(PKG_ABI_VERSION)-bin
	$(INSTALL_BIN) ./files/ruby $(1)/usr/bin/ruby
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ruby/vendor_ruby/$(PKG_ABI_VERSION)/* $(1)/usr/lib/ruby/vendor_ruby/$(PKG_ABI_VERSION)/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/ruby/site_ruby/$(PKG_ABI_VERSION)/* $(1)/usr/lib/ruby/site_ruby/$(PKG_ABI_VERSION)/
	$(SED) "s%@RUBY_LIBPATH@%/usr/lib/ruby/$(PKG_ABI_VERSION)%" $(1)/usr/bin/ruby
	$(SED) "s%@RUBY_BINPATH@%/usr/lib/ruby/ruby$(PKG_ABI_VERSION)-bin%" $(1)/usr/bin/ruby
endef

define Package/libruby
$(call Package/ruby/Default)
  SUBMENU:=
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE+= (shared library)
  DEPENDS+= +libpthread +librt +libgmp +zlib
  ABI_VERSION:=$(PKG_ABI_VERSION)
endef
define Package/libruby/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libruby.so.* $(1)/usr/lib/
endef

define Package/ruby-dev
$(call Package/ruby/Default)
  TITLE+= (dev files)
  DEPENDS:=+libruby ruby
endef
define Package/ruby-dev/description
  Header files for compiling extension modules for the Ruby $(PKG_ABI_VERSION)
endef
define Package/ruby-dev/install
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/ruby-$(PKG_ABI_VERSION) $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libruby.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/ruby-$(PKG_ABI_VERSION).pc $(1)/usr/lib/pkgconfig/
endef

RUBY_STDLIB :=
define Package/ruby-stdlib
$(call Package/ruby/Default)
  TITLE:=Ruby standard libraries (metadata for all stdlib subsets)
  DEPENDS:=ruby $(foreach subpackage,$(RUBY_STDLIB),$(strip $(call RubyDependency,$(subpackage))))
  HIDDEN:=1
endef

define Package/ruby-stdlib/description
 This metapackage currently install all ruby-* packages,
 providing a complete Ruby Standard Library.

endef

# nothing to do
define Package/ruby-stdlib/install
	/bin/true
endef

define Package/ruby-abbrev/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/abbrev.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/abbrev-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/abbrev-*.gemspec
endef

define Package/ruby-base64/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/base64.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/base64-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/base64-*.gemspec
endef

define Package/ruby-benchmark/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/benchmark.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/benchmark/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/benchmark-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/benchmark-*.gemspec
endef

define Package/ruby-bigdecimal/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/bigdecimal.so
/usr/lib/ruby/$(PKG_ABI_VERSION)/bigdecimal/
/usr/lib/ruby/$(PKG_ABI_VERSION)/bigdecimal.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/bigdecimal-*.gemspec
endef

define Package/ruby-bundler/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/bundler.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/bundler/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/bundler-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/bundler-*.gemspec
endef
define Package/ruby-bundler/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/bundler-*/doc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/bundler-*/test
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/bundler-*/sample
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/bundler-*/man
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/bundler-*/*.md
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/bundler-*/*.txt
endef
define Package/ruby-bundler/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bundle $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bundler $(1)/usr/bin/
	$(call RubyBuildPackage/install,bundler,$(1))
endef

define Package/ruby-cgi/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/cgi.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/cgi/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/cgi-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/cgi-*.gemspec
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/cgi/escape.so
endef

define Package/ruby-coverage/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/coverage.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/coverage.so
endef

define Package/ruby-continuation/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/continuation.so
endef

define Package/ruby-csv/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/csv.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/csv/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/csv-*.gemspec
endef

define Package/ruby-date/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/date.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/date_core.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/date-*.gemspec
endef

define Package/ruby-dbm/description
 The DBM class provides a wrapper to a Unix-style dbm or Database Manager library.
 This package provides dbm.so file.

endef
define Package/ruby-dbm/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/dbm.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/dbm-*.gemspec
endef

define Package/ruby-debug/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/debug.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/debug-*.gemspec
endef

define Package/ruby-delegate/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/delegate.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/delegate/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/delegate-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/delegate-*.gemspec
endef

define Package/ruby-did-you-mean/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/did_you_mean.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/did_you_mean/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/did_you_mean-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/did_you_mean-*.gemspec
endef

define Package/ruby-digest/description
 Provides digest* files. Can be configured to use OpenSSL or
 bundled hash functions.

endef
define Package/ruby-digest/config

	config RUBY_DIGEST_USE_OPENSSL
		bool "Use OpenSSL functions for ruby digest hash functions"
        depends on PACKAGE_ruby-digest
		help
			Ruby can use OpenSSL hash functions or compile alternative implementations. Using
			OpenSSL saves about 30KBytes (less when compressed) but requires OpenSSL (that
			is way bigger than that). However, if OpenSSL is already needed by another usage,
			as ruby-openssl or any other non ruby package, it is better to mark this option.
		default n

endef
define Package/ruby-digest/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/digest
/usr/lib/ruby/$(PKG_ABI_VERSION)/digest.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/digest.so
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/digest/*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/digest-*.gemspec
endef

define Package/ruby-drb/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/drb.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/drb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/drb-*.gemspec
endef

define Package/ruby-enc/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/enc/encdb.so
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/enc/iso_8859_1.so
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/enc/utf_*.so
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/enc/euc_jp.so
endef

define Package/ruby-enc-extra/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/enc
endef
define Package/ruby-enc-extra/files-excluded
$(call Package/ruby-enc/files)
endef

define Package/ruby-english/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/English.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/english-*.gemspec
endef

define Package/ruby-erb/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/erb.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/erb-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/erb-*.gemspec
endef
define Package/ruby-erb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/erb $(1)/usr/bin/
	$(call RubyBuildPackage/install,erb,$(1))
endef

define Package/ruby-etc/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/etc.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/etc-*.gemspec
endef

define Package/ruby-expect/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/expect.rb
endef

define Package/ruby-fcntl/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/fcntl.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/fcntl-*.gemspec
endef

define Package/ruby-fiber/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/fiber.so
endef

define Package/ruby-fiddle/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/fiddle.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/fiddle/
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/fiddle.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/fiddle-*.gemspec
endef

define Package/ruby-fileutils/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/fileutils.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/fileutils-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/fileutils-*.gemspec
endef

define Package/ruby-find/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/find.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/find-*.gemspec
endef

define Package/ruby-forwardable/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/forwardable.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/forwardable
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/forwardable-*.gemspec
endef

define Package/ruby-gdbm/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/gdbm.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/gdbm-*.gemspec
endef

define Package/ruby-gems/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/rubygems.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/rubygems
endef
define Package/ruby-gems/files-excluded
/usr/lib/ruby/$(PKG_ABI_VERSION)/rubygems/test_case.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/rubygems/package/tar_test_case.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/rubygems/installer_test_case.rb
endef
define Package/ruby-gems/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/gem $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default
	$(INSTALL_DIR) $(1)/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems
	$(INSTALL_DIR) $(1)/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/doc
	$(INSTALL_DIR) $(1)/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/cache
	$(INSTALL_DIR) $(1)/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/extensions
	$(INSTALL_DIR) $(1)/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/build_info
	$(call RubyBuildPackage/install,gems,$(1))
endef

define Package/ruby-getoptlong/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/getoptlong.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/getoptlong/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/getoptlong-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/getoptlong-*.gemspec
endef

define Package/ruby-io-console/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/io/console.so
/usr/lib/ruby/$(PKG_ABI_VERSION)/io/console/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/io-console-*.gemspec
endef

define Package/ruby-io-nonblock/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/io/nonblock.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/io-nonblock-*.gemspec
endef

define Package/ruby-io-wait/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/io/wait.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/io-wait-*.gemspec
endef

define Package/ruby-ipaddr/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/ipaddr.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/ipaddr-*.gemspec
endef

define Package/ruby-irb/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/irb
/usr/lib/ruby/$(PKG_ABI_VERSION)/irb.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/irb-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/irb-*.gemspec
endef
define Package/ruby-irb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/irb $(1)/usr/bin/
	$(call RubyBuildPackage/install,irb,$(1))
endef

define Package/ruby-json/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/json.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/json
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/json
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/json-*.gemspec
endef
define Package/ruby-json/files-excluded
$(call Package/ruby-psych/files)
endef

define Package/ruby-logger/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/logger.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/logger/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/logger-*.gemspec
endef

define Package/ruby-matrix/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/matrix.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/matrix
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/matrix-*.gemspec
endef

define Package/ruby-minitest/files
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/minitest-*.gemspec
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/minitest-*
endef
define Package/ruby-minitest/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/minitest-*/test
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/minitest-*/*.rdoc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/minitest-*/*.txt
endef

define Package/ruby-mkmf/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/mkmf.rb
endef

define Package/ruby-monitor/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/monitor.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/monitor.so
endef

define Package/ruby-mutex_m/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/mutex_m.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/mutex_m-*.gemspec
endef

define Package/ruby-net-ftp/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/ftp.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/net-ftp-*.gemspec
endef

define Package/ruby-net-http/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/http.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/https.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/http/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/net-http-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/net-http-*.gemspec
endef

define Package/ruby-net-imap/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/imap.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/net-imap-*.gemspec
endef

define Package/ruby-net-pop/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/pop.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/pop/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/net-pop-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/net-pop-*.gemspec
endef

define Package/ruby-net-protocol/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/protocol.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/net-protocol-*.gemspec
endef

define Package/ruby-net-pop/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/pop.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/pop/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/net-pop-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/net-pop-*.gemspec
endef

define Package/ruby-net-smtp/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/smtp.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/net/smtp/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/net-smtp-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/net-smtp-*.gemspec
endef

define Package/ruby-nkf/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/kconv.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/nkf.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/nkf-*.gemspec
endef

define Package/ruby-objspace/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/objspace.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/objspace.so
endef

define Package/ruby-observer/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/observer.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/observer/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/observer-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/observer-*.gemspec
endef

define Package/ruby-open-uri/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/open-uri.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/open-uri-*.gemspec
endef

define Package/ruby-open3/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/open3.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/open3/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/open3-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/open3-*.gemspec
endef

define Package/ruby-openssl/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/openssl
/usr/lib/ruby/$(PKG_ABI_VERSION)/openssl.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/openssl.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/openssl-*.gemspec
endef

define Package/ruby-optparse/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/optparse.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/optionparser.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/optparse
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/optparse-*.gemspec
endef

define Package/ruby-ostruct/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/ostruct.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/ostruct/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/ostruct-*.gemspec
endef

define Package/ruby-pathname/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/pathname.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/pathname.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/pathname-*.gemspec
endef

define Package/ruby-powerassert/files
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/power_assert-*.gemspec
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/power_assert-*
endef
define Package/ruby-powerassert/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/power_assert-*/*.rdoc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/power_assert-*/.travis.yml
endef

define Package/ruby-pp/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/pp.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/pp-*.gemspec
endef

define Package/ruby-prettyprint/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/prettyprint.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/prettyprint-*.gemspec
endef

define Package/ruby-prime/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/prime.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/prime-*.gemspec
endef

define Package/ruby-pstore/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/pstore.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/pstore/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/pstore-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/pstore-*.gemspec
endef

define Package/ruby-psych/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/psych
/usr/lib/ruby/$(PKG_ABI_VERSION)/psych.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/psych.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/psych-*.gemspec
endef

define Package/ruby-pty/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/pty.so
endef

define Package/ruby-racc/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/racc.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/racc
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/racc/*.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/racc-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/racc-*.gemspec
endef
define Package/ruby-racc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/racc $(1)/usr/bin/;
	$(call RubyBuildPackage/install,racc,$(1))
endef

define Package/ruby-rake/files
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/rake-*.gemspec
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rake-*/
endef
define Package/ruby-rake/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rake-*/doc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rake-*/*.rdoc
endef
define Package/ruby-rake/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rake $(1)/usr/bin/;
	$(call RubyBuildPackage/install,rake,$(1))
endef

define Package/ruby-rbconfig/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/rbconfig.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/rbconfig/*.so
endef

define Package/ruby-rbs/files
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rbs-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/rbs-*.gemspec
endef
define Package/ruby-rbs/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rbs-*/docs
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rbs-*/test
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rbs-*/sample
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rbs-*/*.md
endef
define Package/ruby-rbs/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rbs $(1)/usr/bin/
	$(call RubyBuildPackage/install,rbs,$(1))
endef

define Package/ruby-rdoc/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/rdoc.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/rdoc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rdoc-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/rdoc-*.gemspec
endef
define Package/ruby-rdoc/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/rdoc $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/ri $(1)/usr/bin/
	$(call RubyBuildPackage/install,rdoc,$(1))
endef

define Package/ruby-readline/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/readline.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/readline-0*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/readline-0*.gemspec
endef

define Package/ruby-readline-ext/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/readline.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/readline-ext-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/readline-ext-*.gemspec
endef

define Package/ruby-reline/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/reline.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/reline
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/reline-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/reline-*.gemspec
endef

define Package/ruby-resolv/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/resolv.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/resolv-*.gemspec
endef
define Package/ruby-resolv/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/resolv-replace*.gemspec
endef

define Package/ruby-resolv-replace/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/resolv-replace.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/resolv-replace*.gemspec
endef

define Package/ruby-rexml/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/rexml
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rexml-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/rexml-*.gemspec
endef
define Package/ruby-rexml/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rexml-*/doc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rexml-*/test
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rexml-*/sample
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rexml-*/*.md
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rexml-*/.travis.yml
endef

define Package/ruby-rinda/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/rinda
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/rinda-*.gemspec
endef

define Package/ruby-ripper/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/ripper.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/ripper
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/ripper.so
endef

define Package/ruby-rss/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/rss
/usr/lib/ruby/$(PKG_ABI_VERSION)/rss.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rss-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/rss-*.gemspec
endef
define Package/ruby-rss/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rss-*/doc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rss-*/test
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rss-*/sample
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rss-*/*.md
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/rss-*/*.txt
endef

define Package/ruby-securerandom/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/securerandom.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/securerandom-*.gemspec
endef

define Package/ruby-set/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/set.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/set/*.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/set-*.gemspec
endef

define Package/ruby-shellwords/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/shellwords.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/shellwords-*.gemspec
endef

define Package/ruby-singleton/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/singleton.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/singleton/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/singleton-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/singleton-*.gemspec
endef

define Package/ruby-socket/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/socket.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/socket.so
endef

define Package/ruby-stringio/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/stringio.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/stringio-*.gemspec
endef

define Package/ruby-strscan/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/strscan.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/strscan-*.gemspec
endef

define Package/ruby-syslog/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/syslog.so
/usr/lib/ruby/$(PKG_ABI_VERSION)/syslog/logger.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/syslog-*.gemspec
endef

define Package/ruby-testunit/files
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/test-unit-*.gemspec
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/test-unit-*
endef
define Package/ruby-testunit/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/test-unit-*/doc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/test-unit-*/test
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/test-unit-*/sample
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/test-unit-*/*.md
endef

define Package/ruby-time/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/time.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/time-*.gemspec
endef

define Package/ruby-timeout/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/timeout.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/timeout/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/timeout-*
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/timeout-*.gemspec
endef

define Package/ruby-tempfile/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/tempfile.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/tempfile-*.gemspec
endef

define Package/ruby-tmpdir/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/tmpdir.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/tmpdir-*.gemspec
endef

define Package/ruby-tracer/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/tracer.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/tracer/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/tracer-*.gemspec
endef

define Package/ruby-tsort/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/tsort.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/tsort-*.gemspec
endef

define Package/ruby-typeprof/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/typeprof.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/typeprof
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/typeprof-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/typeprof-*.gemspec
endef
define Package/ruby-typeprof/files-excluded
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/typeprof-*/doc
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/typeprof-*/test
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/typeprof-*/sample
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/typeprof-*/*.md
endef
define Package/ruby-typeprof/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/typeprof $(1)/usr/bin/
	$(call RubyBuildPackage/install,typeprof,$(1))
endef

define Package/ruby-un/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/un.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/un-*.gemspec
endef

define Package/ruby-unicodenormalize/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/unicode_normalize
endef

define Package/ruby-uri/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/uri.rb
/usr/lib/ruby/$(PKG_ABI_VERSION)/uri
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/uri-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/uri-*.gemspec
endef

define Package/ruby-weakref/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/weakref.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/weakref-*.gemspec
endef

define Package/ruby-yaml/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/yaml
/usr/lib/ruby/$(PKG_ABI_VERSION)/yaml.rb
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/gems/yaml-*/
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/yaml-*.gemspec
endef

define Package/ruby-zlib/files
/usr/lib/ruby/$(PKG_ABI_VERSION)/*/zlib.so
/usr/lib/ruby/gems/$(PKG_ABI_VERSION)/specifications/default/zlib-*.gemspec
endef

RUBY_FILES = $(strip $(call Package/ruby-$(1)/files))
RUBY_FILES_EXCLUDED = $(strip $(call Package/ruby-$(1)/files-excluded))

# 1: short name
# 2: install dir
define RubyBuildPackage/install
	( \
	  cd $(PKG_INSTALL_DIR) && \
	  $(TAR) -cf - \
	    $(if $(RUBY_FILES_EXCLUDED),--exclude-from <(ls -1d $(patsubst /%,%,$(RUBY_FILES_EXCLUDED)))) \
	    --files-from <(ls -1d $(patsubst /%,%,$(RUBY_FILES))) \
	) | ( \
	  [ -n "$(2)" ] && cd $(2) && $(TAR) -xf - \
	)
endef

# 1: short name
# 2: description
# 3: dependencies on other packages
define RubyBuildPackage
  RUBY_STDLIB += $(1)

  # Package definition
  ifndef Package/ruby-$(1)
    define Package/ruby-$(1)
      $(call Package/ruby/Default)
      TITLE:=Ruby $(2)
      DEPENDS:=ruby $(3)
    endef
  endif

  ifndef Package/ruby-$(1)/description
    define Package/ruby-$(1)/description
      This package contains the ruby $(2).

    endef
  endif

  # Description
  ifndef Package/ruby-$(1)/install
    ifndef Package/ruby-$(1)/files
       $$(error It must exists either a Package/ruby-$(1)/install or Package/ruby-$(1)/files)
    endif

    define Package/ruby-$(1)/description +=

Provides:
$(patsubst /%,
 - /%,$(RUBY_FILES))

    endef

    ifneq ($(RUBY_FILES_EXCLUDED),)
      define Package/ruby-$(1)/description +=

Except:
$(patsubst /%,
 - /%,$(RUBY_FILES_EXCLUDED))

      endef
    endif

    Package/ruby-$(1)/install=$(call RubyBuildPackage/install,$(1),$$(1))
  endif

  $$(eval $$(call BuildPackage,ruby-$(1)))
endef

$(eval $(call BuildPackage,libruby))
$(eval $(call BuildPackage,ruby))
$(eval $(call BuildPackage,ruby-dev))
$(eval $(call RubyBuildPackage,abbrev,Calculates the set of unambiguous abbreviations for a given set of strings,))
$(eval $(call RubyBuildPackage,base64,Encode and decode base64,))
$(eval $(call RubyBuildPackage,benchmark,Performance benchmarking library,))
$(eval $(call RubyBuildPackage,bigdecimal,Arbitrary-precision decimal floating-point library,))
$(eval $(call RubyBuildPackage,bundler,Manage dependencies,+ruby-erb +ruby-irb +ruby-open-uri +ruby-readline))
$(eval $(call RubyBuildPackage,cgi,CGI support toolkit,+ruby-pstore +ruby-securerandom +ruby-shellwords +ruby-tempfile))
$(eval $(call RubyBuildPackage,continuation,Similar to C setjmp/longjmp with extra states,))
$(eval $(call RubyBuildPackage,coverage,Coverage measurement,))
$(eval $(call RubyBuildPackage,csv,CSV Reading and Writing,+ruby-date +ruby-english +ruby-forwardable +ruby-stringio +ruby-strscan))
$(eval $(call RubyBuildPackage,date,Comparable module for handling dates,))
$(eval $(call RubyBuildPackage,dbm,Wrapper for the UNIX-style Database Manager Library,+libdb47))
$(eval $(call RubyBuildPackage,debug,generic command line interface for ruby-debug,+ruby-continuation +ruby-pp +ruby-readline +ruby-tracer))
$(eval $(call RubyBuildPackage,delegate,lib to delegate method calls to an object,))
$(eval $(call RubyBuildPackage,did-you-mean,did you mean? experience,))
$(eval $(call RubyBuildPackage,digest,Digest Library,+RUBY_DIGEST_USE_OPENSSL:libopenssl))
$(eval $(call RubyBuildPackage,drb,distributed object system,+ruby-ipaddr +ruby-monitor +ruby-observer +ruby-singleton +ruby-tmpdir))
$(eval $(call RubyBuildPackage,enc,character re-coding library charset (small subset),))
$(eval $(call RubyBuildPackage,enc-extra,character re-coding library charset (extra subset),+ruby-enc))
$(eval $(call RubyBuildPackage,english,Reference some global vars as english variables,))
$(eval $(call RubyBuildPackage,erb,(embedded interpreter),+ruby-gems))
$(eval $(call RubyBuildPackage,etc,Access info typically stored in /etc,))
$(eval $(call RubyBuildPackage,expect,Expect-like for IO,))
$(eval $(call RubyBuildPackage,fcntl,Loads constants defined in the OS fcntl.h C header file,))
$(eval $(call RubyBuildPackage,fiber,Light weight cooperative concurrency,))
$(eval $(call RubyBuildPackage,fiddle,Libffi wrapper for Ruby,+libffi))
$(eval $(call RubyBuildPackage,fileutils,File utility methods for copying moving removing etc,+ruby-enc +ruby-etc +ruby-rbconfig +ruby-socket))
$(eval $(call RubyBuildPackage,find,top-down traversal of a set of file paths,+ruby-enc))
$(eval $(call RubyBuildPackage,forwardable,delegation of methods to a object,))
$(eval $(call RubyBuildPackage,gdbm,Ruby extension for GNU dbm,+libgdbm))
$(eval $(call RubyBuildPackage,gems,gems packet management,+ruby-base64 +ruby-cgi +ruby-json +ruby-net-http +ruby-open3 +ruby-pathname +ruby-pp +ruby-rake +ruby-resolv +ruby-yaml))
$(eval $(call RubyBuildPackage,getoptlong,implementation of getoptLong,))
$(eval $(call RubyBuildPackage,io-console,Console interface,))
$(eval $(call RubyBuildPackage,io-nonblock,Non-blocking mode with IO class,))
$(eval $(call RubyBuildPackage,io-wait,Waits until IO is readable or writable without blocking,))
$(eval $(call RubyBuildPackage,ipaddr,Set of methods to manipulate an IP address,+ruby-socket))
$(eval $(call RubyBuildPackage,irb,(interactive shell),+ruby-gems +ruby-reline +ruby-ripper +ruby-tracer))
$(eval $(call RubyBuildPackage,json,JSON Implementation for Ruby,+ruby-date +ruby-ostruct))
$(eval $(call RubyBuildPackage,logger,logger and syslog library,+ruby-monitor))
$(eval $(call RubyBuildPackage,matrix,implementation of Matrix and Vector classes,))
$(eval $(call RubyBuildPackage,minitest,Gem minitest,+ruby-gems +ruby-mutex_m))
$(eval $(call RubyBuildPackage,mkmf,makefile library,+ruby-shellwords +ruby-tmpdir))
$(eval $(call RubyBuildPackage,monitor,Object or module methods are executed with mutual exclusion,))
$(eval $(call RubyBuildPackage,mutex_m,extend objects to be handled like a Mutex,))
$(eval $(call RubyBuildPackage,net-ftp,FTP lib,+ruby-monitor +ruby-net-protocol +ruby-openssl +ruby-time))
$(eval $(call RubyBuildPackage,net-http,HTTP lib,+ruby-net-protocol +ruby-securerandom +ruby-tempfile +ruby-uri +ruby-zlib))
$(eval $(call RubyBuildPackage,net-imap,IMAP lib,+ruby-monitor +ruby-net-protocol +ruby-openssl +ruby-strscan))
$(eval $(call RubyBuildPackage,net-pop,POP3 lib,+ruby-net-protocol +ruby-openssl))
$(eval $(call RubyBuildPackage,net-protocol,Abstract for net-* clients,+ruby-socket +ruby-timeout))
$(eval $(call RubyBuildPackage,net-smtp,SMTP lib,+ruby-net-protocol +ruby-openssl))
$(eval $(call RubyBuildPackage,nkf,Network Kanji Filter,+ruby-enc))
$(eval $(call RubyBuildPackage,objspace,Routines to interact with the garbage collection facility,+ruby-tempfile))
$(eval $(call RubyBuildPackage,observer,Observer design pattern,))
$(eval $(call RubyBuildPackage,open-uri,Wrapper for Net::HTTP Net::HTTPS and Net::,+ruby-net-ftp +ruby-net-http))
$(eval $(call RubyBuildPackage,open3,popen with stderr,))
$(eval $(call RubyBuildPackage,openssl,SSL TLS and general purpose cryptography,+ruby-digest +ruby-enc +ruby-io-nonblock +ruby-ipaddr +ruby-stringio +libopenssl))
$(eval $(call RubyBuildPackage,optparse,command-line option analysis,+ruby-shellwords +ruby-time))
$(eval $(call RubyBuildPackage,ostruct,build custom data structures,))
$(eval $(call RubyBuildPackage,pathname,Pathname lib,+ruby-fileutils +ruby-find))
$(eval $(call RubyBuildPackage,powerassert,Gem power_assert,+ruby-pp +ruby-ripper))
$(eval $(call RubyBuildPackage,pp,Pretty print objects,+ruby-etc +ruby-prettyprint))
$(eval $(call RubyBuildPackage,prettyprint,PrettyPrint library,))
$(eval $(call RubyBuildPackage,prime,Prime numbers and factorization library,+ruby-forwardable +ruby-singleton))
$(eval $(call RubyBuildPackage,pstore,file based persistence,+ruby-digest +ruby-enc))
$(eval $(call RubyBuildPackage,psych,YAML parser and emitter,+ruby-bigdecimal +ruby-date +ruby-enc +ruby-forwardable +ruby-stringio +ruby-strscan +libyaml))
$(eval $(call RubyBuildPackage,pty,Creates and manages pseudo terminals,))
$(eval $(call RubyBuildPackage,racc,LALR parser generator,+ruby-forwardable +ruby-optparse +ruby-rbconfig +ruby-stringio))
$(eval $(call RubyBuildPackage,rake,Rake (make replacement),+ruby-fileutils +ruby-monitor +ruby-optparse +ruby-ostruct +ruby-set +ruby-singleton))
$(eval $(call RubyBuildPackage,rbconfig,RbConfig,))
$(eval $(call RubyBuildPackage,rbs,RBS provides syntax and semantics definition for the Ruby Signature language,+ruby-gems +ruby-logger +ruby-racc +ruby-ripper))
$(eval $(call RubyBuildPackage,rdoc,RDoc produces HTML and command-line documentation for Ruby projects,+ruby-abbrev +ruby-did-you-mean +ruby-erb +ruby-racc +ruby-ripper))
$(eval $(call RubyBuildPackage,readline-ext,support for native GNU readline,+libncurses +libreadline))
$(eval $(call RubyBuildPackage,readline,loads readline-ext(native) or reline(ruby),+ruby-reline))
$(eval $(call RubyBuildPackage,reline,alternative to readline-ext in pure ruby,+ruby-fiddle +ruby-forwardable +ruby-io-console +ruby-tempfile +ruby-timeout))
$(eval $(call RubyBuildPackage,resolv,DNS resolver library,+ruby-securerandom +ruby-timeout))
$(eval $(call RubyBuildPackage,resolv-replace,Replace Socket DNS with Resolv,+ruby-resolv))
$(eval $(call RubyBuildPackage,rexml,XML toolkit,+ruby-enc +ruby-forwardable +ruby-pp +ruby-set +ruby-stringio +ruby-strscan))
$(eval $(call RubyBuildPackage,rinda,Linda paradigm implementation,+ruby-drb +ruby-forwardable))
$(eval $(call RubyBuildPackage,ripper,script parser,))
$(eval $(call RubyBuildPackage,rss,RSS toolkit,+ruby-english +ruby-nkf +ruby-open-uri +ruby-rexml))
$(eval $(call RubyBuildPackage,securerandom,Secure random number generators,+ruby-openssl))
$(eval $(call RubyBuildPackage,set,Set collection,+ruby-tsort))
$(eval $(call RubyBuildPackage,shellwords,Manipulate strings as Bourne Shell,))
$(eval $(call RubyBuildPackage,singleton,Singleton pattern,))
$(eval $(call RubyBuildPackage,socket,socket support,+ruby-io-wait))
$(eval $(call RubyBuildPackage,stringio,Pseudo `IO` class from/to `String`,))
$(eval $(call RubyBuildPackage,strscan,Lexical scanning operations on a String,))
$(eval $(call RubyBuildPackage,syslog,Syslog Lib,+ruby-logger))
$(eval $(call RubyBuildPackage,tempfile,Manages temporary files,+ruby-delegate +ruby-tmpdir))
$(eval $(call RubyBuildPackage,testunit,Gem test-unit,+ruby-csv +ruby-erb +ruby-powerassert +ruby-rexml))
$(eval $(call RubyBuildPackage,time,Extends Time with additional methods for parsing and converting Times,+ruby-date))
$(eval $(call RubyBuildPackage,timeout,Auto-terminate potentially long-running operations,))
$(eval $(call RubyBuildPackage,tmpdir,Get temp dir path,+ruby-fileutils))
$(eval $(call RubyBuildPackage,tracer,Outputs a source level execution trace of a Ruby program,))
$(eval $(call RubyBuildPackage,tsort,Topological sorting using Tarjan s algorithm,))
$(eval $(call RubyBuildPackage,typeprof,A type analysis tool for Ruby code based on abstract interpretation,+ruby-coverage +ruby-rbs))
$(eval $(call RubyBuildPackage,unicodenormalize,String additions for Unicode normalization,+ruby-enc +ruby-enc-extra))
$(eval $(call RubyBuildPackage,un,Utilities to replace common UNIX commands in Makefiles,+ruby-mkmf +ruby-optparse))
$(eval $(call RubyBuildPackage,uri,library to handle URI,+ruby-enc))
$(eval $(call RubyBuildPackage,weakref,Weak reference to be garbage collected,+ruby-delegate))
$(eval $(call RubyBuildPackage,yaml,YAML toolkit,+ruby-dbm +ruby-pstore +ruby-psych))
$(eval $(call RubyBuildPackage,zlib,compression/decompression library interface,))
$(eval $(call BuildPackage,ruby-stdlib))
$(eval $(call HostBuild))
