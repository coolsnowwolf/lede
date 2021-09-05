#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

#
# Original Boost 1.51 Makefile by Mirko Vogt <mirko@openwrt.org>
# Dude, this "boost" is really one of the most crude stuff I ported yet.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=boost
PKG_VERSION:=1.76.0
PKG_SOURCE_VERSION:=1_76_0
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE:=$(PKG_NAME)_$(PKG_SOURCE_VERSION).tar.bz2
PKG_SOURCE_URL:=@SF/$(PKG_NAME)/$(PKG_NAME)/$(PKG_VERSION) https://dl.bintray.com/boostorg/release/$(PKG_VERSION)/source/
PKG_HASH:=f0397ba6e982c4450f27bf32a2a83292aba035b827a5623a14636ea583318c41

PKG_MAINTAINER:=Carlos M. Ferreira <carlosmf.pt@gmail.com>
PKG_LICENSE:=BSL-1.0
PKG_LICENSE_FILES:=LICENSE_1_0.txt
PKG_CPE_ID:=cpe:/a:boost:boost

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)_$(PKG_SOURCE_VERSION)
HOST_BUILD_DIR:=$(BUILD_DIR_HOST)/$(PKG_NAME)_$(PKG_SOURCE_VERSION)

HOST_BUILD_PARALLEL:=1
PKG_BUILD_PARALLEL:=1
PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/boost/Default
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Boost C++ source library
  URL:=https://www.boost.org
  DEPENDS:=+libstdcpp +libpthread +librt
endef

define Package/boost/description
This package provides the Boost v1.76.0 libraries.
Boost is a set of free, peer-reviewed, portable C++ source libraries.

This package provides the following run-time libraries:
 - atomic
 - chrono
 - container
 - context
 - contract
 - coroutine and coroutine2 (Coroutine is deprecated - use Coroutine2)
 - date_time
 - exception
 - filesystem
 - fiber
 - graph
 - - graph-parallel
 - iostreams
 - json
 - locale
 - log
 - math
 - nowide
 - program_options
 - python3
 - random
 - regex
 - serialization and wserialization
 - stackstrace
 - system
 - thread
 - timer
 - type_erasure
 - wave

There are many more header-only libraries supported by Boost.
See more at http://www.boost.org/doc/libs/1_76_0/
endef

PKG_BUILD_DEPENDS:=boost/host

include ../../lang/python/python3-version.mk
BOOST_PYTHON3_VER=$(PYTHON3_VERSION)

BOOST_LIBS =

define Package/boost-libs
$(call Package/boost/Default)
  TITLE+= (all libs)
  DEPENDS+= $(BOOST_DEPENDS)
  HIDDEN:=1
endef

define Package/boost-libs/description
 This meta package contains only dependencies to the other libraries from
 the boost libraries collection.
endef

# Create a meta-package of dependent libraries (for ALL)
define Package/boost-libs/install
  true
endef

define Package/boost/install
  true
endef

define Package/boost
  $(call Package/boost/Default)
  TITLE+= packages
endef

define Package/boost/config
	# Invisible config dependency
	config boost-context-exclude
		bool
		default y if (TARGET_arc770 || TARGET_archs38)
		default n

	config boost-coroutine-exclude
		bool
		default y if boost-context-exclude
		default n

	config boost-fiber-exclude
		bool
		default y if boost-coroutine-exclude
		default n

	menu "Select Boost Options"
		depends on PACKAGE_boost
		comment "Boost compilation options."

		choice
			prompt "Compile Visibility."
				default boost-compile-visibility-hidden
				help
					Choose Boost symbols compilation visibility.
					-> Global:
						- a.k.a. "default" in gcc documentation. Global symbols are considered public,
						 they are exported from shared libraries and can be redefined by another
						 shared library or executable.
					-> Protected:
						- a.k.a. "symbolic". Protected symbols are exported from shared libraries but
						 cannot be redefined by another shared library or executable. This mode is
						 not supported on some platforms, for example OS X.
					-> Hidden:
						- Hidden symbols are not exported from shared libraries and cannot be
						 redefined by a different shared library or executable loaded in a process.
						 In this mode, public symbols have to be explicitly marked in the source code
						 to be exported from shared libraries. This is the recommended mode.

			config boost-compile-visibility-global
				bool "Global"

			config boost-compile-visibility-protected
				bool "Protected"

			config boost-compile-visibility-hidden
				bool "Hidden"
		endchoice

		choice
			prompt "Compile Boost libraries."
				default boost-static-and-shared-libs
				help
					Choose which version to compile.
					-> Shared:
						- Only Shared libs will be compiled.
					-> Static:
						- Only Static libs will be compiled.
					-> Both:
						- Both Static and Shared libs will be compiled.

			config boost-shared-libs
				bool "Shared"

			config boost-static-libs
				bool "Static"

			config boost-static-and-shared-libs
				bool "Both"
		endchoice

		choice
			prompt "Selects Boost Runtime linkage."
			default boost-runtime-shared
			help
				Choose which C and C++ runtimes to use:
				-> Use Shared runtimes.
				-> Use Static runtimes.
					- Not available if Shared libs are to be built.
				-> Use both runtimes.
					- Not available if Shared libs are to be built.
					- Two separate versions of Boost are built, linking each to a different runtime.
					- This option requires "Use tagged names" option to be active.

			config boost-runtime-shared
				bool "Shared"

			config boost-runtime-static
				depends on (!boost-shared-libs && !boost-static-and-shared-libs)
				bool "Static"

			config boost-runtime-static-and-shared
				depends on (boost-use-name-tags && !boost-shared-libs && !boost-static-and-shared-libs)
				bool "Both"
		endchoice

		choice
			prompt "Select a Variant."
			default boost-variant-release
			help
				Chooses which boost variant should be selected:
				-> Release: Optimizes Boost for release.
					- Optimization: Speed;  Debug Symbols: Off; Inlining: Full; Runtime Debugging: Off.
				-> Debug:
					- Optimization: Off; Debug Symbols: On; Inlining: Off; Runtime Debugging: On.
				-> Profile:
					- Profiling: On;  Debug Symbols: On.

			config boost-variant-release
				bool "Release"

			config boost-variant-debug
				bool "Debug"

			config boost-variant-profile
				bool "Profile"
		endchoice

		config boost-use-name-tags
			bool "Use tagged names."
			help
				Add name tags the lib files, to diferentiate each library version:
				  "-mt" for multi-threading.
				  "-d" for debugging.
				  "-s" for runtime static link".
				Might break compatibility with libraries that expect boost libs with default names.
			default n

		config boost-single-thread
			depends on boost-use-name-tags
			bool "Single thread Support."
			help
				Compile Boost libraries in single-thread mode.
			default n

		config boost-build-type-complete
			depends on boost-use-name-tags
			bool "Complete Boost Build."
			help
				Builds both release and debug libs. It will take much longer to compile.
			default n
	endmenu

	menu "Select Boost libraries"
		depends on PACKAGE_boost
		comment "Libraries"

		config boost-libs-all
			bool "Include all Boost libraries."
			default m if ALL
			select PACKAGE_boost-libs
			select boost-test-pkg
			select boost-graph-parallel

		config boost-test-pkg
			bool "Boost test package."
			default m if ALL
			select PACKAGE_boost-test

		config boost-graph-parallel
			bool "Boost parallel graph support."
			select PACKAGE_boost-graph
			default m if ALL

		$(foreach lib,$(BOOST_LIBS),
		config PACKAGE_boost-$(lib)
			prompt "Boost $(lib) $(if $(findstring python3,$(lib)),$(paren_left)v$(BOOST_PYTHON3_VER)$(paren_right) ,)library."
			default m if ALL
			$(if $(findstring fiber,$(lib)),depends on !boost-fiber-exclude,)\
			$(if $(findstring context,$(lib)),depends on !boost-context-exclude,)
			$(if $(findstring coroutine,$(lib)),depends on !boost-coroutine-exclude,)
		)
	endmenu
endef

PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_boost-test

define Package/boost-test
	$(call Package/boost/Default)
	TITLE+= (test)
	HIDDEN:=1
	DEPENDS+=+boost-system +boost-timer
endef

define Build/Configure
endef

define Package/boost/Default/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/libboost_$(2)*.so* $(1)/usr/lib/
endef

# 1: short name
# 2: dependencies on other boost libraries (short name)
# 3: dependencies on other packages
# 4: conditional/inward dependencies
# 5: dependencies compiled only when this package has been selected
define DefineBoostLibrary
  BOOST_DEPENDS+= +$(if $(4),$(4):boost-$(1),boost-$(1))
  PKG_CONFIG_DEPENDS+= CONFIG_PACKAGE_boost-$(1)

  BOOST_LIBS+= $(1)

  define Package/boost-$(1)
    $(call Package/boost/Default)
    TITLE+= ($(1))
    DEPENDS+= $(foreach lib,$(2),+boost-$(lib)) $(3) $(if $(4),@$(4),) $(patsubst %,+PACKAGE_boost-$(1):%,$(5))
    HIDDEN:=1
  endef

  define Package/boost-$(1)/description
   This package contains the Boost $(1) library.
  endef

  define Package/boost-$(1)/install
    $(if $(CONFIG_boost-static-libs),true,$(call Package/boost/Default/install,$$(1),$(1)))
  endef
endef

$(eval $(call DefineBoostLibrary,atomic,system))
$(eval $(call DefineBoostLibrary,chrono,system))
$(eval $(call DefineBoostLibrary,container))
$(eval $(call DefineBoostLibrary,context,chrono system,,!boost-context-exclude))
$(eval $(call DefineBoostLibrary,contract,system))
$(eval $(call DefineBoostLibrary,coroutine,system chrono context thread,,!boost-coroutine-exclude))
$(eval $(call DefineBoostLibrary,date_time))
#$(eval $(call DefineBoostLibrary,exception,,))
$(eval $(call DefineBoostLibrary,fiber,coroutine filesystem,,!boost-fiber-exclude))
$(eval $(call DefineBoostLibrary,filesystem,system))
$(eval $(call DefineBoostLibrary,graph,regex))
$(eval $(call DefineBoostLibrary,iostreams,,,,zlib liblzma libbz2 libzstd))
$(eval $(call DefineBoostLibrary,json,container))
$(eval $(call DefineBoostLibrary,locale,system chrono thread,,,icu))
$(eval $(call DefineBoostLibrary,log,system chrono date_time thread filesystem regex))
$(eval $(call DefineBoostLibrary,math))
#$(eval $(call DefineBoostLibrary,mpi,,)) # OpenMPI does no exist in OpenWRT at this time.
$(eval $(call DefineBoostLibrary,nowide))
$(eval $(call DefineBoostLibrary,program_options))
$(eval $(call DefineBoostLibrary,python3,,,,python3-base))
$(eval $(call DefineBoostLibrary,random,system))
$(eval $(call DefineBoostLibrary,regex,,,,icu))
$(eval $(call DefineBoostLibrary,serialization))
$(eval $(call DefineBoostLibrary,wserialization,serialization))
$(eval $(call DefineBoostLibrary,stacktrace))
$(eval $(call DefineBoostLibrary,system))
$(eval $(call DefineBoostLibrary,thread,system chrono atomic))
$(eval $(call DefineBoostLibrary,timer,chrono))
$(eval $(call DefineBoostLibrary,type_erasure,chrono system thread))
$(eval $(call DefineBoostLibrary,wave,date_time thread filesystem))

include $(INCLUDE_DIR)/host-build.mk

define Host/Compile
	# b2 does not provide a configure-script nor a Makefile
	( cd $(HOST_BUILD_DIR)/tools/build/src/engine ; ./build.sh gcc )

	( cd $(HOST_BUILD_DIR) ; \
		./bootstrap.sh --prefix=$(STAGING_DIR_HOSTPKG) \
			--with-libraries=context,filesystem,program_options,regex,system ;\
		./b2 --ignore-site-config install )
endef

CONFIGURE_PREFIX:=$(PKG_INSTALL_DIR)
TARGET_LDFLAGS += -pthread -lrt -lstdc++ -Wl,--gc-sections,--as-needed,--print-gc-sections

TARGET_CFLAGS += \
	$(if $(CONFIG_SOFT_FLOAT),-DBOOST_NO_FENV_H) -fPIC -ffunction-sections -fdata-sections -flto

EXTRA_CXXFLAGS += $(if $(CONFIG_GCC_USE_VERSION_10),-std=gnu++20,$(if $(CONFIG_GCC_USE_VERSION_5),-std=gnu++14,-std=gnu++17))

ifneq ($(findstring mips,$(ARCH)),)
    BOOST_ABI = o32
    ifneq ($(findstring 64,$(ARCH)),)
        BOOST_ABI = n64
    endif
else ifneq ($(findstring arm,$(ARCH)),)
    BOOST_ABI = aapcs
else ifeq ($(ARCH),aarch64)
    BOOST_ABI = aapcs
else
    BOOST_ABI = sysv
endif

comma := ,

define Build/Compile
	$(info Selected Boost API $(BOOST_ABI) for architecture $(ARCH) and cpu type $(CONFIG_CPU_TYPE) $(if $(CONFIG_CPU_SUBTYPE),and cpu subtype $(CONFIG_CPU_SUBTYPE),))
	( cd $(PKG_BUILD_DIR) ; \
		echo "using gcc : $(GCC_VERSION) : $(GNU_TARGET_NAME)-gcc : <compileflags>\"$(TARGET_CFLAGS)\" <cxxflags>\"$(TARGET_CXXFLAGS) $(EXTRA_CXXFLAGS)\" <linkflags>\"$(TARGET_LDFLAGS)\" ;" > \
			tools/build/src/user-config.jam ; \
		b2 \
			$(CONFIGURE_ARGS) \
			--ignore-site-config \
			--toolset=gcc abi=$(BOOST_ABI) \
			--disable-long-double \
			$(if $(CONFIG_boost-compile-visibility-global), visibility=global,) \
			$(if $(CONFIG_boost-compile-visibility-protected), visibility=protected,) \
			$(if $(CONFIG_boost-compile-visibility-hidden), visibility=hidden,) \
			$(if $(CONFIG_boost-variant-release), variant=release,) \
			$(if $(CONFIG_boost-variant-debug), variant=debug,) \
			$(if $(CONFIG_boost-variant-profile), variant=profile,) \
			$(if $(CONFIG_boost-use-name-tags),--layout=tagged,--layout=system) \
			$(if $(CONFIG_boost-build-type-complete),--build-type=complete,--build-type=minimal) \
			$(if $(CONFIG_boost-shared-libs),link=shared,) \
			$(if $(CONFIG_boost-static-libs),link=static,) \
			$(if $(CONFIG_boost-static-and-shared-libs),link=static$(comma)shared,) \
			$(if $(CONFIG_boost-runtime-shared),runtime-link=shared,) \
			$(if $(CONFIG_boost-runtime-static),runtime-link=static,) \
			$(if $(CONFIG_boost-runtime-static-and-shared),runtime-link=shared$(comma)static,) \
			$(if $(CONFIG_boost-single-thread),threading=single,) \
			threading=multi \
			--without-mpi \
			$(if $(CONFIG_boost-graph-parallel),,--without-graph_parallel) \
			$(if $(CONFIG_PACKAGE_boost-test),,--without-test) \
			--without-python \
			$(foreach lib,$(BOOST_LIBS), \
				$(if $(findstring python3,$(lib)),, \
					$(if $(CONFIG_PACKAGE_boost-$(lib)),, \
						$(if $(findstring wserialization,$(lib)),,--without-$(lib)) \
					) \
				) \
			) \
			$(if $(CONFIG_PACKAGE_boost-locale),boost.locale.std=off boost.locale.posix=off) \
			\
			$(if $(CONFIG_PACKAGE_boost-iostreams),-sNO_BZIP2=1 -sZLIB_INCLUDE=$(STAGING_DIR)/usr/include \
				-sZLIB_LIBPATH=$(STAGING_DIR)/usr/lib) \
			install ;\
			$(if $(CONFIG_PACKAGE_boost-python3), \
				echo "using gcc : $(GCC_VERSION) : $(GNU_TARGET_NAME)-gcc : <compileflags>\"$(TARGET_CFLAGS) -I$(STAGING_DIR)/usr/include/python$(BOOST_PYTHON3_VER)/ \" <cxxflags>\"$(TARGET_CXXFLAGS) $(EXTRA_CXXFLAGS)\" <linkflags>\"$(TARGET_LDFLAGS)\" ;" > \
					tools/build/src/user-config.jam ; \
				echo "using python : $(BOOST_PYTHON3_VER) : : $(STAGING_DIR)/usr/include/python$(BOOST_PYTHON3_VER)/ : $(STAGING_DIR)/usr/lib/libpython$(BOOST_PYTHON3_VER).so ;" >> \
					tools/build/src/user-config.jam; \
				b2 -a \
					$(CONFIGURE_ARGS) \
					--ignore-site-config \
					--toolset=gcc abi=$(BOOST_ABI) \
					--disable-long-double \
					$(if $(CONFIG_boost-variant-release), variant=release,) \
					$(if $(CONFIG_boost-variant-debug), variant=debug,) \
					$(if $(CONFIG_boost-variant-profile), variant=profile,) \
					$(if $(CONFIG_boost-use-name-tags),--layout=tagged,--layout=system) \
					$(if $(CONFIG_boost-build-type-complete),--build-type=complete,--build-type=minimal) \
					$(if $(CONFIG_boost-shared-libs),link=shared,) \
					$(if $(CONFIG_boost-static-libs),link=static,) \
					$(if $(CONFIG_boost-static-and-shared-libs),link=static$(comma)shared,) \
					$(if $(CONFIG_boost-runtime-shared),runtime-link=shared,) \
					$(if $(CONFIG_boost-runtime-static),runtime-link=static,) \
					$(if $(CONFIG_boost-runtime-static-and-shared),runtime-link=shared$(comma)static,) \
					$(if $(CONFIG_boost-single-thread),threading=single,) \
					threading=multi \
					--with-python \
				install ;\
			,) \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/include/boost/

	$(CP) \
		$(PKG_INSTALL_DIR)/include/boost/* \
		$(1)/usr/include/boost/ \
		# copies _all_ header files - independent of <--with-library>-argument above

	$(INSTALL_DIR) $(1)/usr/lib
	# copies all cmake files, compiled archive and shared object files
	$(CP) -v $(PKG_INSTALL_DIR)/lib/{*.{a,so*},cmake} $(1)/usr/lib/ || :
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/bin
	$(CP) $(HOST_BUILD_DIR)/tools/build/src/engine/b2 $(STAGING_DIR_HOSTPKG)/bin/
endef

define Package/boost-test/install
	$(if $(CONFIG_boost-static-libs),true,
		$(INSTALL_DIR) $(1)/usr/lib
		$(CP) $(PKG_INSTALL_DIR)/lib/libboost_{unit_test_framework,prg_exec_monitor}*.so* $(1)/usr/lib/
	)
endef

$(eval $(call HostBuild))

$(foreach lib,$(BOOST_LIBS),$(eval $(call BuildPackage,boost-$(lib))))
$(eval $(call BuildPackage,boost-test))
$(eval $(call BuildPackage,boost-libs))
$(eval $(call BuildPackage,boost))
