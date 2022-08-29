//
// Created by juno on 2022/1/7.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/syscall.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <byteswap.h>
#include <limits.h>
#include "insmod.h"

#define init_module(module, len, opts) syscall(__NR_init_module, module, len, opts)

#if __BYTE_ORDER == __BIG_ENDIAN
#define SWAP_BE32(x) (x)
#define SWAP_BE64(x) (x)
#else
#define SWAP_BE32(x) bswap_32(x)
#define SWAP_BE64(x) bswap_64(x)
#endif

static char *parse_cmdline_module_options(const char **argv, int quote_space)
{
    char *options;
    int optlen;

    options = calloc(1, sizeof(char));
    optlen = 0;

    while (*++argv)
    {
        const char *fmt;
        const char *var;
        const char *val;

        var = *argv;

        options = realloc(options, optlen + 2 + strlen(var) + 2);

        fmt = "%.*s%s ";

        var = strchrnul(var, '=');

        if (quote_space)
        {
            if (*val)
            {
                val++;
                if (strchr(val, ' '))
                    fmt = "%.*s\"%s\" ";
            }
        }

        optlen += sprintf(options + optlen, fmt, (int)(val - var), var, val);
    }

    return options;
}

static void *mmap_read(int fd, size_t size)
{
    return mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
}

static void *try_to_map_module(const char *filename, size_t *image_size_p)
{
    void *image = NULL;
    struct stat st;
    int fd;

    fd = open(filename, O_RDONLY);

    if (fd < 0)
    {
        fprintf(stderr, "open %s failed:%s\n", filename, strerror(errno));
        return NULL;
    }

    fstat(fd, &st);

    if (st.st_size <= *image_size_p)
    {
        size_t image_size = st.st_size;
        image = mmap_read(fd, image_size);

        if (image == MAP_FAILED)
        {
            image = NULL;
        }
        else if (*(u_int32_t *)image != SWAP_BE32(0x7f454C46))
        {
            munmap(image, image_size);
        }
        else
        {
            *image_size_p = image_size;
        }
    }

    close(fd);
    return image;
}

static void *malloc_open_zipped_read_close(const char *filename, size_t *image_size_p)
{
    return NULL;
}

int do_insmod(const char *modname, const char **argv)
{
    int mmaped = 0;
    size_t image_size = INT_MAX - 4095;
    void *image = NULL;
    int ret;

    if (!modname)
    {
        return -1;
    }

    char *options = *argv == NULL ? "" : parse_cmdline_module_options(argv, 0);

    image = try_to_map_module(modname, &image_size);

    if (image)
    {
        mmaped = 1;
    }
    else
    {
        errno = ENOMEM;
        image = malloc_open_zipped_read_close(modname, &image_size);
        if (!image)
            return -ENOMEM;
    }

    errno = 0;

    init_module(image, image_size, options);

    ret = errno;

    if (mmaped)
        munmap(image, image_size);
    else
        free(image);

    if (ret)
    {
        fprintf(stderr, "insmod failed:%s\n", strerror(ret));
    }

    return ret;
}
