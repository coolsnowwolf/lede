//
// Created by juno on 2022/1/11.
//

#include <stdio.h>
#include <blkid/blkid.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "blkid2.h"

int get_blkid(const char *blkdev, char **uuid)
{
    blkid_probe pr;
    int ret;
    size_t len;
    const char *data;
    void *out;

    pr = blkid_new_probe_from_filename(blkdev);
    if (!pr)
    {
        fprintf(stderr, "Failed open %s:%s\n", blkdev, strerror(errno));
        return -1;
    }

    blkid_do_probe(pr);
    ret = blkid_probe_lookup_value(pr, "UUID", &data, &len);

    if (ret == 0)
    {
        out = calloc(1, len + 1);
        memcpy(out, data, len);
        *uuid = out;
    }

    blkid_free_probe(pr);

    return ret;
}
