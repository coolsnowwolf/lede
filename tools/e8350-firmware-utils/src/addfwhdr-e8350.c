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

#include "cyutils.h"
#include "code_pattern.h"

#include "typedefs.h"

#define MAX_BUF	1024
#define CRC32_INIT_VALUE 0xffffffff	/* Initial CRC32 checksum value */

extern uint32 crc32(uint8 *pdata, uint nbytes, uint32 crc);

int fd, fd_w;

void die(const char * str, ...)
{
	va_list args;
	va_start(args, str);
	vfprintf(stderr, str, args);
	fputc('\n', stderr);
	exit(1);
}

int 
fill_null0(int size)
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

long
file_open(const char *name)
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
	
	crc = crc32((uint8 *)&fwhdr->res, 4, CRC32_INIT_VALUE);
	crc = crc32((uint8 *)&buf[0], input_size, crc);
	
	fwhdr->crc32 = crc;

	/* write code pattern header */
	write(fd_w, fwhdr, sizeof(struct cbt_fw_header));

	if(write(fd_w, buf, c)!=c)
		die("Write call failed!\n");
	
#if 0
	if(garbage == 1){			// add garbage, let image is a multiple of 1024, and the web upgrade will ok
		printf("Add %d bytes garbage\n",1024-32);	
		if(!fill_null0(1024-32))
			die("error!\n");
	}
#endif
fail:
	if(buf)
		free(buf);
	close (fd);
	close (fd_w);
	
	return 0;
}
