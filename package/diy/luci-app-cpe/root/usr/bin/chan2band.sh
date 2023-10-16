#!/bin/sh
CHAN=$1
CHAN=$(echo "$CHAN" | grep -o "[0-9]*")

decode_lte() {
	if [ $CHAN -lt 600 ]; then
		BAND="B1"
	elif [ $CHAN -lt 1200 ]; then
		BAND="B2"
	elif [ $CHAN -lt 1950 ]; then
		BAND="B3"
	elif [ $CHAN -lt 2400 ]; then
		BAND="B4"
	elif [ $CHAN -lt 2650 ]; then
		BAND="B5"
	elif [ $CHAN -lt 2750 ]; then
		BAND="B6"
	elif [ $CHAN -lt 3450 ]; then
		BAND="B7"
	elif [ $CHAN -lt 3800 ]; then
		BAND="B8"
	elif [ $CHAN -lt 4150 ]; then
		BAND="B9"
	elif [ $CHAN -lt 4750 ]; then
		BAND="B10"
	elif [ $CHAN -lt 4950 ]; then
		BAND="B11"
	elif [ $CHAN -lt 5010 ]; then
		BAND="-"
	elif [ $CHAN -lt 5180 ]; then
		BAND="B12"
	elif [ $CHAN -lt 5280 ]; then
		BAND="B13"
	elif [ $CHAN -lt 5380 ]; then
		BAND="B14"
	elif [ $CHAN -lt 5730 ]; then
		BAND="-"
	elif [ $CHAN -lt 5850 ]; then
		BAND="B17"
	elif [ $CHAN -lt 6000 ]; then
		BAND="B18"
	elif [ $CHAN -lt 6150 ]; then
		BAND="B19"
	elif [ $CHAN -lt 6450 ]; then
		BAND="B20"
	elif [ $CHAN -lt 6600 ]; then
		BAND="B21"
	elif [ $CHAN -lt 7400 ]; then
		BAND="B22"
	elif [ $CHAN -lt 7500 ]; then
		BAND="-"
	elif [ $CHAN -lt 7700 ]; then
		BAND="B23"
	elif [ $CHAN -lt 8040 ]; then
		BAND="B24"
	elif [ $CHAN -lt 8690 ]; then
		BAND="B25"
	elif [ $CHAN -lt 9040 ]; then
		BAND="B26"
	elif [ $CHAN -lt 9210 ]; then
		BAND="B27"
	elif [ $CHAN -lt 9660 ]; then
		BAND="B28"
	elif [ $CHAN -lt 9770 ]; then
		BAND="B29"
	elif [ $CHAN -lt 9870 ]; then
		BAND="B30"
	elif [ $CHAN -lt 9920 ]; then
		BAND="B31"
	elif [ $CHAN -lt 10400 ]; then
		BAND="B32"
	elif [ $CHAN -lt 36000 ]; then
		BAND="-"
	elif [ $CHAN -lt 36200 ]; then
		BAND="B33"
	elif [ $CHAN -lt 36350 ]; then
		BAND="B34"
	elif [ $CHAN -lt 36950 ]; then
		BAND="B35"
	elif [ $CHAN -lt 37550 ]; then
		BAND="B36"
	elif [ $CHAN -lt 37750 ]; then
		BAND="B37"
	elif [ $CHAN -lt 38250 ]; then
		BAND="B38"
	elif [ $CHAN -lt 38650 ]; then
		BAND="B39"
	elif [ $CHAN -lt 39650 ]; then
		BAND="B40"
	elif [ $CHAN -lt 41590 ]; then
		BAND="B41"
	elif [ $CHAN -lt 43590 ]; then
		BAND="B42"
	elif [ $CHAN -lt 45590 ]; then
		BAND="B43"
	elif [ $CHAN -lt 46590 ]; then
		BAND="B44"
	elif [ $CHAN -lt 46790 ]; then
		BAND="B45"
	elif [ $CHAN -lt 54540 ]; then
		BAND="B46"
	elif [ $CHAN -lt 55240 ]; then
		BAND="B47"
	elif [ $CHAN -lt 56740 ]; then
		BAND="B48"
	elif [ $CHAN -lt 58240 ]; then
		BAND="B49"
	elif [ $CHAN -lt 59090 ]; then
		BAND="B50"
	elif [ $CHAN -lt 59140 ]; then
		BAND="B51"
	elif [ $CHAN -lt 60140 ]; then
		BAND="B52"
	elif [ $CHAN -lt 60255 ]; then
		BAND="B53"
	elif [ $CHAN -lt 65536 ]; then
		BAND="-"
	elif [ $CHAN -lt 66436 ]; then
		BAND="B65"
	elif [ $CHAN -lt 67336 ]; then
		BAND="B66"
	elif [ $CHAN -lt 67536 ]; then
		BAND="B67"
	elif [ $CHAN -lt 67836 ]; then
		BAND="B68"
	elif [ $CHAN -lt 68336 ]; then
		BAND="B69"
	elif [ $CHAN -lt 68586 ]; then
		BAND="B70"
	elif [ $CHAN -lt 68936 ]; then
		BAND="B71"
	elif [ $CHAN -lt 68986 ]; then
		BAND="B72"
	elif [ $CHAN -lt 69036 ]; then
		BAND="B73"
	elif [ $CHAN -lt 69466 ]; then
		BAND="B74"
	elif [ $CHAN -lt 70316 ]; then
		BAND="B75"
	elif [ $CHAN -lt 70366 ]; then
		BAND="B76"
	elif [ $CHAN -lt 70546 ]; then
		BAND="B85"
	elif [ $CHAN -lt 70596 ]; then
		BAND="B87"
	elif [ $CHAN -lt 70646 ]; then
		BAND="B88"
	else
		BAND="-"
	fi
}

decode_nr5g() {
	if [ $CHAN -lt 123400 ]; then
		BAND="-"
	elif [ $CHAN -le 130400 ]; then
		BAND="n71"
	elif [ $CHAN -lt 143400 ]; then
		BAND="-"
	elif [ $CHAN -lt 145600 ]; then
		BAND="n29"
	elif [ $CHAN -eq 145600 ]; then
		BAND="n29|n85"
	elif [ $CHAN -lt 145800 ]; then
		BAND="n85"
	elif [ $CHAN -eq 145800 ]; then
		BAND="n12|n85"
	elif [ $CHAN -lt 147600 ]; then
		BAND="n12|n85"
	elif [ $CHAN -lt 149200 ]; then
		BAND="n12|n67|n85"
	elif [ $CHAN -eq 149200 ]; then
		BAND="n12|n13|n67|n85"
	elif [ $CHAN -le 151200 ]; then
		BAND="n13|n67"
	elif [ $CHAN -lt 151600 ]; then
		BAND="n67"
	elif [ $CHAN -eq 151600 ]; then
		BAND="n14|n28|n67"
	elif [ $CHAN -le 153600 ]; then
		BAND="n14|n28"
	elif [ $CHAN -lt 158200 ]; then
		BAND="n28"
	elif [ $CHAN -eq 158200 ]; then
		BAND="n14|n20|n28"
	elif [ $CHAN -le 160600 ]; then
		BAND="n20|n28"
	elif [ $CHAN -le 164200 ]; then
		BAND="n20"
	elif [ $CHAN -lt 171800 ]; then
		BAND="-"
	elif [ $CHAN -lt 172000 ]; then
		BAND="n26"
	elif [ $CHAN -lt 173800 ]; then
		BAND="n18|n26"
	elif [ $CHAN -le 175000 ]; then
		BAND="n5|n18|n26"
	elif [ $CHAN -le 178800 ]; then
		BAND="n5|n26"
	elif [ $CHAN -lt 185000 ]; then
		BAND="-"
	elif [ $CHAN -le 192000 ]; then
		BAND="n8"
	elif [ $CHAN -lt 285400 ]; then
		BAND="-"
	elif [ $CHAN -lt 286400 ]; then
		BAND="n51|n76|n91|n93"
	elif [ $CHAN -eq 286400 ]; then
		BAND="n50|n51|n75|n76|n91|92|n93|94"
	elif [ $CHAN -lt 295000 ]; then
		BAND="n50|n75|n92|n94"
	elif [ $CHAN -eq 295000 ]; then
		BAND="n50|n74|n75|n92|n94"
	elif [ $CHAN -le 303400 ]; then
		BAND="n50|n74|n75|n92|n94"
	elif [ $CHAN -le 303600 ]; then
		BAND="n74"
	elif [ $CHAN -lt 305000 ]; then
		BAND="-"
	elif [ $CHAN -le 311800 ]; then
		BAND="n24"
	elif [ $CHAN -lt 361000 ]; then
		BAND="-"
	elif [ $CHAN -lt 376000 ]; then
		BAND="n3"
	elif [ $CHAN -eq 376000 ]; then
		BAND="n3|n39"
	elif [ $CHAN -le 384000 ]; then
		BAND="n39"
	elif [ $CHAN -lt 386000 ]; then
		BAND="-"
	elif [ $CHAN -le 398000 ]; then
		BAND="n2|n25"
	elif [ $CHAN -lt 399000 ]; then
		BAND="n25"
	elif [ $CHAN -eq 399000 ]; then
		BAND="n25|n70"
	elif [ $CHAN -lt 402000 ]; then
		BAND="n70"
	elif [ $CHAN -eq 402000 ]; then
		BAND="n34|n70"
	elif [ $CHAN -le 404000 ]; then
		BAND="n34|n70"
	elif [ $CHAN -le 405000 ]; then
		BAND="n34"
	elif [ $CHAN -lt 422000 ]; then
		BAND="-"
	elif [ $CHAN -le 434000 ]; then
		BAND="n1|n65|n66"
	elif [ $CHAN -le 440000 ]; then
		BAND="n65|n66"
	elif [ $CHAN -lt 460000 ]; then
		BAND="-"
	elif [ $CHAN -lt 470000 ]; then
		BAND="n40"
	elif [ $CHAN -eq 470000 ]; then
		BAND="n30|n40"
	elif [ $CHAN -le 472000 ]; then
		BAND="n30|n40"
	elif [ $CHAN -le 480000 ]; then
		BAND="n40"
	elif [ $CHAN -lt 496700 ]; then
		BAND="-"
	elif [ $CHAN -le 499000 ]; then
		BAND="n53"
	elif [ $CHAN -lt 499200 ]; then
		BAND="-"
	elif [ $CHAN -lt 514000 ]; then
		BAND="n41|n90"
	elif [ $CHAN -eq 514000 ]; then
		BAND="n38|n41|n90"
	elif [ $CHAN -lt 524000 ]; then
		BAND="n38|n41|n90"
	elif [ $CHAN -eq 524000 ]; then
		BAND="n7|n38|n41|n90"
	elif [ $CHAN -lt 538000 ]; then
		BAND="n7|n41|n90"
	elif [ $CHAN -eq 538000 ]; then
		BAND="n7|n90"
	elif [ $CHAN -lt 620000 ]; then
		BAND="-"
	elif [ $CHAN -lt 636667 ]; then
		BAND="n77|n78"
	elif [ $CHAN -le 646666 ]; then
		BAND="n48|n77|n78"
	elif [ $CHAN -le 653333 ]; then
		BAND="n77|n78"
	elif [ $CHAN -le 680000 ]; then
		BAND="n77"
	elif [ $CHAN -lt 693334 ]; then
		BAND="-"
	elif [ $CHAN -le 733333 ]; then
		BAND="n79"
	elif [ $CHAN -lt 743333 ]; then
		BAND="-"
	elif [ $CHAN -lt 795000 ]; then
		BAND="n46"
	elif [ $CHAN -eq 795000 ]; then
		BAND="n46|n96"
	elif [ $CHAN -le 875000 ]; then
		BAND="n96"
	else
		BAND="-"
	fi
}

if [ -z "$CHAN" ]; then
	BAND="-"
elif [ "$CHAN" -lt 123400 ]; then
	decode_lte
elif [ "$CHAN" -le 875000 ]; then
	decode_nr5g
else
	BAND="-"
fi
echo $BAND
exit
