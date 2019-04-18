# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .136
LINUX_VERSION-4.9 = .168
LINUX_VERSION-4.14 = .111
LINUX_VERSION-4.19 = .34

LINUX_KERNEL_HASH-3.18.136 = 48c8775013d23229462134f911bbb14c7935096fcccfb19ce28ecd5f7154f35c
LINUX_KERNEL_HASH-4.9.168 = 4d451c21effad77de323edc9bfeae095aa1faed1a801ef427d66f5763bef091e
LINUX_KERNEL_HASH-4.14.111 = f8197d56553f864d1d2e97abbe4fca50f8ab5e72089c292d22f0e4395340a6e8
LINUX_KERNEL_HASH-4.19.34 = dd795e2a1fddbee5b03c3bb55a1926829cc08df4fdcabce62dda717ba087b8cc

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
