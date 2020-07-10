#!/bin/sh

echo -e `date '+%Y-%m-%d %H:%M:%S %A'` >/www/JD_DailyBonus.htm 2>/dev/null
nohup node /usr/share/jd-dailybonus/JD_DailyBonus.js >>/www/JD_DailyBonus.htm 2>/dev/null &