/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：myfunc.h
* 摘	要：认证相关算法及方法
* 作	者：HustMoon@BYHH
*/
#ifndef HUSTMOON_MYFUNC_H
#define HUSTMOON_MYFUNC_H

#include <sys/types.h>

char *formatIP(u_int32_t ip);	/* 格式化IP */
char *formatHex(const void *buf, int length);	/* 格式化成十六进制形式 */
void newBuffer();	/* 检测数据文件有效性并分配内存 */
int fillHeader();	/* 填充网络地址及校验值部分 */
void fillStartPacket();	/* 填充Start包 */
void fillMd5Packet(const u_char *md5Seed);	/* 填充Md5包 */
void fillEchoPacket(u_char *buf);	/* 填充Echo包 */
void getEchoKey(const u_char *capBuf);	/* 获取EchoKey */
u_char *checkPass(u_char id, const u_char *md5Seed, int seedLen);	/* 计算密码的md5 */
void fillCernetAddr(u_char *buf);	/* 填充赛尔网络地址 */
int isOnline();	/* ping主机判断是否掉线 */

#endif

