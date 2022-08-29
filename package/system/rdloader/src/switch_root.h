//
// Created by juno on 2022/1/12.
//

#ifndef INIT_SWITCH_ROOT_H
#define INIT_SWITCH_ROOT_H

#define DOT_OR_DOTDOT(s) ((s)[0] == '.' && (!(s)[1] || ((s)[1] == '.' && !(s)[2])))
void switch_root(const char *newroot, const char *prog, char *const argv[]);


#endif //INIT_SWITCH_ROOT_H
