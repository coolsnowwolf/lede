//
// Created by juno on 2022/1/10.
//
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <blkid/blkid.h>

#include "mkdev_node.h"
#include "mknod.h"
#include "blkid2.h"

static int parse_devno(const char *devname, int *maj, int *min)
{
    FILE *fp;
    char *major_s, *minor_s;
    int major, minor;
    char dev_path[1024] = {0};
    char dev_no[32] = {0};

    snprintf(dev_path, sizeof(dev_path) - 1, "/sys/class/block/%s/dev", devname);

    fp = fopen(dev_path, "r");

    if (fp == NULL)
        return -1;

    if (!fgets(dev_no, 31, fp))
        return -1;

    fclose(fp);
    major_s = strtok(dev_no, ":");
    minor_s = strtok(NULL, ":");

    major = (int)strtoul(major_s, NULL, 0);

    minor = (int)strtoul(minor_s, NULL, 0);

    *maj = major;
    *min = minor;

    return 0;
}

static int mkpartion_node(const char *devname)
{
    int ret = 0;
    dev_t part_devno;
    dev_t disk_devno;
    char *disk_name;
    blkid_probe disk_pr;
    blkid_partlist ls;
    blkid_partition par;
    const char *uuid;
    char *dev_path = NULL;

    asprintf(&dev_path, "/dev/%s", devname);

    if (dev_path == NULL)
        return -1;

    blkid_probe part_pr = blkid_new_probe_from_filename(dev_path);

    if (!part_pr)
    {
        ret = -1;
        goto out1;
    }

    part_devno = blkid_probe_get_devno(part_pr);

    if (!part_devno)
    {
        ret = -1;
        goto out2;
    }

    disk_devno = blkid_probe_get_wholedisk_devno(part_pr);

    if (!disk_devno)
    {
        ret = -1;
        goto out2;
    }

    disk_name = blkid_devno_to_devname(disk_devno);

    if (!disk_name)
    {
        ret = -1;
        goto out2;
    }

    disk_pr = blkid_new_probe_from_filename(disk_name);

    if (!disk_pr)
    {
        ret = -1;
        goto out3;
    }

    ls = blkid_probe_get_partitions(disk_pr);

    if (!ls)
    {
        ret = -1;
        goto out4;
    }

    par = blkid_partlist_devno_to_partition(ls, part_devno);

    uuid = blkid_partition_get_uuid(par);

    if (uuid)
    {
        char long_path[1024] = {0};
        snprintf(long_path, sizeof(long_path) - 1, "/dev/disk/by-partuuid/%s", uuid);

        if (access(long_path, F_OK) == 0)
            goto out4;

        if (access(dev_path, F_OK) != 0)
            goto out4;

        symlink(dev_path, long_path);
    }
    else
    {
        ret = -1;
    }

out4:
    blkid_free_probe(disk_pr);
out3:
    free(disk_name);
out2:
    blkid_free_probe(part_pr);
out1:
    free(dev_path);
    return ret;
}

static int dev_by_partuuit(void)
{
    DIR *dir;
    DIR *disk_dir;
    struct dirent *filename;
    struct dirent *partname;
    char *disk = NULL;

    mkdir("/dev/disk/by-partuuid", 0755);

    dir = opendir("/sys/block");

    if (!dir)
    {
        return -1;
    }

    while ((filename = readdir(dir)))
    {
        if (filename->d_type != DT_LNK)
            continue;

        asprintf(&disk, "/sys/block/%s", filename->d_name);

        disk_dir = opendir(disk);

        if (disk)
            free(disk);

        if (disk_dir)
        {
            while ((partname = readdir(disk_dir)))
            {
                char *dev = NULL;
                asprintf(&dev, "/sys/block/%s/%s/dev", filename->d_name, partname->d_name);

                if (access(dev, F_OK) == 0)
                {
                    mkpartion_node(partname->d_name);
                }

                if (dev)
                    free(dev);
            }
        }

        closedir(disk_dir);
    }

    closedir(dir);

    return 0;
}

int mkdev_node(void)
{
    int ret;
    struct dirent *filename;
    DIR *dir = NULL;
    char dev_path[1024];
    char uuid_node[1024];

    dir = opendir("/sys/class/block");

    ret = mkdir("/dev/disk", 0755);

    if (ret < 0 && errno != EEXIST)
    {
        fprintf(stderr, "create /dev/disk directory failed:%s\n", strerror(errno));
        return -1;
    }

    ret = mkdir("/dev/disk/by-uuid", 0755);

    if (ret < 0 && errno != EEXIST)
    {
        fprintf(stderr, "create /dev/disk/by-uuid directory failed:%s\n", strerror(errno));
        return -1;
    }

    while ((filename = readdir(dir)))
    {
        if (filename->d_type == DT_LNK)
        {
            int major, minor;
            char *uuid = NULL;

            memset(dev_path, 0, sizeof(dev_path));

            snprintf(dev_path, sizeof(dev_path) - 1, "/dev/%s", filename->d_name);

            if (access(dev_path, F_OK) == 0)
                continue;

            if (parse_devno(filename->d_name, &major, &minor) < 0)
                continue;

            ret = do_mknod("0755", dev_path, 'b', major, minor);

            if (ret < 0)
                return ret;

            ret = get_blkid(dev_path, &uuid);

            if (ret < 0)
                continue;

            memset(uuid_node, 0, sizeof(uuid_node));
            snprintf(uuid_node, sizeof(uuid_node) - 1, "/dev/disk/by-uuid/%s", uuid);

            symlink(dev_path, uuid_node);
            if (uuid)
            {
                free(uuid);
                uuid = NULL;
            }
        }
    }

    closedir(dir);

    dev_by_partuuit();

    return 0;
}