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
/* For calendar in Encapsulated Postscript form with PS characters */
#ifndef PSMONTH_H
#define PSMONTH_H

void MonthStartPS(int nstart, char *monstart);

/* Input:
   month, month number with .5 indicating leap month
   nstart, day of calendar month that starts this lunar month
   ndays, number of days in this lunar month
   bYear, whether to be used in yearly or monthly calendar
   Output:
   monname, C string containing the PS characters for the month.
            monname should be allocated at least 21 bytes before the call
*/
void Number2MonthPS(double month, int nstart, int ndays, bool bYear, char *monname);

/* Input:
   nday, day number (1 - 30)
   Output:
   dayname, C string containing the PS characters for the day.
            dayname should be allocated at least 9 bytes before the call
*/
void Number2DayPS(int nday, char *dayname);

/* Input:
   titlestr, string to be included in the title, normally either
             Year XXXX or Month Year
   bIsSim: true for simplified characters, false for traditional characters
   bNeedsRun: true if character Run is needed, false otherwise
*/
void PrintHeaderPS(char *titlestr, bool bIsSim, bool bNeedsRun);

/* Inputs:
   monthhead, string header for the monthly calendar if bSingle is
              true, don't care otherwise
   month, month number for the monthly calendar if bSingle is
          true, don't care otherwise
   bSingle, true for single month, false for whole year
   bIsSim: true for simplified characters, false for traditional characters
   bNeedsRun: true if character Run is needed, false otherwise
   nWeeks: number of week lines in month
*/
void PrintHeaderMonthPS(char *monthhead, short int month, bool bSingle,
                        bool bIsSim, bool bNeedsRun, int nWeeks);

void PrintClosingPS();

#endif /*PSMONTH_H*/
