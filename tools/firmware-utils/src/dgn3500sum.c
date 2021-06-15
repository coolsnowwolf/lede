/* **************************************************************************

   This program creates a modified 16bit checksum used for the Netgear
   DGN3500 series routers. The difference between this and a standard
   checksum is that every 0x100 bytes added 0x100 have to be subtracted
   from the sum.

   (C) 2013 Marco Antonio Mauro <marcus90 at gmail.com>

   Based on previous unattributed work.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

 ************************************************************************* */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

unsigned char PidDataWW[70] =
{
    0x73, 0x45, 0x72, 0x43, 0x6F, 0x4D, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x59, 0x50, 0x35, 0x37, 0x32,
    0x33, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x37,
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73,
    0x45, 0x72, 0x43, 0x6F, 0x4D, 0x6D,
} ;

unsigned char PidDataDE[70] =
{
    0x73, 0x45, 0x72, 0x43, 0x6F, 0x4D, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x59, 0x50, 0x35, 0x37, 0x32,
    0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x42, 0x00, 0x37,
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73,
    0x45, 0x72, 0x43, 0x6F, 0x4D, 0x6D,
} ;

unsigned char PidDataNA[70] =
{
    0x73, 0x45, 0x72, 0x43, 0x6F, 0x4D, 0x6D, 0x00, 0x00, 0x00, 0x00, 0x59, 0x50, 0x35, 0x37, 0x32,
    0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x41, 0x00, 0x37,
    0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x73,
    0x45, 0x72, 0x43, 0x6F, 0x4D, 0x6D,
} ;

/* *******************************************************************
   Reads the file into memory and returns pointer to the buffer. */
static char *readfile(char *filename, int *size)
{
	FILE		*fp;
	char		*buffer;
	struct stat	info;

	if (stat(filename,&info)!=0)
		return NULL;

	if ((fp=fopen(filename,"r"))==NULL)
		return NULL;

	buffer=NULL;
	for (;;)
	{
		if ((buffer=(char *)malloc(info.st_size+1))==NULL)
			break;

		if (fread(buffer,1,info.st_size,fp)!=info.st_size)
		{
			free(buffer);
			buffer=NULL;
			break;
		}

		buffer[info.st_size]='\0';
		if(size) *size = info.st_size;

		break;
	}

	(void)fclose(fp);

	return buffer;
}


/* ******************************************************************* */
int main(int argc, char** argv)
{
  unsigned long start, i;
  char *endptr, *buffer, *p;
  int count;  // size of file in bytes
  unsigned short sum = 0, sum1 = 0;
  char sumbuf[8 + 8 + 1];

  if(argc < 3) {
    printf("ERROR: Argument missing!\n\nUsage %s filename starting offset in hex [PID code]\n\n", argv[0]);
    return 1;
  }


  FILE *fp = fopen(argv[1], "a");
  if(!fp) {
    printf("ERROR: File not writeable!\n");
    return 1;
  }
  if(argc == 4)
  {
    printf("%s: PID type: %s\n", argv[0], argv[3]);
    if(strcmp(argv[3], "DE")==0)
      fwrite(PidDataDE, sizeof(PidDataDE), sizeof(char), fp);  /* write DE pid */
    else if(strcmp(argv[3], "NA")==0)
      fwrite(PidDataNA, sizeof(PidDataNA), sizeof(char), fp);  /* write NA pid */
    else /* if(strcmp(argv[3], "WW")) */
      fwrite(PidDataWW, sizeof(PidDataWW), sizeof(char), fp);  /* write WW pid */
  }
  else
    fwrite(PidDataWW, sizeof(PidDataWW), sizeof(char), fp);  /* write WW pid if unspecified */

  fclose(fp);

  /* Read the file to calculate the checksums */
  buffer = readfile(argv[1], &count);
  if(!buffer) {
    printf("ERROR: File %s not found!\n", argv[1]);
    return 1;
  }

  p = buffer;
  for(i = 0; i < count; i++)
  {
	sum += p[i];
  }

  start = strtol(argv[2], &endptr, 16);
  p = buffer+start;
  for(i = 0; i < count - start; i++)
  {
	sum1 += p[i];
  }

  sprintf(sumbuf,"%04X%04X",sum1,sum);
  /* Append the 2 checksums to end of file */
  fp = fopen(argv[1], "a");
  if(!fp) {
    printf("ERROR: File not writeable!\n");
    return 1;
  }
  fwrite(sumbuf, 8, sizeof(char), fp);
  fclose(fp);
  free(buffer);
  return 0;
}
