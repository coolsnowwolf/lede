# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .71
LINUX_VERSION-4.4 = .121
LINUX_VERSION-4.9 = .105
LINUX_VERSION-4.14 = .44

LINUX_KERNEL_HASH-3.18.71 = 5abc9778ad44ce02ed6c8ab52ece8a21c6d20d21f6ed8a19287b4a38a50c1240
LINUX_KERNEL_HASH-4.4.121 = 44a88268b5088dc326b30c9b9133ac35a9a200b636b7268d08f32abeae6ca729
LINUX_KERNEL_HASH-4.9.105 = d085d228e3ac1fdbdf5a31bb8154e4e8a0943a9085f0384842601db8e9d96dc4
LINUX_KERNEL_HASH-4.14.44 = 2eb356e6af25f6ca65affe7704be8c4e0cdf224505e7441ac9d5b6e8d96ec8e4

remove_uri_prefix=$(subst git://,,$(subst http://,,$(subst https://,,$(1))))
sanitize_uri=$(call qstrip,$(subst @,_,$(subst :,_,$(subst .,_,$(subst -,_,$(subst /,_,$(1)))))))

ifneq ($(call qstrip,$(CONFIG_KERNEL_GIT_CLONE_URI)),)
  LINUX_VERSION:=$(call sanitize_uri,$(call remove_uri_prefix,$(CONFIG_KERNEL_GIT_CLONE_URI)))
  ifeq ($(call qstrip,$(CONFIG_KERNEL_GIT_REF)),)
    CONFIG_KERNEL_GIT_REF:=HEAD
  endif
  LINUX_VERSION:=$(LINUX_VERSION)-$(call sanitize_uri,$(CONFIG_KERNEL_GIT_REF))
else
ifdef KERNEL_PATCHVER
  LINUX_VERSION:=$(KERNEL_PATCHVER)$(strip $(LINUX_VERSION-$(KERNEL_PATCHVER)))
endif
endif

split_version=$(subst ., ,$(1))
merge_version=$(subst $(space),.,$(1))
KERNEL_BASE=$(firstword $(subst -, ,$(LINUX_VERSION)))
KERNEL=$(call merge_version,$(wordlist 1,2,$(call split_version,$(KERNEL_BASE))))
KERNEL_PATCHVER ?= $(KERNEL)

# disable the md5sum check for unknown kernel versions
LINUX_KERNEL_HASH:=$(LINUX_KERNEL_HASH-$(strip $(LINUX_VERSION)))
LINUX_KERNEL_HASH?=x
