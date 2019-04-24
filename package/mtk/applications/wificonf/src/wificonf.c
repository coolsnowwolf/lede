/****************************************************************************
 *
 * Copyright (c) Hua Shao <nossiac@163.com>
 *
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <memory.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#undef OK
#undef NG
#undef SHDBG
#undef IS_SPACE
#undef IS_NEWLINE
#undef ASSERT

#define OK (0)

#define NG (-1)

#define SHDBG(...)  if(!__quiet) fprintf(stderr, __VA_ARGS__)

#define IS_SPACE(c) ((c) == ' ' || (c) == '\t' || (c) == '\r' || (c) == '\n')

#define IS_NEWLINE(c) ((c) == '\r' || (c) == '\n')

#define ASSERT(_cond_) \
    do { \
        if (!(_cond_)) {\
            SHDBG("assert failure: %s, %s, %s L%d.\n", #_cond_, errno != 0?strerror(errno):"", __FUNCTION__, __LINE__); \
            exit(-1); \
        } \
    } while(0);

int __quiet = 0;
int __base64 = 0;
char * __profile = NULL;
char * EMPTY = "";


int base64 = 0;
static const unsigned char base64_table[65] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

unsigned char * base64_encode(const unsigned char *src, size_t len,
                              size_t *out_len)
{
    unsigned char *out, *pos;
    const unsigned char *end, *in;
    size_t olen;
    int line_len;

    olen = len * 4 / 3 + 4; /* 3-byte blocks to 4-byte */
    olen += olen / 72; /* line feeds */
    olen++; /* nul termination */
    if (olen < len)
        return NULL; /* integer overflow */
    out = malloc(olen+1); /* null terminated */
    if (out == NULL)
        return NULL;
    out[olen] = 0;

    end = src + len;
    in = src;
    pos = out;
    line_len = 0;
    while (end - in >= 3)
    {
        *pos++ = base64_table[in[0] >> 2];
        *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
        *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
        *pos++ = base64_table[in[2] & 0x3f];
        in += 3;
        line_len += 4;
        if (line_len >= 72)
        {
            *pos++ = '\n';
            line_len = 0;
        }
    }

    if (end - in)
    {
        *pos++ = base64_table[in[0] >> 2];
        if (end - in == 1)
        {
            *pos++ = base64_table[(in[0] & 0x03) << 4];
            *pos++ = '=';
        }
        else
        {
            *pos++ = base64_table[((in[0] & 0x03) << 4) |
                                  (in[1] >> 4)];
            *pos++ = base64_table[(in[1] & 0x0f) << 2];
        }
        *pos++ = '=';
        line_len += 4;
    }

    if (line_len)
        *pos++ = '\n';

    *pos = '\0';
    if (out_len)
        *out_len = pos - out;
    return out;
}


unsigned char * base64_decode(const unsigned char *src, size_t len,
                              size_t *out_len)
{
    unsigned char dtable[256], *out, *pos, block[4], tmp;
    size_t i, count, olen;
    int pad = 0;

    memset(dtable, 0x80, 256);
    for (i = 0; i < sizeof(base64_table) - 1; i++)
        dtable[base64_table[i]] = (unsigned char) i;
    dtable['='] = 0;

    count = 0;
    for (i = 0; i < len; i++)
    {
        if (dtable[src[i]] != 0x80)
            count++;
    }

    if (count == 0 || count % 4)
        return NULL;

    olen = count / 4 * 3;
    pos = out = malloc(olen+1);
    if (out == NULL)
        return NULL;
    pos[olen] = 0;

    count = 0;
    for (i = 0; i < len; i++)
    {
        tmp = dtable[src[i]];
        if (tmp == 0x80)
            continue;

        if (src[i] == '=')
            pad++;
        block[count] = tmp;
        count++;
        if (count == 4)
        {
            *pos++ = (block[0] << 2) | (block[1] >> 4);
            *pos++ = (block[1] << 4) | (block[2] >> 2);
            *pos++ = (block[2] << 6) | block[3];
            count = 0;
            if (pad)
            {
                if (pad == 1)
                    pos--;
                else if (pad == 2)
                    pos -= 2;
                else
                {
                    /* Invalid padding */
                    free(out);
                    return NULL;
                }
                break;
            }
        }
    }

    if (out_len)
        *out_len = pos - out;
    return out;
}


char * __get_token(char * str, int idx)
{
    int i = 0;
    char * p = NULL;
    char * q = NULL;
    char * tmp = strdup(str);
    char * new = EMPTY;

    do
    {
        p = strchr(tmp, '=');
        if (!p) break;
        p++;
        i = 0;
        while(i<idx)
        {
            if (*p == 0) break;
            if (*p == ';') i++;
            p++;
        }
        if (i == idx)
        {
            q = p;
            while(*q != ';' && *q != 0) q++;
            *q = 0;
            new = strdup(p);
        }
    }
    while(0);

    free(tmp);
    return new;
}


char * __set_token(char * str, int idx, char * value)
{
    int i = 0;
    char * p = NULL;
    char * q = NULL;
    char * tmp = strdup(str);
    char * new = calloc(1, strlen(str)+strlen(value));

    do
    {
        p = strchr(tmp, '=');
        if (!p) break;
        p++;

        memcpy(new+strlen(new), tmp, p-tmp);
        i = 0;
        do
        {
            q = p;
            while (*q != ';' && *q != 0) q++;
            if (i == idx)
                strncat(new, value, strlen(value));
            else
                strncat(new, p, q-p);
            strncat(new, q, 1); /* now q == 0 or ';' */
            i++;
            p = q + 1;
        }
        while(p <= (tmp + strlen(tmp)));

        /* every thing is fine */
        free(tmp);
        return new;

    }
    while(0);

    /* you got here only if something was wrong! */
    free(tmp);
    return str;
}


/* return it's line number, if not found, return -1 */
int __locate_key(FILE * fp, char * key)
{
    int offset = -1;
    char buffer[1024] = {0};
    char * p = NULL;
    char * q = NULL;

    ASSERT(fp);
    ASSERT(key);

    fseek(fp, 0, SEEK_SET);
    do
    {
        offset = ftell(fp); /* record current pos */
        memset(buffer, 0, sizeof(buffer));
        p = fgets(buffer, sizeof(buffer), fp);
        if(!p) break;
        q = strstr(buffer, "=");
        if(!q) continue;
        *q = 0; /* cut off the part from '=' */
        while(IS_SPACE(*p)) p++; /* trim leading spaces */
        if(*p == 0) continue; /* skip empty line */
        if(*p == '#') continue; /* skip comment line */
        if (0 == strcmp(key, p))
            break;
    }
    while(1);
    fseek(fp, offset, SEEK_SET);

    return offset;
}

char * __get_profile(char * alias)
{
    int ret;
    char * profile = NULL;
    char * link = alias? alias : "/tmp/.wificonf/current";
    struct stat sb;

    if (__profile) return __profile;

    if (lstat(link, &sb) == -1)
    {
        SHDBG("use \"wificonf use <profile>\" first\n");
        exit(-1);
    }

    profile = (char *)malloc(sb.st_size + 1);
    ASSERT(profile);

    ret = readlink(link, profile, sb.st_size+1);
    if (ret < 0)
    {
        SHDBG("use \"wificonf use <profile>\" first\n");
        return NULL;
    }
    ASSERT(ret <= sb.st_size+1);
    profile[sb.st_size] = 0;


    return profile;
}


int conf_get(char * key)
{
    int offset = 0;
    FILE * fp = NULL;
    char buffer[1024] = {0};
    char * p = NULL;
    char * q = NULL;
    char * profile = NULL;

    profile = __get_profile(NULL);
    ASSERT(profile);

    fp = fopen(profile, "rb");
    ASSERT(fp);

    offset = __locate_key(fp, key);
    ASSERT(offset >= 0);

    memset(buffer, 0, sizeof(buffer));
    fseek(fp, offset, SEEK_SET);
    p = fgets(buffer, sizeof(buffer)-1, fp);
    if(!p) return OK;
    p = strstr(buffer, "=");
    if(!p) return OK;
    p++;

    while(IS_SPACE(*p)) p++; /* trim head spaces */
    q = p;
    while(*q != 0) q++;
    q--;/* q points to the last character */
    while(q > p && IS_SPACE(*q))
    {
        *q = 0;
        q--;
    };  /* trim tail spaces */
    printf("%s\n", p);

    if(fp) fclose(fp);
    return OK;
}


int conf_set(char * key, char * value)
{
    int ret = 0;
    FILE * fp = NULL;
    char * buffer = NULL;
    int nbytes = 0;
    char * profile = NULL;
    char * p = NULL;
    struct stat sb;
    size_t len = strlen(value);

    if (__base64)
    {
        value = (char *)base64_decode((unsigned char *)value, strlen(value), &len);
        ASSERT(value);
    }

    profile = __get_profile(NULL);
    ASSERT(profile);

    fp = fopen(profile, "rb");
    ASSERT(fp);

    nbytes = __locate_key(fp, key);
    ASSERT(nbytes >= 0);

    ASSERT(stat(profile, &sb) == 0);
    buffer = (char *)malloc(sb.st_size + 1);
    ASSERT(buffer);
    buffer[sb.st_size] = 0;

    fseek(fp, 0, SEEK_SET);
    ret = fread(buffer, 1, sb.st_size, fp);
    ASSERT(ret == sb.st_size);
    fclose(fp);

    fp = fopen(profile, "wb");
    ASSERT(fp);
    ret = fwrite(buffer, 1, nbytes, fp);
    ASSERT(ret == nbytes);

    fprintf(fp, "%s=", key);
    for (ret=0; ret<len; ret++)
        fprintf(fp, "%c", value[ret]);
    fprintf(fp, "\n");

    p = buffer + nbytes;
    while('\n' != *p && 0 != *p) p++;
    if (0 != *p) p++; /* now p is the next line */

    nbytes = buffer + sb.st_size - p; /* remain data */
    ret = fwrite(p, 1, nbytes, fp);
    ASSERT(ret == nbytes);

    if(fp) fclose(fp);
    return OK;
}



int conf_get_token(char * key, int idx)
{
    int offset = 0;
    FILE * fp = NULL;
    char buffer[1024] = {0};
    char * p = NULL;
    char * profile = NULL;

    ASSERT(key);
    ASSERT(idx >= 0);

    profile = __get_profile(NULL);
    ASSERT(profile);

    fp = fopen(profile, "rb");
    ASSERT(fp);

    offset = __locate_key(fp, key);
    ASSERT(offset>=0);

    fseek(fp, offset, SEEK_SET);
    memset(buffer, 0, sizeof(buffer));
    p = fgets(buffer, sizeof(buffer)-1, fp);
    if(!p) return OK;

    printf("%s\n", __get_token(buffer, idx));

    if(fp) fclose(fp);
    return OK;
}


int conf_set_token(char * key, int idx, char * value)
{
    int ret = 0;
    int nbytes = 0;
    FILE * fp = NULL;
    char * buffer = NULL;
    char * p = NULL;
    char * q = NULL;
    char * profile = NULL;
    struct stat sb;
    size_t len = strlen(value);

    ASSERT(key);
    ASSERT(idx >= 0 && idx < 32);

    if (__base64)
    {
        value = (char *)base64_decode((unsigned char *)value, strlen(value), &len);
        ASSERT(value);
    }

    profile = __get_profile(NULL);
    ASSERT(profile);

    ASSERT(stat(profile, &sb) == 0);
    buffer = (char *)malloc(sb.st_size + 1);
    ASSERT(buffer);
    buffer[sb.st_size] = 0;

    fp = fopen(profile, "rb");
    ASSERT(fp);

    ret = fread(buffer, 1, sb.st_size, fp);
    ASSERT(ret == sb.st_size);

    nbytes = __locate_key(fp, key);
    ASSERT(nbytes>=0);

    fclose(fp);

    /* write the first part. */
    fp = fopen(profile, "wb");
    ASSERT(fp);
    ret = fwrite(buffer, 1, nbytes, fp);
    ASSERT(ret == nbytes);

    /* write new key-value. */
    p = buffer+nbytes;
    q = p;
    while(*q != '\n' && q != 0) q++;
    *q = 0;
    q++;
    p = __set_token(p, idx, value);
    fprintf(fp, "%s\n", p);

    /* write rest data */
    nbytes = buffer + sb.st_size - q; /* remain data */
    ret = fwrite(q, 1, nbytes, fp);
    ASSERT(ret == nbytes);

    if(fp) fclose(fp);
    return OK;
}



int usage(void)
{
    printf("Usage:\n"
           "    wificonf [option] use <profile>\n"
           "    wificonf [option] get <key>\n"
           "    wificonf [option] get <key> [idx]\n"
           "    wificonf [option] set <key> <value>\n"
           "    wificonf [option] set <key> <idx> <value>\n"
           "\n"
           "Options:\n"
           "    -e   use base64 encoded <value>.\n"
           "    -f   specify the profile.\n"
           "    -q   be quiet, no error will be reported.\n"
           "\n\n"
           "Examples:\n"
           "  1. set the default profile you want to access\n"
           "    wificonf use /etc/wireless/mt7603.dat\n"
           "    wificonf get SSID1\n"
           "  2. change the default profile you want to access\n"
           "    wificonf use /etc/wireless/mt7615.dat\n"
           "    wificonf set SSID1 MasterNet\n"
           "    wificonf set SSID2 GuestNet\n"
           "    wificonf set BssidNum 2\n"
           "  3. also you can specify the profile with each command\n"
           "    wificonf -f /etc/wireless/mt7603.dat set SSID1 WiFi-7603\n"
           "  4. access a key that has a list of values\n"
           "    wificonf set TestKey \"1;2;3;4\"\n"
           "    wificonf get TestKey                --> 1;2;3;4\n"
           "    wificonf set TestKey 0 99\n"
           "    wificonf get TestKey                --> 99;2;3;4\n"
           );

    return OK;
}



int use_profile(char * profile, char * alias)
{
    char softlink[512];
    int ret;

    struct stat st;

    if (stat("/tmp/.wificonf", &st) < 0)
    {
        ret = mkdir("/tmp/.wificonf", 0700);
        ASSERT(ret == 0);
    }

    snprintf(softlink, sizeof(softlink), "/tmp/.wificonf/current");
    if (lstat(softlink, &st) == 0)
    {
        ret = unlink(softlink);
        ASSERT(ret == 0);
    }

    ret = symlink(profile, softlink);
    ASSERT(ret == 0);

    if (!alias) return OK;

    snprintf(softlink, sizeof(softlink), "/tmp/.wificonf/%s", alias);

    if (lstat(softlink, &st) == 0)
    {
        SHDBG("you are overwritting %s to %s\n", softlink, profile);
        ret = unlink(softlink);
        ASSERT(ret == 0);
    }
    ret = symlink(profile, softlink);
    ASSERT(ret == 0);

    return OK;
}


int main(int argn, char ** args)
{
    int argc = 1;
    int c;
    int i;
    char * argv[8];

    argv[0] = strdup(args[0]);

    while ((c = getopt (argn, args, "ef:qh")) != -1)
    {
        switch (c)
        {
            case 'f':
                __profile = optarg;
                break;
            case 'q':
                __quiet = 1;
                break;
            case 'e':
                __base64 = 1;
                break;
            case 'h':
            default:
                return usage();
        }
    }

    for (i = optind; i < argn; i++)
        argv[argc++] = args[i];

    if (argc < 2)
        return usage();

    if (0 == strcmp(argv[1], "use"))
    {
        if (argc == 3)
            return use_profile(argv[2], NULL);
        else if (argc == 4)
            return use_profile(argv[2], argv[3]);
        else
            return usage();
    }

    if (0 == strcmp(argv[1], "get"))
    {
        /* case 1: wificonf get <key> */
        /* case 2: wificonf get <key> <idx> */
        if (argc == 3)
            return conf_get(argv[2]);
        else if (argc == 4)
            return conf_get_token(argv[2], atoi(argv[3]));
        else
            return usage();
    }

    if (0 == strcmp(argv[1], "set"))
    {
        /* case 1: wificonf set <key> <value> */
        /* case 2: wificonf set <key> <idx> <value> */
        if (argc == 4)
            return conf_set(argv[2], argv[3]);
        else if (argc == 5)
            return conf_set_token(argv[2], atoi(argv[3]), argv[4]);
        else
            return usage();
    }

    return OK;
}




