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
/* Calendar with Chinese calendar */

#include <math.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include "lunaryear.h"
#include "htmlmonth.h"
#include "psmonth.h"
#include "verstr.h"
#include "novas.h"

#ifndef NO_NAMESPACE
using namespace std;
#endif

#ifndef WIN32
#define HILIGHTTODAY
#endif

static char jieqi[24][3] = {"XH", "DH", "LC", "YS", "JZ", "CF", "QM", "GY",
                            "LX", "XM", "MZ", "XZ", "XS", "DS", "LQ", "CS",
                            "BL", "QF", "HL", "SJ", "LD", "XX", "DX", "DZ"};
char monnames[12][10] = {"January", "February", "March", "April",
                         "May", "June", "July", "August",
                         "September", "October", "November", "December"};
char daynames[7][10]= {"Sunday", "Monday", "Tuesday", "Wednesday",
                       "Thursday", "Friday", "Saturday"};
char daynamesGB[7][10]= {"Sun  \xc8\xd5", "Mon  \xd2\xbb", "Tue  \xb6\xfe",
                         "Wed  \xc8\xfd", "Thu  \xcb\xc4", "Fri  \xce\xe5",
                         "Sat  \xc1\xf9"};
char daynamesB5[7][10]= {"Sun  \xa4\xe9", "Mon  \xa4\x40", "Tue  \xa4\x47",
                         "Wed  \xa4\x54", "Thu  \xa5\x7c", "Fri  \xa4\xad",
                         "Sat  \xa4\xbb"};
char daynamesU8[7][10]= {"Sun  \xe6\x97\xa5", "Mon  \xe4\xb8\x80",
                         "Tue  \xe4\xba\x8c", "Wed  \xe4\xb8\x89",
                         "Thu  \xe5\x9b\x9b", "Fri  \xe4\xba\x94",
                         "Sat  \xe5\x85\xad"};
unsigned short int daysinmonth[12] = {31, 28, 31, 30, 31, 30,
                                      31, 31, 30, 31, 30, 31};
static char tiangan[10][5] = {"Jia", "Yi", "Bing", "Ding", "Wu",
                              "Ji", "Geng", "Xin", "Ren", "Gui"};
static char dizhi[12][5] = {"Zi", "Chou", "Yin", "Mou", "Chen", "Si",
                            "Wu", "Wei", "Shen", "You", "Xu", "Hai"};
extern char GBjieqi[24][7];
extern char B5jieqi[24][7];
extern char U8jieqi[24][7];
extern char PSjieqi[24][9];
extern char GBmiscchar[22][4];
extern char B5miscchar[22][4];
extern char U8miscchar[22][4];
extern char PSbigmchar[21][5];
extern char GBtiangan[10][4];
extern char B5tiangan[10][4];
extern char U8tiangan[10][4];
extern char PStiangan[10][5];
extern char GBdizhi[12][4];
extern char B5dizhi[12][4];
extern char U8dizhi[12][4];
extern char PSdizhi[12][5];

typedef char c24_7[24][7];
typedef c24_7 *pc24_7;
typedef char c22_4[22][4];
typedef c22_4 *pc22_4;
typedef char c10_4[10][4];
typedef c10_4 *pc10_4;
typedef char c12_4[12][4];
typedef c12_4 *pc12_4;
typedef char c7_10[7][10];
typedef c7_10 *pc7_10;

/* Input:
   year: year number in AD
   Return:
   true -- Leap year, false -- normal year
*/
bool IsLeapYear(short int year)
{
    if (year / 100 * 100 != year) /* Non century year */
    {
        if (year / 4 * 4 == year)
            return true;
        else
            return false;
    }
    else
    {
        if (year / 400 * 400 == year)
            return true;
        else
            return false;
    }
}

/* Inputs:
   mnumber: month number with .5 indicating leap month
   Output:
   month: integer month number
   leap: zero for normal month, 'R' for leap month
*/
void GetMonthNumber(double mnumber, short int& month, char* leap)
{
    month = int(mnumber);
    if (mnumber - month == 0.0)
        *leap = 0;
    else
        *leap = 'R';
}

/* Input:
   mnumber: month number with .5 indicating leap month
*/
void PrintMonthNumber(double mnumber)
{
    short int month;
    char leap[2] = {0x00, 0x00};
    GetMonthNumber(mnumber, month, leap);
    printf(" [%2d]Y%1s ", month, leap);
}

/* Inputs:
   year: year number in AD
   month: month number (Gregorian Calendar)
   vterms: vector of days of solarterms in the year
   lastnew: julian day of last new moon of the previous year
   lastmon: month number for that started on last new moon day of previous year
   vmoons: vector of days of new moons in the year
   vmonth: vector of month numbers with .5 indicating leap month
   nextnew: julian day of the first new moon of the next year
   pmode: mode of printing, 0: ASCII, 1: HTML Table, 2: PostScript, 3: XML
   bSingle: true--single month, false--part of whole year
   nEncoding: 'a' for ASCII, 'g' for GB, 'b' for BIG5, 'u' for UTF-8
   bNeedsRun: true if character Run is needed, false otherwise
*/
void PrintMonth(short int year, short int month, vdouble& vterms,
                double lastnew, double lastmon, vdouble& vmoons,
                vdouble& vmonth, double nextnew, int pmode,
                bool bSingle, int nEncoding, bool bNeedsRun)
{
#ifdef HILIGHTTODAY
#define ANSI_REV "\x1b[7m"
#define ANSI_NORMAL "\x1b[0m"
    time_t now = time(NULL);
    struct tm *today = localtime(&now);
#endif
    pc10_4 CHtiangan = &GBtiangan;
    pc12_4 CHdizhi = &GBdizhi;
    pc22_4 CHmiscchar = &GBmiscchar;
    pc24_7 CHjieqi = &GBjieqi;
    pc7_10 daynamesCH = &daynames;
    bool bIsSim = (nEncoding == 'g');
    if (nEncoding == 'u')
    {
        CHtiangan = &U8tiangan;
        CHdizhi = &U8dizhi;
        CHmiscchar = &U8miscchar;
        CHjieqi = &U8jieqi;
        daynamesCH = &daynamesU8;
    }
    else if (nEncoding == 'g')
    {
        CHtiangan = &GBtiangan;
        CHdizhi = &GBdizhi;
        CHmiscchar = &GBmiscchar;
        CHjieqi = &GBjieqi;
        daynamesCH = &daynamesGB;
    }
    else if (nEncoding == 'b')
    {
        CHtiangan = &B5tiangan;
        CHdizhi = &B5dizhi;
        CHmiscchar = &B5miscchar;
        CHjieqi = &B5jieqi;
        daynamesCH = &daynamesB5;
    }
    int nCHchars = (int)strlen((*CHmiscchar)[14]);
    char space1[] = "&#160;";
    char space2[] = " ";
    char *sp;
    if (pmode == 1)
        sp = space1;
    else
        sp = space2;
    /* Set julian day counter to 1st of the month */
    double jdcnt = julian_date(year, month, 1, 12.0);
    /* Find julian day of the start of the next month */
    double jdnext;
    if (month < 12)
        jdnext = julian_date(year, month + 1, 1, 12.0);
    else
        jdnext = julian_date(year + 1, 1, 1, 12.0);
    /* Set solarterm counter to day of 1st term of the calendar month */
    int termcnt = (month - 1) * 2;
    if (vterms[termcnt] < jdcnt)
    	termcnt ++;
    /* Set lunar month counter to the 1st lunar month of the calendar month */
    int moncnt = 0;
    while (moncnt < int(vmoons.size()) && vmoons[moncnt] < jdcnt)
        moncnt++;
    /* In case solarterm and 1st of lunar month falls on the same day */
    bool sameday = false;
    /* Initialize counters for lunar days and days of month */
    int ldcnt, dcnt = 1;
    if (month != 1)
        ldcnt = int(jdcnt - vmoons[moncnt - 1] + 1);
    else
        ldcnt = int(jdcnt - lastnew + 1);
    if (jdcnt == vmoons[moncnt])
        ldcnt = 1;
    /* Day of week of the 1st of month */
    int dofw = (int(jdcnt) + 1) % 7;
    int nWeeks = 5;
    if ((dofw > 4 && daysinmonth[month - 1] == 31) || (dofw > 5 && daysinmonth[month - 1] == 30))
    	nWeeks = 6;
    /* Header of the month */
    short int cyear = (year - 1984) % 60;
    if (cyear < 0)
        cyear += 60;
    int tiancnt = cyear % 10;
    int dicnt = cyear % 12;
    short int cmonth;
    char leap[2] = {0x00, 0x00};
    char monthhead[200], cmonname[100];
    int nstartlm = int(vmoons[moncnt] - jdcnt + 1);
    int ndayslm;
    if (moncnt < int(vmoons.size()) - 1)
        ndayslm = int(vmoons[moncnt + 1] - vmoons[moncnt]);
    else
        ndayslm = int(nextnew - vmoons.back());
    GetMonthNumber(vmonth[moncnt], cmonth, leap);
    if ((pmode == 0 && nEncoding != 'a') || pmode == 1 || pmode == 3)
        Number2MonthCH(vmonth[moncnt], nstartlm, ndayslm, nEncoding, cmonname);
    else if (pmode == 2)
        Number2MonthPS(vmonth[moncnt], nstartlm, ndayslm, !bSingle, cmonname);
    /* January is special if lunar New Year is in February */
    if (month == 1 && cmonth != 1)
    {
        int tiancnt0 = (cyear + 59) % 10;
        int dicnt0 = (cyear + 59) % 12;
        if (vmoons[moncnt + 1] < jdnext) /* Two lunar months in one month */
        {
            short int cmonth1;
            char leap1[2] = {0x00, 0x00};
            int nstartlm1 = int(vmoons[moncnt + 1] - jdcnt + 1);
            int ndayslm1;
            char cmonname1[100];
            if (moncnt < int(vmoons.size()) - 2)
                ndayslm1 = int(vmoons[moncnt + 2] - vmoons[moncnt + 1]);
            else
                ndayslm1 = int(nextnew - vmoons.back());
            if (pmode == 0 && nEncoding == 'a')
            {
                GetMonthNumber(vmonth[moncnt + 1], cmonth1, leap1);
                sprintf(monthhead,
                    "%s %d (Year %s%s, Month %s%d%c S%d, Year %s%s, Month %s%d%c S%d)",
                    monnames[month - 1], year, tiangan[tiancnt0], dizhi[dicnt0],
                    leap, cmonth, (ndayslm == 30) ? 'D' : 'X', nstartlm,
                    tiangan[tiancnt], dizhi[dicnt],
                    leap1, cmonth1, (ndayslm1 == 30) ? 'D' : 'X', nstartlm1);
            }
            else if ((pmode == 0 && nEncoding != 'a') || pmode == 1)
            {
                Number2MonthCH(vmonth[moncnt + 1], nstartlm1, ndayslm1, nEncoding, cmonname1);
                sprintf(monthhead,
                    "%s %d%s%s%s%s%s%s%s%s%s%s%s",
                    monnames[month - 1], year, sp, sp, (*CHtiangan)[tiancnt0], (*CHdizhi)[dicnt0],
                    (*CHmiscchar)[16], cmonname, (*CHmiscchar)[19], (*CHtiangan)[tiancnt],
                    (*CHdizhi)[dicnt], (*CHmiscchar)[16], cmonname1);
            }
            else if (pmode == 3)
            {
                Number2MonthCH(vmonth[moncnt + 1], nstartlm1, ndayslm1, nEncoding, cmonname1);
                sprintf(monthhead,
                    "%s%s%s%s%s%s%s%s%s",
                    (*CHtiangan)[tiancnt0], (*CHdizhi)[dicnt0],
                    (*CHmiscchar)[16], cmonname, (*CHmiscchar)[19], (*CHtiangan)[tiancnt],
                    (*CHdizhi)[dicnt], (*CHmiscchar)[16], cmonname1);
            }
            else if (bSingle)
            {
                Number2MonthPS(vmonth[moncnt + 1], nstartlm1, ndayslm1, !bSingle, cmonname1);
                sprintf(monthhead,
                    "20 1 SF (%s %d) S\n/fsc 21 def gsave ptc\n"
                    "%s%s%s%s%s\n%s%s%s%s%sgrestore",
                    monnames[month - 1], year, PSbigmchar[20], PStiangan[tiancnt0],
                    PSdizhi[dicnt0], PSbigmchar[16], cmonname, PSbigmchar[19],
                    PStiangan[tiancnt], PSdizhi[dicnt], PSbigmchar[16], cmonname1);
            }
        }
        else
        {
            if (pmode == 0 && nEncoding == 'a')
            {
                sprintf(monthhead, "%s %d (Year %s%s, Month %s%d%c S%d)",
                    monnames[month - 1], year, tiangan[tiancnt0], dizhi[dicnt0],
                    leap, cmonth, (ndayslm == 30) ? 'D' : 'X', nstartlm);
            }
            else if ((pmode == 0 && nEncoding != 'a') || pmode == 1)
            {
                sprintf(monthhead,
                    "%s %d%s%s%s%s%s%s",
                    monnames[month - 1], year, sp, sp, (*CHtiangan)[tiancnt0],
                    (*CHdizhi)[dicnt0], (*CHmiscchar)[16], cmonname);
            }
            else if (pmode == 3)
            {
                sprintf(monthhead,
                    "%s%s%s%s",
                    (*CHtiangan)[tiancnt0],
                    (*CHdizhi)[dicnt0], (*CHmiscchar)[16], cmonname);
            }
            else if (bSingle)
            {
                sprintf(monthhead,
                    "20 1 SF (%s %d) S\n/fsc 21 def gsave ptc\n"
                    "%s%s%s%s%sgrestore",
                    monnames[month - 1], year, PSbigmchar[20], PStiangan[tiancnt0],
                    PSdizhi[dicnt0], PSbigmchar[16], cmonname);
            }
        }
    }
    else
    {
        if (moncnt < int(vmoons.size()) - 1 && vmoons[moncnt + 1] < jdnext)
        /* Two lunar months in one month */
        {
            short int cmonth1;
            char leap1[2] = {0x00, 0x00};
            int nstartlm1 = int(vmoons[moncnt + 1] - jdcnt + 1);
            int ndayslm1;
            char cmonname1[100];
            if (moncnt < int(vmoons.size()) - 2)
                ndayslm1 = int(vmoons[moncnt + 2] - vmoons[moncnt + 1]);
            else
                ndayslm1 = int(nextnew - vmoons.back());
            if (pmode == 0 && nEncoding == 'a')
            {
                GetMonthNumber(vmonth[moncnt + 1], cmonth1, leap1);
                sprintf(monthhead,
                    "%s %d (Year %s%s, Month %s%d%c S%d, %s%d%c S%d)",
                    monnames[month - 1], year, tiangan[tiancnt], dizhi[dicnt],
                    leap, cmonth, (ndayslm == 30) ? 'D' : 'X', nstartlm,
                    leap1, cmonth1, (ndayslm1 == 30) ? 'D' : 'X', nstartlm1);
            }
            else if ((pmode == 0 && nEncoding != 'a') || pmode == 1)
            {
                Number2MonthCH(vmonth[moncnt + 1], nstartlm1, ndayslm1, nEncoding, cmonname1);
                sprintf(monthhead,
                    "%s %d%s%s%s%s%s%s%s%s", monnames[month - 1], year, sp, sp,
                    (*CHtiangan)[tiancnt], (*CHdizhi)[dicnt],
                    (*CHmiscchar)[16], cmonname, (*CHmiscchar)[19], cmonname1);
            }
            else if (pmode == 3)
            {
                Number2MonthCH(vmonth[moncnt + 1], nstartlm1, ndayslm1, nEncoding, cmonname1);
                sprintf(monthhead,
                    "%s%s%s%s%s%s",
                    (*CHtiangan)[tiancnt], (*CHdizhi)[dicnt],
                    (*CHmiscchar)[16], cmonname, (*CHmiscchar)[19], cmonname1);
            }
            else if (bSingle)
            {
                Number2MonthPS(vmonth[moncnt + 1], nstartlm1, ndayslm1, !bSingle, cmonname1);
                sprintf(monthhead,
                    "20 1 SF (%s %d) S\n/fsc 21 def gsave ptc\n"
                    "%s%s%s%s%s%s%sgrestore",
                    monnames[month - 1], year, PSbigmchar[20], PStiangan[tiancnt],
                    PSdizhi[dicnt], PSbigmchar[16], cmonname, PSbigmchar[19],
                    cmonname1);
            }
        }
        else if (month == 2 && vmoons[moncnt] >= jdnext)
        /* No new moon in February */
        {
            ndayslm = int(vmoons[moncnt] - vmoons[moncnt - 1]);
            if (pmode == 0 && nEncoding == 'a')
            {
                GetMonthNumber(vmonth[moncnt - 1], cmonth, leap);
                sprintf(monthhead, "%s %d (Year %s%s, Month %s%d%c)",
                    monnames[month - 1], year, tiangan[tiancnt], dizhi[dicnt],
                    leap, cmonth, (ndayslm == 30) ? 'D' : 'X');
            }
            else if ((pmode == 0 && nEncoding != 'a') || pmode == 1)
            {
                Number2MonthCH(vmonth[moncnt - 1], nstartlm, ndayslm, nEncoding, cmonname);
                char *p = strstr(cmonname, (*CHmiscchar)[14]) + 2 * nCHchars;
                *p = 0;
                sprintf(monthhead,
                    "%s %d%s%s%s%s%s%s", monnames[month - 1], year, sp, sp,
                    (*CHtiangan)[tiancnt], (*CHdizhi)[dicnt],
                    (*CHmiscchar)[16], cmonname);
            }
            else if (pmode == 3)
            {
                Number2MonthCH(vmonth[moncnt - 1], nstartlm, ndayslm, nEncoding, cmonname);
                char *p = strstr(cmonname, (*CHmiscchar)[14]) + 2 * nCHchars;
                *p = 0;
                sprintf(monthhead,
                    "%s%s%s%s",
                    (*CHtiangan)[tiancnt], (*CHdizhi)[dicnt],
                    (*CHmiscchar)[16], cmonname);
            }
            else if (bSingle)
            {
                Number2MonthPS(vmonth[moncnt - 1], nstartlm, ndayslm, !bSingle, cmonname);
                char *p = strstr(cmonname, PSbigmchar[14]) + 8;
                *p = 0;
                sprintf(monthhead,
                    "20 1 SF (%s %d) S\n/fsc 21 def gsave ptc\n"
                    "%s%s%s%s%sgrestore",
                    monnames[month - 1], year, PSbigmchar[20], PStiangan[tiancnt],
                    PSdizhi[dicnt], PSbigmchar[16], cmonname);
            }
        }
        else
        {
            if (pmode == 0 && nEncoding == 'a')
            {
                sprintf(monthhead, "%s %d (Year %s%s, Month %s%d%c S%d)",
                    monnames[month - 1], year, tiangan[tiancnt], dizhi[dicnt],
                    leap, cmonth, (ndayslm == 30) ? 'D' : 'X', nstartlm);
            }
            else if ((pmode == 0 && nEncoding != 'a') || pmode == 1)
            {
                sprintf(monthhead,
                    "%s %d%s%s%s%s%s%s", monnames[month - 1], year, sp, sp,
                    (*CHtiangan)[tiancnt], (*CHdizhi)[dicnt],
                    (*CHmiscchar)[16], cmonname);
            }
            else if (pmode == 3)
            {
                sprintf(monthhead,
                    "%s%s%s%s",
                    (*CHtiangan)[tiancnt], (*CHdizhi)[dicnt],
                    (*CHmiscchar)[16], cmonname);
            }
            else if (bSingle)
            {
                sprintf(monthhead,
                    "20 1 SF (%s %d) S\n/fsc 21 def gsave ptc\n"
                    "%s%s%s%s%sgrestore",
                    monnames[month - 1], year, PSbigmchar[20], PStiangan[tiancnt],
                    PSdizhi[dicnt], PSbigmchar[16], cmonname);
            }
        }
    }
    int nmove, i;
    if (pmode == 0)
    {
        int nHeadLen = strlen(monthhead);
        if (nEncoding == 'u')
        {
            int nasc = 0;
            for (i = 0; i < nHeadLen; i++)
                if ((unsigned char)(monthhead[i]) < 0x80)
                    nasc++;
            nHeadLen = (nHeadLen - nasc) * 2 / 3 + nasc;
        }
        nmove = (68 - nHeadLen) / 2;
        if (nmove < 0)
            nmove = 0;
        for (i = 0; i < nmove; i++)
            printf(" ");
        printf("%s\n", monthhead);
    }
    else if (pmode == 1)
    {
        printf("<tr>\n<th colspan=\"7\" width=\"100%%\">");
        printf("%s</th>\n</tr>\n", monthhead);
    }
    else if (pmode == 3)
    {
        printf("<ccal:month value=\"%d\" name=\"%s\" cname=\"%s\">\n",
            month, monnames[month - 1], monthhead);
    }
    else if (bSingle)
    {
        PrintHeaderMonthPS(monthhead, month, true, bIsSim, bNeedsRun, nWeeks);
        char *p1 = strstr(monthhead, "(");
        char *p2 = strstr(monthhead, ")");
        int nlen = int(p2 - p1) - 1;
        int nheadlen = (int)strlen(monthhead);
        if (nheadlen - nlen - 66 > 56)
            nmove = (4200 - 78 * (nlen + (nheadlen - nlen - 86) / 2 + 3)) / 2;
        else if (nheadlen - nlen - 66 > 30)
            nmove = (4200 - 78 * (nlen + (nheadlen - nlen - 66) / 2 + 3)) / 2;
        else
            nmove = (4200 - 78 * (nlen + (nheadlen - nlen - 46) / 2 + 2)) / 2;
        if (nmove < 0)
            nmove = 0;
        printf("%d 2475 m\n", nmove);
        printf("%s\n", monthhead);
    }
    else
    {
        printf("%% %s %d\n", monnames[month - 1], year);
        int posx = (month - 1) % 4 * 140;
        int posy = 3450 - (month - 1) / 4 * 1150 - 173;
        printf("%d lpts %d m gsave ct\n",
               posx, posy);
        printf("%% Month number\n%d lpts %d m gsave\n100 1 SF 1 0.9 1 K",
               ((month < 10) ? 35 : 10), -750);
        printf(" (%d) S grestore\n", month);
    }
    /* Day of week */
    char dayshort[4];
    if (pmode == 0)
    {
        for (i = 0; i < 7; i++)
        {
            if (nEncoding != 'u')
                printf("%-10s", (*daynamesCH)[i]);
            else
                printf("%s   ", (*daynamesCH)[i]);
        }
        printf("\n");
    }
    else if (pmode == 1)
    {
        printf("<tr align=\"center\">\n");
        for (i = 0; i < 7; i++)
        {
            strncpy(dayshort, daynames[i], 3);
            dayshort[3] = 0;
            if (i == 0)
                printf("<td width=\"15%%\"><font color=\"#FF0000\">"
                       "%s %s</font></td>\n", dayshort, (*CHmiscchar)[15]);
            else if (i == 6)
                printf("<td width=\"15%%\"><font color=\"#00E600\">"
                       "%s %s</font></td>\n", dayshort, (*CHmiscchar)[i]);
            else
                printf("<td width=\"14%%\">%s %s</td>\n",
                       dayshort, (*CHmiscchar)[i]);
        }
        printf("</tr>\n");
    }
    else if (pmode == 2)
    {
        if (bSingle)
        {
            printf("%% Day names\n14 1 SF\n/fsc 15 def\n");
            for (i = 0; i < 7; i++)
            {
                strncpy(dayshort, daynames[i], 3);
                dayshort[3] = 0;
                if (i == 0)
                    printf("gsave 1 0 0 K\n100 2270 m (%s) S\n400 2270 m "
                           "gsave ptc %s grestore\ngrestore\n", dayshort, PSbigmchar[15]);
                else if (i == 6)
                    printf("gsave 0 0.8 0 K\n3700 2270 m (%s) S\n4000 2270 m "
                           "gsave ptc %s grestore\ngrestore\n", dayshort, PSbigmchar[i]);
                else
                {
                    int posx = i * 600 + 100;
                    printf("%d 2270 m (%s) S\n%d 2270 m gsave ptc "
                           "%s grestore\n", posx, dayshort,
                           (posx + 300), PSbigmchar[i]);
                }
            }
            printf("%% Days\n17 1 SF\n/fsc 7.5 def\n");
        }
        else
        {
            printf("%% Week heading\n0 0 m Ymh\n");
            printf("%% Days\n9 0 SF\n");
        }
    }
    /* At most can be six weeks */
    int w;
    char cdayname[21];
    for (w = 0; w < nWeeks; w++)
    {
        if (pmode == 1)
        {
            printf("<tr align=\"right\">\n");
        }
        if (pmode == 3)
        {
            printf("<ccal:week>\n");
        }
        for (i = 0; i < 7; i++)
        {
            if (pmode == 1)
            {
                if (i == 0)
                    printf("<td width=\"15%%\"><font color=\"#FF0000\">");
                else if (i == 6)
                    printf("<td width=\"15%%\"><font color=\"#00E600\">");
                else
                    printf("<td width=\"14%%\">");
            }
            if (pmode == 3)
            {
                printf("<ccal:day ");
            }
            if (dcnt > daysinmonth[month - 1])
            {
                if (pmode == 1)
                {
                    if (i == 0 || i == 6)
                        printf("&#160;</font></td>\n");
                    else
                        printf("&#160;</td>\n");
                    continue;
                }
                if (pmode == 3)
                {
                    printf("/>\n");
                    continue;
                }
                break;
            }
            if (w == 0)
            {
                if (i < dofw)
                {
                    if (pmode == 0)
                        printf("%10s", " ");
                    else if (pmode == 1)
                    {
                        if (i == 0 || i == 6)
                            printf("&#160;</font></td>\n");
                        else
                            printf("&#160;</td>\n");
                    }
                    else if (pmode == 3)
                    {
                        printf("/>\n");
                    }
                    continue;
                }
            }
            if (dcnt == 1 || ldcnt == 1)
            {
                int mcnt = moncnt;
                if (ldcnt != 1)
                    mcnt --;
                if (mcnt >= 0)
                    GetMonthNumber(vmonth[mcnt], cmonth, leap);
                else
                    GetMonthNumber(lastmon, cmonth, leap);
                if (nEncoding != 'a')
                {
                    if (mcnt >= 0)
                        Number2MonthCH(vmonth[mcnt], 1, 30, nEncoding, cmonname);
                    else
                        Number2MonthCH(lastmon, 1, 30, nEncoding, cmonname);
                    char *p = strstr(cmonname, (*CHmiscchar)[14]) + nCHchars;
                    *p = 0;
                }
            }
#ifdef HILIGHTTODAY
            if (today->tm_year + 1900 == (int)year && today->tm_mon + 1 == (int)month && today->tm_mday == (int)dcnt)
            {
            	if (pmode == 0)
                    printf(ANSI_REV);
            }
#endif
            if (pmode == 0 || pmode == 1)
                printf("%2d", dcnt);
            else if (pmode == 3)
            {
                printf("value=\"%d\" cmonth=\"%d\" leap=\"%s\" cdate=\"%d\" ", dcnt, cmonth, leap, ldcnt);
                Number2DayCH(ldcnt, nEncoding, cdayname);
            }
            else if (bSingle)
            {
                int posx = i * 600 + 50;
                int posy = 1875 - w * 375 + 220;
                if (nWeeks == 5)
                	posy = 1800 - w * 450 + 295;
                if (i == 0)
                    printf("gsave 1 0 0 K\n");
                else if (i == 6)
                    printf("gsave 0 0.8 0 K\n");
                printf("%d %d m (%2d) S\ngsave 8 0 SF ", posx, posy, dcnt);
                posx += 170;
                if (dcnt == 1 || ldcnt == 1)
                {
                    if (*leap == 'R')
                        printf("%d %d m (%dR.%d) S grestore\n%d %d m",
                            (posx + 5), (posy + 56), cmonth, ldcnt, posx,
                            (posy - 2));
                    else
                        printf("%d %d m (%d.%d) S grestore\n%d %d m",
                            (posx + 5), (posy + 56), cmonth, ldcnt, posx,
                            (posy - 2));
                }
                else
                    printf("%d %d m (%2d) S grestore\n%d %d m",
                        (posx + 5), (posy + 56), ldcnt, posx, (posy - 2));
                printf(" gsave ptc");
            }
            else
            {
                int posx = i * 19;
                int posy = -w * 19 - 14;
                if (i == 0)
                    printf("gsave 1 0 0 K\n");
                else if (i == 6)
                    printf("gsave 0 0.8 0 K\n");
                printf("%d %d moveto (%2d) S\n", posx, posy, dcnt);
                posy -= 7;
                printf("%d %d moveto gsave ptc", posx, posy);
            }
			if (!sameday && (termcnt >= int(vterms.size()) || jdcnt != vterms[termcnt]) && (moncnt >= int(vmoons.size()) || jdcnt != vmoons[moncnt]))
            {
                if (pmode == 0)
                {
                    if (nEncoding == 'a')
                        printf(" [%2d]   ", ldcnt);
                    else
                    {
                        Number2DayCH(ldcnt, nEncoding, cdayname);
                        printf(" %s   ", cdayname);
                    }
                }
                else if (pmode == 1)
                {
                    if (dcnt == 1)
                        printf(" %s", cmonname);
                    else
                        printf(" ");
                    Number2DayCH(ldcnt, nEncoding, cdayname);
                    int nlen = (int)strlen(cdayname);
                    printf("%s", cdayname);
                    if (i == 0 || i == 6)
                        printf("</font>");
                    if (nlen == 2 * nCHchars && dcnt != 1)
                        printf("&#160;&#160;</td>\n");
                    else
                        printf("</td>\n");
                }
                else if (pmode == 2 && bSingle)
                {
                    if (dcnt == 1)
                    {
                        if (moncnt > 0)
                            Number2MonthPS(vmonth[moncnt - 1], 1, 30, true, cmonname);
                        else
                            Number2MonthPS(lastmon, 1, 30, true, cmonname);
                        printf(" %s", cmonname);
                    }
                    else
                        printf(" ");
                    Number2DayPS(ldcnt, cdayname);
                    printf("%s\n", cdayname);
                    if (i == 0 || i == 6)
                        printf("grestore\n");
                    printf("grestore\n");
                }
                else if (pmode == 2)
                {
                    Number2DayPS(ldcnt, cdayname);
                    int nlen = (int)strlen(cdayname);
                    if (nlen > 8)
                        printf(" grestore -%d 0 rmoveto gsave ptc",
                               ((nlen - 8) / 4 * 3));
                    printf(" %s\n", cdayname);
                    if (i == 0 || i == 6)
                        printf("grestore\n");
                    printf("grestore\n");
                }
            }
            else if (sameday)
            {
                if (pmode == 0)
                {
                    if (nEncoding == 'a')
                        PrintMonthNumber(vmonth[moncnt++]);
                    else
                    {
                        Number2MonthCH(vmonth[moncnt++], 1, 30, nEncoding, cmonname);
                        char *p = strstr(cmonname, (*CHmiscchar)[14]) + nCHchars;
                        *p = 0;
                        int nlen = (int)strlen(cmonname);
                        if (nlen <= 3 * nCHchars)
                            printf(" ");
                        printf("%s", cmonname);
                        if (nlen == 2 * nCHchars)
                            printf("   ");
                        if (nlen == 3 * nCHchars)
                            printf(" ");
                    }
                }
                else if (pmode == 1)
                {
                    Number2DayCH(ldcnt, nEncoding, cdayname);
                    int nlen = (int)strlen(cdayname);
                    printf(" %s", cdayname);
                    if (i == 0 || i == 6)
                        printf("</font>");
                    if (nlen == 2 * nCHchars)
                        printf("&#160;&#160;</td>\n");
                    else
                        printf("</td>\n");
                }
                else if (pmode == 2)
                {
                    Number2DayPS(ldcnt, cdayname);
                    printf(" %s\n", cdayname);
                    if (i == 0 || i == 6)
                        printf("grestore\n");
                    printf("grestore\n");
                }
                sameday = false;
            }
            else if (termcnt < int(vterms.size()) && jdcnt == vterms[termcnt])
            {
                if (moncnt < int(vmoons.size()) && jdcnt == vmoons[moncnt])
                    sameday = true;
                if (pmode == 0)
                {
                    if (nEncoding == 'a')
                        printf(" [%s]   ", jieqi[termcnt++]);
                    else
                    {
                        printf(" %s   ", (*CHjieqi)[termcnt++]);
                    }
                }
                else if (pmode == 1)
                {
                    if (sameday)
                        printf(" %s", cmonname);
                    else
                        printf(" ");
                    printf("%s", (*CHjieqi)[termcnt++]);
                    if (i == 0 || i == 6)
                        printf("</font>");
                    if (!sameday)
                        printf("&#160;&#160;</td>\n");
                    else
                        printf("</td>\n");
                }
                else if (pmode == 3)
                {
                    strcpy(cdayname, (*CHjieqi)[termcnt++]);
                }
                else if (pmode == 2)
                {
                    if (sameday)
                    {
                        Number2MonthPS(vmonth[moncnt++], 1, 30, true, cmonname);
                        int nlen = (int)strlen(cmonname);
                        if (!bSingle)
                            printf(" grestore -6 0 rmoveto gsave ptc");
                        if (!bSingle && nlen > 8)
                        {
                            double fac = 16.0 / (nlen + 8.0);
                            printf(" gsave %f 1 scale %s", fac, cmonname);
                        }
                        else
                            printf(" %s", cmonname);
                    }
                    else
                        printf(" ");
                    printf("%s\n", PSjieqi[termcnt++]);
                    if (sameday && !bSingle && strlen(cmonname) > 8)
                        printf("grestore\n");
                    if (i == 0 || i == 6)
                        printf("grestore\n");
                    printf("grestore\n");
                }
            }
            else
            {
                if (pmode == 0)
                {
                    if (nEncoding == 'a')
                        PrintMonthNumber(vmonth[moncnt++]);
                    else
                    {
                        Number2MonthCH(vmonth[moncnt++], 1, 30, nEncoding, cmonname);
                        char *p = strstr(cmonname, (*CHmiscchar)[14]) + nCHchars;
                        *p = 0;
                        int nlen = (int)strlen(cmonname);
                        if (nlen <= 3 * nCHchars)
                            printf(" ");
                        printf("%s", cmonname);
                        if (nlen == 2 * nCHchars)
                            printf("   ");
                        if (nlen == 3 * nCHchars)
                            printf(" ");
                    }
                }
                else if (pmode == 1 || pmode == 3)
                {
                    Number2MonthCH(vmonth[moncnt++], 1, 30, nEncoding, cmonname);
                    char *p = strstr(cmonname, (*CHmiscchar)[14]) + nCHchars;
                    *p = 0;
                    if (pmode == 1)
                    {
                        int nlen = (int)strlen(cmonname);
                        printf(" %s", cmonname);
                        if (i == 0 || i == 6)
                            printf("</font>");
                        if (nlen == 2 * nCHchars)
                            printf("&#160;&#160;</td>\n");
                        else
                            printf("</td>\n");
                    }
                }
                else if (pmode == 2 && bSingle)
                {
                    Number2MonthPS(vmonth[moncnt++], 1, 30, true, cmonname);
                    printf(" %s\n", cmonname);
                    if (i == 0 || i == 6)
                        printf("grestore\n");
                    printf("grestore\n");
                }
                else if (pmode == 2)
                {
                    Number2MonthPS(vmonth[moncnt++], 1, 30, true, cmonname);
                    int nlen = (int)strlen(cmonname);
                    if (nlen > 8)
                        printf(" grestore -%d 0 rmoveto gsave ptc",
                               ((nlen - 8) / 4 * 3));
                    printf(" %s\n", cmonname);
                    if (i == 0 || i == 6)
                        printf("grestore\n");
                    printf("grestore\n");
                }
            }
            if (pmode == 3)
                printf("cmonthname=\"%s\" cdatename=\"%s\" />\n", cmonname, cdayname);
#ifdef HILIGHTTODAY
            if (today->tm_year + 1900 == (int)year && today->tm_mon + 1 == (int)month && today->tm_mday == (int)dcnt)
            {
            	if (pmode == 0)
                    printf(ANSI_NORMAL);
            }
#endif
            dcnt++;
            jdcnt++;
            if (moncnt < int(vmoons.size()) && jdcnt == vmoons[moncnt])
                ldcnt = 1;
            else
                ldcnt++;
        }
        if (pmode == 0)
        {
            printf("\n");
        }
        else if (pmode == 1)
        {
            printf("</tr>\n");
        }
        else if (pmode == 3)
        {
            printf("</ccal:week>\n");
        }
    }
    if (pmode == 2 && !bSingle)
    {
        printf("grestore\n");
    }
    else if (pmode == 3)
    {
        printf("</ccal:month>\n");
    }
}

bool ProcessArg(int argc, char** argv, short int& year, short int& month,
                int& pmode, bool& bSingle, int& nEncoding)
{
    if (argc > 6)
        return false;
    pmode = 0;
    bSingle = true;
    nEncoding = 'a';
    bool bIsUTF8 = false;
    if (argc == 1)
        return true;
    int i;
    int nParam = 0;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] != '-')
        {
            if (nParam == 0)
            {
                year = atoi(argv[i]);
                bSingle = false;
                nParam++;
            }
            else if (nParam == 1)
            {
                month = year;
                year = atoi(argv[i]);
                bSingle = true;
                nParam++;
            }
            else
            {
                printf("ccal: Too many parameters.\n");
                return false;
            }
        }
        else if (argv[i][1] == 'x')
            pmode = 3;
        else if (argv[i][1] == 'p')
            pmode = 2;
        else if (argv[i][1] == 't')
            pmode = 1;
        else if (argv[i][1] == 'g')
            nEncoding = 'g';
        else if (argv[i][1] == 'b')
            nEncoding = 'b';
        else if (argv[i][1] == 'u')
            bIsUTF8 = true;
        else
        {
            printf("ccal: Unrecognized option.\n");
            return false;
        }
    }
    if (pmode != 2)
    {
        if (bIsUTF8 || pmode == 3)
        {
            if (nEncoding == 'b')
                SetU8Characters(false);
            nEncoding = 'u';
        }
    }
    if (pmode != 0)
    {
        if (nEncoding == 'a')
            nEncoding = 'g';
    }
    return true;
}

int main(int argc, char** argv)
{
    time_t now = time(NULL);
    struct tm *tmnow = localtime(&now);
    short int year, month;
    int pmode;
    bool bSingle;
    int nEncoding;
    year = (short int) (tmnow->tm_year + 1900);
    month = (short int) (tmnow->tm_mon + 1);
    if (!ProcessArg(argc, argv, year, month, pmode, bSingle, nEncoding))
    {
        printf("ccal version %s: Displays Chinese calendar (Gregorian with Chinese dates).\n", versionstr);
        printf("Usage: ccal [-t|-p|-x] [-g|-b] [-u] [[<month>] <year>].\n");
        printf("\t-t:\tGenerates HTML table output.\n");
        printf("\t-p:\tGenerates encapsulated PostScript output.\n");
        printf("\t-x:\tGenerates XML output.\n");
        printf("\t-g:\tGenerates simplified Chinese output.\n");
        printf("\t-b:\tGenerates traditional Chinese output.\n");
        printf("\t-u:\tUses UTF-8 rather than GB or Big5 for Chinese output.\n");
        exit(1);
    }
    if (month < 1 || month > 12)
    {
        printf("ccal: Invalid month value: month 1-12.\n");
        exit(1);
    }
    if (year < 1645 || year > 7000)
    {
        printf("ccal: Invalid year value: year 1645-7000.\n");
        exit(1);
    }
    if (IsLeapYear(year))
        daysinmonth[1] = 29;
    vdouble vterms, vmoons, vmonth;
    double lastnew, lastmon, nextnew;
    double lmon = lunaryear(year, vterms, lastnew, lastmon, vmoons, vmonth, nextnew);
    bool bIsSim = (nEncoding == 'g');
    char titlestr[20];
    if (pmode == 3)
    {
        printf("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
        printf("<ccal:year value=\"%d\" xmlns:ccal=\"http://ccal.chinesebay.com/ccal/\">\n", year);
    }
    if (bSingle)
    {
        sprintf(titlestr, "%s %d", monnames[month - 1], year);
        if (pmode == 1)
            PrintHeaderHTML(titlestr, month, year, nEncoding);
        else if (pmode == 2)
            PrintHeaderPS(titlestr, bIsSim, false);
        PrintMonth(year, month, vterms, lastnew, lastmon, vmoons, vmonth, nextnew, pmode, bSingle, nEncoding,
                   ((short int)(lmon) == month || (short int)(lmon + 0.9) == month));
    }
    else
    {
        sprintf(titlestr, "Year %d", year);
        if (pmode == 1)
            PrintHeaderHTML(titlestr, 0, year, nEncoding);
        else if (pmode == 2)
            PrintHeaderPS(titlestr, bIsSim, (lmon != 0.0));
        short int i;
        for (i = 1; i <= 12; i++)
            PrintMonth(year, i, vterms, lastnew, lastmon, vmoons, vmonth, nextnew, pmode, bSingle, nEncoding, false);
    }
    if (pmode == 1)
        PrintClosingHTML();
    else if (pmode == 2)
        PrintClosingPS();
    else if (pmode == 3)
        printf("</ccal:year>\n");
    return 0;
}

