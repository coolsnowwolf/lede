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

int bin2h(char *infname, char *outfname, char *fw_name)
{
	int ret = 0;
    FILE *infile, *outfile;	
    unsigned char c;
    int i=0;

    infile = fopen(infname,"r");

    if (infile == (FILE *) NULL) {
		printf("Can't read file %s \n",infname);
		return -1;
    }

    outfile = fopen(outfname,"w");
    
    if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s \n",outfname);
       	return -1;
    }
    
    fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */ \n",outfile);
    fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */ \n",outfile);
    fputs("\n",outfile);
    fputs("\n",outfile);

	fprintf(outfile, "UCHAR %s[] = {\n", fw_name);

    while(1) {
		char cc[3];    

		c = getc(infile);
	
		if (feof(infile))
	   		break;
	
		memset(cc,0,2);
	
		if (i >= 16) {	
	   		fputs("\n", outfile);	
	   		i = 0;
		}
    
		fputs("0x", outfile); 
		sprintf(cc,"%02x",c);
		fputs(cc, outfile);
		fputs(", ", outfile);
		i++;
    } 
    
    fputs("} ;\n", outfile);
    fclose(infile);
    fclose(outfile);
}	

int main(int argc ,char *argv[])
{
    char infname[512], ine2pname[512], in_rom_patch[512];
    char outfname[512], oute2pname[512], out_rom_patch[512];
	char chipsets[1024];
	char fw_name[128], e2p_name[128], rom_patch_name[128];
    char *rt28xxdir;
    char *chipset, *token;
	char *wow, *rt28xx_mode;
	int is_bin2h_fw = 0, is_bin2h_rom_patch = 0, is_bin2h_e2p=0;
   
    rt28xxdir = (char *)getenv("RT28xx_DIR");
    chipset = (char *)getenv("CHIPSET");
	memcpy(chipsets, chipset, strlen(chipset));
	wow = (char *)getenv("HAS_WOW_SUPPORT");
	rt28xx_mode = (char *)getenv("RT28xx_MODE");

    if(!rt28xxdir) {
		printf("Environment value \"RT28xx_DIR\" not export \n");
	 	return -1;
    }

    if(!chipset) {
		printf("Environment value \"CHIPSET\" not export \n");
		return -1;
    }	    
	
	if (strlen(rt28xxdir) > (sizeof(infname)-100)) {
		printf("Environment value \"RT28xx_DIR\" is too long!\n");
		return -1;
	}
    
	chipset = strtok(chipsets, " ");

	while (chipset != NULL) {
		printf("chipset = %s\n", chipset);
    	memset(infname, 0, 512);
		memset(ine2pname, 0, 512);
    	memset(outfname, 0, 512);
		memset(oute2pname, 0, 512);
		memset(fw_name, 0, 128);
		memset(e2p_name, 0, 128);
		memset(in_rom_patch, 0, 512);
		memset(out_rom_patch, 0, 512);	
		memset(rom_patch_name, 0, 128);
    	strcat(infname,rt28xxdir);
		strcat(ine2pname, rt28xxdir);
		strcat(in_rom_patch, rt28xxdir);
    	strcat(outfname,rt28xxdir);
		strcat(oute2pname, rt28xxdir);
		strcat(out_rom_patch, rt28xxdir);
		is_bin2h_fw = 0;
		is_bin2h_rom_patch = 0;
		is_bin2h_e2p = 0;
		if (strncmp(chipset, "2860",4) == 0) {
			strcat(infname,"/mcu/bin/rt2860.bin");
    		strcat(outfname,"/include/mcu/rt2860_firmware.h");
			strcat(fw_name, "RT2860_FirmwareImage");
			is_bin2h_fw = 1;
		} else if (strncmp(chipset, "2870",4) == 0) {
			if ((strncmp(wow, "y", 1) == 0) && (strncmp(rt28xx_mode, "STA", 3) == 0)) {
	    		strcat(infname,"/mcu/bin/rt2870_wow.bin");
    			strcat(outfname,"/include/mcu/rt2870_wow_firmware.h");
				strcat(fw_name, "RT2870_WOW_FirmwareImage");
				is_bin2h_fw = 1;
			} else {
	    		strcat(infname,"/mcu/bin/rt2870.bin");
    			strcat(outfname,"/include/mcu/rt2870_firmware.h");
				strcat(fw_name, "RT2870_FirmwareImage");
				is_bin2h_fw = 1;
			}
		} else if (strncmp(chipset, "3090",4) == 0) {
	    	strcat(infname,"/mcu/bin/rt2860.bin");
    		strcat(outfname,"/include/mcu/rt2860_firmware.h");
			strcat(fw_name, "RT2860_FirmwareImage");
			is_bin2h_fw = 1;
		} else if (strncmp(chipset, "2070",4) == 0) {
			if ((strncmp(wow, "y", 1) == 0) && (strncmp(rt28xx_mode, "STA", 3) == 0)) {
	    		strcat(infname,"/mcu/bin/rt2870_wow.bin");
    			strcat(outfname,"/include/mcu/rt2870_wow_firmware.h");
				strcat(fw_name, "RT2870_WOW_FirmwareImage");
				is_bin2h_fw = 1;
			} else {
	    		strcat(infname,"/mcu/bin/rt2870.bin");
    			strcat(outfname,"/include/mcu/rt2870_firmware.h");
				strcat(fw_name, "RT2870_FirmwareImage");
				is_bin2h_fw = 1;
			}
		} else if (strncmp(chipset, "3070",4) == 0) {
			if ((strncmp(wow, "y", 1) == 0) && (strncmp(rt28xx_mode, "STA", 3) == 0)) {
	    		strcat(infname,"/mcu/bin/rt2870_wow.bin");
    			strcat(outfname,"/include/mcu/rt2870_wow_firmware.h");
				strcat(fw_name, "RT2870_WOW_FirmwareImage");
				is_bin2h_fw = 1;
			} else {
	    		strcat(infname,"/mcu/bin/rt2870.bin");
    			strcat(outfname,"/include/mcu/rt2870_firmware.h");
				strcat(fw_name, "RT2870_FirmwareImage");
				is_bin2h_fw = 1;
			}
		} else if (strncmp(chipset, "3572",4) == 0) {
			if ((strncmp(wow, "y", 1) == 0) && (strncmp(rt28xx_mode, "STA", 3) == 0)) {
	    		strcat(infname,"/mcu/bin/rt2870_wow.bin");
    			strcat(outfname,"/include/mcu/rt2870_wow_firmware.h");
				strcat(fw_name, "RT2870_WOW_FirmwareImage");
				is_bin2h_fw = 1;
			} else {
	    		strcat(infname,"/mcu/bin/rt2870.bin");
    			strcat(outfname,"/include/mcu/rt2870_firmware.h");
				strcat(fw_name, "RT2870_FirmwareImage");
				is_bin2h_fw = 1;
			}
		} else if (strncmp(chipset, "3573",4) == 0) {
			if ((strncmp(wow, "y", 1) == 0) && (strncmp(rt28xx_mode, "STA", 3) == 0)) {
	    		strcat(infname,"/mcu/bin/rt2870_wow.bin");
    			strcat(outfname,"/include/mcu/rt2870_wow_firmware.h");
				strcat(fw_name, "RT2870_WOW_FirmwareImage");
				is_bin2h_fw = 1;
			} else {
	    		strcat(infname,"/mcu/bin/rt2870.bin");
    			strcat(outfname,"/include/mcu/rt2870_firmware.h");
				strcat(fw_name, "RT2870_FirmwareImage");
				is_bin2h_fw = 1;
			}
		} else if (strncmp(chipset, "3370",4) == 0) {
			if ((strncmp(wow, "y", 1) == 0) && (strncmp(rt28xx_mode, "STA", 3) == 0)) {
	    		strcat(infname,"/mcu/bin/rt2870_wow.bin");
    			strcat(outfname,"/include/mcu/rt2870_wow_firmware.h");
				strcat(fw_name, "RT2870_WOW_FirmwareImage");
				is_bin2h_fw = 1;
			} else {
	    		strcat(infname,"/mcu/bin/rt2870.bin");
    			strcat(outfname,"/include/mcu/rt2870_firmware.h");
				strcat(fw_name, "RT2870_FirmwareImage");
				is_bin2h_fw = 1;
			}
		} else if (strncmp(chipset, "5370",4) == 0) {
			if ((strncmp(wow, "y", 1) == 0) && (strncmp(rt28xx_mode, "STA", 3) == 0)) {
	    		strcat(infname,"/mcu/bin/rt2870_wow.bin");
    			strcat(outfname,"/include/mcu/rt2870_wow_firmware.h");
				strcat(fw_name, "RT2870_WOW_FirmwareImage");
				is_bin2h_fw = 1;
			} else {
	    		strcat(infname,"/mcu/bin/rt2870.bin");
    			strcat(outfname,"/include/mcu/rt2870_firmware.h");
				strcat(fw_name, "RT2870_FirmwareImage");
				is_bin2h_fw = 1;
			}
		} else if (strncmp(chipset, "5572",4) == 0) {
			strcat(infname,"/mcu/bin/rt2870.bin");
    		strcat(outfname,"/include/mcu/rt2870_firmware.h");
			strcat(fw_name, "RT2870_FirmwareImage");
			is_bin2h_fw = 1;
		} else if (strncmp(chipset, "5592",4) == 0) {
			strcat(infname,"/mcu/bin/rt2860.bin");
    		strcat(outfname,"/include/mcu/rt2860_firmware.h");
			strcat(fw_name, "RT2860_FirmwareImage");
			is_bin2h_fw = 1;
		} else if ((strncmp(chipset, "mt7601e", 7) == 0)
				|| (strncmp(chipset, "mt7601u", 7) == 0)) {
			strcat(infname,"/mcu/bin/MT7601_formal_1.6.bin");
			//strcat(infname,"/mcu/bin/MT7601.bin");
			strcat(outfname,"/include/mcu/mt7601_firmware.h");
			strcat(fw_name, "MT7601_FirmwareImage");
			is_bin2h_fw = 1;
			strcat(ine2pname, "/eeprom/MT7601_USB_V0_D-20130416.bin");
			strcat(oute2pname, "/include/eeprom/mt7601_e2p.h");
			strcat(e2p_name, "MT7601_E2PImage");
			is_bin2h_e2p = 1;
		} else if ((strncmp(chipset, "mt7650e", 7) == 0)
				|| (strncmp(chipset, "mt7650u", 7) == 0)
				|| (strncmp(chipset, "mt7630e", 7) == 0)
				|| (strncmp(chipset, "mt7630u", 7) == 0)) {
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr.20121031.modify.USB.flow.bin"); // pmu
			strcat(infname, "/mcu/bin/MT7650.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210302000.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_10292045.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr.for.PMU.print.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_20121029.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210231140.bin"); // atomic bw
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210181030.bin"); // PMU
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210180939.bin"); // PMU
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210151438.bin"); // led
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210171346.bin");
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210151547.bin");
			//strcat(infname, "/mcu/bin/MT7650_1012.bin");
			//strcat(infname,"/mcu/bin/MT7610.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_201210031435.bin");
			//strcat(infname,"/mcu/bin/MT7610_201210021430.bin"); // turn on debug log same as 10020138.bin
			//strcat(infname,"/mcu/bin/MT7610_201210020138.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_10021442.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_1002.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_shang_1001.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_Lv5.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_Lv4.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_20120919.bin"); // wifi
			//strcat(infname,"/mcu/bin/MT7650E2_V01007870_20120921.bin"); // bt
			//strcat(infname,"/mcu/bin/MT7650.bin");
    		strcat(outfname,"/include/mcu/mt7650_firmware.h");
			strcat(fw_name, "MT7650_FirmwareImage");
			is_bin2h_fw = 1;
		} else if ((strncmp(chipset, "mt7610e", 7) == 0)
				|| (strncmp(chipset, "mt7610u", 7) == 0)) {
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr.20121031.modify.USB.flow.bin"); // pmu
			strcat(infname, "/mcu/bin/MT7650.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr.20121031.modify.USB.flow.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650E2_3_4V01008449_20121207.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210302000.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_10292045.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr.for.PMU.print.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_20121029.bin"); // pmu
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210231140.bin"); // atomic bw
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210181030.bin"); // PMU
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210181030.bin"); // PMU
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210180939.bin"); // PMU
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210151438.bin"); // led
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210171346.bin");
			//strcat(infname, "/mcu/bin/MT7650_E2_hdr_201210151547.bin");
			//strcat(infname, "/mcu/bin/MT7650_1012.bin");
			//strcat(infname,"/mcu/bin/MT7610.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_201210031435.bin");
			//strcat(infname,"/mcu/bin/MT7610_201210021430.bin"); // turn on debug log same as 10020138.bin
			//strcat(infname,"/mcu/bin/MT7610_201210031425.bin"); 
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_10021442.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_shang_1001.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_1002.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_Lv5.bin");
			//strcat(infname,"/mcu/bin/MT7610_201210020138.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_Lv4.bin");
			//strcat(infname,"/mcu/bin/MT7650_E2_hdr_20120919.bin");
    		strcat(outfname, "/include/mcu/mt7610_firmware.h");
			strcat(fw_name, "MT7610_FirmwareImage");
			is_bin2h_fw = 1;

			if ((strncmp(chipset, "mt7610e", 7) == 0)) {
				strcat(ine2pname, "/eeprom/MT7610U_FEM_V1_1.bin");
				strcat(oute2pname, "/include/eeprom/mt7610e_e2p.h");
				strcat(e2p_name, "MT7610E_E2PImage");
			} else if ((strncmp(chipset, "mt7610u", 7) == 0)) {
				strcat(ine2pname, "/eeprom/MT7610U_FEM_V1_1.bin");
				strcat(oute2pname, "/include/eeprom/mt7610u_e2p.h");
				strcat(e2p_name, "MT7610U_E2PImage");
			}
		} else if ((strncmp(chipset, "mt7662e", 7) == 0)
				|| (strncmp(chipset, "mt7662u", 7) == 0)
				|| (strncmp(chipset, "mt7632e", 7) == 0)
				|| (strncmp(chipset, "mt7632u", 7) == 0) 
				|| (strncmp(chipset, "mt7612e", 7) == 0)
				|| (strncmp(chipset, "mt7612u", 7) == 0)) {
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_2SS.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_1SS.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_debug.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130605_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_RADIO_OFF.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_tssi_average.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_tssi_0618.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_v1.4.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_v1.8.bin");
			strcat(infname, "/mcu/bin/MT7662_E3_v1.9.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_v1.7.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_v1.6_20140905.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_v1.6.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_v1.5.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20140220.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20140213.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_v1.3.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20131211.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20131127.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_v1.1.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20131111.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130913.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130904.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130903.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130829.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130826.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130817.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130814.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130813.bin");
			//strcat(infname, "/mcu/bin/mt7662_firmware_e3_20130811.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130729_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130703_b2.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130701_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130624_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130619_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130529_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_COEX_20130528.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_BT_COEX_20130502.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130521_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130520_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130514_tssi_fix.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130514_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130509_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130508_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130507_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130430_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130424.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130423_b2.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130423.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130422.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130419.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130418.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130416.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130415.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130411_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130408_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130402.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130321_b2.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130314_b1.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130313.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130311.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130308.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_latest_0306.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_0306_ram_reset.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_0306.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_0305_new3.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_0305_new.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_2013_03_03.bin");
			//strcat(infname, "/mcu/bin/WIFI_RAM_CODE_ALL_20130304_fpga.bin");
    			strcat(outfname, "/include/mcu/mt7662_firmware.h");
			strcat(fw_name, "MT7662_FirmwareImage");
			strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e3_hdr_v0.0.2_P69.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e3_hdr_v0.0.2_P48.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e3_hdr_v0.0.0.1.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e1_hdr_v0.0.0.9.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e1_hdr_coex_v0.0.0.2.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e1_hdr_v0.0.0.8.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e1_hdr_v0.0.0.7.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e1_hdr_v0.0.0.6.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e1_hdr_20130426.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e1_hdr_v0.0.0.5.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7612_patch_e1_hdr_0417.bin");
			//strcat(in_rom_patch, "/mcu/bin/mt7662_patch_e1_hdr_v0.0.0.3.bin");
			strcat(out_rom_patch, "/include/mcu/mt7662_rom_patch.h");
			strcat(rom_patch_name, "mt7662_rom_patch");
			strcat(ine2pname, "/eeprom/MT7612E_EEPROM_layout_20131121_2G5G_ePAeLNA_TXTC_off.bin");
			//strcat(ine2pname, "/eeprom/MT7612E3_EEPROM_layout_20131022_2G5G_iPAiLNA_wTSSI_default_slope_offset.bin");
			strcat(oute2pname, "/include/eeprom/mt76x2_e2p.h");
			strcat(e2p_name, "MT76x2_E2PImage");
			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;
			is_bin2h_e2p = 1;
		} else if ((strncmp(chipset, "mt7662tu", 8) == 0)
			|| (strncmp(chipset, "mt7632tu", 8) == 0)
			|| (strncmp(chipset, "mt7612tu", 8) == 0)) {
			strcat(infname, "/mcu/bin/mt7662t_firmware_e1.bin");
			strcat(outfname, "/include/mcu/mt7662t_firmware.h");
			strcat(fw_name, "MT7662T_FirmwareImage");
			strcat(in_rom_patch, "/mcu/bin/mt7662t_patch_e1_hdr.bin");
			strcat(out_rom_patch, "/include/mcu/mt7662t_rom_patch.h");
			strcat(rom_patch_name, "mt7662t_rom_patch");
			is_bin2h_fw = 1;
			is_bin2h_rom_patch = 1;

		} else {
			printf("unknown chipset = %s\n", chipset);
		}

		if (is_bin2h_fw)
     		bin2h(infname, outfname, fw_name);

		if (is_bin2h_rom_patch)
			bin2h(in_rom_patch, out_rom_patch, rom_patch_name);

		if (is_bin2h_e2p)
			bin2h(ine2pname, oute2pname, e2p_name);

		chipset = strtok(NULL, " ");
	}

    exit(0);
}	
