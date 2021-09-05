return {
	vendorName = "Linksys",
	deviceName = "EA7300v1",
	boardName = "linksys,ea7300-v1",
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
