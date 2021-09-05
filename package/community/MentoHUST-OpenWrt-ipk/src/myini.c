/* -*- Mode: C; tab-width: 4; -*- */
/*
* Copyright (C) 2009, HustMoon Studio
*
* 文件名称：myini.c
* 摘	要：读取ini文件+写入ini文件
* 作	者：HustMoon@BYHH
* 修	改：2009.10.8
*/
#include "myini.h"
#include <stdio.h>
#include <string.h>

#define NOT_COMMENT(c)	(c!=';' && c!='#')	/* 不是注释行 */

#ifndef strnicmp
#define strnicmp strncasecmp
#endif

static void getLine(const char *buf, int inStart, int *lineStart, int *lineEnd);
static int findKey(const char *buf, const char *section, const char *key,
	int *sectionStart, int *valueStart, unsigned long *valueSize);
static int getSection(const char *buf, int inStart);

char *loadFile(const char *fileName)
{
	FILE *fp = NULL;
	long size = 0;
	char *buf = NULL;
	if ((fp=fopen(fileName, "rb")) == NULL)
		return NULL;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	buf = (char *)malloc(size+1);
	buf[size] = '\0';
	if (fread(buf, size, 1, fp) < 1)
	{
		free(buf);
		buf = NULL;
	}
	fclose(fp);
	return buf;
}

static void getLine(const char *buf, int inStart, int *lineStart, int *lineEnd)
{
	int start, end;
	for (start=inStart; buf[start]==' ' || buf[start]=='\t' || buf[start]=='\r' || buf[start]=='\n'; start++);
	for (end=start; buf[end]!='\r' && buf[end]!='\n' && buf[end]!='\0'; end++);
	*lineStart = start;
	*lineEnd = end;
}

static int findKey(const char *buf, const char *section, const char *key,
	int *sectionStart, int *valueStart, unsigned long *valueSize)
{
	int lineStart, lineEnd, i;
	for (*sectionStart=-1, lineEnd=0; buf[lineEnd]!='\0'; )
	{
		getLine(buf, lineEnd, &lineStart, &lineEnd);
		if (buf[lineStart] == '[')
		{
			for (i=++lineStart; i<lineEnd && buf[i]!=']'; i++);
			if (i<lineEnd && strnicmp(buf+lineStart, section, i-lineStart)==0)	/* 找到Section？ */
			{
				*sectionStart = lineStart-1;
				if (key == NULL)
					return -1;
			}
			else if (*sectionStart != -1)	/* 找到Section但未找到Key */
				return -1;
		}
		else if (*sectionStart!=-1 && NOT_COMMENT(buf[lineStart]))	/* 找到Section且该行不是注释 */
		{
			for (i=lineStart+1; i<lineEnd && buf[i]!='='; i++);
			if (i<lineEnd && strnicmp(buf+lineStart, key, i-lineStart)==0)	/* 找到Key？ */
			{
				*valueStart = i + 1;
				*valueSize = lineEnd - *valueStart;
				return 0;
			}
		}
	}
	return -1;
}

int getString(const char *buf, const char *section, const char *key,
	const char *defaultValue, char *value, unsigned long size)
{
	int sectionStart, valueStart;
	unsigned long valueSize;

	if (findKey(buf, section, key, &sectionStart, &valueStart, &valueSize)!=0 || valueSize==0)	/* 未找到？ */
	{
		strncpy(value, defaultValue, size);
		return -1;
	}
	if (size-1 < valueSize)		/* 找到但太长？ */
		valueSize = size - 1;
	memset(value, 0, size);
	strncpy(value, buf+valueStart, valueSize);
	return 0;
}

int getInt(const char *buf, const char *section, const char *key, int defaultValue)
{
	char value[21] = {0};
	getString(buf, section, key, "", value, sizeof(value));
	if (value[0] == '\0')	/* 找不到或找到但为空？ */
		return defaultValue;
	return atoi(value);
}

void setString(char **buf, const char *section, const char *key, const char *value)
{
	int sectionStart, valueStart;
	unsigned long valueSize;
	char *newBuf = NULL;

	if (findKey(*buf, section, key, &sectionStart, &valueStart, &valueSize) == 0)	/* 找到key */
	{
		if (value == NULL)	/* 删除key? */
			memmove(*buf+valueStart-strlen(key)-1, *buf+valueStart+valueSize, 
				strlen(*buf)+1-valueStart-valueSize);
		else	/* 修改key */
		{
			newBuf = (char *)malloc(strlen(*buf)-valueSize+strlen(value)+1);
			memcpy(newBuf, *buf, valueStart);
			strcpy(newBuf+valueStart, value);
			strcpy(newBuf+valueStart+strlen(value), *buf+valueStart+valueSize);
			free(*buf);
			*buf = newBuf;
		}
	}
	else if (sectionStart != -1)	/* 找到section，找不到key */
	{
		if (key == NULL)	/* 删除section? */
		{
			valueStart = getSection(*buf, sectionStart+3);
			if (valueStart <= sectionStart)	/* 后面没有section */
				(*buf)[sectionStart] = '\0';
			else
				memmove(*buf+sectionStart, *buf+valueStart, strlen(*buf)+1-valueStart);
		}
		else if (value != NULL)	/* 不是要删除key */
		{
			newBuf = (char *)malloc(strlen(*buf)+strlen(key)+strlen(value)+4);
			valueSize = sectionStart+strlen(section)+2;
			memcpy(newBuf, *buf, valueSize);
			sprintf(newBuf+valueSize, "\n%s=%s", key, value);
			strcpy(newBuf+strlen(newBuf), *buf+valueSize);
			free(*buf);
			*buf = newBuf;
		}
	}
	else	/* 找不到section? */
	{
		if (key!=NULL && value!=NULL)
		{
			newBuf = (char *)malloc(strlen(*buf)+strlen(section)+strlen(key)+strlen(value)+8);
			strcpy(newBuf, *buf);
			sprintf(newBuf+strlen(newBuf), "\n[%s]\n%s=%s", section, key, value);
			free(*buf);
			*buf = newBuf;
		}
	}
}

static int getSection(const char *buf, int inStart)
{
	int lineStart, lineEnd, i;
	for (lineEnd=inStart; buf[lineEnd]!='\0'; )
	{
		getLine(buf, lineEnd, &lineStart, &lineEnd);
		if (buf[lineStart] == '[')
		{
			for (i=lineStart+1; i<lineEnd && buf[i]!=']'; i++);
			if (i < lineEnd)
				return lineStart;
		}
	}
	return -1;
}

void setInt(char **buf, const char *section, const char *key, int value)
{
	char svalue[21];
	sprintf(svalue, "%d", value);
	setString(buf, section, key, svalue);
}

int saveFile(const char *buf, const char *fileName)
{
	FILE *fp;
	int result;
	
	if ((fp=fopen(fileName, "wb")) == NULL)
		return -1;
	result = fwrite(buf, strlen(buf), 1, fp)<1 ? -1 : 0;
	fclose(fp);
	return result;
}
