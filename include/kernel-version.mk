# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .130
LINUX_VERSION-4.9 = .146
LINUX_VERSION-4.14 = .90
LINUX_VERSION-4.19 = .9

LINUX_KERNEL_HASH-3.18.130 = d1bf85ed3fd0067b1134178ed5492ae0053cb3fdd5361986fe0b85234fc82723
LINUX_KERNEL_HASH-4.9.146 = 58195a8be3085d117c83a2ed1caa3b46ea7c1614c75f951b9f13f7adb03f8e59
LINUX_KERNEL_HASH-4.14.90 = 0c1ed0c93085e44ad89cd279647b0e4617d06ce5a482213b5481b612ffa186ca
LINUX_KERNEL_HASH-4.19.9 = fc116cc6829c73944215d3b3ac0fc368dde9e8235b456744afffde001269dbf2

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
