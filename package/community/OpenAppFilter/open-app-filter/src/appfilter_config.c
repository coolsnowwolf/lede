/*
Copyright (C) 2020 Derry <destan19@126.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "appfilter_config.h"
#include <uci.h>

app_name_info_t app_name_table[MAX_SUPPORT_APP_NUM];
int g_app_count = 0;
int g_cur_class_num = 0;
char CLASS_NAME_TABLE[MAX_APP_TYPE][MAX_CLASS_NAME_LEN];

const char *config_path = "./config";
static struct uci_context *uci_ctx = NULL;
static struct uci_package *uci_appfilter;
//
static struct uci_package *
config_init_package(const char *config)
{
    struct uci_context *ctx = uci_ctx;
    struct uci_package *p = NULL;

    if (!ctx)
    {
        ctx = uci_alloc_context();
        uci_ctx = ctx;
        ctx->flags &= ~UCI_FLAG_STRICT;
        //if (config_path)
        //	uci_set_confdir(ctx, config_path);
    }
    else
    {
        p = uci_lookup_package(ctx, config);
        if (p)
            uci_unload(ctx, p);
    }

    if (uci_load(ctx, config, &p))
        return NULL;

    return p;
}
char *get_app_name_by_id(int id)
{
    int i;
    for (i = 0; i < g_app_count; i++)
    {
        if (id == app_name_table[i].id)
            return app_name_table[i].name;
    }
    return "";
}

void init_app_name_table(void)
{
    int count = 0;
    char line_buf[2048] = {0};

    FILE *fp = fopen("/etc/appfilter/feature.cfg", "r");
    if (!fp)
    {
        printf("open file failed\n");
        return;
    }

    while (fgets(line_buf, sizeof(line_buf), fp))
    {
        if (strstr(line_buf, "#"))
            continue;
        if (strlen(line_buf) < 10)
            continue;
        if (!strstr(line_buf, ":"))
            continue;
        char *pos1 = strstr(line_buf, ":");
        char app_info_buf[128] = {0};
        int app_id;
        char app_name[64] = {0};
        memset(app_name, 0x0, sizeof(app_name));
        strncpy(app_info_buf, line_buf, pos1 - line_buf);
        sscanf(app_info_buf, "%d %s", &app_id, app_name);
        app_name_table[g_app_count].id = app_id;
        strcpy(app_name_table[g_app_count].name, app_name);
        g_app_count++;
    }
    fclose(fp);
}

void init_app_class_name_table(void)
{
    char line_buf[2048] = {0};
    int class_id;
    char class_name[64] = {0};
    FILE *fp = fopen("/etc/appfilter/app_class.txt", "r");
    if (!fp)
    {
        printf("open file failed\n");
        return;
    }
    while (fgets(line_buf, sizeof(line_buf), fp))
    {
        sscanf(line_buf, "%d %s", &class_id, class_name);
        strcpy(CLASS_NAME_TABLE[class_id - 1], class_name);
        g_cur_class_num++;
    }
    fclose(fp);
}
//00:00 9:1
int check_time_valid(char *t)
{
    if (!t)
        return 0;
    if (strlen(t) < 3 || strlen(t) > 5 || (!strstr(t, ":")))
        return 0;
    else
        return 1;
}

void dump_af_time(af_ctl_time_t *t)
{
    int i;
    printf("---------dump af time-------------\n");
    printf("%d:%d ---->%d:%d\n", t->start.hour, t->start.min,
           t->end.hour, t->end.min);
    for (i = 0; i < 7; i++)
    {
        printf("%d ", t->days[i]);
    }
    printf("\n");
}

af_ctl_time_t *load_appfilter_ctl_time_config(void)
{
    int ret = 0;
    af_ctl_time_t *t = NULL;
    appfilter_config_alloc();

    struct uci_section *time_sec = uci_lookup_section(uci_ctx, uci_appfilter, "time");
    if (!time_sec)
    {
        printf("get time section failed\n");
        appfilter_config_free();
        return NULL;
    }
    t = malloc(sizeof(af_ctl_time_t));
    if (!t)
    {
        appfilter_config_free();
        return NULL;
    }
    memset(t, 0x0, sizeof(af_ctl_time_t));
    char *start_time_str = uci_lookup_option_string(uci_ctx, time_sec, "start_time");

    if (check_time_valid(start_time_str))
        sscanf(start_time_str, "%d:%d", &t->start.hour, &t->start.min);
    else
    {
        printf("start time check failed\n");
        free(t);
        t = NULL;
        goto EXIT1;
    }
    char *end_time_str = uci_lookup_option_string(uci_ctx, time_sec, "end_time");

    if (check_time_valid(end_time_str))
        sscanf(end_time_str, "%d:%d", &t->end.hour, &t->end.min);
    else
    {
        printf("end time check failed\n");
        free(t);
        t = NULL;
        goto EXIT2;
    }

    char *days_str = uci_lookup_option_string(uci_ctx, time_sec, "days");
    if (!days_str)
    {
        printf("not found days\n");
        goto EXIT2;
    }
    char *p = strtok(days_str, " ");
    if (!p)
        goto EXIT3;
    do
    {
        int day = atoi(p);
        if (day >= 0 && day <= 6)
            t->days[day] = 1;
        else
            ret = 0;
    } while (p = strtok(NULL, " "));

EXIT3:
    if (days_str)
        free(days_str);
EXIT2:
    if (end_time_str)
        free(end_time_str);
EXIT1:
    if (start_time_str)
        free(start_time_str);
    appfilter_config_free();
    return t;
}

int config_get_appfilter_enable(void)
{
    int ret = 0;
    int enable = 0;
    af_ctl_time_t *t = NULL;

    appfilter_config_alloc();
    struct uci_section *global_sec = uci_lookup_section(uci_ctx, uci_appfilter, "global");
    if (!global_sec)
    {
        printf("get global section failed\n");
        appfilter_config_free();
        return NULL;
    }

    char *enable_opt = uci_lookup_option_string(uci_ctx, global_sec, "enable");
    if (!enable_opt)
    {
        printf("enable option not found.\n");
        appfilter_config_free();
        return 0;
    }
    enable = atoi(enable_opt);
    free(enable_opt);
    appfilter_config_free();
    return enable;
}

int appfilter_config_alloc(void)
{
    char *err;
    uci_appfilter = config_init_package("appfilter");
    if (!uci_appfilter)
    {
        uci_get_errorstr(uci_ctx, &err, NULL);
        printf("Failed to load appfilter config (%s)\n", err);
        free(err);
        return -1;
    }

    return 0;
}

int appfilter_config_free(void)
{
    if (uci_appfilter)
    {
        uci_unload(uci_ctx, uci_appfilter);
        uci_appfilter = NULL;
    }
    if (uci_ctx)
    {
        uci_free_context(uci_ctx);
        uci_ctx = NULL;
    }
}
