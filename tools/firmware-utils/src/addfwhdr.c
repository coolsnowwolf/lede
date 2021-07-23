/*
	Linksys e8350 v1 firmware header generator
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>

#include "cyg_crc.h"
#define AC2350          20
#define USA             1

#if LOCALE == USA
	#define CYBERTAN_VERSION        "v1.0.03"
	#define SERIAL_NUMBER   "003"
	#define MINOR_VERSION   ""
	#define BUILD_KEYWORD   " B"
	#define BUILD_NUMBER    SERIAL_NUMBER
	#define BETA_VERSION    " "
	#define CYBERTAN_UBOOT_VERSION      "v1.0"
#else   // ETSI
	#define CYBERTAN_VERSION    "v1.0.00"
	#define SERIAL_NUMBER       "01"
	#define MINOR_VERSION       ""
	#define BUILD_KEYWORD   " B"
	#define BUILD_NUMBER    SERIAL_NUMBER
	#define BETA_VERSION    " "
	#define CYBERTAN_UBOOT_VERSION      "v1.0"
#endif

//add for AC2350 F/W header
#define FWHDR_MAGIC_STR "CHDR"
#define FWHDR_MAGIC 0X52444843
struct cbt_fw_header
{
	unsigned int magic;             /* "CHDR" */
	unsigned int len;               /* Length of file including header */
	unsigned int crc32;             /* 32-bit CRC */
	unsigned int res;
};

#define MAX_BUF	1024
#define CRC32_INIT_VALUE 0xffffffff	/* Initial CRC32 checksum value */

#ifndef TYPEDEF_UINT8
typedef unsigned char   uint8;
#endif

#ifndef TYPEDEF_UINT16
typedef unsigned short  uint16;
#endif

#ifndef TYPEDEF_UINT32
typedef unsigned int    uint32;
#endif

typedef uint32_t cyg_uint32;
typedef uint16_t cyg_uint16;

extern cyg_uint32 cyg_crc32_accumulate(cyg_uint32 crc, void *s, int len);
//extern uint32 crc32(uint8 *pdata, uint nbytes, uint32 crc);

int fd, fd_w;

void die(const char * str, ...)
{
	va_list args;
	va_start(args, str);
	vfprintf(stderr, str, args);
	fputc('\n', stderr);
	exit(1);
}

int fill_null0(int size)
{
	unsigned char buf[1];
	int i;

	fprintf(stderr,"Fill null\n");

	buf[0] = 0xff;
	for (i=0 ; i< size; i++ )
		if (write(fd_w, buf, 1) != 1)
			return 0;

	return 1;
}

long file_open(const char *name)
{
	struct stat sb;
	if ((fd = open(name, O_RDONLY, 0)) < 0){
		die("Unable to open `%s' : %m", name);
	}

	if (fstat (fd, &sb))
		die("Unable to stat `%s' : %m", name);

	return sb.st_size;
}

void usage(void)
{
	die("Usage: addfwhdr [-i|--input] sysupgrade.o [-o|--output] code.bin\n");
}

int main(int argc, char ** argv)
{
	uint input_size,c;
	char *input_file=NULL, *output_file=NULL;
	int opt;
	int option_index=0;
	int garbage = 0;
	char *buf = NULL;
	extern char *optarg;
	extern int optind, opterr, optopt;
	
	struct cbt_fw_header *fwhdr;
	uint32 crc;	

	static struct option long_options[] =
	{
		{"input", 1, 0, 'i'},
		{"output", 1, 0, 'o'},
		{"garbage", 0, 0, 'g'},
		{0, 0, 0, 0}
	};

	printf("\n---------- add fw header --------\n");

	fwhdr  = malloc(sizeof(struct cbt_fw_header));
	memset(fwhdr, 0, sizeof(struct cbt_fw_header));	

	while(1){
		opt = getopt_long(argc, argv, "i:o:g",long_options, &option_index);
		if(opt == -1)
			break;
		switch(opt){
			case 'h' : 
				usage(); break;
			case 'i' :
				input_file = optarg;
				printf("input file is [%s]\n",input_file); break;
			case 'o' :
				output_file = optarg;
				printf("output file is [%s]\n",output_file); break;
			case 'g' :
				garbage = 1; break;
			default :
				usage();
		}
	}

	if(!input_file || !output_file)
	{
		printf("You must specify the input and output file!\n");
		usage();
	}

	
	unlink(output_file);
	if ((fd_w = open(output_file,O_RDWR|O_CREAT, S_IREAD | S_IWRITE)) < 0){
		die("Unable to open `%s' : %m", output_file);
	}
	
	fwhdr = malloc(sizeof(struct cbt_fw_header));
	memset(fwhdr, 0, sizeof(struct cbt_fw_header));	
	
	memcpy((char *)&fwhdr->magic, FWHDR_MAGIC_STR, sizeof(fwhdr->magic));
	
	input_size = file_open(input_file);
	if(!(buf = malloc(input_size)))
	{
		perror("malloc");
		goto fail;
	}
	c = read(fd, buf, input_size);
	fwhdr->len = input_size + sizeof(struct cbt_fw_header);

	fwhdr->res = fwhdr->res | 0x1;
	
	//junzhao add fw build date in cbt_fw_header for calculate MD5
	#if 0
	{
	       static char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
	       int year, day, mon=1;
	char temp[10]={0};
	       unsigned char *p_date = (unsigned char *)&fwhdr->res;
	int j;

	sscanf(__DATE__, "%s %d %d", temp, &day, &year);

	for(j=0;j<12;j++) {
		if(!strcmp(temp, months[j])) {
			mon = j+1;
			break;
		}
	}
	p_date[0]= year-2000;
	p_date[1]= mon;
	p_date[2]= day;

	       printf("%s: Build date(20%d-%d-%d)\n", __FUNCTION__, p_date[0], p_date[1], p_date[2]);

	}
	#endif
	//end

	crc = cyg_crc32_accumulate(CRC32_INIT_VALUE,(uint8 *)&fwhdr->res, 4);
	crc = cyg_crc32_accumulate(crc,(uint8 *)&buf[0], input_size);
	
	fwhdr->crc32 = crc;

	/* write code pattern header */
	write(fd_w, fwhdr, sizeof(struct cbt_fw_header));

	if(write(fd_w, buf, c)!=c)
		die("Write call failed!\n");
	
fail:
	if(buf)
		free(buf);
	close (fd);
	close (fd_w);
	
	return 0;
}
