# Use the default kernel version if the Makefile doesn't override it

LINUX_RELEASE?=1

LINUX_VERSION-3.18 = .71
LINUX_VERSION-4.4 = .110
LINUX_VERSION-4.9 = .75
LINUX_VERSION-4.14 = .12

LINUX_KERNEL_HASH-3.18.71 = 5abc9778ad44ce02ed6c8ab52ece8a21c6d20d21f6ed8a19287b4a38a50c1240
LINUX_KERNEL_HASH-4.4.110 = d099175aac5678e6cad2f23cd56ed22a2857143c0c18489390c95ba8c441db58
LINUX_KERNEL_HASH-4.9.75 = de6c31b7668d4047e370f4625362a02b9ebf7da56d5c0879a4961f620ab1cadf
LINUX_KERNEL_HASH-4.14.12 = 6608f7d480efc5c1078888e9344f4e0b0cd9bef0b6541cbdaec7d138e03856af

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
