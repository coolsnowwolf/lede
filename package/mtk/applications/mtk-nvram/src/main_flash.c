/******************************************************************
 * $File:   main.c
 *
 * $Author: Hua Shao
 * $Date:   Oct, 2014
 *
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#include "flash.h"


int usage(void)
{
    printf("%s", "flash r <device> <offset> <length>\n");
    printf("%s", "flash w <device> <offset> <0xNN> [<0xNN> ...]\n");
    return 0;
}

int main(int argc, char ** argv)
{
    int off = 0;
    int len = 0;
    int ret = 0;
    int i = 0;
    char * buf = NULL;

    if (argc < 5)
        return usage();

    if (0 == strcmp(argv[1], "r"))
    {
        off = atoi(argv[3]);
        len = atoi(argv[4]);
        buf = (char *)malloc(len);
        if (!buf)
        {
            fprintf(stderr, "failed to alloc buf[%d], %s\n", len, strerror(errno));
            return -1;
        }
        memset(buf, 0, len);

        ret = flash_read(argv[2], buf, off, len);
        if (ret < 0)
        {
            fprintf(stderr, "failed to read buf[%d], %s\n", len, strerror(errno));
            if (buf) free(buf);
            return -1;
        }

        for(i=0; i<len; i++)
        {
            printf("%02X", buf[i]);
        }
        printf("\n");
        if (buf) free(buf);
    }
    else
    {
        off = atoi(argv[3]);
        buf = (char *)malloc(argc-4);
        if (!buf)
        {
            fprintf(stderr, "failed to alloc buf[%d], %s\n", argc-4, strerror(errno));
            return -1;
        }
        memset(buf, 0, argc-4);

        for (i=4; i<argc; i++)
        {
            //fprintf(stderr, "%ld\n", strtol(argv[i], NULL, 16));
            buf[i-4] = (char)strtol(argv[i], NULL, 16);
        }

        ret = flash_write(argv[2], buf, off, argc-4);
        if (buf) free(buf);
        if (ret < 0)
        {
            fprintf(stderr, "failed to write buf[%d], %s\n", argc-4, strerror(errno));
            return -1;
        }
    }

    return 0;
}


