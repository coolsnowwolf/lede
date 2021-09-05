return {
	vendorName = "Linksys",
	deviceName = "EA4500",
	boardNames = { "linksys-e4500", "linksys,e4500" },
	partition1MTD = "mtd3",
	partition2MTD = "mtd5",
	labelOffset = 32,
	bootEnv1 = "boot_part",
	bootEnv1Partition1Value = 1,
	bootEnv1Partition2Value = 2,
	bootEnv2 = "bootcmd",
	bootEnv2Partition1Value = "run nandboot",
	bootEnv2Partition2Value = "run altnandboot"
}
