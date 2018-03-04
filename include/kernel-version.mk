# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .71
LINUX_VERSION-4.4 = .112
LINUX_VERSION-4.9 = .77
LINUX_VERSION-4.14 = .14

LINUX_KERNEL_HASH-3.18.71 = 5abc9778ad44ce02ed6c8ab52ece8a21c6d20d21f6ed8a19287b4a38a50c1240
LINUX_KERNEL_HASH-4.4.112 = 544b42cbeed022896115c76a18fc97b4507d5b41d7ac0ce1dce9afd6ffd11ecd
LINUX_KERNEL_HASH-4.9.77 = 7c29bc3fb96f1e23d98f664e786dddd53a1599f56431b9b7fdfba402a4b3705c
LINUX_KERNEL_HASH-4.14.14 = 8b96362eb55ae152555980e7193fe2585b487176fb936cc69b8947d7dd32044a

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
