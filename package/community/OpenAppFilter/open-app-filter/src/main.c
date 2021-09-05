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
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <libubus.h>
#include "appfilter_user.h"
#include "appfilter_netlink.h"
#include "appfilter_ubus.h"
#include "appfilter_config.h"
#include <time.h>
void check_appfilter_enable(void)
{
    int enable = 1;
    struct tm *t;
	af_ctl_time_t *af_t = NULL;
    time_t tt;
    time(&tt);
    enable = config_get_appfilter_enable();
    if (0 == enable)
        goto EXIT;
    af_t = load_appfilter_ctl_time_config();
    if (!af_t)
    {
        enable = 0;
        goto EXIT;
    }

    t = localtime(&tt);
    if (af_t->days[t->tm_wday] != 1)
    {
        enable = 0;
        goto EXIT;
    }
    if (af_t->start.hour <= af_t->end.hour)
    {
        int cur_mins = t->tm_hour * 60 + t->tm_min;
        if ((af_t->start.hour * 60 + af_t->start.min > cur_mins) || (cur_mins > af_t->end.hour * 60 + af_t->end.min))
        {
            enable = 0;
        }
    }
    else
        enable = 0;
EXIT:
    if (enable)
    {
        system("echo 1 >/proc/sys/oaf/enable ");
    }
    else
        system("echo 0 >/proc/sys/oaf/enable ");
	if (af_t)
   		free(af_t);
}

void dev_list_timeout_handler(struct uloop_timeout *t)
{
    dump_dev_list();
    
    check_dev_visit_info_expire();
    flush_expire_visit_info();
    //dump_dev_visit_list();
    check_appfilter_enable();
    //todo: dev list expire
    uloop_timeout_set(t, 10000);
}

struct uloop_timeout dev_tm = {
    .cb = dev_list_timeout_handler};

static struct uloop_fd appfilter_nl_fd = {
    .cb = appfilter_nl_handler,
};

int main(int argc, char **argv)
{
    int ret = 0;
    uloop_init();
    printf("init appfilter\n");
    init_dev_node_htable();
    init_app_name_table();
    init_app_class_name_table();
    if (appfilter_ubus_init() < 0)
    {
        fprintf(stderr, "Failed to connect to ubus\n");
        return 1;
    }

    appfilter_nl_fd.fd = appfilter_nl_init();
    uloop_fd_add(&appfilter_nl_fd, ULOOP_READ);
    af_msg_t msg;
    msg.action = AF_MSG_INIT;

    send_msg_to_kernel(appfilter_nl_fd.fd, (void *)&msg, sizeof(msg));
    uloop_timeout_set(&dev_tm, 5000);
    uloop_timeout_add(&dev_tm);
    uloop_run();
    uloop_done();
    return 0;
}
