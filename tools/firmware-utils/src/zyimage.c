/*
 * Copyright (C) 2014 Soul Trace <S-trace@list.ru>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>

#define szbuf 32768

u_int32_t crc_tab[256];

u_int32_t chksum_crc32 (FILE *f)
{
  register unsigned long crc;
  unsigned long i, j;
  char *buffer = malloc(szbuf);
  char *buf;

  crc = 0xFFFFFFFF;
  while (!feof(f))
  {
    j = fread(buffer, 1, szbuf, f);
    buf = buffer;
    for (i = 0; i < j; i++)
      crc = ((crc >> 8) & 0x00FFFFFF) ^ crc_tab[(crc ^ *buf++) & 0xFF];
  }
  free(buffer);
  return crc;
}

void chksum_crc32gentab ()
{
  unsigned long crc, poly;
  int i, j;

  poly = 0xEDB88320L;
  for (i = 0; i < 256; i++)
  {
    crc = i;
    for (j = 8; j > 0; j--)
    {
      if (crc & 1)
        crc = (crc >> 1) ^ poly;
      else
        crc >>= 1;
    }
    crc_tab[i] = crc;
  }
}

void usage(char *progname)
{
  printf("Usage: %s [ -v Version ] [ -d Device_ID ] <input file>\n", progname);
  exit(1);
}

int main(int argc, char *argv[]) {
  struct signature
  {
    const char magic[4];
    unsigned int device_id;
    char firmware_version[48];
    unsigned int crc32;
  }
  sign =
  {
    { 'Z', 'N', 'B', 'G' },
    1,
    { "V.1.0.0(1.0.0)" },
    0
  };
  FILE *f;
  struct signature oldsign;
  char *filename;
  static const char *optString;
  int opt;

  if (argc < 1)
    usage(argv[0]);

  optString = "v:d:h";
  opt = getopt( argc, argv, optString );
  while( opt != -1 ) {
    switch( opt ) {
      case 'v':
        if (optarg == NULL)
          usage(argv[0]);
        strncpy(sign.firmware_version, optarg, sizeof(sign.firmware_version)-1);
       sign.firmware_version[sizeof(sign.firmware_version)-1]='\0'; /* Make sure that string is terminated correctly */
        break;

      case 'd':
        sign.device_id = atoi(optarg);
        if (sign.device_id == 0)
          sign.device_id = (int)strtol(optarg, NULL, 16);
        break;

      case '?':
      case 'h':
        usage(argv[0]);
        break;

      default:
        break;
    }

    opt = getopt( argc, argv, optString );
  }

  chksum_crc32gentab();

  filename=argv[optind];
  if (access(filename, W_OK) || access(filename, R_OK))
  {
    printf("Not open input file %s\n", filename);
    exit(1);
  }
  f = fopen(argv[optind], "r+");
  if (f != NULL)
  {
    fseek(f, sizeof(sign)*-1, SEEK_END);
    fread(&oldsign, sizeof(oldsign), 1, f);

    if (strncmp(oldsign.magic,"ZNBG", sizeof(oldsign.magic)) == 0 )
    {
      printf("Image is already signed as:\nDevice ID: 0x%08x\nFirmware version: %s\nImage CRC32: 0x%x\n", oldsign.device_id, oldsign.firmware_version, oldsign.crc32);
      exit(0);
    }

    fseek(f, 0, SEEK_SET);
    sign.crc32 = chksum_crc32(f);
    fwrite(&sign, sizeof(sign), 1, f);
    fclose(f);

    printf("Image signed as:\nDevice ID: 0x%08x\nFirmware version: %s\nImage CRC32: 0x%x\n", sign.device_id, sign.firmware_version, sign.crc32);
  }
  return 0;
}
