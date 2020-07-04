/*
 ***************************************************************************
 * MediaTek Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 1997-2012, MediaTek, Inc.
 *
 * All rights reserved. MediaTek source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of MediaTek. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of MediaTek Technology, Inc. is obtained.
 ***************************************************************************

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


#define PATH_OF_MCU_BIN_IN "/bin/"
#define PATH_OF_MCU_BIN_OUT "/embedded/include/mcu/"
#define PATH_OF_EEPROM_IN "/bin/"
#define PATH_OF_EEPROM_OUT "/embedded/include/eeprom/"
#define PATH_OF_ROM_PATCH_IN "/bin/"
#define PATH_OF_ROM_PATCH_OUT "/embedded/include/mcu/"


int bin2h(char *infname, char *outfname, char *fw_name, const char *mode)
{
	int ret = 0;
	FILE *infile, *outfile;
	unsigned char c;
	int i = 0;

	infile = fopen(infname, "r");

	if (infile == (FILE *) NULL) {
		printf("Can't read file %s\n", infname);
		return -1;
	}

	outfile = fopen(outfname, mode);

	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname);
		fclose(infile);
		return -1;
	}

	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	fprintf(outfile, "UCHAR %s[] = {\n", fw_name);

	while (1) {
		char cc[3];

		c = getc(infile);

		if (feof(infile))
			break;

		memset(cc, 0, 2);

		if (i >= 16) {
			fputs("\n", outfile);
			i = 0;
		}

		if (i == 0)
			fputs("\t", outfile);
		else if (i < 16)
			fputs(" ", outfile);

		fputs("0x", outfile);
		sprintf(cc, "%02x", c);
		fputs(cc, outfile);
		fputs(",", outfile);
		i++;
	}

	fputs("\n};\n", outfile);
	fclose(infile);
	fclose(outfile);
}

int main(int argc, char *argv[])
{
	char infname[512], ine2pname[512], in_rom_patch[512], in_rom_patch_e2[512], in_rom_patch_e3[512];
	char infname1[512];
	char infname_e2[512], infname_e3[512];
	char outfname[512], oute2pname[512], out_rom_patch[512], out_rom_patch_e2[512], out_rom_patch_e3[512];
	char outfname1[512];
	char outfname_e2[512], outfname_e3[512];
	char chipsets[1024];
	char fpga_mode[20], rx_cut_through_mode[20];
	char fw_name[128], e2p_name[128], rom_patch_name[128], rom_patch_name_e2[128], rom_patch_name_e3[128];
	char fw_name_e2[128], fw_name_e3[128];
	char fw_name1[128];
	char *rt28xxdir, *rt28xxbin_dir;
	char *chipset, *token;
	char *wow, *rt28xx_mode;
	char *fpga, *rx_cut_through;
	int is_bin2h_fw = 0, is_bin2h_rom_patch = 0, is_bin2h_e2p = 0, is_bin2h_rom_patch_e2 = 0, is_bin2h_rom_patch_e3 = 0;
	char ine2pname2[512], ine2pname3[512], e2p_name2[128], e2p_name3[128];

	rt28xxdir = (char *)getenv("RT28xx_DIR");
	rt28xxbin_dir = (char *)getenv("RT28xx_BIN_DIR");
	chipset = (char *)getenv("CHIPSET");
	wow = (char *)getenv("HAS_WOW_SUPPORT");
	fpga = (char *)getenv("HAS_FPGA_MODE");
	rx_cut_through = (char *)getenv("HAS_RX_CUT_THROUGH");
	rt28xx_mode = (char *)getenv("RT28xx_MODE");

	if (!rt28xxdir) {
		printf("Environment value \"RT28xx_DIR\" not export\n");
		return -1;
	}

	if (!rt28xxbin_dir) {
		printf("Environment value \"RT28xx_BIN_DIR\" not export\n");
		return -1;
	}

	if (!chipset) {
		printf("Environment value \"CHIPSET\" not export\n");
		return -1;
	}

	if (rt28xx_mode)
		printf("Build %s %s\n", chipset, rt28xx_mode);

	memset(chipsets, 0, sizeof(chipsets));
	memcpy(chipsets, chipset, strlen(chipset));
	chipsets[strlen(chipset)] = '\0';

	if (!fpga) {
		printf("Environment value \"HAS_FPGA_MODE\" not export\n");
		return -1;
	}

	if (strlen(fpga) > (sizeof(fpga_mode) - 1)) {
		printf("Environment value \"HAS_FPGA_MODE\" is too long, need less than 20\n");
		return -1;
	}

	memset(fpga_mode, 0, sizeof(fpga_mode));
	memcpy(fpga_mode, fpga, strlen(fpga));
	fpga_mode[strlen(fpga)] = '\0';

	if (!rx_cut_through) {
		printf("Environment value \"HAS_RX_CUT_THROUGH\" not export\n");
		return -1;
	}

	if (strlen(rx_cut_through) > (sizeof(rx_cut_through_mode) - 1)) {
		printf("Environment value \"HAS_RX_CUT_THROUGH\" is too long, need less than 20\n");
		return -1;
	}

	memcpy(rx_cut_through_mode, rx_cut_through, strlen(rx_cut_through));
	rx_cut_through_mode[strlen(rx_cut_through)] = '\0';

	if (strlen(rt28xxdir) > (sizeof(infname) - 100)) {
		printf("Environment value \"RT28xx_DIR\" is too long!\n");
		return -1;
	}

	if (strlen(rt28xxbin_dir) > (sizeof(infname) - 100)) {
		printf("Environment value \"RT28xx_BIN_DIR\" is too long!\n");
		return -1;
	}

	chipsets[strlen(chipset)] = '\0';
	chipset = strtok(chipsets, " ");

	while (chipset != NULL) {
		printf("chipset = %s\n", chipset);
		memset(infname, 0, 512);
		memset(infname_e2, 0, 512);
		memset(infname_e3, 0, 512);
		memset(infname1, 0, 512);
		memset(ine2pname, 0, 512);
		memset(ine2pname2, 0, 512);
		memset(ine2pname3, 0, 512);
		memset(outfname, 0, 512);
		memset(outfname_e2, 0, 512);
		memset(outfname_e3, 0, 512);
		memset(outfname1, 0, 512);
		memset(oute2pname, 0, 512);
		memset(fw_name, 0, 128);
		memset(fw_name_e2, 0, 128);
		memset(fw_name_e3, 0, 128);
		memset(fw_name1, 0, 128);
		memset(e2p_name, 0, 128);
		memset(e2p_name2, 0, 128);
		memset(e2p_name3, 0, 128);
		memset(in_rom_patch, 0, 512);
		memset(in_rom_patch_e2, 0, 512);
		memset(in_rom_patch_e3, 0, 512);
		memset(out_rom_patch, 0, 512);
		memset(out_rom_patch_e2, 0, 512);
		memset(out_rom_patch_e3, 0, 512);
		memset(rom_patch_name, 0, 128);
		memset(rom_patch_name_e2, 0, 128);
		memset(rom_patch_name_e3, 0, 128);
		strcat(infname, rt28xxbin_dir);
		strcat(infname_e2, rt28xxbin_dir);
		strcat(infname_e3, rt28xxbin_dir);
		strcat(infname1, rt28xxbin_dir);
		strcat(ine2pname, rt28xxbin_dir);
		strcat(in_rom_patch, rt28xxbin_dir);
		strcat(in_rom_patch_e2, rt28xxbin_dir);
		strcat(in_rom_patch_e3, rt28xxbin_dir);
		strcat(outfname, rt28xxdir);
		strcat(outfname_e2, rt28xxdir);
		strcat(outfname_e3, rt28xxdir);
		strcat(outfname1, rt28xxdir);
		strcat(oute2pname, rt28xxdir);
		strcat(out_rom_patch, rt28xxdir);
		strcat(out_rom_patch_e2, rt28xxdir);
		strcat(out_rom_patch_e3, rt28xxdir);
		is_bin2h_fw = 0;
		is_bin2h_rom_patch = 0;
		is_bin2h_e2p = 0;
		strcat(infname, PATH_OF_MCU_BIN_IN);
		strcat(infname_e2, PATH_OF_MCU_BIN_IN);
		strcat(infname_e3, PATH_OF_MCU_BIN_IN);
		strcat(outfname, PATH_OF_MCU_BIN_OUT);
		strcat(outfname_e2, PATH_OF_MCU_BIN_OUT);
		strcat(outfname_e3, PATH_OF_MCU_BIN_OUT);
		strcat(ine2pname, PATH_OF_EEPROM_IN);
		strcat(oute2pname, PATH_OF_EEPROM_OUT);
		strcat(in_rom_patch, PATH_OF_ROM_PATCH_IN);
		strcat(in_rom_patch_e2, PATH_OF_ROM_PATCH_IN);
		strcat(in_rom_patch_e3, PATH_OF_ROM_PATCH_IN);
		strcat(out_rom_patch, PATH_OF_ROM_PATCH_OUT);
		strcat(out_rom_patch_e2, PATH_OF_ROM_PATCH_OUT);
		strcat(out_rom_patch_e3, PATH_OF_ROM_PATCH_OUT);
		strcat(infname1, PATH_OF_MCU_BIN_IN);
		strcat(outfname1, PATH_OF_MCU_BIN_OUT);

		/* For bin input, select chipset folder inside /bin */
		strcat(infname, chipset);
		strcat(infname_e2, chipset);
		strcat(infname_e3, chipset);
		strcat(infname1, chipset);
		strcat(ine2pname, chipset);
		strcat(in_rom_patch, chipset);
		strcat(in_rom_patch_e2, chipset);
		strcat(in_rom_patch_e3, chipset);
		strcat(infname, "/");
		strcat(infname_e2, "/");
		strcat(infname_e3, "/");
		strcat(infname1, "/");
		strcat(ine2pname, "/");
		strcat(in_rom_patch, "/");
		strcat(in_rom_patch_e2, "/");
		strcat(in_rom_patch_e3, "/");

		if (strncmp(chipset, "mt7615", 7) == 0) {
			strcat(in_rom_patch, "mt7615_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7615_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7615_rom_patch_e1");
			strcat(in_rom_patch_e3, "mt7615_patch_e3_hdr.bin"); /* mt7615_patch_e3_hdr.bin */
			strcat(out_rom_patch_e3, "mt7615_rom_patch.h");
			strcat(rom_patch_name_e3, "mt7615_rom_patch");

			if ((strncmp(fpga, "y", 1) == 0))
				strcat(infname, "WIFI_RAM_CODE_MT7615_FPGA.bin");
			else {
				strcat(infname, "WIFI_RAM_CODE_MT7615_E1.bin");
				strcat(infname_e3, "WIFI_RAM_CODE_MT7615.bin");
			}

			strcat(outfname_e3, "mt7615_firmware.h");
			strcat(fw_name_e3, "MT7615_FirmwareImage");
			strcat(outfname, "mt7615_firmware_e1.h");
			strcat(fw_name, "MT7615_FirmwareImage_E1");

			if ((strncmp(rx_cut_through, "y", 1) == 0))
				strcat(infname1, "MT7615_cr4.bin");
			else
				strcat(infname1, "MT7615_cr4_noReOrdering.bin");

			strcat(outfname1, "mt7615_cr4_firmware.h");
			strcat(fw_name1, "MT7615_CR4_FirmwareImage");
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e3 = 1;
			is_bin2h_fw = 1;
			/* iPAiLNA */
			strncpy(ine2pname2, ine2pname, 512);
			strncpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7615_E2PImage1_iPAiLNA");
			strcat(e2p_name2, "MT7615_E2PImage2_iPAiLNA");
			strcat(e2p_name3, "MT7615_E2PImage3_iPAiLNA");
			strcat(ine2pname, "iPAiLNA/MT7615_EEPROM1.bin");
			strcat(ine2pname2, "iPAiLNA/MT7615_EEPROM2.bin");
			strcat(ine2pname3, "iPAiLNA/MT7615_EEPROM3.bin");
			strcat(oute2pname, "mt7615_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			bin2h(ine2pname2, oute2pname, e2p_name2, "a");
			bin2h(ine2pname3, oute2pname, e2p_name3, "a");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(ine2pname2, 0, 512);
			memset(ine2pname3, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			memset(e2p_name2, 0, 128);
			memset(e2p_name3, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512);
			strncpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7615_E2PImage1_iPAeLNA");
			strcat(e2p_name2, "MT7615_E2PImage2_iPAeLNA");
			strcat(e2p_name3, "MT7615_E2PImage3_iPAeLNA");
			strcat(ine2pname, "iPAeLNA/MT7615_EEPROM1.bin");
			strcat(ine2pname2, "iPAeLNA/MT7615_EEPROM2.bin");
			strcat(ine2pname3, "iPAeLNA/MT7615_EEPROM3.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7615_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			bin2h(ine2pname2, oute2pname, e2p_name2, "a");
			bin2h(ine2pname3, oute2pname, e2p_name3, "a");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(ine2pname2, 0, 512);
			memset(ine2pname3, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			memset(e2p_name2, 0, 128);
			memset(e2p_name3, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512);
			strncpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7615_E2PImage1_ePAeLNA");
			strcat(e2p_name2, "MT7615_E2PImage2_ePAeLNA");
			strcat(e2p_name3, "MT7615_E2PImage3_ePAeLNA");
			strcat(ine2pname, "ePAeLNA/MT7615_EEPROM1.bin");
			strcat(ine2pname2, "ePAeLNA/MT7615_EEPROM2.bin");
			strcat(ine2pname3, "ePAeLNA/MT7615_EEPROM3.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7615_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			bin2h(ine2pname2, oute2pname, e2p_name2, "a");
			bin2h(ine2pname3, oute2pname, e2p_name3, "a");
			/* ePAiLNA */
			memset(ine2pname, 0, 512);
			memset(ine2pname2, 0, 512);
			memset(ine2pname3, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			memset(e2p_name2, 0, 128);
			memset(e2p_name3, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strncpy(ine2pname2, ine2pname, 512);
			strncpy(ine2pname3, ine2pname, 512);
			strcat(e2p_name, "MT7615_E2PImage1_ePAiLNA");
			strcat(e2p_name2, "MT7615_E2PImage2_ePAiLNA");
			strcat(e2p_name3, "MT7615_E2PImage3_ePAiLNA");
			strcat(ine2pname, "ePAiLNA/MT7615_EEPROM1.bin");
			strcat(ine2pname2, "ePAiLNA/MT7615_EEPROM2.bin");
			strcat(ine2pname3, "ePAiLNA/MT7615_EEPROM3.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7615_e2p_ePAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			bin2h(ine2pname2, oute2pname, e2p_name2, "a");
			bin2h(ine2pname3, oute2pname, e2p_name3, "a");
			/* is_bin2h_e2p = 1;   //b2h is already done */
		} else if (strncmp(chipset, "mt7622", 7) == 0) {
			strcat(in_rom_patch, "mt7622_patch_e1_hdr.bin");
			strcat(out_rom_patch, "mt7622_rom_patch_e1.h");
			strcat(rom_patch_name, "mt7622_rom_patch_e1");
			strcat(in_rom_patch_e2, "mt7622_patch_e2_hdr.bin");
			strcat(out_rom_patch_e2, "mt7622_rom_patch_e2.h");
			strcat(rom_patch_name_e2, "mt7622_rom_patch_e2");

			if ((strncmp(fpga, "y", 1) == 0))
				strcat(infname, "WIFI_RAM_CODE_MT7622_FPGA.bin");
			else {
				strcat(infname, "WIFI_RAM_CODE_MT7622.bin");
			}
			strcat(infname_e2, "WIFI_RAM_CODE_MT7622_E2.bin");

			strcat(outfname, "mt7622_firmware.h");
			strcat(outfname_e2, "mt7622_firmware_e2.h");
			strcat(fw_name, "MT7622_FirmwareImage_E1");
			strcat(fw_name_e2, "MT7622_FirmwareImage_E2");
			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_rom_patch_e2 = 1;

			/* iPAiLNA */
			strcat(e2p_name, "MT7622_E2PImage_iPAiLNA");
			strcat(ine2pname, "iPAiLNA/MT7622_EEPROM.bin");
			strcat(oute2pname, "mt7622_e2p_iPAiLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* iPAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7622_E2PImage_iPAeLNA");
			strcat(ine2pname, "iPAeLNA/MT7622_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7622_e2p_iPAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
			/* ePAeLNA */
			memset(ine2pname, 0, 512);
			memset(oute2pname, 0, 512);
			memset(e2p_name, 0, 128);
			strcat(ine2pname, rt28xxbin_dir);
			strcat(ine2pname, PATH_OF_EEPROM_IN);
			strcat(ine2pname, chipset);
			strcat(ine2pname, "/");
			strcat(e2p_name, "MT7622_E2PImage_ePAeLNA");
			strcat(ine2pname, "ePAeLNA/MT7622_EEPROM.bin");
			strcat(oute2pname, rt28xxdir);
			strcat(oute2pname, PATH_OF_EEPROM_OUT);
			strcat(oute2pname, "mt7622_e2p_ePAeLNA.h");
			bin2h(ine2pname, oute2pname, e2p_name, "w");
		} else
			printf("unknown chipset = %s\n", chipset);

		if (is_bin2h_fw) {
			if (strlen(fw_name) != 0)
				bin2h(infname, outfname, fw_name, "w");              /* N9 E1 */
			if (strlen(fw_name_e2) != 0)
				bin2h(infname_e2, outfname_e2, fw_name_e2, "w");     /* N9 E2 */
			if (strlen(fw_name_e3) != 0)
				bin2h(infname_e3, outfname_e3, fw_name_e3, "w");     /* N9 E3 */
			if (strlen(fw_name1) != 0)
				bin2h(infname1, outfname1, fw_name1, "w");           /* CR4 */
		}

		if (is_bin2h_rom_patch)
			bin2h(in_rom_patch, out_rom_patch, rom_patch_name, "w");

		if (is_bin2h_rom_patch_e2)
			bin2h(in_rom_patch_e2, out_rom_patch_e2, rom_patch_name_e2, "w");

		if (is_bin2h_rom_patch_e3)
			bin2h(in_rom_patch_e3, out_rom_patch_e3, rom_patch_name_e3, "w");

		if (is_bin2h_e2p) {
			bin2h(ine2pname, oute2pname, e2p_name, "w");

			if (e2p_name2[0])
				bin2h(ine2pname2, oute2pname, e2p_name2, "a");

			if (e2p_name3[0])
				bin2h(ine2pname3, oute2pname, e2p_name3, "a");
		}

		chipset = strtok(NULL, " ");
	}

	exit(0);
}
