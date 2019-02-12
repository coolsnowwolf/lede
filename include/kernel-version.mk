# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .134
LINUX_VERSION-4.9 = .155
LINUX_VERSION-4.14 = .98
LINUX_VERSION-4.19 = .20

LINUX_KERNEL_HASH-3.18.134 = 36bdd04cab3b6c824a4b7e32ae02503f437e0916d5a4ff04c90aa22da2749c2f
LINUX_KERNEL_HASH-4.9.155 = a52ac60be33c0fe312d99361f8e30211599956e958ba415d93194c6c9d073c9d
LINUX_KERNEL_HASH-4.14.98 = 8f021171b1b1efb41326e603116a45126fc030d06aaa9b0784ae612323010c5f
LINUX_KERNEL_HASH-4.19.20 = dc7d2776dad4bf738e741ed05e7d1bea685855cfb7a62d1706f5f7aeabfa04a4

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
