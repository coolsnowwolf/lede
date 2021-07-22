#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <getopt.h>

#define MAX_BUF	1024

int fd, fd_w;
int total_size = 0;
int total_part = 0;

struct rom_image
{
	char *filename;
	unsigned int offset;
	unsigned int part_size;
};

#if 0
struct rom_image nor_images[] = 
{
	{"nor_sbl1.mbn", 0, 0x20000}, 
	{"norplusnand-system-partition.bin", 0x20000, 0x20000}, 
	{"nor_sbl2.mbn", 0x40000, 0x40000}, 
	{"nor_sbl3.mbn", 0x80000, 0x80000}, 
	{"AP148-cdt.mbn", 0x100000, 0x10000}, 
	{"ssd.mbn", 0x110000, 0x10000}, 
	{"tz.mbn", 0x120000, 0x80000}, 
	{"rpm.mbn", 0x1a0000, 0x80000}, 
	{"openwrt-ipq806x-cdp-u-boot.mbn", 0x220000, 0x80000}, 
	{"appsblenv", 0x2a0000, 0x40000}, 
	{"ART", 0x2e0000, 0x40000}, 
	{"retail_router0.uImage", 0x320000, 0x100000}, 
	{"retail_router1.uImage", 0x420000, 0x100000}, 
	{"nvram", 0x520000, 0x20000}, 
	{"openwrt-ipq806x-3.4-uImage", 0x540000, 0x200000}
};	
#endif

//junzhao modify for 4MB nor flash!!
struct rom_image nor_images[] = 
{
	{"nor_sbl1.mbn", 0, 0x10000}, 
	{"norplusnand-system-partition.bin", 0x10000, 0x20000}, 
	{"nor_sbl2.mbn", 0x30000, 0x20000}, 
	{"nor_sbl3.mbn", 0x50000, 0x30000}, 
	{"AP148-cdt.mbn", 0x80000, 0x10000}, 
	{"ssd.mbn", 0x90000, 0x10000}, 
	{"tz.mbn", 0xa0000, 0x30000}, 
	{"rpm.mbn", 0xd0000, 0x20000},
	{"oldappsbl", 0xf0000, 0x40000},
	{"appsblenv", 0x130000, 0x40000}, 
	{"ART", 0x170000, 0x20000}, 
	{"openwrt-ipq806x-u-boot.mbn", 0x190000, 0x50000},
        {"nss1-null", 0x1e0000, 0x20000},
	{"nvram", 0x200000, 0x20000}, 
	{"kernel-null", 0x220000, 0x1e0000},
};
//end

struct rom_image nand_images[] = 
{
	{"openwrt-ipq806x-ubi-root.img", 0, 0x4000000}
};

void die(const char * str, ...)
{
	va_list args;
	va_start(args, str);
	vfprintf(stderr, str, args);
	fputc('\n', stderr);
	exit(1);
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
	//die("Usage: fillff [-i|--input] boot.bin [-o|--output] boot-mac.trx -[b|offset] 8192 -c 1024 [-s|--span] 256");
	die("Usage: buildrom [-o|--output] boot-mac.trx [-t|--type] nor");
}

int main(int argc, char ** argv)
{
	int i;
	int j;
	long input_size;
	char *output_file=NULL;
	char *type=NULL;
        int opt;
        int option_index=0;
	char *buf = NULL, *ptr_buf = NULL;
        extern char *optarg;
        extern int optind, opterr, optopt;
	//struct stat sb;
	//
	struct rom_image *rom_images = NULL;
	struct rom_image *ptr_rom_image = NULL;

        static struct option long_options[] =
	{
		{"output", 1, 0, 'o'},
		{"type", 1, 0, 't'},
		{0, 0, 0, 0}
	};

	while(1){
		opt = getopt_long(argc, argv, "o:t:",long_options, &option_index);
		if(opt == -1)
			break;
		switch(opt){
			case 'h' : 
				usage(); break;
			case 'o' :
				output_file = optarg;
				printf("output file is [%s]\n",output_file); break;
			case 't' :
				type = optarg;
				printf("type is [%s]\n", type); break;
			default :
				usage();
		}
	}
	//if(!input_file || !output_file || !offset ||!count)
	if(!type || !output_file)
		usage();

	if(!strcmp(type, "nor"))
	{
		rom_images = (struct rom_image *)malloc(sizeof(nor_images));

		if(!rom_images)
		{
			perror("malloc");
			goto fail;
		}	
		memcpy((char *)rom_images, (char *)nor_images, sizeof(nor_images));
		total_part = sizeof(nor_images)/sizeof(struct rom_image);
	}
	else
	{
		rom_images = (struct rom_image *)malloc(sizeof(nand_images));

		if(!rom_images)
		{
			perror("malloc");
			goto fail;
		}	

		memcpy((char *)rom_images, (char *)nand_images, sizeof(nand_images));
		total_part = sizeof(nand_images)/sizeof(struct rom_image);
	}

	for(i=0; i<total_part; i++)
	{
		total_size += ((struct rom_image *) ((char *)rom_images + i * sizeof(struct rom_image)))->part_size;
	}

	printf("output file total size is %d\n", total_size);

	if(!(buf = malloc(total_size))){
		perror("malloc");
		goto fail;
	}

	memset(buf, 0, sizeof(total_size));

	printf("\n---------- build rom --------\n\n");
	
	if ((fd_w = open(output_file, O_RDWR|O_CREAT, S_IREAD | S_IWRITE)) < 0){
		die("Unable to open `%s' : %m", output_file);
	}

	ptr_buf = buf;
	for(i=0; i<total_part; i++)
	{
		//struct rom_image *ptr_rom_image = (struct rom_image *)(rom_images + i * sizeof(struct rom_image));
		ptr_rom_image = (struct rom_image *)((char *)(rom_images) + i * sizeof(struct rom_image));
		printf("%d: filename = %s\n", i, ptr_rom_image->filename);
		input_size = file_open(ptr_rom_image->filename);
		fprintf(stderr, "%s is %ld bytes\n", ptr_rom_image->filename, input_size);
		if(read(fd, ptr_buf, input_size) != input_size)
			die("Read %s failed!\n", ptr_rom_image->filename);
		
		//printf("%02x %02x %02x %02x %02x\n", *(ptr_buf), *(ptr_buf+1), *(ptr_buf+2), *(ptr_buf+3), *(ptr_buf+4));		
		
		if(ptr_rom_image->part_size > input_size)
		{
			for(j=input_size; j<ptr_rom_image->part_size; j++)
			{
				*(ptr_buf+j) = 0xFF;
			}
			
		}


		ptr_buf += ptr_rom_image->part_size;

		if(fd)
			close(fd);
	}

	if(write(fd_w, buf, total_size)!= total_size)
		die("Write call failed!\n");

	
	printf("Write to new file[%s]: new size is [%d]\n", output_file, total_size);

fail:
	if(buf)
		free(buf);

	if(rom_images)
		free(rom_images);

	if(fd)
		close (fd);
	if(fd_w)
		close (fd_w);
	
	return 0;
}
