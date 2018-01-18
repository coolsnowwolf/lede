# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .71
LINUX_VERSION-4.4 = .111
LINUX_VERSION-4.9 = .76
LINUX_VERSION-4.14 = .13

LINUX_KERNEL_HASH-3.18.71 = 5abc9778ad44ce02ed6c8ab52ece8a21c6d20d21f6ed8a19287b4a38a50c1240
LINUX_KERNEL_HASH-4.4.111 = a201282e8eaf62b4f51edb2241e98ff805fe2d1b04a72a2328c5a6e2d77ec008
LINUX_KERNEL_HASH-4.9.76 = b82b94332cffaaeade74495264c989dafb96d4748503f87a5b7c436f5de887de
LINUX_KERNEL_HASH-4.14.13 = 4ab46d1b5a0f8ef83b80760f89ae4f5c88431b19b3cf79ffa0c66d6b33e45772

ifdef KERNEL_PATCHVER
  LINUX_VERSION:=$(KERNEL_PATCHVER)$(strip $(LINUX_VERSION-$(KERNEL_PATCHVER)))
endif

split_version=$(subst ., ,$(1))
merge_version=$(subst $(space),.,$(1))
KERNEL_BASE=$(firstword $(subst -, ,$(LINUX_VERSION)))
KERNEL=$(call merge_version,$(wordlist 1,2,$(call split_version,$(KERNEL_BASE))))
KERNEL_PATCHVER ?= $(KERNEL)

# disable the md5sum check for unknown kernel versions
LINUX_KERNEL_HASH:=$(LINUX_KERNEL_HASH-$(strip $(LINUX_VERSION)))
LINUX_KERNEL_HASH?=x
