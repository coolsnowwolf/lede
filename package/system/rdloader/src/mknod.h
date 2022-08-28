//
// Created by juno on 2022/1/7.
//

#ifndef INIT_MKNOD_H
#define INIT_MKNOD_H

#define ALIGN1 __attribute__((aligned(1)))
int do_mknod(const char *mod, const char *name, char type, int maj, int min);

#endif //INIT_MKNOD_H
