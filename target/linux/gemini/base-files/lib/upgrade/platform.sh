REQUIRE_IMAGE_METADATA=1
MTDSYSFS=/sys/class/mtd

gemini_check_redboot_parts() {
	MTD1OF=`cat ${MTDSYSFS}/mtd1/offset`
	MTD2OF=`cat ${MTDSYSFS}/mtd2/offset`
	MTD3OF=`cat ${MTDSYSFS}/mtd3/offset`
	MTD4OF=`cat ${MTDSYSFS}/mtd4/offset`
	MTD1SZ=$((${MTD2OF} - ${MTD1OF}))
	MTD2SZ=$((${MTD3OF} - ${MTD2OF}))
	MTD3SZ=$((${MTD4OF} - ${MTD3OF}))
	ESZ=`cat ${MTDSYSFS}/mtd1/erasesize`
	if test ${ESZ} == 131072 ; then
		echo "MTD has 128kb EB size"
	else
		echo "MTD has wrong EB size!"
	fi
	KSZ=$(($ESZ * $2))
	RSZ=$(($ESZ * $3))
	ASZ=$(($ESZ * $4))
	NAME=`cat ${MTDSYSFS}/mtd1/name`
	if test "x${NAME}" == "xKern" ; then
		if test ${MTD1SZ} == ${KSZ} ; then
			echo "MTD1 Kern ${MTD1SZ} OK..."
		else
			echo "MTD1 is wrong size, aborting" >&2
			exit 1
		fi
	else
		echo "MTD1 has wrong name, aborting" >&2
		exit 1
	fi
	NAME=`cat ${MTDSYSFS}/mtd2/name`
	if test "x${NAME}" == "xRamdisk" ; then
		if test ${MTD2SZ} == ${RSZ} ; then
			echo "MTD2 Ramdisk ${MTD2SZ} OK..."
		else
			echo "MTD2 is at wrong offset, aborting" >&2
			exit 1
		fi
	else
		echo "MTD2 has wrong name, aborting" >&2
		exit 1
	fi
	NAME=`cat ${MTDSYSFS}/mtd3/name`
	if test "x${NAME}" == "xApplication" ; then
		if test ${MTD3SZ} == ${ASZ} ; then
			echo "MTD3 Application ${MTD3SZ} OK..."
		else
			echo "MTD3 is at wrong offset, aborting" >&2
			exit 1
		fi
	else
		echo "MTD3 has wrong name, aborting" >&2
		exit 1
	fi
}

# This converts the old RedBoot partitioning to the new shared
# "firmware" partition.
gemini_do_flat_redboot_upgrade() {
	ESZ=131072
	KSZ=$(($ESZ * $2))
	RSZ=$(($ESZ * $3))
	KRSZ=$(($KSZ + $RSZ))
	ASZ=$(($ESZ * $4))
	echo "Partition sizes: Kern ${KSZ}, Ramdisk ${RSZ}, Application ${ASZ}"
	echo "Extract Kern from flat image ${1}"
	echo "Write Kern from flat image ${1}"
	dd if="$1" bs=1 count=${KSZ} | mtd write - Kern
	echo "Write rd.gz from flat image ${1}"
	dd if="$1" bs=1 skip=${KSZ} count=${RSZ} | mtd write - Ramdisk
	echo "Write hddapp.tgz from flat image ${1}"
	dd if="$1" bs=1 skip=${KRSZ} count=${ASZ} | mtd write - Application
}

# Check if we have the new partition scheme, else do it the old
# way.
gemini_do_combined_upgrade() {
	NAME=`cat ${MTDSYSFS}/mtd1/name`
	if test "x${NAME}" == "xfirmware" ; then
		PART_NAME=firmware
		default_do_upgrade "$1"
	else
		gemini_check_redboot_parts "$1" $2 $3 $4
		gemini_do_flat_redboot_upgrade "$1" $2 $3 $4
	fi
}

platform_check_image() {
	local board=$(board_name)

	case "$board" in
	dlink,dir-685|\
	itian,sq201|\
	storlink,gemini324|\
	raidsonic,ib-4210-b|\
	raidsonic,ib-4220-b|\
	verbatim,s08v1901-d1)
		return 0
		;;
	esac

	echo "Sysupgrade is not yet supported on $board."
	return 1
}

platform_do_upgrade() {
	local board=$(board_name)

	case "$board" in
	dlink,dir-685)
		PART_NAME=firmware
		default_do_upgrade "$1"
		;;
	raidsonic,ib-4210-b|\
	raidsonic,ib-4220-b|\
	storlink,gemini324|\
	verbatim,s08v1901-d1)
		gemini_do_combined_upgrade "$1" 24 48 48
		;;
	itian,sq201)
		gemini_do_combined_upgrade "$1" 16 48 48
		;;
	esac
}
