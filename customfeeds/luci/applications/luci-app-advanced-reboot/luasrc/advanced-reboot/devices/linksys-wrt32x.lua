return {
	vendorName = "Linksys",
	deviceName = "WRT32X",
	boardNames = { "linksys-venom", "linksys,venom", "linksys,wrt32x" },
	partition1MTD = "mtd5",
	partition2MTD = "mtd7",
	labelOffset = nil,
	bootEnv1 = "boot_part",
	bootEnv1Partition1Value = 1,
	bootEnv1Partition2Value = 2,
	bootEnv2 = "bootcmd",
	bootEnv2Partition1Value = "run nandboot",
	bootEnv2Partition2Value = "run altnandboot"
}
