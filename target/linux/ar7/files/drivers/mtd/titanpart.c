#include <linux/kernel.h>
#include <linux/slab.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/partitions.h>
#include <linux/bootmem.h>
#include <linux/magic.h>
#include <asm/mach-ar7/prom.h>

#define IMAGE_A_SIZE 0X3c0000
#define WRTP_PARTS	14
#define NSP_IMG_MAGIC_NUMBER		le32_to_cpu(0x4D544443)
#define NSP_IMG_SECTION_TYPE_KERNEL		(0x01)
#define NSP_IMG_SECTION_TYPE_FILESYSTEM_ROOT	(0x02)
#define NSP_IMG_SECTION_TYPE_FILESYSTEM		(0x03)
#define MAX_NUM_PARTITIONS 14

static int part_count=0;
static struct mtd_partition titan_parts[WRTP_PARTS];


struct nsp_img_hdr_head
{
	unsigned int	magic;		/* Magic number to identify this image header */
	unsigned int	boot_offset;	/* Offset from start of header to kernel code. */
	unsigned int	flags;		/* Image flags. */
	unsigned int	hdr_version;	/* Version of this header. */
	unsigned int	hdr_size;	/* The complete size of all portions of the header */
	unsigned int	prod_id;	/* This product id */
	unsigned int	rel_id;		/* Which release this is */
	unsigned int	version;	/* name-MMM.nnn.ooo-rxx => 0xMMnnooxx. See comment
					   below */
	unsigned int	image_size;	/* Image size (including header) */
	unsigned int	info_offset;	/* Offset from start of header to info block */
	unsigned int	sect_info_offset;	/* Offset from start of header to section desc */
	unsigned int	chksum_offset;	/* Offset from start of header to chksum block */
	unsigned int    pad1;
};

struct nsp_img_hdr_section_info
{
	unsigned int	num_sects;	/* Number of section (and section desc blocks) in this image */
	unsigned int	sect_size;	/* Size of a SINGLE section_desc block */
	unsigned int	sections_offset;	/* Offset to from start of header to the start of the section blocks */
};

/* There will be one of more of the following stuctures in the image header. Each
   section will have one of these blocks. */
struct nsp_img_hdr_sections
{
	unsigned int	offset;		/* Offset of section from start of NSP_IMG_HDR_HEAD */
	unsigned int	total_size;	/* Size of section (including pad size.) */
	unsigned int	raw_size;	/* Size of section only */
	unsigned int	flags;		/* Section flags */
	unsigned int	chksum;		/* Section checksum */
	unsigned int	type;		/* Section type. What kind of info does this section describe */
	char		name[16];	/* Reference name for this section. */
};





static int	titan_parse_env_address(char *env_name, unsigned int *flash_base,
				unsigned int *flash_end)
{
	char	image_name[30];
	char	*env_ptr;
	char	*base_ptr;
	char	*end_ptr;
	char * string_ptr;
	/* Get the image variable */
	env_ptr = prom_getenv(env_name);
	if(!env_ptr){
		printk("titan: invalid env name, %s.\n", env_name);
		return -1;	/* Error, no image variable */
	}
	strncpy(image_name, env_ptr, 30);
	image_name[29]=0;
	string_ptr = image_name;
	/* Extract the start and stop addresses of the partition */
	base_ptr = strsep(&string_ptr, ",");
	end_ptr = strsep(&string_ptr, ",");
	if ((base_ptr == NULL) || (end_ptr == NULL)) {	
		printk("titan: Couldn't tokenize %s start,end.\n", image_name);
		return -1;
	}

	*flash_base = (unsigned int) simple_strtol(base_ptr, NULL, 0);
	*flash_end = (unsigned int) simple_strtol(end_ptr, NULL, 0);
	if((!*flash_base) || (!*flash_end)) {
		printk("titan: Unable to convert :%s: :%s: into start,end values.\n",
				env_name, image_name);
		return -1;
	}
	*flash_base &= 0x0fffffff;
	*flash_end &= 0x0fffffff;
	return 0;
}



static int	titan_get_single_image(char *bootcfg_name, unsigned int *flash_base,
				unsigned int *flash_end)
{
	char	*env_ptr;
	char	*base_ptr;
	char	*end_ptr;
	char	image_name[30];
	char * string_ptr;

	if(!bootcfg_name || !flash_base || !flash_end)
		return -1;

	env_ptr = prom_getenv(bootcfg_name);
	if(!env_ptr){
		printk("titan: %s variable not found.\n", bootcfg_name);
		return -1;	/* Error, no bootcfg variable */
	}

	string_ptr = image_name;
	/* Save off the image name */
	strncpy(image_name, env_ptr, 30);
	image_name[29]=0;

	end_ptr=strsep(&string_ptr, "\"");
	base_ptr=strsep(&string_ptr, "\"");		/* Loose the last " */
	if(!end_ptr || !base_ptr){
		printk("titan: invalid bootcfg format, %s.\n", image_name);
		return -1;	/* Error, invalid bootcfg variable */
	}

	/* Now, parse the addresses */
	return titan_parse_env_address(base_ptr, flash_base, flash_end);
}



static void titan_add_partition(char * env_name, unsigned int flash_base, unsigned int flash_end)
{
		titan_parts[part_count].name = env_name;
		titan_parts[part_count].offset = flash_base;
		titan_parts[part_count].size = flash_end-flash_base;
		titan_parts[part_count].mask_flags = (strcmp(env_name, "bootloader")==0|| 
			strcmp(env_name, "boot_env")==0 ||
			strcmp(env_name, "full_image")==0 )?MTD_WRITEABLE:0;
		part_count++;

}
int create_titan_partitions(struct mtd_info *master,
	struct mtd_partition **pparts,
	unsigned long origin)
{
	struct nsp_img_hdr_head		hdr;
	struct nsp_img_hdr_section_info	sect_info;
	struct nsp_img_hdr_sections	section;
	unsigned int			flash_base, flash_end;
	unsigned int			start, end;
	char				*name;
	int	i;
	int	total_sects=0;
	size_t len;

	/* Get the bootcfg env variable first */
	if(titan_get_single_image("BOOTCFG", &flash_base, &flash_end)) {
		/* Error, fallback */
		return -1;
	}

	/* Get access to the header, and do some validation checks */
	//hdr=(struct nsp_img_hdr_head*)flash_base;
	mtd_read(master, flash_base, sizeof(struct nsp_img_hdr_head), &len, (uint8_t *)&hdr);
	if(hdr.magic != NSP_IMG_MAGIC_NUMBER)
		return -1;	/* Not a single image */

	mtd_read(master, flash_base + hdr.sect_info_offset, sizeof(struct nsp_img_hdr_section_info), &len, (uint8_t *)&sect_info);

	/* Look for the root fs, and add it first. This way we KNOW where the rootfs is */
	for(i=0; i< sect_info.num_sects && i<MAX_NUM_PARTITIONS; i++){
		mtd_read(master, flash_base + sect_info.sections_offset + (i * sect_info.sect_size) , sizeof(struct nsp_img_hdr_sections), &len, (uint8_t *)&section);
		/* Add only the root partition */
		if(section.type != NSP_IMG_SECTION_TYPE_FILESYSTEM_ROOT){
			continue;
		}
		start=flash_base + section.offset;
		end=start + section.total_size;
		titan_add_partition("root", start, end);
		total_sects++;
		
	}

	for(i=0; i< sect_info.num_sects && i<MAX_NUM_PARTITIONS; i++){

		mtd_read(master, flash_base + sect_info.sections_offset + (i * sect_info.sect_size) , sizeof(struct nsp_img_hdr_sections), &len, (uint8_t *)&section);

		name=section.name;
		if(section.type == NSP_IMG_SECTION_TYPE_FILESYSTEM_ROOT)
		{
			name = "rootfs";
			start=flash_base + section.offset;
			end=flash_end;
			titan_add_partition(name, start, end);
			total_sects++;
		}
		else if(section.type == NSP_IMG_SECTION_TYPE_KERNEL)
		{
			name = "kernel";
			start=flash_base + section.offset;
			end=start + section.total_size;
			titan_add_partition(name, start, end);
			total_sects++;
		}

	}

	/* Next, lets add the single image */
	titan_add_partition("primary_image", flash_base, flash_end);
	total_sects++;


	titan_add_partition("full_image", 0,  master->size);
	total_sects++;

	if (!titan_parse_env_address("BOOTLOADER", &start, &end)){
		titan_add_partition("bootloader", start, end);
		total_sects++;
	}
	if (!titan_parse_env_address("boot_env", &start, &end)){
		titan_add_partition("boot_env", start, end);
		total_sects++;
	}
	*pparts = titan_parts;
	return total_sects;
}
