# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .133
LINUX_VERSION-4.9 = .153
LINUX_VERSION-4.14 = .96
LINUX_VERSION-4.19 = .16

LINUX_KERNEL_HASH-3.18.133 = 3ec7f47365a8a050e629a5016e90e38a800e840c844901c979e9e796f8dc6711
LINUX_KERNEL_HASH-4.9.153 = 9066929ec2550794ae107350a5f3c5b648438aa915cfc62bac5b7a54b9d7731a
LINUX_KERNEL_HASH-4.14.96 = 110daeae1a416b7e0ec8dce5e86d67552deeb4567f696c3869389be239f0ecb5
LINUX_KERNEL_HASH-4.19.16 = d8a088381fe3e7e5484c060dabcdda4b053ef7114f91cfd56db003a89bb11bdf

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
