#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/mm.h>
#include <linux/slab.h>
//#include "regexp.h"

typedef enum{CHAR, DOT, BEGIN, END, STAR, PLUS, QUES, LIST, TYPENUM}TYPE;

typedef struct RE{
	TYPE type;
	int ch;
	char *ccl;
	int nccl;
	struct RE *next;
}RE;

int match_longest = 0;
char *match_first = NULL;


static void * getmem(size_t size)
{
	void *tmp;
	if((tmp = kmalloc(size, GFP_ATOMIC))==NULL)
	{
		printk("malloc failed");
		return NULL;
	}
	return tmp;
}

static size_t creat_list(char *str, int start, int end)
{
	size_t cnt = end - start + 1;
	for(; start <= end ;start++)
		*str++ = start;
	return (cnt > 0)?cnt:0;
}

static int in_list(char ch, RE *regexp)
{
	char *str = regexp->ccl;
	if(regexp->type != LIST)
		return 0;
	for(; *str && ch != *str; str++)
		;
	return (*str != '\0') ^ regexp->nccl;
}

static void regexp_free(RE *regexp)
{
	RE *tmp;
	for(; regexp; regexp = tmp)
	{
		tmp = regexp->next;
		kfree(regexp);
	}
}

static RE* compile(char *regexp)
{
	RE head, *tail, *tmp;
	char *pstr;
	int err_flag = 0;

	for(tail = &head; *regexp != '\0' && err_flag == 0; regexp++)
	{
		tmp = getmem(sizeof(RE));
		switch(*regexp){
			case '\\':
				regexp++;
				if(*regexp == 'd')
				{
					tmp->type = LIST;
					tmp->nccl = 0;
					tmp->ccl = getmem(11);
					creat_list(tmp->ccl, '0','9');
					tmp->ccl[11] = '\0';
				}else if(*regexp == 'D')
				{
					tmp->type = LIST;
					tmp->nccl = 1;
					tmp->ccl = getmem(11);
					creat_list(tmp->ccl, '0','9');
					tmp->ccl[11] = '\0';
				}else
				{
					tmp->type = CHAR;
					tmp->ch = *(regexp + 1);
				}
				break;
			case '.':
				tmp->type = DOT;
				break;
			case '^':
				tmp->type = BEGIN;
				tmp->ch = '^';
				break;
			case '$':
				tmp->type = END;
				tmp->ch = '$';
				break;
			case '*':
				tmp->type = STAR;
				break;
			case '+':
				tmp->type = PLUS;
				break;
			case '?':
				tmp->type = QUES;
				break;
			case '[':
				pstr = tmp->ccl = getmem(256);
				tmp->nccl = 0;
				if(*++regexp == '^')
				{
					tmp->nccl = 1;
					regexp++;
				}
				while(*regexp != '\0' && *regexp != ']')
				{
					if(*regexp != '-')
					{
						*pstr++ = *regexp++;
						continue;
					}
					if(pstr == tmp->ccl || *(regexp + 1) == ']')
					{
						err_flag = 1;
						break;
					}
					pstr += creat_list(pstr, *(regexp - 1) + 1, *(regexp + 1));
					regexp += 2;
				}
				*pstr = '\0';
				if(*regexp == '\0')
					err_flag = 1;
				tmp->type = LIST;
				break;
			default:
				tmp->type = CHAR;
				tmp->ch = *regexp;
		}

		tail->next = tmp;
		tail = tmp;
	}

	tail->next = NULL;
	if(err_flag)
	{
		regexp_free(head.next);
		return NULL;
	}
	return head.next;
}

#define MATCH_ONE(reg, text) \
   	(reg->type == DOT || in_list(*text, reg) || *text == reg->ch)
#define MATCH_ONE_P(reg, text) \
   	(in_list(*text++, reg) || *(text - 1) == reg->ch || reg->type == DOT)

static int matchhere(RE *regexp, char *text);

static int matchstar(RE *cur, RE *regexp, char *text)
{
	do{
		if(matchhere(regexp, text))
			return 1;
	}while(*text != '\0' && MATCH_ONE_P(cur, text));
	return 0;
}

static int matchstar_l(RE *cur, RE *regexp, char *text)
{
	char *t;
	for(t = text; *t != '\0' && MATCH_ONE(cur, t); t++)
		;
	do{
		if(matchhere(regexp, t))
			return 1;
	}while(t-- > text);
	return 0;
}

static int matchplus(RE *cur, RE *regexp, char *text)
{
	while(*text != '\0' && MATCH_ONE_P(cur, text))
	{
		if(matchhere(regexp, text))
			return 1;
	}
	return 0;
}

static int matchplus_l(RE *cur, RE *regexp, char *text)
{
	char *t;
	for(t = text; *t != '\0' && MATCH_ONE(cur, t); t++)
		;
	for(; t > text; t--)
	{
		if(matchhere(regexp, t))
			return 1;
	}
	return 0;
}

static int matchques(RE *cur, RE *regexp, char *text)
{
	int cnt = 1;
	char *t = text;
	if(*t != '\0' && cnt-- && MATCH_ONE(cur, t))
		t++;
	do{
		if(matchhere(regexp, t))
			return 1;
	}while(t-- > text);
	return 0;
}

static int (*matchfun[TYPENUM][2])(RE *, RE *, char *) = {
	0, 0, 0, 0, 0, 0, 0, 0,
	matchstar, matchstar_l,
	matchplus, matchplus_l,
	matchques, matchques,
};

static int matchhere(RE *regexp, char *text)
{
	if(regexp == NULL)
		return 1;
	if(regexp->type == END && regexp->next == NULL)
		return *text == '\0';
	if(regexp->next && matchfun[regexp->next->type][match_longest])
		return matchfun[regexp->next->type][match_longest](regexp, regexp->next->next, text);

	if(*text != '\0' && MATCH_ONE(regexp, text))
		return matchhere(regexp->next, text + 1);
	return 0;
}

/* 
 * return value:
 *		-1		error
 *		0		not match
 *		1		matched
 */
int regexp_match(char *reg, char *text)
{
	int ret;
	RE *regexp = compile(reg);
	if(regexp == NULL)
		return -1;

	if(regexp->type == BEGIN)
	{
		ret = matchhere(regexp->next, text);
		goto out;
	}

	do{
		if(ret = matchhere(regexp, text))
		{
			goto out;
		}
	}while(*text++ != '\0');

out:
	regexp_free(regexp);
	return ret;
}


void TEST_reg_func(char *reg, char * str, int ret)
{
	
	if (ret != regexp_match(reg, str)) {
		if (reg)
			printk("reg = %s,", reg);
		else
			printk("reg = null");
		if (str)
			printk("str = %s ", str);
		else
			printk("str= null");
		printk("error, unit test.... failed, ret = %d\n",ret);
	}
	else {
		if (reg && str)
			printk("[unit test] %s %s......ok,ret = %d\n", reg, str, ret);
	}
}

void TEST_regexp(void)
{
	TEST_reg_func(".*baidu.com$", "www.baidu.com", 1);
	TEST_reg_func("^sina.com", "www.sina.com.cn", 0);
	TEST_reg_func("^sina.com", "sina.com.cn", 1);
	TEST_reg_func(".*baidu.com$", "www.baidu.com223", 0);
}
