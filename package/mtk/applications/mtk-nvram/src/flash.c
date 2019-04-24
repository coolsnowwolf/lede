/******************************************************************
 * $File:   flash.c
 *
 * $Author: Hua Shao
 * $Date:   Oct, 2014
 *
 ******************************************************************/

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>

#include "nvram.h"
#include "flash.h"

struct mtd_info_user
{
    uint8_t type;
    uint32_t flags;
    uint32_t size;
    uint32_t erasesize;
    uint32_t oobblock;
    uint32_t oobsize;
    uint32_t ecctype;
    uint32_t eccsize;
};

struct erase_info_user
{
    uint32_t start;
    uint32_t length;
};

#define MEMGETINFO  _IOR('M', 1, struct mtd_info_user)
#define MEMERASE    _IOW('M', 2, struct erase_info_user)
#define min(x,y) ({ typeof(x) _x = (x); typeof(y) _y = (y); (void) (&_x == &_y); _x < _y ? _x : _y; })

int32_t mtd_open(const char *name, int32_t flags)
{
    FILE *fp;
    char dev[80];
    int i, ret;

    if (strstr(name, "/dev/mtd"))
    {
        return open(name, flags);
    }

    if ((fp = fopen("/proc/mtd", "r")))
    {
        while (fgets(dev, sizeof(dev), fp))
        {
            if (sscanf(dev, "mtd%d:", &i) && strstr(dev, name))
            {
                snprintf(dev, sizeof(dev), "/dev/mtd%d", i);
                if ((ret = open(dev, flags)) < 0)
                {
                    snprintf(dev, sizeof(dev), "/dev/mtd/%d", i);
                    ret = open(dev, flags);
                }
                fclose(fp);
                return ret;
            }
        }
        fclose(fp);
    }
    fprintf(stderr, "Could not open mtd device, %s\n", strerror(errno));
    return -1;
}

uint32_t mtd_size(const char *name)
{
    FILE *fp;
    char buf[80];

    if ((fp = fopen("/proc/mtd", "r")))
    {
        while (fgets(buf, sizeof(buf), fp))
        {
            if (strstr(buf, name))
            {
                char * p = strchr(buf, ' ');
                ASSERT(p);
                p++;
                fclose(fp);
                return strtoul(p,0,16);
            }
        }
    }
    fclose(fp);
    return 0;
}


int32_t flash_read(char *path, char *buf, off_t from, size_t len)
{
    int32_t fd, ret;
    struct mtd_info_user info;

    fd = mtd_open(path, O_RDONLY);
    if (fd < 0)
    {
        fprintf(stderr, "Could not open mtd device\n");
        return -1;
    }

    if (ioctl(fd, MEMGETINFO, &info))
    {
        fprintf(stderr, "Could not get mtd device info\n");
        close(fd);
        return -1;
    }
    if (len > info.size)
    {
        fprintf(stderr, "Too many bytes - %zu > %u bytes\n", len, info.erasesize);
        close(fd);
        return -1;
    }

    lseek(fd, from, SEEK_SET);
    ret = read(fd, buf, len);
    if (ret == -1)
    {
        fprintf(stderr, "Reading from mtd failed\n");
        close(fd);
        return -1;
    }

    close(fd);
    return ret;
}


int32_t flash_write(char *path, char *buf, off_t to, size_t len)
{
    int32_t fd, ret = 0;
    struct mtd_info_user info;
    struct erase_info_user ei;

    fd = mtd_open(path, O_RDWR | O_SYNC);
    if (fd < 0)
    {
        fprintf(stderr, "Could not open mtd device\n");
        return -1;
    }

    if (ioctl(fd, MEMGETINFO, &info))
    {
        fprintf(stderr, "Could not get mtd device info\n");
        close(fd);
        return -1;
    }
    if (len > info.size)
    {
        fprintf(stderr, "Too many bytes - %zu > %u bytes\n", len, info.erasesize);
        close(fd);
        return -1;
    }

    while (len > 0)
    {
        if ((len & (info.erasesize-1)) || (len < info.erasesize))
        {
            int piece_size;
            unsigned int piece, bakaddr;
            char *bak = NULL;

            bak = (char *)malloc(info.erasesize);
            if (bak == NULL)
            {
                fprintf(stderr, "Not enough memory\n");
                close(fd);
                return -1;
            }

            bakaddr = to & ~(info.erasesize - 1);
            lseek(fd, bakaddr, SEEK_SET);

            ret = read(fd, bak, info.erasesize);
            if (ret == -1)
            {
                fprintf(stderr, "Reading from mtd failed\n");
                close(fd);
                free(bak);
                return -1;
            }

            piece = to & (info.erasesize - 1);
            piece_size = min((uint32_t)len, info.erasesize - piece);
            memcpy(bak + piece, buf, piece_size);

            ei.start = bakaddr;
            ei.length = info.erasesize;
            if (ioctl(fd, MEMERASE, &ei) < 0)
            {
                fprintf(stderr, "Erasing mtd failed\n");
                close(fd);
                free(bak);
                return -1;
            }

            lseek(fd, bakaddr, SEEK_SET);
            ret = write(fd, bak, info.erasesize);
            if (ret == -1)
            {
                fprintf(stderr, "Writing to mtd failed\n");
                close(fd);
                free(bak);
                return -1;
            }

            free(bak);
            buf += piece_size;
            to += piece_size;
            len -= piece_size;
        }
        else
        {
            ei.start = to;
            ei.length = info.erasesize;
            if (ioctl(fd, MEMERASE, &ei) < 0)
            {
                fprintf(stderr, "Erasing mtd failed\n");
                close(fd);
                return -1;
            }

            ret = write(fd, buf, info.erasesize);
            if (ret == -1)
            {
                fprintf(stderr, "Writing to mtd failed\n");
                close(fd);
                return -1;
            }

            buf += info.erasesize;
            to += info.erasesize;
            len -= info.erasesize;
        }
    }

    close(fd);
    return ret;
}


