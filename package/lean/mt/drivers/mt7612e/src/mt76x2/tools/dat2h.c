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

int dat2h(char *infname, char *outfname, char *struct_name)
{
	int ret = 0;
    FILE *infile, *outfile;	
    int i=0;
    char *check;

	outfile = fopen(outfname,"w");
    
    if (outfile == (FILE *) NULL) {
		printf("Can't open write file %s \n",outfname);
       	return -1;
    }
	
	fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */ \n",outfile);
    fputs("/* AUTO GEN PLEASE DO NOT MODIFY IT */ \n",outfile);
    fputs("\n",outfile);
    fputs("\n",outfile);

	
	
    infile = fopen(infname,"r");
	

    if (infile == (FILE *) NULL) {
		printf("Can't read file %s \n",infname);
		fprintf(outfile, "PUCHAR %s = NULL;\n", struct_name);
	    fclose(outfile);
		return -1;
    }   
    
    fprintf(outfile, "PUCHAR %s = \n", struct_name);
	

    while(1) {
		char c[512];
		fgets (c , 510 , infile);
		if(feof(infile))
	   		break;
		if(strstr(c,"#The word of \"Default\"")!=NULL)
			continue;

	   	if(c[strlen(c)-1]=='\n')
	   		c[strlen(c)-1] = '\0';

		fputc('"', outfile);		
		strcat(c,"\\n\"");
		fputs(c, outfile);
	
		fputs("\n", outfile);
		
    } 
    
    fputs(";\n", outfile);
    fclose(infile);
    fclose(outfile);
}	

int main(int argc ,char *argv[])
{
    char infname[512];
    char outfname[512];

	char struct_name[128]="default_profile_buffer";
	char *rt28xxdir,*rt28xx_mode;

	rt28xxdir = (char *)getenv("RT28xx_DIR");
	rt28xx_mode = (char *)getenv("RT28xx_MODE");
	

    if(!rt28xxdir) {
		printf("Environment value \"RT28xx_DIR\" not export \n");
	 	return -1;
    }   
	
	if (strlen(rt28xxdir) > (sizeof(infname)-100)) {
		printf("Environment value \"RT28xx_DIR\" is too long!\n");
		return -1;
	}

	memset(infname, 0, 512);
	memset(outfname, 0, 512);
	strcat(infname,rt28xxdir);
	strcat(outfname,rt28xxdir);
	if(strncmp(rt28xx_mode, "STA", 3) == 0)
		strcat(infname,"/conf/RT2870STA.dat");
	else
		strcat(infname,"/conf/RT2870AP.dat");

	printf("### dat2h input: %s \n",infname);
	strcat(outfname,"/include/default_profile.h");
	
	dat2h(infname, outfname, struct_name);


    exit(0);
}	
