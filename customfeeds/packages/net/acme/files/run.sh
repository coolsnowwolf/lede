#!/bin/sh
# Wrapper for acme.sh to work on openwrt.
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# Author: Toke Høiland-Jørgensen <toke@toke.dk>

CHECK_CRON=$1
ACME=/usr/lib/acme/acme.sh
export CURL_CA_BUNDLE=/etc/ssl/certs/ca-certificates.crt
export NO_TIMESTAMP=1

UHTTPD_LISTEN_HTTP=
STATE_DIR='/etc/acme'
ACCOUNT_EMAIL=
DEBUG=0
NGINX_WEBSERVER=0
UPDATE_NGINX=0
UPDATE_UHTTPD=0
USER_CLEANUP=

. /lib/functions.sh

check_cron()
{
	[ -f "/etc/crontabs/root" ] && grep -q '/etc/init.d/acme' /etc/crontabs/root && return
	echo "0 0 * * * /etc/init.d/acme start" >> /etc/crontabs/root
	/etc/init.d/cron start
}

log()
{
	logger -t acme -s -p daemon.info -- "$@"
}

err()
{
	logger -t acme -s -p daemon.err -- "$@"
}

debug()
{
	[ "$DEBUG" -eq "1" ] && logger -t acme -s -p daemon.debug -- "$@"
}

get_listeners() {
	local proto rq sq listen remote state program
	netstat -nptl 2>/dev/null | while read proto rq sq listen remote state program; do
		case "$proto#$listen#$program" in
			tcp#*:80#[0-9]*/*) echo -n "${program%% *} " ;;
		esac
	done
}

run_acme()
{
	debug "Running acme.sh as '$ACME $@'"
	$ACME "$@"
}

pre_checks()
{
	main_domain="$1"

	log "Running pre checks for $main_domain."

	listeners="$(get_listeners)"

	debug "port80 listens: $listeners"

	for listener in $(get_listeners); do
		pid="${listener%/*}"
		cmd="$(basename $(readlink /proc/$pid/exe))"

		case "$cmd" in
			uhttpd)
				if [ -n "$UHTTPD_LISTEN_HTTP" ]; then
					debug "Already handled uhttpd; skipping"
					continue
				fi

				debug "Found uhttpd listening on port 80; trying to disable."

				UHTTPD_LISTEN_HTTP=$(uci get uhttpd.main.listen_http)

				if [ -z "$UHTTPD_LISTEN_HTTP" ]; then
					err "$main_domain: Unable to find uhttpd listen config."
					err "Manually disable uhttpd or set webroot to continue."
					return 1
				fi

				uci set uhttpd.main.listen_http=''
				uci commit uhttpd || return 1
				if ! /etc/init.d/uhttpd reload ; then
					uci set uhttpd.main.listen_http="$UHTTPD_LISTEN_HTTP"
					uci commit uhttpd
					return 1
				fi
				;;
			nginx)
				if [ "$NGINX_WEBSERVER" -eq "1" ]; then
					debug "Already handled nginx; skipping"
					continue
				fi

				debug "Found nginx listening on port 80; trying to disable."
				NGINX_WEBSERVER=1
				local tries=0
				while grep -sq "$cmd" "/proc/$pid/cmdline" && kill -0 "$pid"; do
					/etc/init.d/nginx stop
					if [ $tries -gt 10 ]; then
						debug "Can't stop nginx. Terminating script."
						return 1
					fi
					debug "Waiting for nginx to stop..."
					tries=$((tries + 1))
					sleep 1
				done
				;;
			"")
				debug "Nothing listening on port 80."
				;;
			*)
				err "$main_domain: Cannot run in standalone mode; another daemon is listening on port 80."
				err "Disable other daemon or set webroot to continue."
				return 1
				;;
		esac
	done

	iptables -I input_rule -p tcp --dport 80 -j ACCEPT -m comment --comment "ACME" || return 1
	ip6tables -I input_rule -p tcp --dport 80 -j ACCEPT -m comment --comment "ACME" || return 1
	debug "v4 input_rule: $(iptables -nvL input_rule)"
	debug "v6 input_rule: $(ip6tables -nvL input_rule)"
	return 0
}

post_checks()
{
	log "Running post checks (cleanup)."
	# The comment ensures we only touch our own rules. If no rules exist, that
	# is fine, so hide any errors
	iptables -D input_rule -p tcp --dport 80 -j ACCEPT -m comment --comment "ACME" 2>/dev/null
	ip6tables -D input_rule -p tcp --dport 80 -j ACCEPT -m comment --comment "ACME" 2>/dev/null

	if [ -e /etc/init.d/uhttpd ] && ( [ -n "$UHTTPD_LISTEN_HTTP" ] || [ "$UPDATE_UHTTPD" -eq 1 ] ); then
		if [ -n "$UHTTPD_LISTEN_HTTP" ]; then
			uci set uhttpd.main.listen_http="$UHTTPD_LISTEN_HTTP"
			UHTTPD_LISTEN_HTTP=
		fi
		uci commit uhttpd
		/etc/init.d/uhttpd reload
	fi

	if [ -e /etc/init.d/nginx ] && ( [ "$NGINX_WEBSERVER" -eq 1 ] || [ "$UPDATE_NGINX" -eq 1 ] ); then
		NGINX_WEBSERVER=0
		/etc/init.d/nginx restart
	fi

	if [ -n "$USER_CLEANUP" ] && [ -f "$USER_CLEANUP" ]; then
		log "Running user-provided cleanup script from $USER_CLEANUP."
		"$USER_CLEANUP" || return 1
	fi
}

err_out()
{
	post_checks
	exit 1
}

int_out()
{
	post_checks
	trap - INT
	kill -INT $$
}

is_staging()
{
	local main_domain
	local domain_dir
	main_domain="$1"
	domain_dir="$2"

	grep -q "acme-staging" "${domain_dir}/${main_domain}.conf"
	return $?
}

issue_cert()
{
	local section="$1"
	local acme_args=
	local enabled
	local use_staging
	local update_uhttpd
	local update_nginx
	local keylength
	local keylength_ecc=0
	local domains
	local main_domain
	local moved_staging=0
	local failed_dir
	local webroot
	local dns
	local user_setup
	local user_cleanup
	local ret
	local domain_dir
	local acme_server
	local days

	config_get_bool enabled "$section" enabled 0
	config_get_bool use_staging "$section" use_staging
	config_get_bool update_uhttpd "$section" update_uhttpd
	config_get_bool update_nginx "$section" update_nginx
	config_get calias "$section" calias
	config_get dalias "$section" dalias
	config_get domains "$section" domains
	config_get keylength "$section" keylength
	config_get webroot "$section" webroot
	config_get dns "$section" dns
	config_get user_setup "$section" user_setup
	config_get user_cleanup "$section" user_cleanup
	config_get acme_server "$section" acme_server
	config_get days "$section" days

	UPDATE_NGINX=$update_nginx
	UPDATE_UHTTPD=$update_uhttpd
	USER_CLEANUP=$user_cleanup

	[ "$enabled" -eq "1" ] || return

	[ "$DEBUG" -eq "1" ] && acme_args="$acme_args --debug"

	set -- $domains
	main_domain=$1

	if [ -n "$user_setup" ] && [ -f "$user_setup" ]; then
		log "Running user-provided setup script from $user_setup."
		"$user_setup" "$main_domain" || return 1
	else
		[ -n "$webroot" ] || [ -n "$dns" ] || pre_checks "$main_domain" || return 1
	fi

	if echo $keylength | grep -q "^ec-"; then
		domain_dir="$STATE_DIR/${main_domain}_ecc"
		keylength_ecc=1
	else
		domain_dir="$STATE_DIR/${main_domain}"
	fi

	log "Running ACME for $main_domain"

	handle_credentials() {
		local credential="$1"
		eval export $credential
	}
	config_list_foreach "$section" credentials handle_credentials

	if [ -e "$domain_dir" ]; then
		if [ "$use_staging" -eq "0" ] && is_staging "$main_domain" "$domain_dir"; then
			log "Found previous cert issued using staging server. Moving it out of the way."
			mv "$domain_dir" "${domain_dir}.staging"
			moved_staging=1
		else
			log "Found previous cert config. Issuing renew."
			[ "$keylength_ecc" -eq "1" ] && acme_args="$acme_args --ecc"
			run_acme --home "$STATE_DIR" --renew -d "$main_domain" $acme_args && ret=0 || ret=1
			post_checks
			return $ret
		fi
	fi


	acme_args="$acme_args $(for d in $domains; do echo -n "-d $d "; done)"
	acme_args="$acme_args --keylength $keylength"
	[ -n "$ACCOUNT_EMAIL" ] && acme_args="$acme_args --accountemail $ACCOUNT_EMAIL"
	[ "$use_staging" -eq "1" ] && acme_args="$acme_args --staging"

	if [ -n "$acme_server" ]; then
		log "Using custom ACME server URL"
		acme_args="$acme_args --server $acme_server"
	fi

	if [ -n "$days" ]; then
		log "Renewing after $days days"
		acme_args="$acme_args --days $days"
	fi

	if [ -n "$dns" ]; then
		log "Using dns mode"
		acme_args="$acme_args --dns $dns"
		if [ -n "$dalias" ]; then
			log "Using domain alias for dns mode"
			acme_args="$acme_args --domain-alias $dalias"
			if [ -n "$calias" ]; then
				err "Both domain and challenge aliases are defined. Ignoring the challenge alias."
			fi
		elif [ -n "$calias" ]; then
			log "Using challenge alias for dns mode"
			acme_args="$acme_args --challenge-alias $calias"
		fi
	elif [ -z "$webroot" ]; then
		log "Using standalone mode"
		acme_args="$acme_args --standalone --listen-v6"
	else
		if [ ! -d "$webroot" ]; then
			err "$main_domain: Webroot dir '$webroot' does not exist!"
			post_checks
			return 1
		fi
		log "Using webroot dir: $webroot"
		acme_args="$acme_args --webroot $webroot"
	fi

	if ! run_acme --home "$STATE_DIR" --issue $acme_args; then
		failed_dir="${domain_dir}.failed-$(date +%s)"
		err "Issuing cert for $main_domain failed. Moving state to $failed_dir"
		[ -d "$domain_dir" ] && mv "$domain_dir" "$failed_dir"
		if [ "$moved_staging" -eq "1" ]; then
			err "Restoring staging certificate"
			mv "${domain_dir}.staging" "${domain_dir}"
		fi
		post_checks
		return 1
	fi

	if [ -e /etc/init.d/uhttpd ] && [ "$update_uhttpd" -eq "1" ]; then
		uci set uhttpd.main.key="${domain_dir}/${main_domain}.key"
		uci set uhttpd.main.cert="${domain_dir}/fullchain.cer"
		# commit and reload is in post_checks
	fi

	local nginx_updated
	nginx_updated=0
	if command -v nginx-util 2>/dev/null && [ "$update_nginx" -eq "1" ]; then
		nginx_updated=1
		for domain in $domains; do
			nginx-util add_ssl "${domain}" acme "${domain_dir}/fullchain.cer" \
				"${domain_dir}/${main_domain}.key" || nginx_updated=0
		done
		# reload is in post_checks
	fi

	if [ "$nginx_updated" -eq "0" ] && [ -w /etc/nginx/nginx.conf ] && [ "$update_nginx" -eq "1" ]; then
		sed -i "s#ssl_certificate\ .*#ssl_certificate ${domain_dir}/fullchain.cer;#g" /etc/nginx/nginx.conf
		sed -i "s#ssl_certificate_key\ .*#ssl_certificate_key ${domain_dir}/${main_domain}.key;#g" /etc/nginx/nginx.conf
		# commit and reload is in post_checks
	fi

	post_checks
}

load_vars()
{
	local section="$1"

	STATE_DIR=$(config_get "$section" state_dir)
	ACCOUNT_EMAIL=$(config_get "$section" account_email)
	DEBUG=$(config_get "$section" debug)
}

check_cron
[ -n "$CHECK_CRON" ] && exit 0
[ -e "/var/run/acme_boot" ] && rm -f "/var/run/acme_boot" && exit 0

config_load acme
config_foreach load_vars acme

if [ -z "$STATE_DIR" ] || [ -z "$ACCOUNT_EMAIL" ]; then
	err "state_dir and account_email must be set"
	exit 1
fi

[ -d "$STATE_DIR" ] || mkdir -p "$STATE_DIR"

trap err_out HUP TERM
trap int_out INT

config_foreach issue_cert cert

exit 0
