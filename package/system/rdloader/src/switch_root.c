//
// Created by juno on 2022/1/12.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mount.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#include "switch_root.h"

static char *last_char_is(const char *s, int c)
{
    if (!s[0])
        return NULL;
    while (s[1])
        s++;

    return (*s == (char)c) ? (char *)s : NULL;
}

static char *concat_path_file(const char *path, const char *filename)
{
    char *lc;
    char *ret;

    if (!path)
        path = "";

    lc = last_char_is(path, '/');

    while (*filename == '/')
        filename++;

    asprintf(&ret, "%s/%s/%s", path, (lc == NULL ? "/" : ""), filename);

    return ret;
}

static void delete_contents(const char *directory, dev_t rootdev)
{
    DIR *dir;
    struct dirent *d;
    struct stat st;

    if (lstat(directory, &st) || st.st_dev != rootdev)
        return;

    if (S_ISDIR(st.st_mode))
    {
        dir = opendir(directory);
        if (dir)
        {
            while ((d = readdir(dir)))
            {
                char *newdir = d->d_name;

                if (DOT_OR_DOTDOT(newdir))
                    continue;

                newdir = concat_path_file(directory, newdir);
                delete_contents(newdir, rootdev);
                free(newdir);
            }
            closedir(dir);
            rmdir(directory);
        }
        else
        {
            unlink(directory);
        }
    }
}

void switch_root(const char *new_root, const char *prog, char *const argv[])
{
    struct stat st;
    dev_t root_dev;

    chdir(new_root);
    stat("/", &st);

    root_dev = st.st_dev;

    stat(".", &st);

    if (st.st_dev == root_dev)
    {
        fprintf(stderr, "%s must be a mountpoint\n", new_root);
        return;
    }

    delete_contents("/", root_dev);
    if (mount(".", "/", NULL, MS_MOVE, NULL))
    {
        fprintf(stderr, "failed to moving root\n");
        return;
    }

    chroot(".");

    execv(prog, argv);
}
