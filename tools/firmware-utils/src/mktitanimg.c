#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include "mktitanimg.h"


struct checksumrecord
{
		unsigned int magic;
        unsigned int    chksum;     /* The checksum for the complete header.
 Excepting the
                                           checksum block */
};
/***************************************************************************
 * void print_help(void)
 ***************************************************************************/
void print_help(void)
{
	static char* help_page[]=
	{
		"mknspimg version 1.0, Texas Instruments, 2004",
		"Syntax:",
		"        mknspimg -o outfile -i image1 image2 -a align1 align2 [-v] [-b] [-p prod_id] [-r rel_id] [-s rel_name] [-f flags]",
		"Example:",
		"        mknspimg -o nsp_image.bin -i kernel.bin files.img -a 0 4096",
		"This generates 'nsp_image.bin' from two input files aligning first to 0 and second to 4096 bytes."
	};

	int num_lines = sizeof(help_page)/sizeof(char*);
	int i;
	for(i=0; i < num_lines; i++) {
		printf("%s\n", help_page[i]);
	}
}

/***************************************************************************
 * void mknspimg_print_hdr(NSP_IMG_HDR* p_img_hdr)
 ***************************************************************************/
void mknspimg_print_hdr(struct nsp_img_hdr *hdr)
{
	struct nsp_img_hdr_chksum	*chksum;
	struct nsp_img_hdr_sections	*section;
	int i;

	printf("****************** NSP Image Summary ******************\n");
	printf("Magic:             0x%x\n",		hdr->head.magic);
	printf("Image Header Size: 0x%x bytes\n",	hdr->head.hdr_size);
	printf("Total Image Size:  %d bytes\n",		hdr->head.image_size);
	printf("Product ID:        0x%x\n",		hdr->head.prod_id);
	printf("Release ID:        0x%x\n",		hdr->head.rel_id);
	printf("Version ID:        0x%x\n",		hdr->head.version);

	printf("Offset Info:       0x%x\n",		hdr->head.info_offset);
	printf("Offset Sect info:  0x%x\n",		hdr->head.sect_info_offset);
	printf("Offset Sections:   0x%x\n",		hdr->sect_info.sections_offset);

	chksum=(struct nsp_img_hdr_chksum *)(hdr+hdr->head.chksum_offset);
	printf("Header Checksum:   0x%x\n",		chksum->hdr_chksum);

	printf("+++ Section Information +++\n");
	printf("# of sections:     %u\n", hdr->sect_info.num_sects);
	section=&(hdr->sections);
	for(i = 0; i < hdr->sect_info.num_sects; i++, section++) {
		printf("+++++ Section %d +++++\n", i);
		printf("Total size:  %u bytes\n",	section->total_size);
		printf("Raw Size:    %u bytes\n",	section->raw_size);
		printf("Offset:      0x%x\n",		section->offset);
		printf("Type:        0x%x\n",		section->type);
		printf("Name:        %s\n",		section->name);
	}
	printf("*******************************************************\n");
}

CMDLINE_CFG	cmd_line_cfg =
{
	{
		/*	MIN	MAX	FLAGS					OPTION	*/
		{	2,	2,	(CMDLINE_OPTFLAG_ALLOW | CMDLINE_OPTFLAG_MANDAT) },	/* '-a' align1 align2 */
		{	0,	0,	CMDLINE_OPTFLAG_ALLOW },		/* '-b' bootstrap */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-c' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-d' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-e' */
		{	1,	1,	CMDLINE_OPTFLAG_ALLOW },		/* '-f' flags */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-g' */
		{	1,	1,	CMDLINE_OPTFLAG_ALLOW },		/* '-h' */
		{	2,	2,	(CMDLINE_OPTFLAG_ALLOW | CMDLINE_OPTFLAG_MANDAT) },	/* '-i arg1 arg2 ' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-j' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-k' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-l' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-m' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-n' */
		{	1,	1,	(CMDLINE_OPTFLAG_ALLOW | CMDLINE_OPTFLAG_MANDAT) },	/* '-o arg' */
		{	1,	1,	CMDLINE_OPTFLAG_ALLOW },		/* '-p' PROD_ID */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-q' */
		{	1,	1,	CMDLINE_OPTFLAG_ALLOW },		/* '-r' REL_ID */
		{	1,	1,	CMDLINE_OPTFLAG_ALLOW },		/* '-s' "Release XXX.XXX" */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-t' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-u' */
		{	0,	0,	CMDLINE_OPTFLAG_ALLOW },		/* '-v' control VERBOSE/NON-VERBOSE mode */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-w' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-x' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW },		/* '-y' */
		{	0,	0,	!CMDLINE_OPTFLAG_ALLOW }		/* '-z' */
	},
	{	0,		0,	!CMDLINE_OPTFLAG_ALLOW },		/* global arguments */
};

/***************************************************************************
 * int nsp_img_write(void* image, char* file, int padding)
 * Write out the image.
 ***************************************************************************/
int main(int argc, char* argv[], char* env[])
{
	FILE*	nsp_image	= NULL;
	int header_version=1;
	int	cmdline_err;
	char*	cmdline_error_msg;

	char*	filen_out;

	int	i,count;			/* loop variables */
	int	num_sects = 2;			/* We require exactly two image with -i option
							   (see CMDLINE_CFG structure above) */
	int	total = 0;

	int	header_size=0;
	struct nsp_img_hdr_head		*img_hdr_head;	/* Start of image header */
	struct nsp_img_hdr_info *img_hdr_info;
	struct nsp_img_hdr_section_info *img_hdr_section_info ;
	struct nsp_img_hdr_sections	*img_hdr_sections, *section;	/* Section pointers */
	

	/* Configure the command line. */
	cmdline_configure(&cmd_line_cfg);

	/* Read and parse the command line. */
	cmdline_err = cmdline_read(argc, argv);

	/* Check for parsing errors. */
	if(cmdline_err != 0) {
		/* Get the parse error message */
		cmdline_error_msg = cmdline_error(cmdline_err);

		/* Print it out */
		printf("%s\n", cmdline_error_msg);

		/* Print our help too */
		print_help();
		return -1;
	}
	if(cmdline_getopt_count('h') > 0)
	{
		header_version=atoi(argv[cmdline_getarg(cmdline_getarg_list('h'),0)]);
	}
	/* Set up arguments */
	filen_out	= argv[cmdline_getarg(cmdline_getarg_list('o'),0)];
	/* Command line arguments have been parsed. Start doing our work. */

	/* Caculate the header size, and allocate the memory, and assign the sub pointers */
	header_size =	sizeof(struct nsp_img_hdr_head) +		/* This has a single section
								   desc block already */
				(header_version==1?0:4) + 
				sizeof(struct nsp_img_hdr_info) + 
				sizeof(struct nsp_img_hdr_section_info) + 
			sizeof(struct nsp_img_hdr_sections) * num_sects ;

	img_hdr_head = (struct nsp_img_hdr_head *)malloc(header_size);
	memset(img_hdr_head, 0x0, header_size);
	img_hdr_info = (struct nsp_img_hdr_info*)((char *)img_hdr_head + sizeof(struct nsp_img_hdr_head) + (header_version==1?0:4));
	img_hdr_section_info = (struct nsp_img_hdr_section_info*)((char *)img_hdr_info + sizeof(struct nsp_img_hdr_info));
	img_hdr_sections = (struct nsp_img_hdr_sections*)((char *)img_hdr_section_info + sizeof(struct nsp_img_hdr_section_info));
	section = img_hdr_sections;
	memset(img_hdr_head, 0xff, (void*)img_hdr_info - (void*)img_hdr_head);
	
	img_hdr_head->hdr_version = header_version;
	img_hdr_head->hdr_size = header_size;
	img_hdr_head->info_offset = (void*)img_hdr_info - (void*)img_hdr_head;
	img_hdr_head->sect_info_offset = (void*)img_hdr_section_info - (void*)img_hdr_head;
	
	img_hdr_section_info->num_sects = num_sects;
	img_hdr_section_info->sect_size = sizeof(struct nsp_img_hdr_sections);
	img_hdr_section_info->sections_offset = (void*)img_hdr_sections - (void*)img_hdr_head;
	
/*	chksum = (struct nsp_img_hdr_chksum *)
			((unsigned int)image_hdr + header_size - sizeof(struct nsp_img_hdr_chksum));*/

	/* Open the out file */
	nsp_image = fopen(filen_out,"wb+");
	if(nsp_image==NULL) {
		printf("ERROR: can't open %s for writing.\n", filen_out);
		return -1;
	}

	/* Skip image header. We'll come back to it after we've written out the images. */	
	fseek(nsp_image,header_size,SEEK_SET);
	total = ftell(nsp_image);
	total = header_size;
	printf("total=%x\n",total);
	{
		int align;
		int	padding;
		char * buf;
		align = (header_version==1?0x10000:0x4000);
		if(align==0) {
			/* The user indicated no padding */
			padding = 0;
		} else {
			/* Calculate number padding bytes */
			if((total %align) ==0)
				padding=0;
			else
				padding = align - (total % align);
		}
		if(padding>0)
		{
			buf=malloc(padding);
			memset(buf, 0xff, padding);
			if(fwrite((void*)buf,1,padding,nsp_image)!=padding) {
				printf("ERROR: can't write to %s.\n", filen_out);
				free(buf);
				fclose(nsp_image);
				return -1;
			}
			free(buf);
			
		}
		total+=padding;
		

	}
	/* Write out all specified images (with -i option) */
	for(i=0; i < num_sects; i++) {
		char*	file_name;		/* input file name */
		FILE*	filep;			/* input file pointer */
		int	padding;		/* number of padding bytes to prepend */
		int	align;			/* align factor from command line */
		int	result;			/* intermediate result */
		char * buf;

		/* Open the specified image for reading */
		file_name	= argv[cmdline_getarg(cmdline_getarg_list('i'),i)];
		filep		= fopen(file_name, "rb");
		if(filep==NULL) {
			printf("ERROR: can't open file %s for reading.\n", file_name);
			return -1;
		}
		section->flags = ~0x00;
		/* Determine file size */
		fseek(filep,0,SEEK_END);
		section->raw_size=ftell(filep);
		fseek(filep,0,SEEK_SET);
		cs_calc_sum(filep,(unsigned long *)&section->chksum,0);
		fseek(filep,0,SEEK_SET);

		/* Retrieve the alignment constant */
		/* Set image offset from the beginning of the out file */
		section->offset=total;// + padding;

		//total += padding;

		/* Copy the image file into nsp_image */
		count = section->raw_size;
		buf=malloc(count);
		result=fread(buf, 1, count, filep);
		fwrite(buf, 1, result, nsp_image);
		free(buf);
		
		/* HACK: This is a hack to get the names and types to the files.
			TODO: Fix this to be a real method */
		if(i==0){
			section->type=NSP_IMG_SECTION_TYPE_KERNEL;
			strncpy(section->name, "kernel", 16);
		} else if(i==1){
			section->type=NSP_IMG_SECTION_TYPE_FILESYSTEM_ROOT;
			strncpy(section->name, "root", 16);
		}

		/* Account for the total */
		align	=  strtoul(argv[cmdline_getarg(cmdline_getarg_list('a'),i)],NULL,0);
		if(i==0){
			if(align==0 || (((section->raw_size+ section->offset)%align)==0))
				padding=0;
			else
				padding = align - ((section->raw_size+ section->offset) % align);

			section->total_size=section->raw_size + padding;
		}
		else{
			#define EXTRA_BLOCK 0x10000
			unsigned int squash_padding;
			squash_padding = EXTRA_BLOCK - section->raw_size % EXTRA_BLOCK;
			buf=malloc(EXTRA_BLOCK + 4);
			memset(buf, 0, squash_padding);
			fwrite(buf, 1, squash_padding, nsp_image);
			memset(buf, 0, EXTRA_BLOCK + 4);
			*((unsigned int *)buf)=0xdec0adde;
			*((unsigned int *)(buf+EXTRA_BLOCK))=0xdec0adde;
			fwrite(buf, 1, EXTRA_BLOCK+4, nsp_image);
			free(buf);
			
			if(align==0 || (((section->raw_size + (EXTRA_BLOCK + 4 + squash_padding)) %align)==0))
				padding=0;
			else
				padding = align - ((section->raw_size + (EXTRA_BLOCK + 4 + squash_padding)) % align);
			section->total_size=section->raw_size + (EXTRA_BLOCK + 4 + squash_padding) + padding;
		}
		if(padding>0){
			buf=malloc(padding);
			memset(buf, 0xff, padding);
			fwrite(buf, 1, padding, nsp_image);
			free(buf);
		}
		printf("*****padding is %d\ttotal_size=%d\traw_size=%d\n",padding, section->total_size, section->raw_size);

		//total += section->raw_size;
		total = section->total_size + section->offset;
		printf("total=0x%x\n",total);
		/* Close the input file */
		fclose(filep);

		/* Move the section pointer to the next slot */
		section++;
	}

	/* Take care of the NSP image header fields */

	/* head fields */
	img_hdr_head->magic		= NSP_IMG_MAGIC_NUMBER;
	img_hdr_head->boot_offset	= img_hdr_sections->offset;
	img_hdr_head->flags		= ~0x00;			/* Set to all 1's */

	if(cmdline_getopt_count('b'))
		img_hdr_head->flags	&= ~(NSP_IMG_FLAG_FAILBACK_5 | NSP_IMG_FLAG_FAILBACK_1);

	if(cmdline_getopt_count('f'))
		img_hdr_head->flags	= strtoul(argv[cmdline_getarg(cmdline_getarg_list('f'),0)], 0, 16);

#if 0
	img_hdr_head->hdr_version	= 2;
	img_hdr_head->hdr_size	= header_size;
#endif

	if(cmdline_getopt_count('p'))
		img_hdr_head->prod_id		= strtoul(argv[cmdline_getarg(cmdline_getarg_list('p'),0)], 0, 16);
	else
		img_hdr_head->prod_id		= 0x4C575943;

	if(cmdline_getopt_count('r'))
		img_hdr_head->rel_id		= strtoul(argv[cmdline_getarg(cmdline_getarg_list('r'),0)], 0, 0);
	else
		img_hdr_head->rel_id		= 0x10203040;

	if(cmdline_getopt_count('s'))
		img_hdr_head->version		= strtoul(argv[cmdline_getarg(cmdline_getarg_list('s'),0)], 0, 0);
	else
		img_hdr_head->version		= 0x0b040000;
	img_hdr_head->image_size	= total;
#if 0
	img_hdr_head->info_offset	= (unsigned int)(&(image_hdr->info)) -
						(unsigned int)image_hdr;
	img_hdr_head->sect_info_offset= (unsigned int)(&(image_hdr->sect_info)) -
						(unsigned int)image_hdr;
#endif
//	image_hdr->head.chksum_offset	= (unsigned int)chksum - (unsigned int)image_hdr;
	img_hdr_head->chksum_offset = 0xffffffff;
//	image_hdr->head.pad1 = 0xffffffff;
	/* info fields */
	/* TODO: Fix. Do nothing yet */
//	strncpy(nsp_img_hdr.id.prod_info,NSP_PRODINFO_STRING,sizeof(NSP_PRODINFO_STRING));
	strcpy(img_hdr_info->image_filename, (const char *)basename(filen_out));
	/* section fields */
#if 0
	img_hdr_section_info->num_sects=		num_sects;
	img_hdr_section_info->sect_size=		sizeof(struct nsp_img_hdr_sections);
	img_hdr_section_info->sections_offset=	(unsigned int)(&(image_hdr->sections)) -
						(unsigned int)image_hdr;
#endif

	/* Calculate checksum(s) */
#if 0
	chksum->hdr_chksum = cs_calc_buf_sum((char*)image_hdr,
			header_size - sizeof(struct nsp_img_hdr_chksum));
#endif
	/* Write out the NSP header. */
	fseek(nsp_image,0,SEEK_SET);
	count = fwrite((void*)img_hdr_head, header_size, 1, nsp_image);
	if(count!=1) {
		printf("ERROR: can't write to %s.\n", filen_out);
		return -1;
	}

	/* Check if -v option was specified (no arg needed) */
	if(cmdline_getopt_count('v') > 0)
	{
		struct nsp_img_hdr_head	head;
		struct nsp_img_hdr	*hdr;

		/* Rewind the file back to the beginning */
		fseek(nsp_image,0,SEEK_SET);

		/* Read header from the file */
		fread((void*)&head, sizeof(struct nsp_img_hdr_head),
				1, nsp_image);

		/* Get memory to store the complete header */
		hdr = (struct nsp_img_hdr *)malloc(head.hdr_size);

		/* Read header from the file */
		fseek(nsp_image,0,SEEK_SET);
		fread((void*)hdr, head.hdr_size, 1, nsp_image);

		/* Print it out */
		mknspimg_print_hdr(hdr);
		printf("Generated total %d bytes\n",total);
		free(hdr);
	}

	free(img_hdr_head);

      {
	  struct checksumrecord cr;
      cr.magic=CKSUM_MAGIC_NUMBER;
      cs_calc_sum(nsp_image, (unsigned long *)&cr.chksum, 0);
      fseek(nsp_image,0, SEEK_END);
      fwrite(&cr, 1, sizeof(cr), nsp_image);
	  }
	  {
		FILE * non_web;
		char fname[256];
		char * img_buf;
		unsigned int len;
		strcpy(fname, filen_out);
		strcat(fname, ".non_web");
		non_web = fopen(fname,"wb+");
		fseek(nsp_image, 0, SEEK_END);
		len = ftell(nsp_image);
		img_buf=malloc(len);
		fseek(nsp_image, 0, SEEK_SET);
		fread(img_buf, 1, len, nsp_image);
		img_buf[0xb] = 0x17;
		fwrite(img_buf, 1, len-sizeof(struct checksumrecord), non_web);
		fclose(non_web);
		free(img_buf);
	  }
	  /* Close NSP image file */
	fclose(nsp_image);

	/* return result */
	return(0);
}

#ifdef DMALLOC
#include <dmalloc.h>
#endif /* DMALLOC */

#define BUFLEN (1 << 16)

static unsigned long crctab[256] =
{
	0x0,
	0x04C11DB7, 0x09823B6E, 0x0D4326D9, 0x130476DC, 0x17C56B6B,
	0x1A864DB2, 0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6,
	0x2B4BCB61, 0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD,
	0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
	0x5BD4B01B, 0x569796C2, 0x52568B75, 0x6A1936C8, 0x6ED82B7F,
	0x639B0DA6, 0x675A1011, 0x791D4014, 0x7DDC5DA3, 0x709F7B7A,
	0x745E66CD, 0x9823B6E0, 0x9CE2AB57, 0x91A18D8E, 0x95609039,
	0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5, 0xBE2B5B58,
	0xBAEA46EF, 0xB7A96036, 0xB3687D81, 0xAD2F2D84, 0xA9EE3033,
	0xA4AD16EA, 0xA06C0B5D, 0xD4326D90, 0xD0F37027, 0xDDB056FE,
	0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
	0xF23A8028, 0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4,
	0xE5FFEB43, 0xE8BCCD9A, 0xEC7DD02D, 0x34867077, 0x30476DC0,
	0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C, 0x2E003DC5,
	0x2AC12072, 0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16,
	0x018AEB13, 0x054BF6A4, 0x0808D07D, 0x0CC9CDCA, 0x7897AB07,
	0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB, 0x6F52C06C,
	0x6211E6B5, 0x66D0FB02, 0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1,
	0x53DC6066, 0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,
	0xACA5C697, 0xA864DB20, 0xA527FDF9, 0xA1E6E04E, 0xBFA1B04B,
	0xBB60ADFC, 0xB6238B25, 0xB2E29692, 0x8AAD2B2F, 0x8E6C3698,
	0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D,
	0x94EA7B2A, 0xE0B41DE7, 0xE4750050, 0xE9362689, 0xEDF73B3E,
	0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2, 0xC6BCF05F,
	0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34,
	0xDC3ABDED, 0xD8FBA05A, 0x690CE0EE, 0x6DCDFD59, 0x608EDB80,
	0x644FC637, 0x7A089632, 0x7EC98B85, 0x738AAD5C, 0x774BB0EB,
	0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
	0x58C1663D, 0x558240E4, 0x51435D53, 0x251D3B9E, 0x21DC2629,
	0x2C9F00F0, 0x285E1D47, 0x36194D42, 0x32D850F5, 0x3F9B762C,
	0x3B5A6B9B, 0x0315D626, 0x07D4CB91, 0x0A97ED48, 0x0E56F0FF,
	0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623, 0xF12F560E,
	0xF5EE4BB9, 0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65,
	0xEBA91BBC, 0xEF68060B, 0xD727BBB6, 0xD3E6A601, 0xDEA580D8,
	0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
	0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2,
	0xAAFBE615, 0xA7B8C0CC, 0xA379DD7B, 0x9B3660C6, 0x9FF77D71,
	0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD, 0x81B02D74,
	0x857130C3, 0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640,
	0x4E8EE645, 0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C, 0x7B827D21,
	0x7F436096, 0x7200464F, 0x76C15BF8, 0x68860BFD, 0x6C47164A,
	0x61043093, 0x65C52D24, 0x119B4BE9, 0x155A565E, 0x18197087,
	0x1CD86D30, 0x029F3D35, 0x065E2082, 0x0B1D065B, 0x0FDC1BEC,
	0x3793A651, 0x3352BBE6, 0x3E119D3F, 0x3AD08088, 0x2497D08D,
	0x2056CD3A, 0x2D15EBE3, 0x29D4F654, 0xC5A92679, 0xC1683BCE,
	0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB,
	0xDBEE767C, 0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18,
	0xF0A5BD1D, 0xF464A0AA, 0xF9278673, 0xFDE69BC4, 0x89B8FD09,
	0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
	0x933EB0BB, 0x97FFAD0C, 0xAFB010B1, 0xAB710D06, 0xA6322BDF,
	0xA2F33668, 0xBCB4666D, 0xB8757BDA, 0xB5365D03, 0xB1F740B4
};

int cs_is_tagged(FILE *fp)
{
	char buf[8];

	fseek(fp, -8, SEEK_END);
	fread(buf, 8, 1, fp);
	if(*(unsigned long*)buf == CKSUM_MAGIC_NUMBER)
		return 1;
	return 0;
}

unsigned long cs_read_sum(FILE *fp)
{
	char buf[8];

	fseek(fp, -8, SEEK_END);
	fread(buf, 8, 1, fp);
	return *((unsigned long*)&buf[4]);
}

int cs_calc_sum(FILE *fp, unsigned long *res, int tagged)
{
	unsigned char buf[BUFLEN];
	unsigned long crc = 0;
	uintmax_t length = 0;
	size_t bytes_read;

	fseek(fp, 0, SEEK_SET);

	while((bytes_read = fread(buf, 1, BUFLEN, fp)) > 0)
	{
		unsigned char *cp = buf;

		if(length + bytes_read < length)
			return 0;

		if(bytes_read != BUFLEN && tagged)
			bytes_read -= 8;

		length += bytes_read;
		while(bytes_read--)
			crc =(crc << 8) ^ crctab[((crc >> 24) ^ *cp++) & 0xFF];
	}

	if(ferror(fp))
		return 0;

	for(; length; length >>= 8)
		crc =(crc << 8) ^ crctab[((crc >> 24) ^ length) & 0xFF];

	crc = ~crc & 0xFFFFFFFF;

	*res = crc;

	return 1;
}

unsigned long cs_calc_buf_sum(char *buf, int size)
{
	unsigned long crc = 0;
	char *cp = buf;
	unsigned long length = size;

	while(size--)
		crc =(crc << 8) ^ crctab[((crc >> 24) ^ *cp++) & 0xFF];

	for(; length; length >>= 8)
		crc =(crc << 8) ^ crctab[((crc >> 24) ^ length) & 0xFF];

	crc = ~crc & 0xFFFFFFFF;

	return crc;
}

unsigned long cs_calc_buf_sum_ds(char *buf, int buf_size, char *sign, int sign_len)
{
	unsigned long crc = 0;
	char *cp = buf;
	unsigned long length = buf_size+sign_len;

	while(buf_size--)
		crc =(crc << 8) ^ crctab[((crc >> 24) ^ *cp++) & 0xFF];

	cp = sign;
	while(sign_len--)
		crc =(crc << 8) ^ crctab[((crc >> 24) ^ *cp++) & 0xFF];


	for(; length; length >>= 8)
		crc =(crc << 8) ^ crctab[((crc >> 24) ^ length) & 0xFF];

	crc = ~crc & 0xFFFFFFFF;

	return crc;
}

int cs_set_sum(FILE *fp, unsigned long sum, int tagged)
{
	unsigned long magic = CKSUM_MAGIC_NUMBER;

	if(tagged)
		fseek(fp, -8, SEEK_END);
	else
		fseek(fp, 0, SEEK_END);

	if(fwrite(&magic, 1, 4, fp) < 4)
		return 0;
	if(fwrite(&sum, 1, 4, fp) < 4)
		return 0;

	return 1;
}

void cs_get_sum(FILE *fp, unsigned long *sum)
{
	unsigned long magic = 0;

	fseek(fp, -8, SEEK_END);

	fread(&magic, 4, 1, fp);
	fread(sum, 4, 1, fp);
}

int cs_validate_file(char *filename)
{
	FILE *pFile = NULL;
	unsigned long sum = 0, res = 0;

	if((pFile = fopen(filename, "r")) == NULL)
		return 0;

	if(!cs_is_tagged(pFile))
	{
		fclose(pFile);
		return 0;
	}
	if(!cs_calc_sum(pFile, &sum, 1))
	{
		fclose(pFile);
		return 0;
	}
	cs_get_sum(pFile, &res);
	fclose(pFile);

	if(sum != res)
		return 0;
	return 1;
}

/* ********* Library internal data ********* */
#define	CMDLINE_TRUE			1
#define	CMDLINE_FALSE			0

typedef	enum CMDLINE_ERR
{
	CMDLINE_ERR_OK		= 0,	/* No Error (OK) */
	CMDLINE_ERR_ERROR	= -1,	/* Unspecified error */
	CMDLINE_ERR_INVKEY	= -3,	/* Invalid option key */
	CMDLINE_ERR_MANYARG	= -4,	/* Too many arguments */
	CMDLINE_ERR_FEWARG	= -5,	/* Too few arguments */
	CMDLINE_ERR_ILLOPT	= -6,	/* Option not allowed (illegal option) */
	CMDLINE_ERR_NOMEM	= -7,	/* No memory */
	CMDLINE_ERR_OPTMIS	= -8	/* A mandatory option is missing */
} CMDLINE_ERR;

/* Argument list */
typedef	struct CMDLINE_ARG
{
	int				index;		/* Index of the argument in the command line */
	struct CMDLINE_ARG*	p_next;	/* Next node in the linked list */
} CMDLINE_ARG;

/* Master control block for an option */
typedef struct CMDLINE_ARGS
{
	int				argc;		/* Total count of arguments found */
	int				optc;		/* Total count of options found */
	CMDLINE_ARG*	list;		/* Argument list */
} CMDLINE_ARGS;

/* Master control block for all found arguments */
typedef	struct CMDLINE_DATA
{
	CMDLINE_ARGS	opt_args[26];	/* Array of MCBs for each option ('a' through 'z') */
	CMDLINE_ARGS	glb_args;		/* Global arguments */
	int				parsed;			/* Internal flag to prevent client calls if library is not initialized */
} CMDLINE_DATA;

/* ********* Local Data ********* */
static CMDLINE_CFG cmdline_cfg;
static CMDLINE_DATA cmdline_data;

char*	cmdline_errmsg = "CMDLINE ERROR";

/* ***************************************************************
* Print all found command line options and their arguments
****************************************************************** */
void* cmdline_getarg_list(char opt)
{
	int index = (opt - 'a');

	/* Check the validity of the index */
	if((index < 0) || (index > 25))
	{
		/* ERROR: Wrong option */
		return NULL;
	}

	/* Return a pointer to the ARGS control structure */
	return((void*)(&cmdline_data.opt_args[index]));
}

/* ***************************************************************
* Print all found command line options and their arguments
****************************************************************** */
int cmdline_getarg_count(void* list)
{
	CMDLINE_ARGS*	p_args = (CMDLINE_ARGS*)list;

	/* Return number of arguments for this option */
	return(p_args->argc);
}

/* ***************************************************************
* Print all found command line options and their arguments
****************************************************************** */
int cmdline_getopt_count(char opt)
{
	int				index;

	/* Calculate index value */
	index = opt - 'a';
	if(index < 0 || index > 25) return -1;

	/* Return number of arguments for this option */
	return(cmdline_data.opt_args[index].optc);
}

/* ***************************************************************
* Print all found command line options and their arguments
****************************************************************** */
int cmdline_getarg(void* list, int num)
{
	int i;
	CMDLINE_ARGS*	p_args = (CMDLINE_ARGS*)list;
	CMDLINE_ARG*	p_arg;

	/* Search the 'num' argument in the list for this option */
	for(i=0,p_arg=p_args->list; (p_arg!=NULL) && (i<p_args->argc); i++, p_arg=p_arg->p_next)
	{
		/* if num matches i, we found it */
		if(i==num) return(p_arg->index);
	}
	/* We did not find the specified argument or the list was empty */
	return -1;
}

/* ***************************************************************
* Print all found command line options and their arguments
****************************************************************** */
int cmdline_configure(CMDLINE_CFG* p_cfg)
{
	/* reset global data */
	memset(&cmdline_cfg,0,sizeof(cmdline_cfg));
	memset(&cmdline_data,0,sizeof(cmdline_data));

	/* Copy the user's config structure */
	cmdline_cfg = *p_cfg;
	return 0;
}

/* ***************************************************************
* Print all found command line options and their arguments
****************************************************************** */
char* cmdline_error(int err)
{
	/* TODO: implement a table of error messages */
	return(cmdline_errmsg);
}

/* ***************************************************************
* Print all found command line options and their arguments
****************************************************************** */
static void cmdline_print_args(CMDLINE_ARGS* p_arglist, char* argv[])
{
	CMDLINE_ARG*	p_arg;

	printf("   Number of times option was specified: %d\n", p_arglist->optc);
	printf("   Number of Arguments:                  %d\n", p_arglist->argc);

	if(p_arglist->argc > 0)
	{
		printf("   Argument List: ");

		for(p_arg=p_arglist->list; p_arg != NULL; p_arg=p_arg->p_next)
			printf("%s ", argv[p_arg->index]);
	}

	printf("\n");
}

/* ***************************************************************
* Print all found command line options and their arguments
****************************************************************** */
void cmdline_print(char* argv[])
{
	int i;

	/* Check if the command line was parsed */
	if(cmdline_data.parsed != CMDLINE_TRUE)
	{
		printf("The command line has not been parsed yet.\n");
		return;
	}

	/* Print out option arguments */
	for( i = 0; i < 26; i++ )
	{
		/* Check if the option was specified */
		if(cmdline_data.opt_args[i].optc !=0 )
		{
			/* Print out option name and arguments */
			printf("Option: -%c\n", (char)('a'+i));
			cmdline_print_args(&(cmdline_data.opt_args[i]), argv);
		}
	}

	/* Print out global arguments */
	printf("Global arguments:\n");
	cmdline_print_args(&(cmdline_data.glb_args), argv);
}

/* ***************************************************************
* Print configuration
****************************************************************** */
void cmdline_print_cfg(void)
{

}

static void cmdline_argadd(CMDLINE_ARGS* p_arglist, CMDLINE_ARG* p_arg)
{
	CMDLINE_ARG*	p_list;
	CMDLINE_ARG*	p_prev=NULL;

	/* See if we had anything in the list */
	if(p_arglist->argc == 0)
	{
		/* Link the argument in */
		p_arglist->list = p_arg;
	}
	else
	{
		/* Find the tail of the list */
		for(p_list=p_arglist->list; p_list != NULL; p_list=p_list->p_next)
			p_prev = p_list;

		/* Link the argument in */
		p_prev->p_next=p_arg;
	}

	/* Keep track of arg number */
	p_arglist->argc++;
}

/* ***************************************************************
* cmdline_read()
* Read and parse command line arguments
****************************************************************** */
int cmdline_read(int argc, char* argv[])
{
	int i, option=0;

	/* Process every command line argument in argv[] array */
	for( i = 1; i < argc; i++ )
	{
		/* Does the argument start with a dash? */
		if( *argv[i] == '-' )
		{
			/* The argument must be two characters: a dash, and a letter */
			if( strlen(argv[i]) != 2 )
			{
				/* ERROR: option syntax (needs to be a dash and one letter) */
				return(CMDLINE_ERR_ERROR);
			}

			/* Check validity of the option key ('a' through 'z') */
			if( ((*(argv[i] + 1)) < 'a') || ((*(argv[i] + 1)) > 'z') )
			{
				/* ERROR: option sysntax (invalid option key) */
				return(CMDLINE_ERR_INVKEY);
			}

			/* Calculate the option index */
			option = (*(argv[i] + 1)) - 'a';
			if((option < 0) || (option > 25)) return(CMDLINE_ERR_INVKEY);

			/* Check to see if the option is allowed */
			if( cmdline_cfg.opts[option].flags & CMDLINE_OPTFLAG_ALLOW )
			{
				/* Option allowed. */
				cmdline_data.opt_args[option].optc++;
				continue;
			}
			else
			{
				/* ERROR: Option is not allowed */
				return(CMDLINE_ERR_ILLOPT);
			}
		}
		else
		{
			/* Read the arguments for the option */
			CMDLINE_ARG*	p_arg;

			/* Allocate space for the argument node */
			p_arg = (CMDLINE_ARG*)calloc(1,sizeof(CMDLINE_ARG));
			if( p_arg== NULL )
			{
				/* ERROR: Can't allocate memory for the argument index */
				return(CMDLINE_ERR_NOMEM);
			}

			/* Initialize the argument */
			p_arg->index	= i;
			p_arg->p_next	= NULL;

			/* Check if we can add to the list of arguments for this option */
			if( (option < 0)																/* Do we have to add to the global list? */
				|| (cmdline_data.opt_args[option].argc == cmdline_cfg.opts[option].max)		/* Did we reach MAX arguments? */
				)
			{
				/* This option does not require arguments. Keep the argument in the global list. */
				cmdline_argadd(&(cmdline_data.glb_args), p_arg);
				continue;
			}
			else
			{
				/* See if the current count has reached max for this option */
				if( cmdline_data.opt_args[option].argc == cmdline_cfg.opts[option].max )
				{
					/* ERROR: too many arguments for an option */
					return(CMDLINE_ERR_MANYARG);
				}
				else
				{
					/* Link the argument to the arg list of the option */
					cmdline_argadd(&(cmdline_data.opt_args[option]), p_arg);
					continue;
				}
			}
		}
	}

	/* ****** We read the complete command line. See if what we collected matches the configuration ******* */

	/* Check every collected option against its configuration */
	for( i=0; i < 26; i++ )
	{
		/* Check if this option was allowed */
		if(cmdline_cfg.opts[i].flags & CMDLINE_OPTFLAG_ALLOW)
		{
			/* See if it was mandatory */
			if(cmdline_cfg.opts[i].flags & CMDLINE_OPTFLAG_MANDAT)
			{
				/* Check if we really collected this option on the command line. */
				if(cmdline_data.opt_args[i].optc == 0)
				{
					/* ERROR: a missing mandatory option */
					return(CMDLINE_ERR_OPTMIS);
				}
				else
				{
					/* Option was there. Check how many args we got for it. */
					if(cmdline_data.opt_args[i].argc < cmdline_cfg.opts[i].min)
					{
						/* ERROR: too few arguments for an option */
						return(CMDLINE_ERR_FEWARG);
					}
					else
					{
						/* This mandatory option was proper. */
						continue;
					}
				}
			}
			else	/* This is non-mandatory option: */
			{
				/* Check if the option was specified on the command line */
				if(cmdline_data.opt_args[i].optc == 0)
				{
					/* option wasn't specified, go to the next */
					continue;
				}
				else
				{
					/* Option was there. Check how many args we collected for it. */
					if(cmdline_data.opt_args[i].argc < cmdline_cfg.opts[i].min)
					{
						/* ERROR: too few arguments for a non-mandatory option */
						return(CMDLINE_ERR_FEWARG);
					}
					else
					{
						/* This non-mandatory option was proper. */
						continue;
					}
				}
			}
		}
		else	/* Option was not allowed. */
		{
			/* We should not get here as the non-allowed options should have been
			trapped eariler. */
		}
	}

	/* Command line was proper as far as the number of options and their arguments */
	cmdline_data.parsed = CMDLINE_TRUE;
	return(CMDLINE_ERR_OK);
}
