# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .123
LINUX_VERSION-4.9 = .129
LINUX_VERSION-4.14 = .72

LINUX_KERNEL_HASH-3.18.123 = c10de32c9b31fb619b016a00d77afc394db5a4542e258e927f06a5ead86f8c64
LINUX_KERNEL_HASH-4.9.129 = 6f5510d3fcfec1bf1e2d9c8e2fdcd7628c1886c6bdb29092adc5ccdf75e39318
LINUX_KERNEL_HASH-4.14.72 = df925906250bbc40fcf0137d7ad0fb8edc528d926832634f1233b7540564557f

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
