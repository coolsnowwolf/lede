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
/* For calendar in HTML form with Chinese characters */
#ifndef HTMLMONTH_H
#define HTMLMONTH_H

/* Input:
   bIsSim: true--set simplified characters, false--set traditional characters
*/
void SetU8Characters(bool bIsSim);

/* Input:
   month, month number with .5 indicating leap month
   nstart, day of calendar month that starts this lunar month
   ndays, number of days in this lunar month
   nEncoding: 'g' for GB, 'b' for BIG5, 'u' for UTF-8
   Output:
   monname, C string containing the Chinese characters for the month.
            monname should be allocated at least 24 bytes before the call
*/
void Number2MonthCH(double month, int nstart, int ndays, int nEncoding,
                    char *monname);

/* Input:
   nday, day number (1 - 30)
   nEncoding: 'g' for GB, 'b' for BIG5, 'u' for UTF-8
   Output:
   dayname, C string containing the Chinese characters for the day.
            dayname should be allocated at least 7 bytes before the call
*/
void Number2DayCH(int nday, int nEncoding, char *dayname);

/* Input:
   titlestr, string to be included in the English part of title, normally either
             Year XXXX or Month Year
   month, Gregorian month number (1 - 12)
   year, Gregorian year number, between 1645 and 9999
   nEncoding: 'g' for GB, 'b' for BIG5, 'u' for UTF-8
*/
void PrintHeaderHTML(char *titlestr, int month, int year, int nEncoding = 'g');

void PrintClosingHTML();

#endif /*HTMLMONTH_H*/
