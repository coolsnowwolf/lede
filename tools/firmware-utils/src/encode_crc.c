/* **************************************************************************

   This program creates a CRC checksum and encodes the file that is named
   in the command line.
   
   Compile with:  gcc encode_crc.c -Wall -o encode_crc

   Author:     Michael Margraf  (michael.margraf@freecom.com)
   Copyright:  Freecom Technology GmbH, Berlin, 2004
               www.freecom.com

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

// *******************************************************************
// CCITT polynom G(x)=x^16+x^12+x^5+1
#define POLYNOM  0x1021

// CRC algorithm with MSB first
int make_crc16(int crc, char new)
{
  int i;
  crc = crc ^ (((int)new) << 8);
  
  for(i=0; i<8; i++) {  // work on 8 bits in "new"
    crc <<= 1;          // MSBs first
    if(crc & 0x10000)  crc ^= POLYNOM;
  }
  return crc & 0xFFFF;
}

// *******************************************************************
// Reads the file "filename" into memory and returns pointer to the buffer.
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


// *******************************************************************
int main(int argc, char** argv)
{
  if(argc < 3) {
    printf("ERROR: Argument missing!\n\n");
    return 1;
  }

  int count;  // size of file in bytes
  char *p, *master = readfile(argv[1], &count);
  if(!master) {
    printf("ERROR: File not found!\n");
    return 1;
  }

  int crc = 0xFFFF, z;

  p = master;
  for(z=0; z<count; z++)
    crc = make_crc16(crc, *(p++));  // calculate CRC
  short crc16 = (short)crc;

	/*
  if(argc > 2) {   // with flag for device recognition ?
    p = argv[2];
    for(z=strlen(p); z>0; z--) {
      crc ^= (int)(*p);
      *(p++) = (char)crc;  // encode device flag
    }
  }
	*/

  p = master;
  for(z=0; z<count; z++) {
    crc ^= (int)(*p);
    *(p++) = (char)crc;  // encode file
  }


  // write encoded file...
  FILE *fp = fopen(argv[2], "w");
  if(!fp) {
    printf("ERROR: File not writeable!\n");
    return 1;
  }

  if(argc > 3) {  // add flag for device recognition ?
    fwrite(argv[3], strlen(argv[3]), sizeof(char), fp);
  }
  else {
    // Device is an FSG, so byte swap (IXP4xx is big endian)
    crc16 = ((crc16 >> 8) & 0xFF) | ((crc16 << 8) & 0xFF00);
  }

  fwrite(&crc16, 1, sizeof(short), fp);     // first write CRC

  fwrite(master, count, sizeof(char), fp);  // write content
  fclose(fp);

  free(master);
  return 0;
}
