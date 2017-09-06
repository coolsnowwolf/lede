#include <stdlib.h>
#include <sys/types.h>
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include <libgen.h>
#include "bcmalgo.h"


int flag_print_version;
int flag_print_help;
int flag_compress;

uint16_t sa2100_magic  = 0x2100;
uint16_t sa3349_magic  = 0x3349;
uint32_t default_date = 0x00000000; //A long time ago in a galaxy far far away....
uint32_t default_load_address = 0x80010000; //The default load_address for the firmware image

static void print_help ( const char* ename )
{
	printf ( "Firmware image packer and calculator for broadcom-based modems.\n" );
	printf ( "Part of bcm-utils package.\n" );
	printf ( "(c) 2009 Necromant (http://necromant.ath.cx). Thanks to Luke-jr for his initial work.\n" );
	printf ( "usage: %s [options]\n", ename );
	printf ( "Valid options are:\n" );
	printf ( "--magic_bytes=value \t- specify magic bytes at the beginning of the image. default - 3349\n" );
	printf ( "\t\t\t these can be sa2100 (for DPC2100 modem),\n\t\t\t sa3349 (haxorware guys use this one for some reason),\n\t\t\t or a custom hex value e.g. 0xFFFF\n" );
	printf ( "--compress \t\t - Make use of LZMA (weird!) compression (Doesn't work yet).\n" );
	printf ( "--rev_maj=value\t\t - major revision number. default 0\n" );
	printf ( "--rev_min=value\t\t - minor revision number default 0\n" );
	printf ( "--filename=value\t - use this filename in header instead of default (input filename)\n" );
	printf ( "--ldaddress=value\t - hex value of the target load address. defaults to 0x80010000\n" );
	printf ( "--input_file=value\t - What file are we packing?\n" );
	printf ( "--output_file=value\t - What file shall we write? (default: image.bin)\n" );
#ifdef _HAX0RSTYLE
	printf ( "--credz\t - Give some credz!\n" );
#endif
	printf ( "\n" );
}

static time_t source_date_epoch = -1;
static void set_source_date_epoch() {
	char *env = getenv("SOURCE_DATE_EPOCH");
	char *endptr = env;
	errno = 0;
        if (env && *env) {
		source_date_epoch = strtoull(env, &endptr, 10);
		if (errno || (endptr && *endptr != '\0')) {
			fprintf(stderr, "Invalid SOURCE_DATE_EPOCH");
			exit(1);
		}
        }
}

int main ( int argc, char** argv )
{
	if ( argc<2 )
	{
		print_help ( argv[0] );
	}

	static struct option long_options[] =
	{
		{"magic_bytes",          required_argument,   0,        'm'},
		{"rev_maj",        required_argument,   0,      'j'},
		{"rev_min",       required_argument,   0,     'n'},
		{"ldaddress",       required_argument,   0,     'l'},
		{"filename",       required_argument,   0,     'f'},
		{"input_file",       required_argument,   0,     'i'},
		{"output_file",       required_argument,   0,     'o'},
		{"compress",     no_argument,       &flag_compress,    'c'},
		{"version",     no_argument,       &flag_print_version,    'v'},
		{"help",        no_argument,       &flag_print_help,    'h'},
		{0, 0, 0, 0}
	};
	int option_index = 0;
	int opt_result=0;
	char* filename=NULL;
	char* input=NULL;
	char* magic=NULL;
	char* major=NULL;
	char* minor=NULL;
	char* ldaddr=NULL;
	char* output=NULL;

	while ( opt_result>=0 )
	{
		opt_result = getopt_long ( argc, argv, "m:j:n:f:i:o:vh", long_options, &option_index );
		switch ( opt_result )
		{
			case 0:
				printf ( "o!\n" );
				break;
			case 'h':
				print_help ( argv[0] );
				break;
			case 'l':
				ldaddr=optarg;
				break;
			case 'f':
				filename=optarg;
				break;
			case 'i':
				input=optarg;
				break;
			case 'o':
				output=optarg;
				break;
			case 'm':
				magic=optarg;
				break;
			case 'j':
				major=optarg;
				break;
			case 'n':
				minor=optarg;
				break;
		}
	}
	if ( input==NULL )
	{
		printf ( "Telepaths are still on holidays. I guess you should tell me what file should I process.\n\n" );
		exit ( 1 );
	}
	if ( access ( input,R_OK ) !=0 )
	{
		printf ( "I cannot access the file %s. Is it there? Am I allowed?\n\n", input );
		exit ( 1 );
	}
	uint32_t magicnum=sa2100_magic;

	if ( magic )
	{
		if ( strcmp ( magic,"sa2100" ) ==0 ) magicnum=sa2100_magic; else
				if ( strcmp ( magic,"sa3349" ) ==0 ) magicnum=sa3349_magic; else
			{
				sscanf ( magic, "0x%04X", &magicnum );
			}
	}
	unsigned int majrev=0;
	if ( major )
	{
		sscanf ( major, "%d", &majrev );
	}
	unsigned int minrev=0;
	if ( minor )
	{
		sscanf ( minor, "%d", &minrev );
	}
	uint32_t ldaddress = default_load_address;
	if ( ldaddr )
	{
		sscanf ( ldaddr, "0x%08X", &ldaddress );
	}
	char* dupe = strdup(input);
	char* fname = basename ( dupe );
	if ( filename )
	{
		fname = filename;
	}

	time_t t = -1;
	set_source_date_epoch();
	if (source_date_epoch != -1) {
		t = source_date_epoch;
	} else if ((time(&t) == (time_t)(-1))) {
		fprintf(stderr, "time call failed\n");
		return EXIT_FAILURE;
	}

	struct stat buf;
	stat ( input,&buf );
	ldr_header_t* head = construct_header ( magicnum, (uint16_t) majrev, (uint16_t) minrev, ( uint32_t ) t, ( uint32_t ) buf.st_size, ldaddress, fname, get_file_crc ( input ) );
	free(dupe);
	//uint32_t magic, uint16_t rev_maj,uint16_t rev_min, uint32_t build_date, uint32_t filelen, uint32_t ldaddress, const char* filename, uint32_t crc
	//FILE* fd = fopen ("/tftpboot/haxorware11rev32.bin","r");
	//fread(head,sizeof(ldr_header_t),1,fd);
	char* filebuffer = malloc ( buf.st_size+10 );
	FILE* fd = fopen ( input,"r" );
	fread ( filebuffer, 1, buf.st_size,fd );
	if (!output)
		{
		output = malloc(strlen(input+5));
		strcpy(output,input);
		strcat(output,".bin");
		}
	dump_header ( head );
	FILE* fd_out = fopen ( output,"w+" );
	if (!fd_out)
		{
		fprintf(stderr, "Failed to open output file: %s\n", output);
		exit(1);
		}
	fwrite ( head,1,sizeof ( ldr_header_t ),fd_out );
	fwrite ( filebuffer,1,buf.st_size,fd_out );
	printf("Firmware image %s is ready\n", output);
	return 0;
}
