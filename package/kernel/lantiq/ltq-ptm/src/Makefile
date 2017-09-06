ifeq ($(BUILD_VARIANT),danube)
  CFLAGS_MODULE+=-DCONFIG_DANUBE
  obj-m = ltq_ptm_danube.o
  ltq_ptm_danube-objs = ifxmips_ptm_adsl.o ifxmips_ptm_danube.o
endif

ifeq ($(BUILD_VARIANT),ase)
  CFLAGS_MODULE+=-DCONFIG_AMAZON_SE
  obj-m = ltq_ptm_ase.o
  ltq_ptm_ase-objs = ifxmips_ptm_adsl.o ifxmips_ptm_amazon_se.o
endif

ifeq ($(BUILD_VARIANT),ar9)
  CFLAGS_MODULE+=-DCONFIG_AR9
  obj-m = ltq_ptm_ar9.o
  ltq_ptm_ar9-objs = ifxmips_ptm_adsl.o ifxmips_ptm_ar9.o
endif

ifeq ($(BUILD_VARIANT),vr9)
  CFLAGS_MODULE+=-DCONFIG_VR9
  obj-m = ltq_ptm_vr9.o
  ltq_ptm_vr9-objs = ifxmips_ptm_vdsl.o ifxmips_ptm_vr9.o
endif
