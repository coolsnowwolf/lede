#!/bin/sh

PSQL="/usr/bin/psql"

free_megs() {
	fsdir=$1
	while [ ! -d "$fsdir" ]; do
		fsdir=$(dirname $fsdir)
	done
	df -m $fsdir | while read fs bl us av cap mnt; do [ "$av" = "Available" ] || echo $av; done
}

pg_init_data() {
	# make sure we got at least 50MB of free space
	[ $(free_megs $1) -lt 50 ] && return 1
	pg_ctl initdb -U postgres -D $1
}

pg_server_ready() {
	t=0
	while [ $t -le 90 ]; do
		pg_ctl status -U postgres -D $1 2>/dev/null >/dev/null && return 0
		t=$((t+1))
		sleep 1
	done
	return 1
}


pg_test_db() {
	if [ "$3" ]; then
		echo "SHOW ALL;" | env PGPASSWORD="$3" $PSQL -U "$2" -d "$1" -q 2>/dev/null >/dev/null
		return $?
	else
		echo "SHOW ALL;" | $PSQL -w -U "$2" -d "$1" -q 2>/dev/null >/dev/null
		return $?
	fi
}

pg_include_sql() {
	if [ "$3" ]; then
		env PGPASSWORD="$3" $PSQL -U "$2" -d "$1" -e -f "$4"
		return $?
	else
		$PSQL -w -U "$2" -d "$1" -e -f "$4"
		return $?
	fi
}

# $1: dbname, $2: username, $3: password, $4: sql populate script
pg_require_db() {
	local ret

	pg_test_db $@ && return 0
	( echo "CREATE DATABASE $1;"
	echo -n "CREATE USER $2"
	[ "$3" ] && echo -n " WITH PASSWORD '$3'"
	echo " NOCREATEDB NOSUPERUSER NOCREATEROLE NOINHERIT;"
	echo "GRANT ALL PRIVILEGES ON DATABASE \"$1\" TO $2;" ) |
		$PSQL -U postgres -d template1 -e
	ret=$?
	[ "$ret" = "0" ] || return $ret

	if [ "$4" ]; then
		pg_include_sql "$@"
		ret=$?
	fi

	return $ret
}

uci_require_db() {
	local dbname dbuser dbpass dbscript
	config_get dbname $1 name
	config_get dbuser $1 user
	config_get dbpass $1 pass
	config_get dbscript $1 script
	pg_require_db "$dbname" "$dbuser" "$dbpass" "$dbscript"
}

[ "$1" = "init" ] && {
	. /lib/functions.sh
	pg_server_ready $2 || exit 1
	config_load postgresql
	config_foreach uci_require_db postgres-db
}
