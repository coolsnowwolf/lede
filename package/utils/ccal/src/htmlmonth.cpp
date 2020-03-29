/*
   Copyright (c) 2000-2012, by Zhuo Meng (zxm8@case.edu).
   All rights reserved.

   Distributed under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
/* For calendar in HTML form with GB characters */

char GBjieqi[24][7] = {"\xd0\xa1\xba\xae", /*XH*/
                       "\xb4\xf3\xba\xae", /*DH*/
                       "\xc1\xa2\xb4\xba", /*LC*/
                       "\xd3\xea\xcb\xae", /*YS*/
                       "\xbe\xaa\xd5\xdd", /*JZ*/
                       "\xb4\xba\xb7\xd6", /*CF*/
                       "\xc7\xe5\xc3\xf7", /*QM*/
                       "\xb9\xc8\xd3\xea", /*GY*/
                       "\xc1\xa2\xcf\xc4", /*LX*/
                       "\xd0\xa1\xc2\xfa", /*XM*/
                       "\xc3\xa2\xd6\xd6", /*MZ*/
                       "\xcf\xc4\xd6\xc1", /*XZ*/
                       "\xd0\xa1\xca\xee", /*XS*/
                       "\xb4\xf3\xca\xee", /*DS*/
                       "\xc1\xa2\xc7\xef", /*LQ*/
                       "\xb4\xa6\xca\xee", /*CS*/
                       "\xb0\xd7\xc2\xb6", /*BL*/
                       "\xc7\xef\xb7\xd6", /*QF*/
                       "\xba\xae\xc2\xb6", /*HL*/
                       "\xcb\xaa\xbd\xb5", /*SJ*/
                       "\xc1\xa2\xb6\xac", /*LD*/
                       "\xd0\xa1\xd1\xa9", /*XX*/
                       "\xb4\xf3\xd1\xa9", /*DX*/
                       "\xb6\xac\xd6\xc1"  /*DZ*/
};

char GBmiscchar[22][4] = {"\xb3\xf5", /*Chu*/
                          "\xd2\xbb", /*Yi*/
                          "\xb6\xfe", /*Er*/
                          "\xc8\xfd", /*San*/
                          "\xcb\xc4", /*Si*/
                          "\xce\xe5", /*Wu*/
                          "\xc1\xf9", /*Liu*/
                          "\xc6\xdf", /*Qi*/
                          "\xb0\xcb", /*Ba*/
                          "\xbe\xc5", /*Jiu*/
                          "\xca\xae", /*Shi(Ten)*/
                          "\xd8\xa5", /*Nian(Twenty)*/
                          "\xd5\xfd", /*Zheng*/
                          "\xc8\xf2", /*Run*/
                          "\xd4\xc2", /*Yue*/
                          "\xc8\xd5", /*Ri*/
                          "\xc4\xea", /*Nian(Year)*/
                          "\xb4\xf3", /*Da*/
                          "\xd0\xa1", /*Xiao*/
                          "\xa3\xac", /*(comma)*/
                          "\xa1\xa1", /*(blank)*/
                          "\xca\xbc"  /*Shi(Start)*/
};

char GBtiangan[10][4] = {"\xbc\xd7", "\xd2\xd2",
                         "\xb1\xfb", "\xb6\xa1",
                         "\xce\xec", "\xbc\xba",
                         "\xb8\xfd", "\xd0\xc1",
                         "\xc8\xc9", "\xb9\xef"
};

char GBdizhi[12][4] = {"\xd7\xd3", "\xb3\xf3",
                       "\xd2\xfa", "\xc3\xae",
                       "\xb3\xbd", "\xcb\xc8",
                       "\xce\xe7", "\xce\xb4",
                       "\xc9\xea", "\xd3\xcf",
                       "\xd0\xe7", "\xba\xa5"
};

/* For calendar in HTML form with BIG5 characters */

char B5jieqi[24][7] = {"\xa4\x70\xb4\x48", /*XH*/
                       "\xa4\x6a\xb4\x48", /*DH*/
                       "\xa5\xdf\xac\x4b", /*LC*/
                       "\xab\x42\xa4\xf4", /*YS*/
                       "\xc5\xe5\xee\x68", /*JZ*/
                       "\xac\x4b\xa4\xc0", /*CF*/
                       "\xb2\x4d\xa9\xfa", /*QM*/
                       "\xbd\x5c\xab\x42", /*GY*/
                       "\xa5\xdf\xae\x4c", /*LX*/
                       "\xa4\x70\xba\xa1", /*XM*/
                       "\xa8\x7e\xba\xd8", /*MZ*/
                       "\xae\x4c\xa6\xdc", /*XZ*/
                       "\xa4\x70\xb4\xbb", /*XS*/
                       "\xa4\x6a\xb4\xbb", /*DS*/
                       "\xa5\xdf\xac\xee", /*LQ*/
                       "\xb3\x42\xb4\xbb", /*CS*/
                       "\xa5\xd5\xc5\x53", /*BL*/
                       "\xac\xee\xa4\xc0", /*QF*/
                       "\xb4\x48\xc5\x53", /*HL*/
                       "\xc1\xf7\xad\xb0", /*SJ*/
                       "\xa5\xdf\xa5\x56", /*LD*/
                       "\xa4\x70\xb3\xb7", /*XX*/
                       "\xa4\x6a\xb3\xb7", /*DX*/
                       "\xa5\x56\xa6\xdc"  /*DZ*/
};

char B5miscchar[22][4] = {"\xaa\xec", /*Chu*/
                          "\xa4\x40", /*Yi*/
                          "\xa4\x47", /*Er*/
                          "\xa4\x54", /*San*/
                          "\xa5\x7c", /*Si*/
                          "\xa4\xad", /*Wu*/
                          "\xa4\xbb", /*Liu*/
                          "\xa4\x43", /*Qi*/
                          "\xa4\x4b", /*Ba*/
                          "\xa4\x45", /*Jiu*/
                          "\xa4\x51", /*Shi(Ten)*/
                          "\xa4\xdc", /*Nian(Twenty)*/
                          "\xa5\xbf", /*Zheng*/
                          "\xb6\x7c", /*Run*/
                          "\xa4\xeb", /*Yue*/
                          "\xa4\xe9", /*Ri*/
                          "\xa6\x7e", /*Nian(Year)*/
                          "\xa4\x6a", /*Da*/
                          "\xa4\x70", /*Xiao*/
                          "\xa1\x41", /*(comma)*/
                          "\xa1\x40", /*(blank)*/
                          "\xa9\x6c"  /*Shi(Start)*/
};

char B5tiangan[10][4] = {"\xa5\xd2", "\xa4\x41",
                         "\xa4\xfe", "\xa4\x42",
                         "\xa5\xb3", "\xa4\x76",
                         "\xa9\xb0", "\xa8\xaf",
                         "\xa4\xd0", "\xac\xd1"
};

char B5dizhi[12][4] = {"\xa4\x6c", "\xa4\xa1",
                       "\xb1\x47", "\xa5\x66",
                       "\xa8\xb0", "\xa4\x78",
                       "\xa4\xc8", "\xa5\xbc",
                       "\xa5\xd3", "\xa8\xbb",
                       "\xa6\xa6", "\xa5\xe8"
};

/* For calendar in HTML form with UTF-8 characters */

char U8jieqi[24][7] = {"\xe5\xb0\x8f\xe5\xaf\x92", /*XH*/
                       "\xe5\xa4\xa7\xe5\xaf\x92", /*DH*/
                       "\xe7\xab\x8b\xe6\x98\xa5", /*LC*/
                       "\xe9\x9b\xa8\xe6\xb0\xb4", /*YS*/
                       "\xe6\x83\x8a\xe8\x9b\xb0", /*JZ*/
                       "\xe6\x98\xa5\xe5\x88\x86", /*CF*/
                       "\xe6\xb8\x85\xe6\x98\x8e", /*QM*/
                       "\xe8\xb0\xb7\xe9\x9b\xa8", /*GY*/
                       "\xe7\xab\x8b\xe5\xa4\x8f", /*LX*/
                       "\xe5\xb0\x8f\xe6\xbb\xa1", /*XM*/
                       "\xe8\x8a\x92\xe7\xa7\x8d", /*MZ*/
                       "\xe5\xa4\x8f\xe8\x87\xb3", /*XZ*/
                       "\xe5\xb0\x8f\xe6\x9a\x91", /*XS*/
                       "\xe5\xa4\xa7\xe6\x9a\x91", /*DS*/
                       "\xe7\xab\x8b\xe7\xa7\x8b", /*LQ*/
                       "\xe5\xa4\x84\xe6\x9a\x91", /*CS*/
                       "\xe7\x99\xbd\xe9\x9c\xb2", /*BL*/
                       "\xe7\xa7\x8b\xe5\x88\x86", /*QF*/
                       "\xe5\xaf\x92\xe9\x9c\xb2", /*HL*/
                       "\xe9\x9c\x9c\xe9\x99\x8d", /*SJ*/
                       "\xe7\xab\x8b\xe5\x86\xac", /*LD*/
                       "\xe5\xb0\x8f\xe9\x9b\xaa", /*XX*/
                       "\xe5\xa4\xa7\xe9\x9b\xaa", /*DX*/
                       "\xe5\x86\xac\xe8\x87\xb3"  /*DZ*/
};

char U8miscchar[22][4] = {"\xe5\x88\x9d", /*Chu*/
                          "\xe4\xb8\x80", /*Yi*/
                          "\xe4\xba\x8c", /*Er*/
                          "\xe4\xb8\x89", /*San*/
                          "\xe5\x9b\x9b", /*Si*/
                          "\xe4\xba\x94", /*Wu*/
                          "\xe5\x85\xad", /*Liu*/
                          "\xe4\xb8\x83", /*Qi*/
                          "\xe5\x85\xab", /*Ba*/
                          "\xe4\xb9\x9d", /*Jiu*/
                          "\xe5\x8d\x81", /*Shi(Ten)*/
                          "\xe5\xbb\xbf", /*Nian(Twenty)*/
                          "\xe6\xad\xa3", /*Zheng*/
                          "\xe9\x97\xb0", /*Run*/
                          "\xe6\x9c\x88", /*Yue*/
                          "\xe6\x97\xa5", /*Ri*/
                          "\xe5\xb9\xb4", /*Nian(Year)*/
                          "\xe5\xa4\xa7", /*Da*/
                          "\xe5\xb0\x8f", /*Xiao*/
                          "\xef\xbc\x8c", /*(comma)*/
                          "  ", /*(blank)*/
                          "\xe5\xa7\x8b"  /*Shi(Start)*/
};

char U8tiangan[10][4] = {"\xe7\x94\xb2", "\xe4\xb9\x99",
                         "\xe4\xb8\x99", "\xe4\xb8\x81",
                         "\xe6\x88\x8a", "\xe5\xb7\xb1",
                         "\xe5\xba\x9a", "\xe8\xbe\x9b",
                         "\xe5\xa3\xac", "\xe7\x99\xb8"
};

char U8dizhi[12][4] = {"\xe5\xad\x90", "\xe4\xb8\x91",
                       "\xe5\xaf\x85", "\xe5\x8d\xaf",
                       "\xe8\xbe\xb0", "\xe5\xb7\xb3",
                       "\xe5\x8d\x88", "\xe6\x9c\xaa",
                       "\xe7\x94\xb3", "\xe9\x85\x89",
                       "\xe6\x88\x8c", "\xe4\xba\xa5"
};

char U8DiffSim[7][4] = {"\xe6\x83\x8a", /*Jing*/
                        "\xe8\x9b\xb0", /*Zhe*/
                        "\xe8\xb0\xb7", /*Gu*/
                        "\xe6\xbb\xa1", /*Man*/
                        "\xe7\xa7\x8d", /*Zhong*/
                        "\xe5\xa4\x84", /*Chu3*/
                        "\xe9\x97\xb0"  /*Run*/
};

char U8DiffTra[7][4] = {"\xe9\xa9\x9a", /*Jing*/
                        "\xe8\x9f\x84", /*Zhe*/
                        "\xe7\xa9\x80", /*Gu*/
                        "\xe6\xbb\xbf", /*Man*/
                        "\xe7\xa8\xae", /*Zhong*/
                        "\xe8\x99\x95", /*Chu3*/
                        "\xe9\x96\x8f"  /*Run*/
};

typedef char c22_4[22][4];
typedef c22_4 *pc22_4;

#include <stdio.h>
#include <string.h>
#include "htmlmonth.h"
#include "verstr.h"

/* Input:
   bIsSim: true--set simplified characters, false--set traditional characters
*/
void SetU8Characters(bool bIsSim)
{
    if (bIsSim)
    {
        strncpy(&(U8jieqi[4][0]), U8DiffSim[0], 3);
        strncpy(&(U8jieqi[4][3]), U8DiffSim[1], 3);
        strncpy(&(U8jieqi[7][0]), U8DiffSim[2], 3);
        strncpy(&(U8jieqi[9][3]), U8DiffSim[3], 3);
        strncpy(&(U8jieqi[10][3]), U8DiffSim[4], 3);
        strncpy(&(U8jieqi[15][0]), U8DiffSim[5], 3);
        strncpy(&(U8miscchar[13][0]), U8DiffSim[6], 3);
    }
    else
    {
        strncpy(&(U8jieqi[4][0]), U8DiffTra[0], 3);
        strncpy(&(U8jieqi[4][3]), U8DiffTra[1], 3);
        strncpy(&(U8jieqi[7][0]), U8DiffTra[2], 3);
        strncpy(&(U8jieqi[9][3]), U8DiffTra[3], 3);
        strncpy(&(U8jieqi[10][3]), U8DiffTra[4], 3);
        strncpy(&(U8jieqi[15][0]), U8DiffTra[5], 3);
        strncpy(&(U8miscchar[13][0]), U8DiffTra[6], 3);
    }
}

/* Input:
   month, month number with .5 indicating leap month
   nstart, day of calendar month that starts this lunar month
   ndays, number of days in this lunar month
   nEncoding: 'g' for GB, 'b' for BIG5, 'u' for UTF-8
   Output:
   monname, C string containing the Chinese characters for the month.
            monname should be allocated at least 24 bytes before the call
*/
void Number2MonthCH(double month, int nstart, int ndays, int nEncoding, char *monname)
{
    pc22_4 miscchar;
    if (nEncoding == 'u')
        miscchar = &U8miscchar;
    else if (nEncoding == 'g')
        miscchar = &GBmiscchar;
    else
        miscchar = &B5miscchar;
    monname[0] = 0; /* Clear */
    int nmonth = int(month);
    if (month - nmonth == 0.5) /* Leap month */
        strcat(monname, (*miscchar)[13]);
    if (nmonth > 10)
    {
        strcat(monname, (*miscchar)[10]);
        nmonth -= 10;
    }
    if (nmonth == 1 && strlen(monname) == 0) /* Zheng month */
        strcat(monname, (*miscchar)[12]);
    else
        strcat(monname, (*miscchar)[nmonth]);
    strcat(monname, (*miscchar)[14]);
    if (ndays == 30)
        strcat(monname, (*miscchar)[17]);
    else
        strcat(monname, (*miscchar)[18]);
    char buf[10];
    sprintf(buf, "%d", nstart);
    strcat(monname, buf);
    strcat(monname, (*miscchar)[15]);
    strcat(monname, (*miscchar)[21]);
}

/* Input:
   nday, day number (1 - 30)
   nEncoding: 'g' for GB, 'b' for BIG5, 'u' for UTF-8
   Output:
   dayname, C string containing the Chinese characters for the day.
            dayname should be allocated at least 7 bytes before the call
*/
void Number2DayCH(int nday, int nEncoding, char *dayname)
{
    pc22_4 miscchar;
    if (nEncoding == 'u')
        miscchar = &U8miscchar;
    else if (nEncoding == 'g')
        miscchar = &GBmiscchar;
    else
        miscchar = &B5miscchar;
    dayname[0] = 0; /* Clear */
    if (nday <= 10)
        strcat(dayname, (*miscchar)[0]);
    if (nday > 10 && nday < 20)
    {
        strcat(dayname, (*miscchar)[10]);
        nday -= 10;
    }
    if (nday == 20)
    {
        strcat(dayname, (*miscchar)[2]);
        nday -= 10;
    }
    if (nday > 20 && nday < 30)
    {
        strcat(dayname, (*miscchar)[11]);
        nday -= 20;
    }
    if (nday == 30)
    {
        strcat(dayname, (*miscchar)[3]);
        nday -= 20;
    }
    strcat(dayname, (*miscchar)[nday]);
}

/* Input:
   titlestr, string to be included in the English part of title, normally either
             Year XXXX or Month Year
   month, Gregorian month number (1 - 12), 0 for whole year
   year, Gregorian year number, between 1645 and 9999
   nEncoding: 'g' for GB, 'b' for BIG5, 'u' for UTF-8
*/
void PrintHeaderHTML(char *titlestr, int month, int year, int nEncoding)
{
    printf("<html>\n<head>\n<meta http-equiv=\"Content-Type\" ");
    if (nEncoding == 'u')
        printf("content=\"text/html; charset=utf-8\">\n");
    else if (nEncoding == 'g')
        printf("content=\"text/html; charset=gb2312\">\n");
    else
        printf("content=\"text/html; charset=big5\">\n");
    printf("<meta name=\"GENERATOR\" content=\"ccal-%s by Zhuo Meng, http://thunder.cwru.edu/ccal/\">\n", versionstr);
    pc22_4 miscchar;
    if (nEncoding == 'u')
        miscchar = &U8miscchar;
    else if (nEncoding == 'g')
        miscchar = &GBmiscchar;
    else
        miscchar = &B5miscchar;
    if (month != 0)
        printf("<title>Chinese Calendar for %s / %d%s%d%s</title>\n", titlestr, year, (*miscchar)[16], month, (*miscchar)[14]);
    else
        printf("<title>Chinese Calendar for %s / %d%s</title>\n", titlestr, year, (*miscchar)[16]);
    printf("</head>\n<body>\n<center>\n");
    printf("<table border=\"1\" cellspacing=\"1\" width=\"90%%\">\n");
}

void PrintClosingHTML()
{
    printf("</table>\n</center>\n</body>\n</html>\n");
}
