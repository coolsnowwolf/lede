# procd API:
#
# procd_open_service(name, [script]):
#   Initialize a new procd command message containing a service with one or more instances
#
# procd_close_service()
#   Send the command message for the service
#
# procd_open_instance([name]):
#   Add an instance to the service described by the previous procd_open_service call
#
# procd_set_param(type, [value...])
#   Available types:
#     command: command line (array).
#     respawn info: array with 3 values $fail_threshold $restart_timeout $max_fail
#     env: environment variable (passed to the process)
#     data: arbitrary name/value pairs for detecting config changes (table)
#     file: configuration files (array)
#     netdev: bound network device (detects ifindex changes)
#     limits: resource limits (passed to the process)
#     user: $username to run service as
#     group: $groupname to run service as
#     pidfile: file name to write pid into
#     stdout: boolean whether to redirect commands stdout to syslog (default: 0)
#     stderr: boolean whether to redirect commands stderr to syslog (default: 0)
#     facility: syslog facility used when logging to syslog (default: daemon)
#
#   No space separation is done for arrays/tables - use one function argument per command line argument
#
# procd_close_instance():
#   Complete the instance being prepared
#
# procd_running(service, [instance]):
#   Checks if service/instance is currently running
#
# procd_kill(service, [instance]):
#   Kill a service instance (or all instances)
#
# procd_send_signal(service, [instance], [signal])
#   Send a signal to a service instance (or all instances)
#

. "$IPKG_INSTROOT/usr/share/libubox/jshn.sh"

PROCD_RELOAD_DELAY=1000
_PROCD_SERVICE=

procd_lock() {
	local basescript=$(readlink "$initscript")
	local service_name="$(basename ${basescript:-$initscript})"

	flock -n 1000 &> /dev/null
	if [ "$?" != "0" ]; then
		exec 1000>"$IPKG_INSTROOT/var/lock/procd_${service_name}.lock"
		flock 1000
		if [ "$?" != "0" ]; then
			logger "warning: procd flock for $service_name failed"
		fi
	fi
}

_procd_call() {
	local old_cb

	json_set_namespace procd old_cb
	"$@"
	json_set_namespace $old_cb
}

_procd_wrapper() {
	procd_lock
	while [ -n "$1" ]; do
		eval "$1() { _procd_call _$1 \"\$@\"; }"
		shift
	done
}

_procd_ubus_call() {
	local cmd="$1"

	[ -n "$PROCD_DEBUG" ] && json_dump >&2
	ubus call service "$cmd" "$(json_dump)"
	json_cleanup
}

_procd_open_service() {
	local name="$1"
	local script="$2"

	_PROCD_SERVICE="$name"
	_PROCD_INSTANCE_SEQ=0

	json_init
	json_add_string name "$name"
	[ -n "$script" ] && json_add_string script "$script"
	json_add_object instances
}

_procd_close_service() {
	json_close_object
	_procd_open_trigger
	service_triggers
	_procd_close_trigger
	_procd_open_data
	service_data
	_procd_close_data
	_procd_ubus_call ${1:-set}
}

_procd_add_array_data() {
	while [ "$#" -gt 0 ]; do
		json_add_string "" "$1"
		shift
	done
}

_procd_add_array() {
	json_add_array "$1"
	shift
	_procd_add_array_data "$@"
	json_close_array
}

_procd_add_table_data() {
	while [ -n "$1" ]; do
		local var="${1%%=*}"
		local val="${1#*=}"
		[ "$1" = "$val" ] && val=
		json_add_string "$var" "$val"
		shift
	done
}

_procd_add_table() {
	json_add_object "$1"
	shift
	_procd_add_table_data "$@"
	json_close_object
}

_procd_open_instance() {
	local name="$1"; shift

	_PROCD_INSTANCE_SEQ="$(($_PROCD_INSTANCE_SEQ + 1))"
	name="${name:-instance$_PROCD_INSTANCE_SEQ}"
	json_add_object "$name"
	[ -n "$TRACE_SYSCALLS" ] && json_add_boolean trace "1"
}

_procd_open_trigger() {
	let '_procd_trigger_open = _procd_trigger_open + 1'
	[ "$_procd_trigger_open" -gt 1 ] && return
	json_add_array "triggers"
}

_procd_close_trigger() {
	let '_procd_trigger_open = _procd_trigger_open - 1'
	[ "$_procd_trigger_open" -lt 1 ] || return
	json_close_array
}

_procd_open_data() {
	let '_procd_data_open = _procd_data_open + 1'
	[ "$_procd_data_open" -gt 1 ] && return
	json_add_object "data"
}

_procd_close_data() {
	let '_procd_data_open = _procd_data_open - 1'
	[ "$_procd_data_open" -lt 1 ] || return
	json_close_object
}

_procd_open_validate() {
	json_select ..
	json_add_array "validate"
}

_procd_close_validate() {
	json_close_array
	json_select triggers
}

_procd_add_jail() {
	json_add_object "jail"
	json_add_string name "$1"

	shift
	
	for a in $@; do
		case $a in
		log)	json_add_boolean "log" "1";;
		ubus)	json_add_boolean "ubus" "1";;
		procfs)	json_add_boolean "procfs" "1";;
		sysfs)	json_add_boolean "sysfs" "1";;
		ronly)	json_add_boolean "ronly" "1";;
		requirejail)	json_add_boolean "requirejail" "1";;
		netns)	json_add_boolean "netns" "1";;
		userns)	json_add_boolean "userns" "1";;
		cgroupsns)	json_add_boolean "cgroupsns" "1";;
		esac
	done
	json_add_object "mount"
	json_close_object
	json_close_object
}

_procd_add_jail_mount() {
	local _json_no_warning=1

	json_select "jail"
	[ $? = 0 ] || return
	json_select "mount"
	[ $? = 0 ] || {
		json_select ..
		return
	}
	for a in $@; do
		json_add_string "$a" "0"
	done
	json_select ..
	json_select ..
}

_procd_add_jail_mount_rw() {
	local _json_no_warning=1

	json_select "jail"
	[ $? = 0 ] || return
	json_select "mount"
	[ $? = 0 ] || {
		json_select ..
		return
	}
	for a in $@; do
		json_add_string "$a" "1"
	done
	json_select ..
	json_select ..
}

_procd_set_param() {
	local type="$1"; shift

	case "$type" in
		env|data|limits)
			_procd_add_table "$type" "$@"
		;;
		command|netdev|file|respawn|watch|watchdog)
			_procd_add_array "$type" "$@"
		;;
		error)
			json_add_array "$type"
			json_add_string "" "$@"
			json_close_array
		;;
		nice|term_timeout)
			json_add_int "$type" "$1"
		;;
		reload_signal)
			json_add_int "$type" $(kill -l "$1")
		;;
		pidfile|user|group|seccomp|capabilities|facility|\
		extroot|overlaydir|tmpoverlaysize)
			json_add_string "$type" "$1"
		;;
		stdout|stderr|no_new_privs)
			json_add_boolean "$type" "$1"
		;;
	esac
}

_procd_add_timeout() {
	[ "$PROCD_RELOAD_DELAY" -gt 0 ] && json_add_int "" "$PROCD_RELOAD_DELAY"
	return 0
}

_procd_add_interface_trigger() {
	json_add_array
	_procd_add_array_data "$1"
	shift

	json_add_array
	_procd_add_array_data "if"

	json_add_array
	_procd_add_array_data "eq" "interface" "$1"
	shift
	json_close_array

	json_add_array
	_procd_add_array_data "run_script" "$@"
	json_close_array

	json_close_array
	_procd_add_timeout
	json_close_array
}

_procd_add_reload_interface_trigger() {
	local script=$(readlink "$initscript")
	local name=$(basename ${script:-$initscript})

	_procd_open_trigger
	_procd_add_interface_trigger "interface.*" $1 /etc/init.d/$name reload
	_procd_close_trigger
}

_procd_add_config_trigger() {
	json_add_array
	_procd_add_array_data "$1"
	shift

	json_add_array
	_procd_add_array_data "if"

	json_add_array
	_procd_add_array_data "eq" "package" "$1"
	shift
	json_close_array

	json_add_array
	_procd_add_array_data "run_script" "$@"
	json_close_array

	json_close_array
	_procd_add_timeout
	json_close_array
}

_procd_add_mount_trigger() {
	json_add_array
	_procd_add_array_data "$1"
	local action="$2"
	local multi=0
	shift ; shift

	json_add_array
	_procd_add_array_data "if"

	if [ "$2" ]; then
		json_add_array
		_procd_add_array_data "or"
		multi=1
	fi

	while [ "$1" ]; do
		json_add_array
		_procd_add_array_data "eq" "target" "$1"
		shift
		json_close_array
	done

	[ $multi = 1 ] && json_close_array

	json_add_array
	_procd_add_array_data "run_script" /etc/init.d/$name $action
	json_close_array

	json_close_array
	_procd_add_timeout
	json_close_array
}

_procd_add_action_mount_trigger() {
	local action="$1"
	shift
	local mountpoints="$(procd_get_mountpoints "$@")"
	[ "${mountpoints//[[:space:]]}" ] || return 0
	local script=$(readlink "$initscript")
	local name=$(basename ${script:-$initscript})

	_procd_open_trigger
	_procd_add_mount_trigger mount.add $action "$mountpoints"
	_procd_close_trigger
}

procd_get_mountpoints() {
	(
		__procd_check_mount() {
			local cfg="$1"
			local path="${2%%/}/"
			local target
			config_get target "$cfg" target
			target="${target%%/}/"
			[ "$path" != "${path##$target}" ] && echo "${target%%/}"
		}
		local mpath
		config_load fstab
		for mpath in "$@"; do
			config_foreach __procd_check_mount mount "$mpath"
		done
	) | sort -u
}

_procd_add_restart_mount_trigger() {
	_procd_add_action_mount_trigger restart "$@"
}

_procd_add_reload_mount_trigger() {
	_procd_add_action_mount_trigger reload "$@"
}

_procd_add_raw_trigger() {
	json_add_array
	_procd_add_array_data "$1"
	shift
	local timeout=$1
	shift

	json_add_array
	json_add_array
	_procd_add_array_data "run_script" "$@"
	json_close_array
	json_close_array

	json_add_int "" "$timeout"

	json_close_array
}

_procd_add_reload_trigger() {
	local script=$(readlink "$initscript")
	local name=$(basename ${script:-$initscript})
	local file

	_procd_open_trigger
	for file in "$@"; do
		_procd_add_config_trigger "config.change" "$file" /etc/init.d/$name reload
	done
	_procd_close_trigger
}

_procd_add_validation() {
	_procd_open_validate
	$@
	_procd_close_validate
}

_procd_append_param() {
	local type="$1"; shift
	local _json_no_warning=1

	json_select "$type"
	[ $? = 0 ] || {
		_procd_set_param "$type" "$@"
		return
	}
	case "$type" in
		env|data|limits)
			_procd_add_table_data "$@"
		;;
		command|netdev|file|respawn|watch|watchdog)
			_procd_add_array_data "$@"
		;;
		error)
			json_add_string "" "$@"
		;;
	esac
	json_select ..
}

_procd_close_instance() {
	local respawn_vals
	_json_no_warning=1
	if json_select respawn ; then
		json_get_values respawn_vals
		if [ -z "$respawn_vals" ]; then
			local respawn_threshold=$(uci_get system.@service[0].respawn_threshold)
			local respawn_timeout=$(uci_get system.@service[0].respawn_timeout)
			local respawn_retry=$(uci_get system.@service[0].respawn_retry)
			_procd_add_array_data ${respawn_threshold:-3600} ${respawn_timeout:-5} ${respawn_retry:-5}
		fi
		json_select ..
	fi

	json_close_object
}

_procd_add_instance() {
	_procd_open_instance
	_procd_set_param command "$@"
	_procd_close_instance
}

procd_running() {
	local service="$1"
	local instance="${2:-*}"
	[ "$instance" = "*" ] || instance="'$instance'"

	json_init
	json_add_string name "$service"
	local running=$(_procd_ubus_call list | jsonfilter -l 1 -e "@['$service'].instances[$instance].running")

	[ "$running" = "true" ]
}

_procd_kill() {
	local service="$1"
	local instance="$2"

	json_init
	[ -n "$service" ] && json_add_string name "$service"
	[ -n "$instance" ] && json_add_string instance "$instance"
	_procd_ubus_call delete
}

_procd_send_signal() {
	local service="$1"
	local instance="$2"
	local signal="$3"

	case "$signal" in
		[A-Z]*)	signal="$(kill -l "$signal" 2>/dev/null)" || return 1;;
	esac

	json_init
	json_add_string name "$service"
	[ -n "$instance" -a "$instance" != "*" ] && json_add_string instance "$instance"
	[ -n "$signal" ] && json_add_int signal "$signal"
	_procd_ubus_call signal
}

_procd_status() {
	local service="$1"
	local instance="$2"
	local data

	json_init
	[ -n "$service" ] && json_add_string name "$service"

	data=$(_procd_ubus_call list | jsonfilter -e '@["'"$service"'"]')
	[ -z "$data" ] && { echo "inactive"; return 3; }

	data=$(echo "$data" | jsonfilter -e '$.instances')
	if [ -z "$data" ]; then
		[ -z "$instance" ] && { echo "active with no instances"; return 0; }
		data="[]"
	fi

	[ -n "$instance" ] && instance="\"$instance\"" || instance='*'
	if [ -z "$(echo "$data" | jsonfilter -e '$['"$instance"']')" ]; then
		echo "unknown instance $instance"; return 4
	else
		echo "running"; return 0
	fi
}

procd_open_data() {
	local name="$1"
	json_set_namespace procd __procd_old_cb
	json_add_object data
}

procd_close_data() {
	json_close_object
	json_set_namespace $__procd_old_cb
}

_procd_set_config_changed() {
	local package="$1"

	json_init
	json_add_string type config.change
	json_add_object data
	json_add_string package "$package"
	json_close_object

	ubus call service event "$(json_dump)"
}

procd_add_mdns_service() {
	local service proto port
	service=$1; shift
	proto=$1; shift
	port=$1; shift
	json_add_object "${service}_$port"
	json_add_string "service" "_$service._$proto.local"
	json_add_int port "$port"
	[ -n "$1" ] && {
		json_add_array txt
		for txt in "$@"; do json_add_string "" "$txt"; done
		json_select ..
	}
	json_select ..
}

procd_add_mdns() {
	procd_open_data
	json_add_object "mdns"
	procd_add_mdns_service "$@"
	json_close_object
	procd_close_data
}

uci_validate_section()
{
	local _package="$1"
	local _type="$2"
	local _name="$3"
	local _result
	local _error
	shift; shift; shift
	_result=$(/sbin/validate_data "$_package" "$_type" "$_name" "$@" 2> /dev/null)
	_error=$?
	eval "$_result"
	[ "$_error" = "0" ] || $(/sbin/validate_data "$_package" "$_type" "$_name" "$@" 1> /dev/null)
	return $_error
}

uci_load_validate() {
	local _package="$1"
	local _type="$2"
	local _name="$3"
	local _function="$4"
	local _option
	local _result
	shift; shift; shift; shift
	for _option in "$@"; do
		eval "local ${_option%%:*}"
	done
	uci_validate_section "$_package" "$_type" "$_name" "$@"
	_result=$?
	[ -n "$_function" ] || return $_result
	eval "$_function \"\$_name\" \"\$_result\""
}

_procd_wrapper \
	procd_open_service \
	procd_close_service \
	procd_add_instance \
	procd_add_raw_trigger \
	procd_add_config_trigger \
	procd_add_interface_trigger \
	procd_add_mount_trigger \
	procd_add_reload_trigger \
	procd_add_reload_interface_trigger \
	procd_add_action_mount_trigger \
	procd_add_reload_mount_trigger \
	procd_add_restart_mount_trigger \
	procd_open_trigger \
	procd_close_trigger \
	procd_open_instance \
	procd_close_instance \
	procd_open_validate \
	procd_close_validate \
	procd_add_jail \
	procd_add_jail_mount \
	procd_add_jail_mount_rw \
	procd_set_param \
	procd_append_param \
	procd_add_validation \
	procd_set_config_changed \
	procd_kill \
	procd_send_signal
