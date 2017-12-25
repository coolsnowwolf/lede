# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .71
LINUX_VERSION-4.4 = .107
LINUX_VERSION-4.9 = .70
LINUX_VERSION-4.14 = .6

LINUX_KERNEL_HASH-3.18.71 = 5abc9778ad44ce02ed6c8ab52ece8a21c6d20d21f6ed8a19287b4a38a50c1240
LINUX_KERNEL_HASH-4.4.107 = d25dc23c8b05d34518ede68f03668cd344f805049a16bc4f189da90533881a17
LINUX_KERNEL_HASH-4.9.70 = baaf45f3826fa0d257a42005240cced402a12c99adf2a8d77402738304d5300a
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