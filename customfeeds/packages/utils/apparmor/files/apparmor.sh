#!/bin/sh

log_write() {
	local facility=kern.$1
	logger -t AppArmor -p $facility "$2"
}

AA_STATUS=/usr/sbin/aa-status
SECURITYFS=/sys/kernel/security
SFS_MOUNTPOINT="${SECURITYFS}/apparmor"
PARSER=/sbin/apparmor_parser
PARSER_OPTS=
ADDITIONAL_PROFILE_DIR=

[ -d /etc/apparmor.d ] && PROFILE_DIRS=/etc/apparmor.d ||
	log_write warning "Unable to find profiles: /etc/apparmor.d"

[ -n "$ADDITIONAL_PROFILE_DIR" ] && [ -d "$ADDITIONAL_PROFILE_DIR" ] &&
	PROFILE_DIRS="$PROFILE_DIRS $ADDITIONAL_PROFILE_DIR"

dir_is_empty() {
	[ "$(du -s $1 | cut -f 1)" -eq 0 ] && return 0 || return 1
}

profiles_loaded_count() {

	[ -f ${SFS_MOUNTPOINT}/profiles ] &&
		return $(cat "${SFS_MOUNTPOINT}/profiles" | wc -l) || return 0
}

is_profiles_loaded() {

	[ -f ${SFS_MOUNTPOINT}/profiles ] && {
		rc=$(cat "${SFS_MOUNTPOINT}/profiles" | wc -l)
		[ "$rc" -ne 0 ] && return 0 || return 1
	}
	return 1
}

is_container_with_internal_policy() {

	local ns_stacked_path="${SFS_MOUNTPOINT}/.ns_stacked"
	local ns_name_path="${SFS_MOUNTPOINT}/.ns_name"
	local ns_stacked
	local ns_name

	if ! [ -f "$ns_stacked_path" ] || ! [ -f "$ns_name_path" ]; then
		return 1
	fi

	read -r ns_stacked < "$ns_stacked_path"
	if [ "$ns_stacked" != "yes" ]; then
		return 1
	fi

	# LXD and LXC set up AppArmor namespaces starting with "lxd-" and
	# "lxc-", respectively. Return non-zero for all other namespace
	# identifiers.

	read -r ns_name < "$ns_name_path"
	if [ "${ns_name#lxd-*}" = "$ns_name" ] && \
	   [ "${ns_name#lxc-*}" = "$ns_name" ]; then
		return 1
	fi

	return 0
}

skip_profile() {

	local profile="$1"

	if [ "${profile%.rpmnew}"   != "$profile" ] || \
	   [ "${profile%.rpmsave}"  != "$profile" ] || \
	   [ "${profile%.orig}"     != "$profile" ] || \
	   [ "${profile%.rej}"      != "$profile" ] || \
	   [ "${profile%\~}"        != "$profile" ] ; then
		return 1
	fi

	# Silently ignore the dpkg, pacman, ipk and xbps files

	if [ "${profile%.dpkg-new}"     != "$profile" ] || \
	   [ "${profile%.dpkg-old}"     != "$profile" ] || \
	   [ "${profile%.dpkg-dist}"    != "$profile" ] || \
	   [ "${profile%.dpkg-bak}"     != "$profile" ] || \
	   [ "${profile%.dpkg-remove}"  != "$profile" ] || \
	   [ "${profile%.ipk}"          != "$profile" ] || \
	   [ "${profile%.ipk-new}"      != "$profile" ] || \
	   [ "${profile%.ipk-old}"      != "$profile" ] || \
	   [ "${profile%.ipk-dist}"     != "$profile" ] || \
	   [ "${profile%.ipk-bak}"      != "$profile" ] || \
	   [ "${profile%.ipk-remove}"   != "$profile" ] || \
	   [ "${profile%.pacsave}"      != "$profile" ] || \
	   [ "${profile%.pacnew}"       != "$profile" ] ; then
		return 2
	fi

	$(echo "$profile" | grep -E -q '^.+\.new-[0-9\.]+_[0-9]+$'); [ "$?" -eq 0 ] && return 2

	return 0
}

__parse_profiles_dir() {

	local parser_cmd="$1"
	local profile_dir="$2"
	local status=0

	[ -x "$PARSER" ] || {
		log_write err "Unable to execute AppArmor parser"
		return 1
	}

	[ -d "$profile_dir" ] || {
		log_write warning "AppArmor profiles not found: $profile_dir"
		return 1
	}

	dir_is_empty "$profile_dir"; [ "$?" -eq 0 ] && {
		log_write err "No profiles found in $profile_dir"
		return 1
	}

	local nprocs=$(cat /proc/cpuinfo |grep "processor\t:"|wc -l)
	local rc=0
	local xargs_args=""
	[ "$nprocs" -ge 2 ] && xargs_args="--max-procs=$nprocs"

	"$PARSER" $PARSER_OPTS "$parser_cmd" -- "$profile_dir" || {

		for profile in "$profile_dir"/*; do
			skip_profile "$profile"
			skip=$?
			[ "$skip" -ne 0 ] && {
				[ "$skip" -ne 2 ] && log_write info "Skipped loading profile $profile"
				continue
			}
			[ -f "$profile" ] || continue
			echo "$profile"
		done | \

		# Use xargs to parallelize calls to the parser over all CPUs

		/usr/libexec/xargs-findutils -n1 -d"\n" $xargs_args \
			"$PARSER" $PARSER_OPTS "$parser_cmd" --

		[ "$?" -ne 0 ] && {
			rc=1
			log_write err "At least one profile failed to load"
		}
	}

	return $rc
}

parse_profiles() {

	case "$1" in
		load)
			PARSER_CMD="--add"
			PARSER_MSG="Loading profiles"
			;;
		reload)
			PARSER_CMD="--replace"
			PARSER_MSG="Reloading profiles"
			;;
		*)
			log_write err "Unknown parameter $1"
			log_write info "parse_profiles parameter must be either 'load' or 'reload'"
			return 1
			;;
	esac

	log_write info "$PARSER_MSG"

	[ -w "$SFS_MOUNTPOINT/.load" ] || {
		log_write err "${SFS_MOUNTPOINT}/.load not writable"
		return 1
	}

	[ -f "$PARSER" ] || {
		log_write err "AppArmor parser not found"
		return 1
	}

	# run parser on all profiles
	local rc=0
	for profile_dir in $PROFILE_DIRS; do
		__parse_profiles_dir "$PARSER_CMD" "$profile_dir" || rc=$?
	done

	return $rc
}

is_apparmor_loaded() {

	is_securityfs_mounted; [ "$?" -eq 0 ] || {
		mount_securityfs
	}

	[ -f "${SFS_MOUNTPOINT}/profiles" ] && return 0
	[ -d /sys/module/apparmor ] && return 0 || return 1
}

is_securityfs_mounted() {

	[ -d "$SECURITYFS" ] && {
		grep -q securityfs /proc/filesystems && grep -q securityfs /proc/mounts
		return $?
	}
	return 1
}

mount_securityfs() {

	local rc=0

	grep -q securityfs /proc/filesystems; [ "$?" -eq 0 ] && {
		mount -t securityfs securityfs "$SECURITYFS"
		rc=$?
		[ "$rc" -eq 0 ] && log_write info "Mounting securityfs" ||
			log_write err "Failed to mount securityfs"
	}
	return $rc
}

apparmor_start() {

	local announced=0
	is_securityfs_mounted; [ "$?" -ne 0 ] && {
		log_write info "Starting AppArmor"
		announced=1
		mount_securityfs; [ "$?" -eq 0 ] || return $?
	}

	is_apparmor_loaded; [ "$?" -eq 0 ] || {
		[ "$announced" -eq 0 ] && log_write info "Starting AppArmor"
		announced=1
		log_write err "AppArmor kernel support is not present"
		return 1
	}

	[ -d /var/lib/apparmor ] || mkdir -p /var/lib/apparmor > /dev/null

	is_profiles_loaded; [ "$?" -eq 0 ] || {
		[ "$announced" -eq 0 ] && log_write info "Starting AppArmor"
		announced=1
		parse_profiles load
		return $?
	} || {
		parse_profiles reload
		return $?
	}
}

remove_profiles() {

	log_write info "Unloading profiles"

	is_apparmor_loaded; [ "$?" -eq 0 ] || {
		log_write err "AppArmor kernel support is not present"
		return 1
	}

	[ -w "$SFS_MOUNTPOINT/.remove" ] || {
		log_write err "${SFS_MOUNTPOINT}/.remove not writable"
		return 1
	}

	[ -x "$PARSER" ] || {
		log_write err "Unable to execute AppArmor parser"
		return 1
	}

	local rc=0

	sed -e "s/ (\(enforce\|complain\))$//" "$SFS_MOUNTPOINT/profiles" | \
	LC_COLLATE=C sort | grep -v // | {
		while read -r profile ; do
			printf "%s" "$profile" > "$SFS_MOUNTPOINT/.remove"
			result=$?
			[ "$result" -eq 0 ] || rc=$result
		done
	}
	return $rc
}

apparmor_stop() {

	is_apparmor_loaded; [ "$?" -eq 0 ] || return 1
	is_profiles_loaded; [ "$?" -eq 0 ] && {
		log_write info "Stopping AppArmor"
		remove_profiles
		return $?
	} || return 0
}

apparmor_restart() {

	is_profiles_loaded; [ "$?" -eq 0 ] || {
		apparmor_start
		return $?
	}

	is_apparmor_loaded; [ "$?" -eq 0 ] || {
		apparmor_start
		return $?
	}

	log_write info "Restarting AppArmor"
	parse_profiles reload
	return $?
}

apparmor_reload() {

	is_profiles_loaded; [ "$?" -eq 0 ] || {
		apparmor_start
		return $?
	}

	is_apparmor_loaded; [ "$?" -eq 0 ] || {
		apparmor_start
		return $?
	}

	log_write info "Reloading AppArmor"
	parse_profiles reload
	return $?
}

apparmor_list_profiles() {

	is_apparmor_loaded; [ "$?" -eq 0 ] || {
		echo "AppArmor kernel support is not present"
		return 1
	}

        [ -x "$PARSER" ] || {
                echo "Unable to execute AppArmor parser"
		return 1
	}

	# run parser on all profiles
	for profile_dir in $PROFILE_DIRS; do
		[ -d "$profile_dir" ] || {
			echo "AppArmor profiles not found: $profile_dir"
			continue
		}

		for profile in "$profile_dir"/*; do
			if skip_profile "$profile" && [ -f "$profile" ] ; then
				LIST_ADD=$("$PARSER" -N "$profile" )
				[ "$?" -eq 0 ] && echo "$LIST_ADD"
			fi
		done
	done
	return 0
}


apparmor_status() {

	is_apparmor_loaded; [ "$?" -eq 0 ] || {
		echo "AppArmor kernel support is not present"
		return 1
	}

	[ -x "$AA_STATUS" ] && {
		"$AA_STATUS" --verbose
		return $?
	}

	echo "AppArmor is enabled."
	echo "Install apparmor-utils to receive more detailed status"
	echo "information or examine $SFS_MOUNTPOINT directly."

	return 0
}
