# To execute ninja from you package's Makefile:
#
# include ../../devel/ninja/ninja.mk
#
# define Build/Compile
#   $(call Ninja,-C $(MY_NINJA_BUILD_DIR),$(MY_NINJA_ENV_VARS))
# endef

HOST_BUILD_DEPENDS += ninja/host
PKG_BUILD_DEPENDS += ninja/host

NINJA_ARGS:=$(filter -j%,$(filter-out -j,$(MAKEFLAGS)))
ifneq ($(findstring c,$(OPENWRT_VERBOSE)),)
  NINJA_ARGS+=-v
endif

define Ninja
	$(2) $(STAGING_DIR_HOSTPKG)/bin/ninja $(NINJA_ARGS) $(1)
endef
