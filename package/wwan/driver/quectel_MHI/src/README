1. porting pcie_mhi driver as next

$ git diff drivers/Makefile
diff --git a/drivers/Makefile b/drivers/Makefile
index 77fbc52..e45837e 100644
--- a/drivers/Makefile
+++ b/drivers/Makefile
@@ -184,3 +184,4 @@ obj-$(CONFIG_FPGA)          += fpga/
 obj-$(CONFIG_FSI)              += fsi/
 obj-$(CONFIG_TEE)              += tee/
 obj-$(CONFIG_MULTIPLEXER)      += mux/
+obj-y          += pcie_mhi/

$ tree drivers/pcie_mhi/ -L 1
drivers/pcie_mhi/
 controllers
 core
 devices
 Makefile

2. check RG500 attach pcie_mhi driver successful 
 
root@OpenWrt:/# lspci
00:00.0 Class 0604: 17cb:0302
01:00.0 Class ff00: 17cb:0306

root@OpenWrt:~# dmesg | grep mhi
[  138.483252] mhi_init Quectel_Linux_PCIE_MHI_Driver_V1.3.0.6
[  138.492350] mhi_pci_probe pci_dev->name = 0000:01:00.0, domain=0, bus=1, slot=0, vendor=17CB, device=0306

3. how to use, see next logs

log/QXDM_OVER_PCIE.txt
log/AT_OVER_PCIE.txt
log/MBIM_OVER_PCIE.txt
log/QMI_OVER_PCIE.txt
