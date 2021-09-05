/* -*- Mode: C; tab-width: 4; -*- */
/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：myini.h
* 摘	要：读取ini文件+写入ini文件
* 作	者：HustMoon@BYHH
* 修	改：2009.10.8
*/
#ifndef HUSTMOON_MYINI_H
#define HUSTMOON_MYINI_H

#include <stdlib.h>	/* for free() */

#ifdef __cplusplus
extern "C"
{
#endif

char *loadFile(const char *fileName);	/* 读取文件 */
int getString(const char *buf, const char *section, const char *key,
	const char *defaultValue, char *value, unsigned long size);	/* 读取字符串 */
int getInt(const char *buf, const char *section, const char *key, int defaultValue);	/* 读取整数 */
void setString(char **buf, const char *section, const char *key, const char *value);	/* 设置字符串，value=NULL则删除key，key=NULL则删除section */
void setInt(char **buf, const char *section, const char *key, int value);	/* 设置整数 */
int saveFile(const char *buf, const char *fileName);	/* 写入文件 */

#ifdef __cplusplus
};
#endif

#endif
