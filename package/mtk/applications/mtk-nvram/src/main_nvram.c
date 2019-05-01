/******************************************************************
 * $File:   main.c
 *
 * $Author: Hua Shao
 * $Date:   Oct, 2014
 *
 ******************************************************************/

#include <stdio.h>
#include "nvram.h"

int32_t usage(int32_t reason)
{
    if (reason != OK)
        printf("Invalid usage! ");
    fprintf(stderr, "Usage:\n"
            "\tnvram get <section> <name>\n"
            "\t\t Get a value from the section by name. It operates in cache.\n"
            "\tnvram set <section> <name> <value>\n"
            "\t\t Set a value into the section by name. It operates in cache.\n"
            "\tnvram del <section> <name>\n"
            "\t\t Delete an entry in the section by name. It operates in cache.\n"
            "\tnvram commit\n"
            "\t\t Flush cache into flash.\n"
            "\tnvram layout\n"
            "\t\t Display nvram sections layout.\n"
            "\tnvram show [section]\n"
            "\t\t Display all entries in nvram. Section name is optional.\n"
            "\tnvram loadfile <section> <filename>\n"
            "\t\t Load nvram entries from a file, and flush it into the section.\n"
            "\tnvram clear <section>\n"
            "\t\t DANGEROUS! Clear all nvram entries inside flash.\n"
           );
    return OK;
}


int32_t main(int32_t argc, char ** argv)
{
#if 0
    int32_t i = 0;

    for(i=0; i<argc; i++)
        DEBUG("argv[%d]:%s\n", i, argv[i]);
#endif

    if (argc < 2)
        return usage(OK);

    if (nvram_filecache(NVRAM_CACHE_FILE) != OK)
        return NG;

    if (nvram_parse(NULL, NVRAM_CACHE_FILE) != OK)
        return NG;

    if (0 == strcasecmp(argv[1], "commit"))
    {
        return nvram_commit(1);
    }
    else if (0 == strcasecmp(argv[1], "set"))
    {
        if (argc == 5)
            return nvram_set(argv[2], argv[3], argv[4]);
        else
            return usage(NG);
    }
    else if (0 == strcasecmp(argv[1], "get"))
    {
        if (argc < 4)
            return usage(NG);
        else
            nvram_get(argv[2], argv[3]);
    }
    else if (0 == strcasecmp(argv[1], "del"))
    {
        if (argc < 4)
            return usage(NG);
        else
            nvram_del(argv[2], argv[3]);
    }
    else if (0 == strcasecmp(argv[1], "show"))
    {
        if (argc < 3)
            return nvram_show(NULL);
        else
            return nvram_show(argv[2]);
    }
    else if (0 == strcasecmp(argv[1], "clear"))
    {
        return nvram_clear(argv[2]);
    }
    else if (0 == strcasecmp(argv[1], "layout"))
    {
        return nvram_layout();
    }
    else if (0 == strcasecmp(argv[1], "loadfile"))
    {
        if (argc < 4)
            return usage(NG);
        else
            return nvram_loadfile(argv[2], argv[3]);
    }
    else
        return usage(NG);

    return OK;
}


