#ifndef __QUERY_PCIE_MODE__
#define __QUERY_PCIE_MODE__

extern int speed_arr[14];
extern int name_arr[14];

extern int get_pcie_mode_debug;

extern int get_private_gateway_debug;

extern int xset1(int fd, struct termios *tio, const char *device);

// set raw tty mode
extern void xget1(int fd, struct termios *t, struct termios *oldt);
extern int get_pcie_mode();
 
#endif
