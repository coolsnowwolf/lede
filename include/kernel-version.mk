# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .71
LINUX_VERSION-4.4 = .108
LINUX_VERSION-4.9 = .72
LINUX_VERSION-4.14 = .6

LINUX_KERNEL_HASH-3.18.71 = 5abc9778ad44ce02ed6c8ab52ece8a21c6d20d21f6ed8a19287b4a38a50c1240
LINUX_KERNEL_HASH-4.4.108 = 5a001198625ce9f7b0ef1e4b43e033aea859e1ef3509532412cce8f0375ba51a
LINUX_KERNEL_HASH-4.9.72 = 69f201f1eb9eade9a3cde26d3896a53df9fddf1e19f9fa7b36331b8b1976b83b
LINUX_KERNEL_HASH-4.14.6 = 0907678ba9ea146ddbdecd0a0b6363f56b896b5c61c9a15e809effb3ea346ccc

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