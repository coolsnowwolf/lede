#!/bin/sh

main() {
	local service="$1"
	shift

	local boot status

	if [ -f "/etc/init.d/${service}" ]; then
		/etc/init.d/"${service}" "$@"
		exit "$?"
	fi

	if [ -n "$service" ]; then
		echo "Service \"$service\" not found:"
		exit 1
	fi

	echo "Usage: $(basename "$0") <service> [command]"
	for service in /etc/init.d/* ; do
		boot="$($service enabled && echo "enabled" || echo "disabled" )"
		status="$( [ "$(ubus call service list "{ 'verbose': true, 'name': '$(basename "$service")' }" \
			| jsonfilter -q -e "@['$(basename "$service")'].instances[*].running" | uniq)" = "true" ] \
			&& echo "running" || echo "stopped" )"

		printf "%-30s\\t%10s\\t%10s\\n"  "$service" "$boot" "$status"
	done
}

main "$@"
