# This makefile simplifies perl module builds.
#

ifeq ($(origin PERL_INCLUDE_DIR),undefined)
  PERL_INCLUDE_DIR:=$(dir $(lastword $(MAKEFILE_LIST)))
endif

include $(PERL_INCLUDE_DIR)/perlver.mk

ifneq ($(PKG_NAME),perl)
  PKG_VERSION:=$(PKG_VERSION)+perl$(PERL_VERSION2)
endif

PERL_VERSION:=$(PERL_VERSION2)

# Build environment
HOST_PERL_PREFIX:=$(STAGING_DIR_HOSTPKG)/usr
ifneq ($(CONFIG_USE_GLIBC),)
	EXTRA_LIBS:=bsd
	EXTRA_LIBDIRS:=$(STAGING_DIR)/lib
endif
PERL_CMD:=$(STAGING_DIR_HOSTPKG)/usr/bin/perl$(PERL_VERSION3)

MOD_CFLAGS_PERL:=-D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 $(TARGET_CFLAGS) $(TARGET_CPPFLAGS)
ifdef CONFIG_PERL_THREADS
	MOD_CFLAGS_PERL+= -D_REENTRANT -D_GNU_SOURCE
endif

# Module install prefix
PERL_SITELIB:=/usr/lib/perl5/$(PERL_VERSION)
PERL_TESTSDIR:=/usr/share/perl/perl-tests
PERLBASE_TESTSDIR:=/usr/share/perl/perlbase-tests
PERLMOD_TESTSDIR:=/usr/share/perl/perlmod-tests

define perlmod/host/relink
	rm -f $(1)/Makefile.aperl
	$(MAKE) -C $(1) perl
	$(CP) $(1)/perl $(PERL_CMD)
	$(CP) $(1)/perl $(STAGING_DIR_HOSTPKG)/usr/bin/perl
endef

define perlmod/host/Configure
	(cd $(HOST_BUILD_DIR); \
	PERL_MM_USE_DEFAULT=1 \
	$(2) \
	$(PERL_CMD) Makefile.PL \
		$(1) \
	);
endef

define perlmod/host/Compile
	$(2) \
	$(MAKE) -C $(HOST_BUILD_DIR) \
		$(1) \
		install
endef

define perlmod/host/Install
	$(2) \
	$(MAKE) -C $(HOST_BUILD_DIR) \
		$(1) \
		install
	$(call perlmod/host/relink,$(HOST_BUILD_DIR))
endef

define perlmod/Configure
	(cd $(if $(3),$(3),$(PKG_BUILD_DIR)); \
	 (echo -e 'use Config;\n\n$$$${tied %Config::Config}{cpprun}="$(GNU_TARGET_NAME)-cpp -E";\n' ; cat Makefile.PL) | \
	 PERL_MM_USE_DEFAULT=1 \
	 $(2) \
	 $(PERL_CMD) -I. -- - \
		$(1) \
		AR=ar \
		CC=$(GNU_TARGET_NAME)-gcc \
		CCFLAGS="$(MOD_CFLAGS_PERL)" \
		CCCDLFLAGS=-fPIC \
		CCDLFLAGS=-Wl,-E \
		DLEXT=so \
		DLSRC=dl_dlopen.xs \
		EXE_EXT=" " \
		FULL_AR=$(GNU_TARGET_NAME)-ar \
		LD=$(GNU_TARGET_NAME)-gcc \
		LDDLFLAGS="-shared -rdynamic $(TARGET_LDFLAGS)"  \
		LDFLAGS="$(EXTRA_LIBDIRS:%=-L%) $(EXTRA_LIBS:%=-l%) " \
		LIBC=" " \
		LIB_EXT=.a \
		OBJ_EXT=.o \
		OSNAME=linux \
		OSVERS=2.4.30 \
		RANLIB=: \
		SITELIBEXP=" " \
		SITEARCHEXP=" " \
		SO=so  \
		VENDORARCHEXP=" " \
		VENDORLIBEXP=" " \
		SITEPREFIX=/usr \
		INSTALLPRIVLIB=$(PERL_SITELIB) \
		INSTALLSITELIB=$(PERL_SITELIB) \
		INSTALLVENDORLIB=" " \
		INSTALLARCHLIB=$(PERL_SITELIB) \
		INSTALLSITEARCH=$(PERL_SITELIB) \
		INSTALLVENDORARCH=" " \
		INSTALLBIN=/usr/bin \
		INSTALLSITEBIN=/usr/bin \
		INSTALLVENDORBIN=" " \
		INSTALLSCRIPT=/usr/bin \
		INSTALLSITESCRIPT=/usr/bin \
		INSTALLVENDORSCRIPT=" " \
		INSTALLMAN1DIR=/usr/man/man1 \
		INSTALLSITEMAN1DIR=/usr/man/man1 \
		INSTALLVENDORMAN1DIR=" " \
		INSTALLMAN3DIR=/usr/man/man3 \
		INSTALLSITEMAN3DIR=/usr/man/man3 \
		INSTALLVENDORMAN3DIR=" " \
		LINKTYPE=dynamic \
		DESTDIR=$(PKG_INSTALL_DIR) \
	)
	sed -i -e 's!^PERL_INC = .*!PERL_INC = $(STAGING_DIR)/usr/lib/perl5/$(PERL_VERSION)/CORE/!' $(if $(3),$(3),$(PKG_BUILD_DIR))/Makefile
endef

define perlmod/Compile
	PERL5LIB=$(PERL_LIB) \
	$(2) \
	$(MAKE) -C $(if $(3),$(3),$(PKG_BUILD_DIR)) \
		$(1) \
		install
endef

define perlmod/Install/NoStrip
	$(INSTALL_DIR) $(strip $(1))$(PERL_SITELIB)
	(cd $(PKG_INSTALL_DIR)$(PERL_SITELIB) && \
	rsync --relative -rlHp --itemize-changes \
		--exclude=\*.pod \
		--exclude=.packlist \
		$(addprefix --exclude=/,$(strip $(3))) \
		--prune-empty-dirs \
		$(strip $(2)) $(strip $(1))$(PERL_SITELIB))

	chmod -R u+w $(strip $(1))$(PERL_SITELIB)
endef


define perlmod/_DoStrip
	@echo "---> Stripping modules in: $(strip $(1))$(PERL_SITELIB)"
	find $(strip $(1))$(PERL_SITELIB) -name \*.pm -or -name \*.pl | \
	xargs -r sed -i \
		-e '/^=\(head\|pod\|item\|over\|back\|encoding\|begin\|end\|for\)/,/^=cut/d' \
		-e '/^=\(head\|pod\|item\|over\|back\|encoding\|begin\|end\|for\)/,$$$$d' \
		-e '/^#$$$$/d' \
		-e '/^#[^!"'"'"']/d'
endef

define perlmod/Install
	$(call perlmod/Install/NoStrip,$(1),$(2),$(3))

	$(if $(CONFIG_PERL_NOCOMMENT),$(if $(PKG_LEAVE_COMMENTS),,$(call perlmod/_DoStrip,$(1),$(2),$(3))))
endef

# You probably don't want to use this directly. Look at perlmod/InstallTests
define perlmod/_InstallTests
	$(INSTALL_DIR) $(strip $(1))
	(cd $(PKG_BUILD_DIR)/$(2) && \
	rsync --relative -rlHp --itemize-changes \
		--exclude=.packlist \
		--prune-empty-dirs \
		$(strip $(3)) $(strip $(1)))

	chmod -R u+w $(strip $(1))
endef

define perlmod/InstallBaseTests
	$(if $(CONFIG_PERL_TESTS),$(call perlmod/_InstallTests,$(1)$(PERL_TESTSDIR),,$(2)))
endef

define perlmod/InstallTests
	$(if $(CONFIG_PERL_TESTS),$(call perlmod/_InstallTests,$(1)$(PERL_TESTSDIR),$(2),$(3)))
endef
