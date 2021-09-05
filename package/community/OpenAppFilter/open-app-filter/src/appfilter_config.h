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
#ifndef __APPFILTER_CONFIG_H__
#define __APPFILTER_CONFIG_H__
#define MAX_SUPPORT_APP_NUM 256
#define MAX_CLASS_NAME_LEN 32
#include "appfilter_user.h"
extern int g_cur_class_num;
extern int g_app_count;
extern char CLASS_NAME_TABLE[MAX_APP_TYPE][MAX_CLASS_NAME_LEN];
typedef struct af_time
{
    int hour;
    int min;
} af_time_t;
typedef struct af_ctl_time
{
    af_time_t start;
    af_time_t end;
    int days[7];
} af_ctl_time_t;

typedef struct app_name_info
{
    int id;
    char name[64];
} app_name_info_t;
void init_app_name_table(void);
void init_app_class_name_table(void);
char *get_app_name_by_id(int id);

int appfilter_config_alloc(void);

int appfilter_config_free(void);
af_ctl_time_t *load_appfilter_ctl_time_config(void);
int config_get_appfilter_enable(void);

#endif
