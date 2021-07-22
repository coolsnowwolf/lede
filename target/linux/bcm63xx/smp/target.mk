BOARDNAME:=smp
FEATURES+=nand

define Target/Description
  Build firmware images for BCM63XX boards with SMP and NAND support.
  SoCs with 2 cores:
    - BCM6328 (some boards only have 1 core)
    - BCM6358 (SMP unsupported due to shared TLB)
    - BCM6362
    - BCM6368
    - BCM63268
  SoCs with NAND controller:
    - BCM6328 (v2.2)
    - BCM6362 (v2.2)
    - BCM6368 (v2.1)
    - BCM63268 (v4.0)
endef
