# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .131
LINUX_VERSION-4.9 = .148
LINUX_VERSION-4.14 = .91
LINUX_VERSION-4.19 = .9

LINUX_KERNEL_HASH-3.18.131 = 04600ce96e4c7642b9eaa4814f4930c79b53010b1c155d23e5ac0aeba6f455e2
LINUX_KERNEL_HASH-4.9.148 = 6067151b0225a8de1ab79abc9be7bae237eaca0cd838eb26684169560d88a994
LINUX_KERNEL_HASH-4.14.91 = 6a587c8b4160918efa082b0c2eda8c2db0f50d8814ad1d1ac94be1edf66ca6a9
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
