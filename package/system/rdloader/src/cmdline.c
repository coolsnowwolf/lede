//
// Created by juno on 2022/1/11.
//

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "cmdline.h"

static void insert_fstype_tail(struct fstype **head, struct fstype *fs)
{
    struct fstype *curr = *head;
    struct fstype *next = NULL;

    if (!curr)
    {
        *head = fs;
        return;
    }

    do
    {
        next = curr->next;
        if (!next)
        {
            curr->next = fs;
            break;
        }
        curr = next;
    } while (curr);
}

static int parse_root(char *args, char **ret)
{
    char *root = NULL;

    if (!args)
        return -1;

    if (strstr(args, "PARTUUID=") != NULL)
    {
        root = strtok(args, "=");
        root = strtok(NULL, "=");
        root = strtok(NULL, "=");
        if (!root)
            return -1;

        asprintf(ret, "/dev/disk/by-partuuid/%s", root);
    }
    else if (strstr(args, "UUID=") != NULL)
    {

        root = strtok(args, "=");
        root = strtok(NULL, "=");
        root = strtok(NULL, "=");

        if (!root)
            return -1;

        asprintf(ret, "/dev/disk/by-uuid/%s", root);
    }
    else if (strstr(args, "/dev/"))
    {
        *ret = strdup(args);
    }
    else
    {
        asprintf(ret, "/dev/%s", args);
    }

    return 0;
}

struct fstype *parse_rootfs(char **fstype_list)
{
    struct fstype *ret = NULL;
    char *list = *fstype_list;
    struct fstype *default_fs = NULL;
    struct fstype *fs = NULL;

    if (list)
    {
        list = strtok(list, "=");

        while ((list = strtok(NULL, ",")))
        {
            fs = calloc(1, sizeof(struct fstype));

            if (!fs)
                return NULL;

            fs->name = strdup(list);
            insert_fstype_tail(&ret, fs);
        }
    }

    default_fs = calloc(1, sizeof(struct fstype));
    default_fs->name = strdup("ext4");
    insert_fstype_tail(&ret, default_fs);

    return ret;
}

int parse_cmdline(struct bootargs_t **ret)
{
    FILE *fp;
    char *cmdline = NULL;
    struct fstype *fs = NULL;
    char str[1024] = {0};
    char *root = NULL;
    char *rootfs = NULL;
    struct bootargs_t *bootargs = NULL;

    fp = fopen("/proc/cmdline", "r");

    if (!fp)
    {
        fprintf(stderr, "get cmdline failed:%s\n", strerror(errno));
        return -1;
    }

    if (fgets(str, sizeof(str) - 1, fp) == NULL)
    {
        fprintf(stderr, "cmdline empty\n");
        fclose(fp);
        return -1;
    }

    fclose(fp);

    cmdline = strtok(str, " ");

    do
    {
        if (!cmdline)
            continue;

        if (strstr(cmdline, "root="))
            root = cmdline;
        if (strstr(cmdline, "rootfstype="))
            rootfs = cmdline;

    } while ((cmdline = strtok(NULL, " ")) != NULL);

    if (parse_root(root, &root) < 0)
    {
        fprintf(stderr, "root args error\n");
        return -1;
    }

    if ((fs = parse_rootfs(&rootfs)) == NULL)
    {
        fprintf(stderr, "fstype error\n");
        return -1;
    }

    bootargs = calloc(1, sizeof(struct bootargs_t));

    if (!bootargs)
    {
        fprintf(stderr, "alloc bootargs failed\n");
        free(fs);
        free(root);
        return -1;
    }

    bootargs->root = root;
    bootargs->fstype = fs;

    *ret = bootargs;

    return 0;
}

void free_bootargs(struct bootargs_t **args)
{
    struct bootargs_t *bootargs = *args;
    struct fstype *curr = NULL;
    struct fstype *next = NULL;

    if (!bootargs)
        return;

    if (bootargs->root)
        free(bootargs->root);

    curr = bootargs->fstype;

    do
    {
        if (curr)
        {
            if (curr->name)
                free(curr->name);

            next = curr->next;
            free(curr);
        }

        curr = next;

    } while (curr);

    free(bootargs);
}
