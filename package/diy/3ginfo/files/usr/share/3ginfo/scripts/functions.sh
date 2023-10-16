pdu2txt() {
# pobrany z https://forum.openwrt.org/viewtopic.php?pid=158891#p158891

	data1=$(echo $1 | sed -e 's/.\{2\}/&\ /g')

	ret=''
	shift=0
	carry=0

	for byte in $data1; do
		if [ $shift -eq 7 ]; then
			ret=$ret$(echo $carry | awk '{printf("%c",$0)}')
			carry=0
			shift=0
		fi

		byte=$((0x$byte))

		: $(( a = (0xFF >> ($shift + 1)) & 0xFF ))
		: $(( b = $a ^ 0xFF ))

		: $(( digit = $carry | (($byte & $a) << $shift) & 0xFF ))
		: $(( carry = ($byte & $b) >> (7 - $shift) ))

		ret=$ret$(echo $digit | awk '{printf("%c",$0)}')

		: $(( shift++ ))
	done

	echo $ret
}
