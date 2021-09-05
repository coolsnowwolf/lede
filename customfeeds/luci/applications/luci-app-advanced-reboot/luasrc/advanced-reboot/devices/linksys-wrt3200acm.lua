return {
	vendorName = "Linksys",
	deviceName = "WRT3200ACM",
	boardNames = { "linksys-rango", "linksys,rango", "linksys,wrt3200acm" },
	partition1MTD = "mtd5",
	partition2MTD = "mtd7",
	labelOffset = 32,
	bootEnv1 = "boot_part",
	bootEnv1Partition1Value = 1,
	bootEnv1Partition2Value = 2,
	bootEnv2 = "bootcmd",
	bootEnv2Partition1Value = "run nandboot",
	bootEnv2Partition2Value = "run altnandboot"
}
