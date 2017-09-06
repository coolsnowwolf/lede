#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "bcmalgo.h"


#define UTIL_VERSION "0.1"
#define ENDIAN_REVERSE_NEEDED

uint32_t reverse_endian32 ( uint32_t data )
{
#ifdef ENDIAN_REVERSE_NEEDED
	return 0 | ( data & 0x000000ff ) << 24
	       | ( data & 0x0000ff00 ) << 8
	       | ( data & 0x00ff0000 ) >> 8
	       | ( data & 0xff000000 ) >> 24;
#else
	return data;
#endif
}

uint16_t reverse_endian16 ( uint16_t data )
{
#ifdef ENDIAN_REVERSE_NEEDED
	return 0 | ( data & 0x00ff ) << 8
	       | ( data & 0xff00 ) >> 8;
#else
	return data;
#endif
}



uint32_t get_buffer_crc ( char* filebuffer,size_t size )
{

	long crc=0xffffffffL;
	long crcxor = 0xffffffffL;
	long num4 = 0xffffffffL;
	long num5 = size;
	long num6 = 0x4c11db7L;
	long num7 = 0x80000000L;
	int i;
	long j;
	for ( i = 0; i < ( num5 ); i++ )
	{
		long num2 = filebuffer[i];
		for ( j = 0x80L; j != 0L; j = j >> 1 )
		{
			long num3 = crc & num7;
			crc = crc << 1;
			if ( ( num2 & j ) != 0L )
			{
				num3 ^= num7;
			}
			if ( num3 != 0L )
			{
				crc ^= num6;
			}
		}
	}
	crc ^= crcxor;
	crc &= num4;

	uint8_t b1 = ( uint8_t ) ( ( crc & -16777216L ) >> 0x18 );
	uint8_t b2 = ( uint8_t ) ( ( crc & 0xff0000L ) >> 0x10 );
	uint8_t b3 = ( uint8_t ) ( ( crc & 0xff00L ) >> 8 );
	uint8_t b4 = ( uint8_t ) ( crc & 0xffL );
	int32_t crc_result = ( b1 | b2 << 8| b3 << 16| b4 <<24 );
	return reverse_endian32 ( crc_result );
}

//Thnx to Vector for the algo.
uint32_t get_file_crc ( char* filename )
{
	struct stat buf;
	stat ( filename,&buf );
	char* filebuffer = malloc ( buf.st_size+10 );
	FILE* fd = fopen ( filename,"r" );
	fread ( filebuffer, 1, buf.st_size,fd );
	fclose ( fd );
	uint32_t crc = get_buffer_crc ( filebuffer,buf.st_size );
	free ( filebuffer );
	return crc;
}



uint16_t get_hcs ( ldr_header_t* hd )
{
	uint8_t* head = ( uint8_t* ) hd;
	uint8_t hcs_minor;
	uint8_t hcs_major;
	uint16_t n = 0xffff;
	uint16_t m = 0;
	int state = 0;
	int i,j;
	for ( i = 0; i < 0x54; i++ )
	{
		uint16_t m = head[i];
		m = m << 8;
		for ( j = 0; j < 8; j++ )
		{
			if ( ( ( n ^ m ) & 0x8000 ) == 0 )
			{
				state = 0;
			}
			else
			{
				state = 1;
			}
			n = n << 1;
			if ( state )
			{
				n ^= 0x1021;
			}
			m = m << 1;
		}
		n &= 0xffff;
	}
	n ^= 0xffff;
	hcs_major = ( uint8_t ) ( ( n & 0xff00 ) >> 8 );
	hcs_minor = ( uint8_t ) ( n & 0xff );
	uint16_t hcs = hcs_major <<8 | hcs_minor;
	return hcs;
}

ldr_header_t* construct_header ( uint32_t magic, uint16_t rev_maj,uint16_t rev_min, uint32_t build_date, uint32_t filelen, uint32_t ldaddress, const char* filename, uint32_t crc_data )
{
	ldr_header_t* hd = malloc ( sizeof ( ldr_header_t ) );
	hd->magic=reverse_endian16 ( magic );
	hd->control=0; //FixMe: Make use of it once compression is around
	hd->rev_min = reverse_endian16 ( rev_min );
	hd->rev_maj = reverse_endian16 ( rev_maj );
	hd->build_date = reverse_endian32 ( build_date );
	hd->filelen = reverse_endian32 ( filelen );
	hd->ldaddress = reverse_endian32 ( ldaddress );
	printf ( "Creating header for %s...\n", filename );
	if ( strlen ( filename ) >63 )
	{
		printf ( "[!] Filename too long - stripping it to 63 bytes.\n" );
		strncpy ( ( char* ) &hd->filename, filename, 63 );
		hd->filename[63]=0x00;
	}
	else
	{
		strcpy ( ( char* ) &hd->filename, filename );
	}
	hd->crc=reverse_endian32 ( crc_data );
	hd->hcs = reverse_endian16 ( get_hcs ( hd ) );
	return hd;
}

static char control_unc[]  = "Uncompressed";
static char control_lz[]   = "LZRW1/KH";
static char control_mlzo[] = "mini-LZO";
static char control_nrv[] = "NRV2D99 [Bootloader?]";
static char control_nstdlzma[] = "(non-standard) LZMA";
static char control_unk[] = "Unknown";
char* get_control_info ( uint16_t control )
{
	control = reverse_endian16 ( control );
	switch ( control )
	{
		case 0:
			return control_unc;
			break;
		case 1:
			return control_lz;
			break;
		case 2:
			return control_mlzo;
			break;
		case 3:
			return control_unc;
			break;
		case 4:
			return control_nrv;
			break;
		case 5:
			return control_nstdlzma;
			break;
		case 6:
			return control_unc;
			break;
		case 7:
			return control_unc;
			break;
		default:
			return control_unk;
			break;
	}

}

int dump_header ( ldr_header_t* hd )
{
	printf ( "=== Header Information ===\n" );
	printf ( "Header magic:\t0x%04X\n",reverse_endian16 ( hd->magic ) );
	printf ( "Control:\t0x%04X (%s)\n",reverse_endian16 ( hd->control ), get_control_info ( hd->control ) );
	printf ( "Major rev. :\t0x%04X\n",reverse_endian16 ( hd->rev_maj ) );
	printf ( "Minor rev. :\t0x%04X\n",reverse_endian16 ( hd->rev_min ) );
	printf ( "File name :\t%s\n", ( char* ) &hd->filename );
	printf ( "File length:\t%d bytes\n", reverse_endian32 ( hd->filelen ) );
	printf ( "Build time:\t0x%08X //FixMe: print in human-readable form\n", reverse_endian32 ( hd->build_date ) ); //FixMe:
	printf ( "HCS:\t\t0x%04X  ",reverse_endian16 ( hd->hcs ) );
	uint16_t hcs = get_hcs ( hd );
	int ret=0;
	if ( hcs ==reverse_endian16 ( hd->hcs ) )
	{
		printf ( "(OK!)\n" );
	}
	else
	{
		printf ( "(ERROR! expected 0x%04X)\n",hcs );
		ret=1;
	}
//printf("HCS:\t0x%02X",reverse_endian32(hd->hcs));
	printf ( "Load address:\t0x%08X\n", reverse_endian32 ( hd->ldaddress ) ); //FixMe:
	printf ( "HNW:\t\t0x%04X\n",reverse_endian16 ( hd->her_znaet_chto ) ); //Hell knows what
	printf ( "CRC:\t\t0x%08X\n",reverse_endian32 ( hd->crc ) );
	printf ( "=== Binary Header Dump===\n" );
	int i,j;
	uint8_t* head = ( uint8_t* ) hd;
	for ( i=0;i<=sizeof ( ldr_header_t );i++ )
	{
		if ( i % 8==0 )
			printf ( "\n" );
		printf ( "0x%02x   ",head[i] );
	}
	printf ( "\n\n== End Of Header dump ==\n" );
	return ret;
}


void print_copyright()
{
	printf ( "Part of bcm-utils package ver. " UTIL_VERSION " \n" );
	printf ( "Copyright (C) 2009  Andrew 'Necromant' Andrianov\n"
	         "This is free software, and you are welcome to redistribute it\n"
	         "under certain conditions. See COPYING for details\n" );
}
