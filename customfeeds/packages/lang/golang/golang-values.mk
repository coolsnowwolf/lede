#
# Copyright (C) 2018, 2020 Jeffery To
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

ifeq ($(origin GO_INCLUDE_DIR),undefined)
  GO_INCLUDE_DIR:=$(dir $(lastword $(MAKEFILE_LIST)))
endif


# Unset environment variables
# There are more magic variables to track down, but ain't nobody got time for that

# From https://golang.org/cmd/go/#hdr-Environment_variables

# General-purpose environment variables:
unexport \
  GCCGO \
  GOARCH \
  GOBIN \
  GOCACHE \
  GOMODCACHE \
  GODEBUG \
  GOENV \
  GOFLAGS \
  GOOS \
  GOPATH \
  GOROOT \
  GOTMPDIR
# Unmodified:
#   GOINSECURE
#   GOPRIVATE
#   GOPROXY
#   GONOPROXY
#   GOSUMDB
#   GONOSUMDB
#   GOVCS

# Environment variables for use with cgo:
unexport \
  AR \
  CC \
  CGO_ENABLED \
  CGO_CFLAGS   CGO_CFLAGS_ALLOW   CGO_CFLAGS_DISALLOW \
  CGO_CPPFLAGS CGO_CPPFLAGS_ALLOW CGO_CPPFLAGS_DISALLOW \
  CGO_CXXFLAGS CGO_CXXFLAGS_ALLOW CGO_CXXFLAGS_DISALLOW \
  CGO_FFLAGS   CGO_FFLAGS_ALLOW   CGO_FFLAGS_DISALLOW \
  CGO_LDFLAGS  CGO_LDFLAGS_ALLOW  CGO_LDFLAGS_DISALLOW \
  CXX \
  FC
# Unmodified:
#   PKG_CONFIG

# Architecture-specific environment variables:
unexport \
  GOARM \
  GO386 \
  GOMIPS \
  GOMIPS64 \
  GOWASM

# Special-purpose environment variables:
unexport \
  GCCGOTOOLDIR \
  GOROOT_FINAL \
  GO_EXTLINK_ENABLED
# Unmodified:
#   GIT_ALLOW_PROTOCOL

# From https://golang.org/cmd/go/#hdr-Module_support
unexport \
  GO111MODULE

# From https://golang.org/pkg/runtime/#hdr-Environment_Variables
unexport \
  GOGC \
  GOMAXPROCS \
  GORACE \
  GOTRACEBACK

# From https://golang.org/cmd/cgo/#hdr-Using_cgo_with_the_go_command
unexport \
  CC_FOR_TARGET \
  CXX_FOR_TARGET
# Todo:
#   CC_FOR_${GOOS}_${GOARCH}
#   CXX_FOR_${GOOS}_${GOARCH}

# From https://golang.org/doc/install/source#environment
unexport \
  GOHOSTOS \
  GOHOSTARCH \
  GOPPC64

# From https://golang.org/src/make.bash
unexport \
  GO_GCFLAGS \
  GO_LDFLAGS \
  GO_LDSO \
  GO_DISTFLAGS \
  GOBUILDTIMELOGFILE \
  GOROOT_BOOTSTRAP

# From https://golang.org/doc/go1.9#parallel-compile
unexport \
  GO19CONCURRENTCOMPILATION

# From https://golang.org/src/cmd/dist/build.go
unexport \
  BOOT_GO_GCFLAGS \
  BOOT_GO_LDFLAGS

# From https://golang.org/src/cmd/dist/buildruntime.go
unexport \
  GOEXPERIMENT

# From https://golang.org/src/cmd/dist/buildtool.go
unexport \
  GOBOOTSTRAP_TOOLEXEC


# GOOS / GOARCH

go_arch=$(subst \
  aarch64,arm64,$(subst \
  i386,386,$(subst \
  mipsel,mipsle,$(subst \
  mips64el,mips64le,$(subst \
  powerpc64,ppc64,$(subst \
  x86_64,amd64,$(1)))))))

GO_OS:=linux
GO_ARCH:=$(call go_arch,$(ARCH))
GO_OS_ARCH:=$(GO_OS)_$(GO_ARCH)

GO_HOST_OS:=$(call tolower,$(HOST_OS))
GO_HOST_ARCH:=$(call go_arch,$(subst \
  armv6l,arm,$(subst \
  armv7l,arm,$(subst \
  i686,i386,$(HOST_ARCH)))))
GO_HOST_OS_ARCH:=$(GO_HOST_OS)_$(GO_HOST_ARCH)

ifeq ($(GO_OS_ARCH),$(GO_HOST_OS_ARCH))
  GO_HOST_TARGET_SAME:=1
else
  GO_HOST_TARGET_DIFFERENT:=1
endif

ifeq ($(GO_ARCH),386)
  ifeq ($(CONFIG_TARGET_x86_geode)$(CONFIG_TARGET_x86_legacy),y)
    GO_386:=softfloat
  else
    GO_386:=sse2
  endif

  # -fno-plt: causes "unexpected GOT reloc for non-dynamic symbol" errors
  GO_CFLAGS_TO_REMOVE:=-fno-plt

else ifeq ($(GO_ARCH),arm)
  GO_TARGET_FPU:=$(word 2,$(subst +,$(space),$(call qstrip,$(CONFIG_CPU_TYPE))))

  # FPU names from https://gcc.gnu.org/onlinedocs/gcc-8.4.0/gcc/ARM-Options.html#index-mfpu-1
  # see also https://github.com/gcc-mirror/gcc/blob/releases/gcc-8.4.0/gcc/config/arm/arm-cpus.in

  ifeq ($(GO_TARGET_FPU),)
    GO_ARM:=5
  else ifneq ($(filter $(GO_TARGET_FPU),vfp vfpv2),)
    GO_ARM:=6
  else
    GO_ARM:=7
  endif

else ifneq ($(filter $(GO_ARCH),mips mipsle),)
  ifeq ($(CONFIG_HAS_FPU),y)
    GO_MIPS:=hardfloat
  else
    GO_MIPS:=softfloat
  endif

  # -mips32r2: conflicts with -march=mips32 set by go
  GO_CFLAGS_TO_REMOVE:=-mips32r2

else ifneq ($(filter $(GO_ARCH),mips64 mips64le),)
  ifeq ($(CONFIG_HAS_FPU),y)
    GO_MIPS64:=hardfloat
  else
    GO_MIPS64:=softfloat
  endif

endif


# Target Go

GO_ARCH_DEPENDS:=@(aarch64||arm||i386||i686||mips||mips64||mips64el||mipsel||powerpc64||x86_64)


# ASLR/PIE

GO_PIE_SUPPORTED_OS_ARCH:= \
  android_386  android_amd64  android_arm  android_arm64 \
  linux_386    linux_amd64    linux_arm    linux_arm64 \
  \
  windows_386  windows_amd64  windows_arm \
  \
  darwin_amd64 darwin_arm64 \
  \
  freebsd_amd64 \
  \
  aix_ppc64 \
  \
  linux_ppc64le linux_riscv64 linux_s390x

go_pie_install_suffix=$(if $(filter $(1),aix_ppc64 windows_386 windows_amd64 windows_arm),,shared)

ifneq ($(filter $(GO_HOST_OS_ARCH),$(GO_PIE_SUPPORTED_OS_ARCH)),)
  GO_HOST_PIE_SUPPORTED:=1
  GO_HOST_PIE_INSTALL_SUFFIX:=$(call go_pie_install_suffix,$(GO_HOST_OS_ARCH))
endif

ifneq ($(filter $(GO_OS_ARCH),$(GO_PIE_SUPPORTED_OS_ARCH)),)
  GO_TARGET_PIE_SUPPORTED:=1
  GO_TARGET_PIE_INSTALL_SUFFIX:=$(call go_pie_install_suffix,$(GO_OS_ARCH))
endif


# Spectre mitigations

GO_SPECTRE_SUPPORTED_ARCH:=amd64

ifneq ($(filter $(GO_HOST_ARCH),$(GO_SPECTRE_SUPPORTED_ARCH)),)
  GO_HOST_SPECTRE_SUPPORTED:=1
endif

ifneq ($(filter $(GO_ARCH),$(GO_SPECTRE_SUPPORTED_ARCH)),)
  GO_TARGET_SPECTRE_SUPPORTED:=1
endif


# General build info

GO_BUILD_CACHE_DIR:=$(or $(call qstrip,$(CONFIG_GOLANG_BUILD_CACHE_DIR)),$(TMP_DIR)/go-build)
GO_MOD_CACHE_DIR:=$(DL_DIR)/go-mod-cache

GO_MOD_ARGS= \
	-modcacherw

GO_GENERAL_BUILD_CONFIG_VARS= \
	CONFIG_GOLANG_MOD_CACHE_WORLD_READABLE="$(CONFIG_GOLANG_MOD_CACHE_WORLD_READABLE)" \
	GO_BUILD_CACHE_DIR="$(GO_BUILD_CACHE_DIR)" \
	GO_MOD_CACHE_DIR="$(GO_MOD_CACHE_DIR)" \
	GO_MOD_ARGS="$(GO_MOD_ARGS)"

define Go/CacheCleanup
	$(GO_GENERAL_BUILD_CONFIG_VARS) \
	$(SHELL) $(GO_INCLUDE_DIR)/golang-build.sh cache_cleanup
endef
