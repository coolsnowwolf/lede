/******************************************************************
 * $File:   nvram.h
 *
 * $Author: Hua Shao
 * $Date:   Oct, 2014
 *
 ******************************************************************/

#ifndef NVRAM_H
#define NVRAM_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef NVRAM_CACHE_FILE
#define NVRAM_CACHE_FILE "/tmp/.nvramcache"
#endif

#ifndef NVRAM_MTD_NAME
#define NVRAM_MTD_NAME "Config"
#endif


#define OK (0)
#define NG (-1)

#if 0
#define DEBUG(...) do{ fprintf(stderr, "<nvram> ");printf(__VA_ARGS__);} while(0)
#define API() fprintf(stderr, "<nvram> api: %s.\n", __FUNCTION__)
#else
#define DEBUG(...)
#define API()
#endif
#define ASSERT(cond) \
    do { \
        if(!(cond)) \
        { \
            fprintf(stderr, "<nvram> assert [%s] fail, %s L%d\n", #cond, __FUNCTION__, __LINE__); \
            exit(-1); \
        } \
    } while(0)

#define sfree(p)  do {if(p) { free(p); p = NULL;} } while(0)

char *  nvram_get(char * section, char * key);
int32_t nvram_set(char * section, char * key, char * value);
int32_t nvram_del(char * section, char * key);
int32_t nvram_show(char * section);
int32_t nvram_layout(void);
int32_t nvram_loadfile(char * section, char * defile);
int32_t nvram_filecache(char * defile);
int32_t nvram_clear(char * section);
int32_t nvram_parse(char * section, char * cachefile);
int32_t nvram_commit(char flash);

#endif /* NVRAM_H */

