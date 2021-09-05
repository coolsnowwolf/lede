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
#ifndef __APPFILTER_NETLINK_H__
#define __APPFILTER_NETLINK_H__
#define DEFAULT_USR_NL_PID 999
#define OAF_NETLINK_ID 29
#define MAX_OAF_NETLINK_MSG_LEN 1024

struct af_msg_hdr
{
    int magic;
    int len;
};

enum E_MSG_TYPE
{
    AF_MSG_INIT,
    AF_MSG_MAX
};

typedef struct af_msg
{
    int action;
    void *data;
} af_msg_t;
int appfilter_nl_init(void);
void appfilter_nl_handler(struct uloop_fd *u, unsigned int ev);
int send_msg_to_kernel(int fd, void *msg, int len);
#endif