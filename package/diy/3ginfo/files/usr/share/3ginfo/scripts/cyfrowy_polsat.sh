#!/bin/sh
O=$(wget -q -c http://www.cyfrowypolsat.pl/internet/biezace-zuzycie/ -O -)
O1=$(echo "$O" | sed -n '/Okres rozliczeniowy/,/_idJsp27/p' | tr -d '\n' | sed -e 's/\t//g;s/  */ /g;s/.*\(<span class="bolder">Okres.*\)\(<br><br><span\).*/\1/' | tr '\277\363\263\346\352\266\261\274\361' zolcesazn |  tr '\241\306\312\243\321\323\246\257\254' ACELNOSZZ)
if [ -z "$O1" ]; then
	echo "<div class=\"c\">Brak możliwości sprawdzenia limitu.</div>"
else
	echo "$O1"
fi

