#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define IMG_SIZE     0x3e0000

#define KERNEL_START 0x020000
#define KERNEL_SIZE  0x0b0000

#define ROOTFS_START 0x0d0000
#define ROOTFS_SIZE  0x30ffb2

char* app_name;




void print_usage(void)
{
  fprintf(stderr, "usage: dgfirmware [<opts>] <img>\n");
  fprintf(stderr, "  <img>               firmware image filename\n");
  fprintf(stderr, "  <opts>  -h          print this message\n");
  fprintf(stderr, "          -f          fix the checksum\n");
  fprintf(stderr, "          -x  <file>  extract the rootfs file to <file>\n");
  fprintf(stderr, "          -xk <file>  extract the kernel to <file>\n");
  fprintf(stderr, "          -m  <file>  merge in rootfs fil\e from <file>\n");
  fprintf(stderr, "          -k  <file>  merge in kernel from <file>\n");
  fprintf(stderr, "          -w  <file>  write back the modified firmware\n");
}


unsigned char* read_img(const char *fname)
{
  FILE *fp;
  int size;
  unsigned char *img;

  fp = fopen(fname, "rb");
  if (fp == NULL) {
    perror(app_name);
    exit(-1);
  }

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  
  if (size != IMG_SIZE) {
    fprintf(stderr, "%s: image file has wrong size\n", app_name);
    fclose(fp);
    exit(-1);
  }

  rewind(fp);

  img = malloc(IMG_SIZE);
  if (img == NULL) {
    perror(app_name);
    fclose(fp);
    exit(-1);
  }

  if (fread(img, 1, IMG_SIZE, fp) != IMG_SIZE) {
    fprintf(stderr, "%s: can't read image file\n", app_name);
    fclose(fp);
    exit(-1);
  }

  fclose(fp);
  return img;
}


void write_img(unsigned char* img, const char *fname)
{
  FILE *fp;

  fp = fopen(fname, "wb");
  if (fp == NULL) {
    perror(app_name);
    exit(-1);
  }

  if (fwrite(img, 1, IMG_SIZE, fp) != IMG_SIZE) {
    fprintf(stderr, "%s: can't write image file\n", app_name);
    fclose(fp);
    exit(-1);
  }

  fclose(fp);
}


void write_rootfs(unsigned char* img, const char *fname)
{
  FILE *fp;

  fp = fopen(fname, "wb");
  if (fp == NULL) {
    perror(app_name);
    exit(-1);
  }
  
  if (fwrite(img+ROOTFS_START, 1, ROOTFS_SIZE, fp) != ROOTFS_SIZE) {
    fprintf(stderr, "%s: can't write image file\n", app_name);
    fclose(fp);
    exit(-1);
  }

  fclose(fp);
}


void write_kernel(unsigned char* img, const char *fname)
{
  FILE *fp;

  fp = fopen(fname, "wb");
  if (fp == NULL) {
    perror(app_name);
    exit(-1);
  }
  
  if (fwrite(img+KERNEL_START, 1, KERNEL_SIZE, fp) != KERNEL_SIZE) {
    fprintf(stderr, "%s: can't write kernel file\n", app_name);
    fclose(fp);
    exit(-1);
  }

  fclose(fp);
}


unsigned char* read_rootfs(unsigned char* img, const char *fname)
{
  FILE *fp;
  int size;
  int i;

  for (i=ROOTFS_START; i<ROOTFS_START+ROOTFS_SIZE; i++)
    img[i] = 0xff;

  fp = fopen(fname, "rb");
  if (fp == NULL) {
    perror(app_name);
    exit(-1);
  }

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  
  if (size > ROOTFS_SIZE) {
    fprintf(stderr, "%s: rootfs image file is too big\n", app_name);
    fclose(fp);
    exit(-1);
  }

  rewind(fp);

  if (fread(img+ROOTFS_START, 1, size, fp) != size) {
    fprintf(stderr, "%s: can't read rootfs image file\n", app_name);
    fclose(fp);
    exit(-1);
  }

  fclose(fp);
  return img;
}


unsigned char* read_kernel(unsigned char* img, const char *fname)
{
  FILE *fp;
  int size;
  int i;

  for (i=KERNEL_START; i<KERNEL_START+KERNEL_SIZE; i++)
    img[i] = 0xff;

  fp = fopen(fname, "rb");
  if (fp == NULL) {
    perror(app_name);
    exit(-1);
  }

  fseek(fp, 0, SEEK_END);
  size = ftell(fp);
  
  if (size > KERNEL_SIZE) {
    fprintf(stderr, "%s: kernel binary file is too big\n", app_name);
    fclose(fp);
    exit(-1);
  }

  rewind(fp);

  if (fread(img+KERNEL_START, 1, size, fp) != size) {
    fprintf(stderr, "%s: can't read kernel file\n", app_name);
    fclose(fp);
    exit(-1);
  }

  fclose(fp);
  return img;
}


int get_checksum(unsigned char* img)
{
  short unsigned s;

  s = img[0x3dfffc] + (img[0x3dfffd]<<8);

  return s;
}


void set_checksum(unsigned char*img, unsigned short sum)
{
  img[0x3dfffc] = sum & 0xff;
  img[0x3dfffd] = (sum>>8) & 0xff;
}


int compute_checksum(unsigned char* img)
{
  int i;
  short s=0;

  for (i=0; i<0x3dfffc; i++)
    s += img[i];

  return s;
}


int main(int argc, char* argv[])
{
  char *img_fname     = NULL;
  char *rootfs_fname  = NULL;
  char *kernel_fname  = NULL;
  char *new_img_fname = NULL;

  int do_fix_checksum = 0;
  int do_write        = 0;
  int do_write_rootfs = 0;
  int do_read_rootfs  = 0;
  int do_write_kernel = 0;
  int do_read_kernel  = 0;

  int i;
  unsigned char *img;
  unsigned short img_checksum;
  unsigned short real_checksum;

  app_name = argv[0];

  for (i=1; i<argc; i++) {
    if (!strcmp(argv[i], "-h")) {
      print_usage();
      return 0;
    }
    else if (!strcmp(argv[i], "-f")) {
      do_fix_checksum = 1;
    }
    else if (!strcmp(argv[i], "-x")) {
      if (i+1 >= argc) {
	fprintf(stderr, "%s: missing argument\n", app_name);
	return -1;
      }
      do_write_rootfs = 1;
      rootfs_fname = argv[i+1];
      i++;
    }
    else if (!strcmp(argv[i], "-xk")) {
      if (i+1 >= argc) {
	fprintf(stderr, "%s: missing argument\n", app_name);
	return -1;
      }
      do_write_kernel = 1;
      kernel_fname = argv[i+1];
      i++;
    }
    else if (!strcmp(argv[i], "-m")) {
      if (i+1 >= argc) {
	fprintf(stderr, "%s: missing argument\n", app_name);
	return -1;
      }
      do_read_rootfs = 1;
      rootfs_fname = argv[i+1];
      i++;
    }
    else if (!strcmp(argv[i], "-k")) {
      if (i+1 >= argc) {
	fprintf(stderr, "%s: missing argument\n", app_name);
	return -1;
      }
      do_read_kernel = 1;
      kernel_fname = argv[i+1];
      i++;
    }
    else if (!strcmp(argv[i], "-w")) {
      if (i+1 >= argc) {
	fprintf(stderr, "%s: missing argument\n", app_name);
	return -1;
      }
      do_write = 1;
      new_img_fname = argv[i+1];
      i++;
    }
    else if (img_fname != 0) {
      fprintf(stderr, "%s: too many arguments\n", app_name);
      return -1;
    }
    else {
      img_fname = argv[i];
    }
  }

  if (img_fname == NULL) {
    fprintf(stderr, "%s: missing argument\n", app_name);
    return -1;
  }

  if ((do_read_rootfs && do_write_rootfs) ||
      (do_read_kernel && do_write_kernel)) {
    fprintf(stderr, "%s: conflictuous options\n", app_name);
    return -1;
  }

  printf ("** Read firmware file\n");
  img = read_img(img_fname);

  printf ("Firmware product: %s\n", img+0x3dffbd);
  printf ("Firmware version: 1.%02d.%02d\n", (img[0x3dffeb] & 0x7f), img[0x3dffec]);

  if (do_write_rootfs) {
    printf ("** Write rootfs file\n");
    write_rootfs(img, rootfs_fname);
  }

  if (do_write_kernel) {
    printf ("** Write kernel file\n");
    write_kernel(img, kernel_fname);
  }

  if (do_read_rootfs) {
    printf ("** Read rootfs file\n");
    read_rootfs(img, rootfs_fname);
    do_fix_checksum = 1;
  }

  if (do_read_kernel) {
    printf ("** Read kernel file\n");
    read_kernel(img, kernel_fname);
    do_fix_checksum = 1;
  }

  img_checksum = get_checksum(img);
  real_checksum = compute_checksum(img);
  
  printf ("image checksum = %04x\n", img_checksum);
  printf ("real checksum  = %04x\n", real_checksum);

  if (do_fix_checksum) {
    if (img_checksum != real_checksum) {
      printf ("** Bad Checksum, fix it\n");
      set_checksum(img, real_checksum);
    }
    else {
      printf ("** Checksum is correct, good\n");
    }
  }

  if (do_write) {
    printf ("** Write image file\n");
    write_img(img, new_img_fname);
  }

  free(img);
  return 0;
}

