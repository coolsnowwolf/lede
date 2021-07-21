#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <getopt.h>
#include <time.h>
#include <sys/time.h>

#include "cyutils.h"
#include "code_pattern.h"

#define MAX_BUF	1024

//wuzh add for code.bin md5 2008-4-15
static char *months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

struct code_header * 
//init_code_header(void) //wuzh for md5 2008-4-15
init_code_header(int flag)
{
	int i;

	struct code_header *pattern;

	struct timeval tv;
        struct timezone tz;
        struct tm tm;
	unsigned char version_buf[64]; 
	unsigned char buf2[64];
	int cnt;

        pattern = malloc(sizeof(struct code_header));
	memset(pattern, 0, sizeof(struct code_header));	

	memcpy(pattern->magic, CODE_PATTERN, sizeof(pattern->magic));
	memcpy(pattern->id, CODE_ID, sizeof(pattern->id));

	//fprintf(stderr, "%s is %ld bytes\n", input_file, input_size);

	//Make date string
//wuzh modify for code.bin md5 2008-4-15
#if 0
        gettimeofday(&tv,&tz);
        memcpy(&tm, localtime(&tv.tv_sec), sizeof(struct tm));
	pattern->fwdate[0]=tm.tm_year+1900-2000;
	pattern->fwdate[1]=tm.tm_mon+1;
	pattern->fwdate[2]=tm.tm_mday;
        printf("\nMake date==>Year:%d,Month:%d,Day:%d,Hour:%d,Min:%d,Sec:%d\n",tm.tm_year+1900,tm.tm_mon+1,tm.tm_mday,tm.tm_hour,tm.tm_min,tm.tm_sec);
#else
       if(flag == 0) {
               int year, day, mon=1;
               char temp[10]={0};
               char string[20];
               int j;

               sscanf(__DATE__, "%s %d %d", temp, &day, &year);
       
               for(j=0;j<12;j++) {
                       if(!strcmp(temp, months[j])) {
                               mon = j+1;
                               break;
                       }
               }
               pattern->fwdate[0]=year-2000;
               pattern->fwdate[1]=mon;
               pattern->fwdate[2]=day;
       }
       else {
               gettimeofday(&tv,&tz);
               memcpy(&tm, localtime(&tv.tv_sec), sizeof(struct tm));

               pattern->fwdate[0]=tm.tm_year+1900-2000;
               pattern->fwdate[1]=tm.tm_mon+1;
               pattern->fwdate[2]=tm.tm_mday;
       }
        printf("\nMake date==>Year:%d,Month:%d,Day:%d", pattern->fwdate[0], pattern->fwdate[1], pattern->fwdate[2]);
#endif
	//Make version
	memset(version_buf,0,64);
	memset(buf2,0,64);
	memcpy(version_buf,CYBERTAN_VERSION ,strlen(CYBERTAN_VERSION));
	printf("\nFirmware version =>%s\n",version_buf);
	cnt=0;
	i=0;
	while(i<strlen(version_buf)) 
	{
		if(version_buf[i] == 0x2e) 
			cnt++;
		i++;
	}
	strcpy(buf2,strtok(version_buf,".")); 
	pattern->fwvern[0]=(char)atoi(&buf2[1]);
	//printf("\n1:%s,%d\n",buf2,pattern->fwvern[0]);
	memset(buf2,0,64);
	strcpy(buf2,strtok(NULL,".")); 
	pattern->fwvern[1]=(char)atoi(buf2);
	//printf("\n2:%s,%d\n",buf2,pattern->fwvern[1]);
	if(cnt==2)
	{
		memset(buf2,0,64);
		strcpy(buf2,strtok(NULL,".")); 
		pattern->fwvern[2]=(char)atoi(buf2);
		//printf("\n3:%s,%d\n",buf2,pattern->fwvern[2]);
	}
	else
	{
		pattern->fwvern[2]=0;
	}

	pattern->sn = atoi(SERIAL_NUMBER);

	pattern->flags |= SUPPORT_4712_CHIP;
	pattern->flags |= SUPPORT_INTEL_FLASH;
	pattern->flags |= SUPPORT_5325E_SWITCH;
	pattern->flags |= SUPPORT_4704_CHIP;
	pattern->flags |= SUPPORT_5352E_CHIP;
	
	return pattern;
}
