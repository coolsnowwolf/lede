# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .71
LINUX_VERSION-4.4 = .88
LINUX_VERSION-4.9 = .51

LINUX_KERNEL_HASH-3.18.71 = 5abc9778ad44ce02ed6c8ab52ece8a21c6d20d21f6ed8a19287b4a38a50c1240
LINUX_KERNEL_HASH-4.4.88 = 144fe8dd773ec317fa06109b8d7bd04141bf1941daa03799fb4f437bbbb919b4
LINUX_KERNEL_HASH-4.9.51 = 83faa66102d8a4c164857014c5304e08ca6c16f1697851862af17ba61378aa8a

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
