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

#define PATH_OF_SKU_TABLE_IN   "/txpwr/sku_tables/"
#define PATH_OF_SKU_TABEL_OUT  "/include/txpwr/"

#define MAX_SKUTABLE_NUM            20

int dat2h(char *infname, char *outfname, char *varname, char *deffname, const char *mode)
{
	int ret = 0;
	FILE *infile, *outfile, *definfile;
	unsigned char c;
	/* int i=0; */
	unsigned int fgDefTable = 0;
	/* Open input file */
	infile = fopen(infname, "r");

	/* Check open file status for input file */
	if (infile == (FILE *) NULL) {
		printf("Can't read file %s\n", infname);
		printf("System would automatically apply default table !!\n");
		/* Flag for use Default SKU table */
		fgDefTable = 1;
		/* Open default input file */
		definfile = fopen(deffname, "r");

		/* Check open file status for default file */
		if (definfile == (FILE *) NULL) {
			printf("Can't read def file %s\n", deffname);
			return -1;
		}
	}

	outfile = fopen(outfname, mode);

	/* Check open file status for output file */
	if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s\n", outfname);

		/* Close input file or default input file */
		if (fgDefTable == 0)
			fclose(infile);
		else
			fclose(definfile);

		return -1;
	}

	/* Comment in header files */
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */\n", outfile);
	fputs("\n", outfile);
	fputs("\n", outfile);
	/* Contents in header file */
	fprintf(outfile, "UCHAR %s[] = \"", varname);

	while (1) {
		char cc[1];

		if (fgDefTable == 0)
			c = getc(infile);
		else
			c = getc(definfile);

		/* backward compatibility for old Excel SKU table */
		if (c == '#') {
			c = '!';
			sprintf(cc, "%c", c);
			fputs(cc, outfile);
		}

		if (fgDefTable == 0) {
			if (feof(infile))
				break;
		} else {
			if (feof(definfile))
				break;
		}

		if (c == '\r')
			continue;
		else if (c == '\n') {
			c = '\t';
			sprintf(cc, "%c", c);
			fputs(cc, outfile);
			c = '\"';
			sprintf(cc, "%c", c);
			fputs(cc, outfile);
			c = '\n';
			sprintf(cc, "%c", c);
			fputs(cc, outfile);
			c = '\"';
			sprintf(cc, "%c", c);
			fputs(cc, outfile);
		} else {
			sprintf(cc, "%c", c);
			fputs(cc, outfile);
		}
	}

	fputs("\";\n", outfile);

	/* close input file or default input file */
	if (fgDefTable == 0)
		fclose(infile);
	else
		fclose(definfile);

	/* close output file */
	fclose(outfile);
}

int main(int argc, char *argv[])
{
	char infname[512];
	char outfname[512];
	char deffname[512];
	char varname[128];
	char *rt28xxdir;
	int  SKUTableIdx;
	char cc[20];

	rt28xxdir = (char *)getenv("RT28xx_DIR");

	/* Trasform SKU table data file to header file */
	for (SKUTableIdx = 1; SKUTableIdx <= MAX_SKUTABLE_NUM; SKUTableIdx++) {
		/* configure input file address and file name */
		memset(infname, 0, 512);
		strcat(infname, rt28xxdir);
		strcat(infname, PATH_OF_SKU_TABLE_IN);
		strcat(infname, "7615_SingleSKU_");
		sprintf(cc, "%d", SKUTableIdx);
		strcat(infname, cc);
		strcat(infname, ".dat");
		printf("Input: [%d] %s\n", SKUTableIdx, infname);
		/* configure output file address and file name */
		memset(outfname, 0, 512);
		strcat(outfname, rt28xxdir);
		strcat(outfname, PATH_OF_SKU_TABEL_OUT);
		strcat(outfname, "SKUTable_");
		sprintf(cc, "%d", SKUTableIdx);
		strcat(outfname, cc);
		strcat(outfname, ".h");
		printf("Output: [%d] %s\n", SKUTableIdx, outfname);
		/* configure default input file address and file name */
		memset(deffname, 0, 512);
		strcat(deffname, rt28xxdir);
		strcat(deffname, PATH_OF_SKU_TABLE_IN);
		strcat(deffname, "7615_SingleSKU_default.dat");
		printf("Def Input: [%d] %s\n", SKUTableIdx, deffname);
		/* Configure variable name for SKU contents in header file */
		memset(varname, 0, 128);
		strcat(varname, "SKUvalue_");
		sprintf(cc, "%d", SKUTableIdx);
		strcat(varname, cc);
		/* Transform data file to header file */
		dat2h(infname, outfname, varname, deffname, "w");
	}

	/* Trasform BF Backoff table data file to header file */
	for (SKUTableIdx = 1; SKUTableIdx <= MAX_SKUTABLE_NUM; SKUTableIdx++) {
		/* configure input file address and file name */
		memset(infname, 0, 512);
		strcat(infname, rt28xxdir);
		strcat(infname, PATH_OF_SKU_TABLE_IN);
		strcat(infname, "7615_SingleSKU_BF_");
		sprintf(cc, "%d", SKUTableIdx);
		strcat(infname, cc);
		strcat(infname, ".dat");
		printf("Input: [%d] %s\n", SKUTableIdx, infname);
		/* configure output file address and file name */
		memset(outfname, 0, 512);
		strcat(outfname, rt28xxdir);
		strcat(outfname, PATH_OF_SKU_TABEL_OUT);
		strcat(outfname, "BFBackoffTable_");
		sprintf(cc, "%d", SKUTableIdx);
		strcat(outfname, cc);
		strcat(outfname, ".h");
		printf("Output: [%d] %s\n", SKUTableIdx, outfname);
		/* configure default input file address and file name */
		memset(deffname, 0, 512);
		strcat(deffname, rt28xxdir);
		strcat(deffname, PATH_OF_SKU_TABLE_IN);
		strcat(deffname, "7615_SingleSKU_BF_default.dat");
		printf("Def Input: [%d] %s\n", SKUTableIdx, deffname);
		/* Configure variable name for SKU contents in header file */
		memset(varname, 0, 128);
		strcat(varname, "BFBackoffvalue_");
		sprintf(cc, "%d", SKUTableIdx);
		strcat(varname, cc);
		/* Transform data file to header file */
		dat2h(infname, outfname, varname, deffname, "w");
	}
}

