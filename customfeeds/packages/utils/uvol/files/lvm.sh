#!/bin/sh

cmd="$1"
shift

if [ "$cmd" = "name" ]; then
	echo "LVM"
	return 0
fi

command -v lvm >/dev/null || return 1

. /lib/functions.sh
. /lib/upgrade/common.sh
. /usr/share/libubox/jshn.sh

export_bootdevice
[ "$BOOTDEV_MAJOR" ] || return 1
export_partdevice rootdev 0
[ "$rootdev" ] || return 1

case "$rootdev" in
	mtd*|\
	ram*|\
	ubi*)
		return 1
esac

lvm_cmd() {
	local cmd="$1"
	shift
	LVM_SUPPRESS_FD_WARNINGS=1 lvm "$cmd" "$@"
	return $?
}

pvs() {
	lvm_cmd pvs --reportformat json --units b "$@"
}

vgs() {
	lvm_cmd vgs --reportformat json --units b "$@"
}

lvs() {
	lvm_cmd lvs --reportformat json --units b "$@"
}

freebytes() {
	echo $((vg_free_count * vg_extent_size * 1024))
}

totalbytes() {
	echo $((vg_extent_count * vg_extent_size * 1024))
}

existvol() {
	[ "$1" ] || return 1
	test -e "/dev/$vg_name/ro_$1" || test -e "/dev/$vg_name/rw_$1"
	return $?
}

vg_name=
exportpv() {
	local reports rep pv pvs
	vg_name=
	json_init
	json_load "$(pvs -o vg_name -S "pv_name=~^/dev/$rootdev.*\$")"
	json_select report
	json_get_keys reports
	for rep in $reports; do
		json_select "$rep"
		json_select pv
		json_get_keys pvs
		for pv in $pvs; do
			json_select "$pv"
			json_get_vars vg_name
			json_select ..
			break
		done
		json_select ..
		break
	done
}

vg_extent_size=
vg_extent_count=
vg_free_count=
exportvg() {
	local reports rep vg vgs
	vg_extent_size=
	vg_extent_count=
	vg_free_count=
	json_init
	json_load "$(vgs -o vg_extent_size,vg_extent_count,vg_free_count -S "vg_name=$vg_name")"
	json_select report
	json_get_keys reports
	for rep in $reports; do
		json_select "$rep"
		json_select vg
		json_get_keys vgs
		for vg in $vgs; do
			json_select "$vg"
			json_get_vars vg_extent_size vg_extent_count vg_free_count
			vg_extent_size=${vg_extent_size%B}
			json_select ..
			break
		done
		json_select ..
		break
	done
}

lv_active=
lv_name=
lv_full_name=
lv_path=
lv_dm_path=
lv_size=
exportlv() {
	local reports rep lv lvs
	lv_active=
	lv_name=
	lv_full_name=
	lv_path=
	lv_dm_path=
	lv_size=
	json_init

	json_load "$(lvs -o lv_active,lv_name,lv_full_name,lv_size,lv_path,lv_dm_path -S "lv_name=~^[rw][owp]_$1\$ && vg_name=$vg_name")"
	json_select report
	json_get_keys reports
	for rep in $reports; do
		json_select "$rep"
		json_select lv
		json_get_keys lvs
		for lv in $lvs; do
			json_select "$lv"
			json_get_vars lv_active lv_name lv_full_name lv_size lv_path lv_dm_path
			lv_size=${lv_size%B}
			json_select ..
			break
		done
		json_select ..
		break
	done
}

getdev() {
	existvol "$1" || return 1
	exportlv "$1"
	echo "$lv_dm_path"
}

getsize() {
	exportlv "$1"
	[ "$lv_size" ] && echo "$lv_size"
}

activatevol() {
	exportlv "$1"
	[ "$lv_path" ] || return 2
	case "$lv_path" in
		/dev/*/wo_*|\
		/dev/*/wp_*)
			return 22
			;;
		*)
			[ "$lv_active" = "active" ] && return 0
			lvm_cmd lvchange -k n "$lv_full_name" || return $?
			lvm_cmd lvchange -a y "$lv_full_name" || return $?
			ubus send block.volume "{\"name\": \"$1\", \"action\": \"up\", \"mode\": \"${lv_name:0:2}\", \"device\": \"$lv_dm_path\"}"
			return 0
			;;
	esac
}

disactivatevol() {
	exportlv "$1"
	[ "$lv_path" ] || return 2
	case "$lv_path" in
		/dev/*/wo_*|\
		/dev/*/wp_*)
			return 22
			;;
		*)
			[ "$lv_active" = "active" ] || return 0
			lvm_cmd lvchange -a n "$lv_full_name" || return $?
			lvm_cmd lvchange -k y "$lv_full_name" || return $?
			ubus send block.volume "{\"name\": \"$1\", \"action\": \"down\", \"mode\": \"${lv_name:0:2}\", \"device\": \"$lv_dm_path\"}"
			return 0
			;;
	esac
}

getstatus() {
	exportlv "$1"
	[ "$lv_full_name" ] || return 2
	existvol "$1" || return 1
	return 0
}

createvol() {
	local mode lvmode ret
	local volsize=$(($2))
	[ "$volsize" ] || return 22
	exportlv "$1"
	[ "$lv_size" ] && return 17
	size_ext=$((volsize / vg_extent_size))
	[ $((size_ext * vg_extent_size)) -lt $volsize ] && size_ext=$((size_ext + 1))

	case "$3" in
		ro|wo)
			lvmode=r
			mode=wo
			;;
		rw)
			lvmode=rw
			mode=wp
			;;
		*)
			return 22
			;;
	esac

	lvm_cmd lvcreate -p "$lvmode" -a n -y -W n -Z n -n "${mode}_$1" -l "$size_ext" "$vg_name"
	ret=$?
	if [ ! $ret -eq 0 ] || [ "$lvmode" = "r" ]; then
		return $ret
	fi
	exportlv "$1"
	[ "$lv_full_name" ] || return 22
	lvm_cmd lvchange -a y "$lv_full_name" || return 1
	if [ "$lv_size" -gt $(( 100 * 1024 * 1024 )) ]; then
		mkfs.f2fs -f -l "$1" "$lv_path"
		ret=$?
		[ $ret != 0 ] && [ $ret != 134 ] && return 1
	else
		mke2fs -F -L "$1" "$lv_path" || return 1
	fi
	lvm_cmd lvrename "$vg_name" "wp_$1" "rw_$1"
	exportlv "$1"
	ubus send block.volume "{\"name\": \"$1\", \"action\": \"up\", \"mode\": \"${lv_name:0:2}\", \"device\": \"$lv_dm_path\"}"
	return 0
}

removevol() {
	exportlv "$1"
	[ "$lv_full_name" ] || return 2
	lvm_cmd lvremove -y "$lv_full_name"
	ubus send block.volume "{\"name\": \"$1\", \"action\": \"down\", \"mode\": \"${lv_name:0:2}\", \"device\": \"$lv_dm_path\"}"
}

updatevol() {
	exportlv "$1"
	[ "$lv_full_name" ] || return 2
	[ "$lv_size" -ge "$2" ] || return 27
	case "$lv_path" in
		/dev/*/wo_*)
			lvm_cmd lvchange -a y -p rw "$lv_full_name"
			dd of="$lv_path"
			lvm_cmd lvchange -p r "$lv_full_name"
			lvm_cmd lvrename "$lv_full_name" "${lv_full_name%%/*}/ro_$1"
			ubus send block.volume "{\"name\": \"$1\", \"action\": \"up\", \"mode\": \"ro\", \"device\": \"$(getdev "$@")\"}"
			return 0
			;;
		default)
			return 22
			;;
	esac
}

listvols() {
	local reports rep lv lvs lv_name lv_size lv_mode volname
	volname=${1:-.*}
	json_init
	json_load "$(lvs -o lv_name,lv_size -S "lv_name=~^[rw][owp]_$volname\$ && vg_name=$vg_name")"
	json_select report
	json_get_keys reports
	for rep in $reports; do
		json_select "$rep"
		json_select lv
		json_get_keys lvs
		for lv in $lvs; do
			json_select "$lv"
			json_get_vars lv_name lv_size
			lv_mode="${lv_name:0:2}"
			lv_name="${lv_name:3}"
			lv_size=${lv_size%B}
			echo "$lv_name $lv_mode $lv_size"
			json_select ..
		done
		json_select ..
		break
	done
}

boot() {
	local reports rep lv lvs lv_name lv_dm_path lv_mode volname
	json_init
	json_load "$(lvs -o lv_name,lv_dm_path -S "lv_name=~^[rw][ow]_.*\$ && vg_name=$vg_name && lv_active=active")"
	json_select report
	json_get_keys reports
	for rep in $reports; do
		json_select "$rep"
		json_select lv
		json_get_keys lvs
		for lv in $lvs; do
			json_select "$lv"
			json_get_vars lv_name lv_dm_path
			lv_mode="${lv_name:0:2}"
			lv_name="${lv_name:3}"
			ubus send block.volume "{\"name\": \"$lv_name\", \"action\": \"up\", \"mode\": \"$lv_mode\", \"device\": \"$lv_dm_path\"}"
			json_select ..
		done
		json_select ..
		break
	done
}

exportpv
exportvg

case "$cmd" in
	align)
		echo "$vg_extent_size"
		;;
	free)
		freebytes
		;;
	total)
		totalbytes
		;;
	boot)
		boot
		;;
	list)
		listvols "$@"
		;;
	create)
		createvol "$@"
		;;
	remove)
		removevol "$@"
		;;
	device)
		getdev "$@"
		;;
	size)
		getsize "$@"
		;;
	up)
		activatevol "$@"
		;;
	down)
		disactivatevol "$@"
		;;
	status)
		getstatus "$@"
		;;
	write)
		updatevol "$@"
		;;
	*)
		echo "unknown command"
		return 1
		;;
esac
