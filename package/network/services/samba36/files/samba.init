#!/bin/sh /etc/rc.common
# Copyright (C) 2008-2012 OpenWrt.org

START=60

smb_header() {
 	local enabled
	config_get_bool enabled $1 enabled 1
	[ $enabled == "0" ] && {
	  echo "Samba disabled!"
	  exit 0
	}
	
	local interface
	config_get interface $1 interface "loopback lan"

	# resolve interfaces
	local interfaces=$(
		. /lib/functions/network.sh

		local net
		for net in $interface; do
			local device
			network_get_device device "$net" && {
				local subnet
				network_get_subnet  subnet "$net" && echo -n "$subnet "
				network_get_subnet6 subnet "$net" && echo -n "$subnet "
			}

			echo -n "${device:-$net} "
		done
	)

	local name workgroup description charset
	local hostname="$(uci_get system.@system[0].hostname)"

	config_get name        $1 name        "${hostname:-PandoraBox}"
	config_get workgroup   $1 workgroup   "${hostname:-PandoraBox}"
	config_get description $1 description "Samba on ${hostname:-PandoraBox}"
	config_get charset     $1 charset     "UTF-8"
	config_get master     $1 master     "yes"
	
	mkdir -p /var/etc
	sed -e "s#|NAME|#$name#g" \
	    -e "s#|WORKGROUP|#$workgroup#g" \
	    -e "s#|DESCRIPTION|#$description#g" \
	    -e "s#|INTERFACES|#$interfaces#g" \
	    -e "s#|CHARSET|#$charset#g" \
	    -e "s#|MASTER|#$master#g" \
	    /etc/samba/smb.conf.template > /var/etc/smb.conf

	local homes
	config_get_bool homes $1 homes 0
	[ $homes -gt 0 ] && {
		cat <<EOT >> /var/etc/smb.conf

[homes]
	comment     = Home Directories
	browsable   = no
	read only   = no
	create mode = 0750
EOT
	}

	[ -L /etc/samba/smb.conf ] || ln -nsf /var/etc/smb.conf /etc/samba/smb.conf
}

smb_add_share() {
	local name
	local path
	local users
	local read_only
	local guest_ok
	local create_mask
	local dir_mask
	local browseable

	config_get name $1 name
	config_get path $1 path
	config_get users $1 users
	config_get read_only $1 read_only
	config_get guest_ok $1 guest_ok
	config_get create_mask $1 create_mask
	config_get dir_mask $1 dir_mask
	config_get browseable $1 browseable

	[ -z "$name" -o -z "$path" ] && return

	echo -e "\n[$name]\n\tpath = $path" >> /var/etc/smb.conf
	[ -n "$users" ] && echo -e "\tvalid users = $users" >> /var/etc/smb.conf
	[ -n "$read_only" ] && echo -e "\tread only = $read_only" >> /var/etc/smb.conf
	[ -n "$guest_ok" ] && echo -e "\tguest ok = $guest_ok" >> /var/etc/smb.conf
	[ -n "$create_mask" ] && echo -e "\tcreate mask = $create_mask" >> /var/etc/smb.conf
	[ -n "$dir_mask" ] && echo -e "\tdirectory mask = $dir_mask" >> /var/etc/smb.conf
	[ -n "$browseable" ] && echo -e "\tbrowseable = $browseable" >> /var/etc/smb.conf
}

start() {
	config_load samba
	config_foreach smb_header samba
	config_foreach smb_add_share sambashare
	
	service_start /usr/sbin/smbd -D
	service_start /usr/sbin/nmbd -D
}

stop() {
	service_stop /usr/sbin/smbd
	service_stop /usr/sbin/nmbd
}
