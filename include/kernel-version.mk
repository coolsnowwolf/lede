# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .71
LINUX_VERSION-4.4 = .88
LINUX_VERSION-4.9 = .50

LINUX_KERNEL_HASH-3.18.71 = 5abc9778ad44ce02ed6c8ab52ece8a21c6d20d21f6ed8a19287b4a38a50c1240
LINUX_KERNEL_HASH-4.4.88 = 144fe8dd773ec317fa06109b8d7bd04141bf1941daa03799fb4f437bbbb919b4
LINUX_KERNEL_HASH-4.9.50 = 2c0914c8ba3606e34c68b086ad0f470a04072d59816452c4c51e06850f2f1536

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
