next_port () {
	local port_start=$1
	local port_end=$2

	ports=$(wg show all listen-port | awk '{print $2}')

	for i in $(seq $port_start $port_end); do
		if ! echo $ports | grep -q "$i"; then
			echo $i
			return
		fi
	done
}
