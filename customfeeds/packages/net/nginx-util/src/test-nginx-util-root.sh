#!/bin/sh

PRINT_PASSED=2

NGINX_UTIL="/usr/bin/nginx-util"

ORIG=".original-test-nginx-util-root"

mkdir -p /tmp/.uci/

uci commit nginx || { printf "Error invoking: uci commit\n Exit."; exit 2; }


pst_exit() {
    printf "\nExit: Recovering original settings ... "

    uci revert nginx

    cd "/etc/config/" && rm "nginx" && mv "nginx.${ORIG}" "nginx" ||
    printf "\n%s: not moved %s to %s\n" "/etc/config/" "nginx${ORIG}" "nginx"

    cd "/etc/crontabs/" && rm "root" && mv "root${ORIG}" "root" ||
    printf "\n%s: not moved %s to %s\n" "/etc/crontabs/" "root${ORIG}" "root"

    cd "$(dirname "${CONF_DIR}")" && rm -r "${CONF_DIR}" &&
    mv "$(basename "${CONF_DIR}")${ORIG}" "$(basename "${CONF_DIR}")" ||
    printf "\n%s: not moved %s to %s\n" "$(dirname "${CONF_DIR}")" \
        "$(basename "${CONF_DIR}")${ORIG}" "$(basename "${CONF_DIR}")"

    printf "done.\n"

    exit "$1"
}


mkdir -p "/etc/config/" && touch "/etc/config/nginx"

cd "/etc/config/" && [ ! -e "nginx${ORIG}" ] && cp "nginx" "nginx.${ORIG}" || {
    printf "\n%s: not copied %s to %s\n" "/etc/config/" "nginx" "nginx${ORIG}"
    pst_exit 3
}

uci set nginx.global.uci_enable=1


mkdir -p "/etc/crontabs/" && touch "/etc/crontabs/root"

cd "/etc/crontabs/" && [ ! -e "root${ORIG}" ] && mv "root" "root${ORIG}" || {
    printf "\n%s: not moved %s to %s\n" "/etc/crontabs/" "root${ORIG}" "root"
    pst_exit 4
}

touch "/etc/crontabs/root"


# ----------------------------------------------------------------------------

__esc_newlines() {
    echo "${1}" | sed -E 's/$/\\n/' | tr -d '\n' | sed -E 's/\\n$/\n/'
}

__esc_sed_rhs() {
    __esc_newlines "${1}" |  sed -E 's/[&/\]/\\&/g'
}

_sed_rhs() {
    __esc_sed_rhs "$(echo "${1}" | sed -E "s/[$]/$(__esc_sed_rhs "${2}")/g")"
}

__esc_regex() {
    __esc_newlines "${1}" | sed -E 's/[^^_a-zA-Z0-9-]/[&]/g; s/\^/\\^/g'
}

_regex() {
    __esc_regex "${1}" | sed -E -e 's/^(\[\s])*/^\\s*/' \
        -e 's/(\[\s])+\[[*]]/(\\s.*)?/g' \
        -e 's/(\[\s])+/\\s+/g' \
        -e 's/(\[\s])*\[[;]]/\\s*;/g' \
        -e "s/\[['\"]]/['\"]?/g" \
        -e "s/\[[$]]/$(__esc_sed_rhs "$(__esc_regex "${2}")")/g"
}

_echo_sed() {
    echo "" | sed -E "c${1}"
}


fileauto="# This file is re-created when Nginx starts."

setpoint_init_lan() {
    echo "${fileauto}"

    sed -n -E '/^\s*#UCI_HTTP_CONFIG\s*$/q;p' "${UCI_CONF}.template"

    local rhs="\t}\n\n\tserver { #see uci show 'nginx.\1'"
    uci -n export nginx \
    | sed -E -e "s/'//g" \
        -e '/^\s*package\s+nginx\s*$/d' \
        -e '/^\s*config\s+main\s/d' \
        -e "s/^\s*config\s+server\s+(.*)$/$rhs/g" \
        -e 's/^\s*list\s/\t\t/g' \
        -e 's/^\s*option\s/\t\t/g' \
        -e 's/^\s*uci_listen_locally\s+/\t\tlisten 127.0.0.1:/g' \
        -e '/^\s*uci_/d' \
        -e '/^$/d' -e "s/[^'\n]$/&;/g" \
    | sed "1,2d"
    printf "\t}\n\n"

    sed -E '1,/^\s*#UCI_HTTP_CONFIG\s*$/ d' "${UCI_CONF}.template"
}


setpoint_add_ssl() {
    local indent="\n$1"
    local name="$2"
    local default=""
    [ "${name}" = "${LAN_NAME}" ] && default=".default"
    local prefix="${CONF_DIR}${name}"

    local ADDS=""
    local CONF
    CONF="$(sed -E \
        -e "s/$(_regex "${NGX_INCLUDE}" "${LAN_LISTEN}${default}")/$1$(\
                _sed_rhs "${NGX_INCLUDE}" "${LAN_SSL_LISTEN}${default}")/g" \
        -e "s/^(\s*listen\s+)([^:]*:|\[[^]]*\]:)?80(\s|$|;)/\1\2443 ssl\3/g" \
            "${prefix}.sans" 2>/dev/null)"
    echo "${CONF}" | grep -qE "$(_regex "${NGX_SSL_CRT}" "${prefix}")" \
    || ADDS="${ADDS}${indent}$(_sed_rhs "${NGX_SSL_CRT}" "${prefix}")"
    echo "${CONF}" | grep -qE "$(_regex "${NGX_SSL_KEY}" "${prefix}")" \
    || ADDS="${ADDS}${indent}$(_sed_rhs "${NGX_SSL_KEY}" "${prefix}")"
    echo "${CONF}" | grep -qE "^\s*ssl_session_cache\s" \
    || ADDS="${ADDS}${indent}$(_sed_rhs "${NGX_SSL_SESSION_CACHE}" "${name}")"
    echo "${CONF}" | grep -qE "^\s*ssl_session_timeout\s" \
    || ADDS="${ADDS}${indent}$(_sed_rhs "${NGX_SSL_SESSION_TIMEOUT}" "")"

    if [ -n "${ADDS}" ]
    then
        ADDS="$(echo "${ADDS}" | sed -E 's/^\\n//')"
        echo "${CONF}" | grep -qE "$(_regex "${NGX_SERVER_NAME}" "${name}")" \
        && echo "${CONF}" \
            | sed -E "/$(_regex "${NGX_SERVER_NAME}" "${name}")/a\\${ADDS}" \
            > "${prefix}.with" \
        && _echo_sed "Added directives to ${prefix}.with:\n${ADDS}" \
        && return 0 \
        || _echo_sed "Cannot add directives to ${prefix}.sans, missing:\
            \n$(_sed_rhs "${NGX_SERVER_NAME}" "${name}")\n${ADDS}"
        return 1
    fi
    return 0
}

# ----------------------------------------------------------------------------

test_setpoint() {
    [ "$(cat "$1")" = "$2" ] && return
    echo "$1:"; cat "$1"
    echo "differs from setpoint:"; echo "$2"
    [ "${PRINT_PASSED}" -gt 1 ] && pst_exit 1
}


test_existence() {
    if [ "$2" -eq "0" ]
    then
        [ ! -f "$1" ] && echo "$1 missing!" &&
        [ "${PRINT_PASSED}" -gt 1 ] && pst_exit 1
    else
        [ -f "$1" ] && echo "$1 existing!" &&
        [ "${PRINT_PASSED}" -gt 1 ] && pst_exit 1
    fi
}


test() {
    eval "$1 2>/dev/null >/dev/null"
    if [ "$?" -eq "$2" ]
    then
        [ "${PRINT_PASSED}" -gt 0 ] \
        && printf "%-72s%-1s\n" "$1" "2>/dev/null >/dev/null (-> $2?) passed."
    else
        printf "%-72s%-1s\n" "$1" "2>/dev/null >/dev/null (-> $2?) failed!!!"
        [ "${PRINT_PASSED}" -gt 0 ] && printf "\n### Snip:\n" && eval "$1"
        [ "${PRINT_PASSED}" -gt 0 ] && printf "### Snap.\n"
        [ "${PRINT_PASSED}" -gt 1 ] && pst_exit 1
    fi
}



[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting %s get_env ...\n" "${NGINX_UTIL}"


eval $("${NGINX_UTIL}" get_env)
test '[ -n "${UCI_CONF}" ]' 0
test '[ -n "${NGINX_CONF}" ]' 0
test '[ -n "${CONF_DIR}" ]' 0
test '[ -n "${LAN_NAME}" ]' 0
test '[ -n "${LAN_LISTEN}" ]' 0
test '[ -n "${LAN_SSL_LISTEN}" ]' 0
test '[ -n "${SSL_SESSION_CACHE_ARG}" ]' 0
test '[ -n "${SSL_SESSION_TIMEOUT_ARG}" ]' 0
test '[ -n "${ADD_SSL_FCT}" ]' 0
test '[ -n "${MANAGE_SSL}" ]' 0

mkdir -p "$(dirname "${LAN_LISTEN}")"

mkdir -p "${CONF_DIR}"

cd "$(dirname "${CONF_DIR}")" && [ ! -e "$(basename "${CONF_DIR}")${ORIG}" ] &&
mv "$(basename "${CONF_DIR}")" "$(basename "${CONF_DIR}")${ORIG}" ||
{
    printf "\n%s: not moved %s to %s\n" "$(dirname "${CONF_DIR}")" \
        "$(basename "${CONF_DIR}")" "$(basename "${CONF_DIR}")${ORIG}"
    pst_exit 3
}


[ "$PRINT_PASSED" -gt 0 ] && printf "\nPrepare files in %s ...\n" "${CONF_DIR}"

mkdir -p "${CONF_DIR}"

cd "${CONF_DIR}" || pst_exit 2

NGX_INCLUDE="include '\$';"
NGX_SERVER_NAME="server_name * '\$' *;"
NGX_SSL_CRT="ssl_certificate '\$.crt';"
NGX_SSL_KEY="ssl_certificate_key '\$.key';"
NGX_SSL_SESSION_CACHE="ssl_session_cache '$(echo "${SSL_SESSION_CACHE_ARG}" \
    | sed -E "s/$(__esc_regex "${LAN_NAME}")/\$/")';"
NGX_SSL_SESSION_TIMEOUT="ssl_session_timeout '${SSL_SESSION_TIMEOUT_ARG}';"

cat > "${LAN_NAME}.sans" <<EOF
# default_server for the LAN addresses getting the IPs by:
# ifstatus lan | jsonfilter -e '@["ipv4-address","ipv6-address"].*.address'
server {
    include '${LAN_LISTEN}.default';
    server_name ${LAN_NAME};
    include conf.d/*.locations;
}
EOF
CONFS="${CONFS} ${LAN_NAME}:0"

cat > minimal.sans <<EOF
server {
    server_name minimal;
}
EOF
CONFS="${CONFS} minimal:0"

cat > listens.sans <<EOF
server {
    listen 80;
    listen 81;
    listen hostname:80;
    listen hostname:81;
    listen [::]:80;
    listen [::]:81;
    listen 1.3:80;
#    listen 1.3:80;
    listen 1.3:81;
    listen [1::3]:80;
    listen [1::3]:81;
    server_name listens;
}
EOF
CONFS="${CONFS} listens:0"

cat > normal.sans <<EOF
server {
    include '${LAN_LISTEN}';
    server_name normal;
}
EOF
CONFS="${CONFS} normal:0"

cat > acme.sans <<EOF
server {
    listen 80;
    include '${LAN_LISTEN}';
    server_name acme;
}
EOF
CONFS="${CONFS} acme:0"

cat > more_server.sans <<EOF
server {
    # include '${LAN_LISTEN}';
    server_name normal;
}
server {
    include '${LAN_LISTEN}';
    server_name more_server;
}
EOF
CONFS="${CONFS} more_server:0"

cat > more_names.sans <<EOF
server {
    include '${LAN_LISTEN}';
    include '${LAN_LISTEN}';
    include '${LAN_LISTEN}';
    not include '${LAN_LISTEN}';
    server_name example.com more_names example.org;
}
EOF
CONFS="${CONFS} more_names:0"

cat > different_name.sans <<EOF
server {
    include '${LAN_LISTEN}';
    server_name minimal;
}
EOF
CONFS="${CONFS} different_name:1"

cat > comments.sans <<EOF
server { # comment1
    # comment2
    include '${LAN_LISTEN}';
    server_name comments;
    # comment3
} # comment4
EOF
CONFS="${CONFS} comments:0"

cat > name_comment.sans <<EOF
server {
    include '${LAN_LISTEN}';
    server_name name_comment; # comment
}
EOF
CONFS="${CONFS} name_comment:0"

cat > tab.sans <<EOF
server {
	include '${LAN_LISTEN}';
	server_name tab;
}
EOF
CONFS="${CONFS} tab:0"



[ "$PRINT_PASSED" -gt 0 ] && printf "\nSetup files in %s ...\n" "${CONF_DIR}"


for conf in ${CONFS}
do test 'setpoint_add_ssl "    " '"${conf%:*}" "${conf#*:}"
done

test 'setpoint_add_ssl "\t" tab' 0 # fixes wrong indentation.



[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting Cron ... \n"


echo -n "prefix" >"/etc/crontabs/root"
test '"${NGINX_UTIL}" add_ssl _lan' 0
echo "postfix" >>"/etc/crontabs/root"
test_setpoint "/etc/crontabs/root" "prefix
3 3 12 12 * ${NGINX_UTIL} 'check_ssl'
postfix"

test '"${NGINX_UTIL}" del_ssl _lan' 0
test_setpoint "/etc/crontabs/root" "prefix
3 3 12 12 * ${NGINX_UTIL} 'check_ssl'
postfix"

test '"${NGINX_UTIL}" check_ssl' 0
test_setpoint "/etc/crontabs/root" "prefix
postfix"

test '"${NGINX_UTIL}" add_ssl _lan' 0
test_setpoint "/etc/crontabs/root" "prefix
postfix
3 3 12 12 * ${NGINX_UTIL} 'check_ssl'"

rm -f "/etc/crontabs/root"


[ "$PRINT_PASSED" -gt 0 ] && printf '\n\t-"-\t(legacy) ... \n'

echo -n "prefix" >"/etc/crontabs/root"
cp "minimal.sans" "minimal.conf"

test '"${NGINX_UTIL}" add_ssl minimal' 0
echo "postfix" >>"/etc/crontabs/root"
test_setpoint "/etc/crontabs/root" "prefix
3 3 12 12 * ${NGINX_UTIL} 'add_ssl' 'minimal'
postfix"

test '"${NGINX_UTIL}" del_ssl minimal' 0
test_setpoint "/etc/crontabs/root" "prefix
postfix"

rm -f "/etc/crontabs/root"



[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting %s init_lan ...\n" "${NGINX_UTIL}"


rm -f "${LAN_NAME}.conf" "_redirect2ssl.conf" "${UCI_ADDED}.conf"
rm -f "$(readlink "${UCI_CONF}")"

test '"${NGINX_UTIL}" init_lan' 0
test_setpoint "${UCI_CONF}" "$(setpoint_init_lan)"
test_setpoint "/etc/crontabs/root" "3 3 12 12 * ${NGINX_UTIL} 'check_ssl'"


[ "$PRINT_PASSED" -gt 0 ] && printf '\n\t-"-\twith temporary UCI config ... \n'

UCI_ADDED="$(uci add nginx server)" &&
uci set nginx.@server[-1].server_name='temp' &&
uci add_list nginx.@server[-1].listen='81 default_server' &&
uci add_list nginx.@server[-1].listen='80' &&
echo "UCI: nginx.${UCI_ADDED} added."

rm -f "${LAN_NAME}.conf" "_redirect2ssl.conf" "${UCI_ADDED}.conf"
rm -f "$(readlink "${UCI_CONF}")"

test '"${NGINX_UTIL}" init_lan' 0
test_setpoint "${UCI_CONF}" "$(setpoint_init_lan)"
test_setpoint "/etc/crontabs/root" "3 3 12 12 * ${NGINX_UTIL} 'check_ssl'"


[ "$PRINT_PASSED" -gt 0 ] && printf '\n\t-"-\t(legacy) ... \n'

cp "${LAN_NAME}.sans" "${LAN_NAME}.conf"
touch "_redirect2ssl.conf" "${UCI_ADDED}.conf"
rm -f "$(readlink "${UCI_CONF}")"
test '"${NGINX_UTIL}" init_lan' 0

skipped() {
    printf "\t# skipped UCI server 'nginx.%s'" "$1"
    printf " as it could conflict with: %s%s.conf\n\n" "${CONF_DIR}" "$1"
}
rhs="$(skipped "$LAN_NAME" && skipped _redirect2ssl && skipped "${UCI_ADDED}")"
sed -E -e "s/^\t#UCI_HTTP_CONFIG$/$(__esc_sed_rhs "$rhs")\n/" \
    -e 's/\\n/\n/g' -e "1i${fileauto}" "${UCI_CONF}.template" >"uci.setpoint"

test_setpoint "${UCI_CONF}" "$(cat "uci.setpoint")"
test_setpoint "/etc/crontabs/root" ""



[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting %s add_ssl ...\n" "${NGINX_UTIL}"


test '[ "${ADD_SSL_FCT}" = "add_ssl" ] ' 0

rm -f "${LAN_NAME}.conf" "_redirect2ssl.conf" "${UCI_ADDED}.conf"
rm -f "$(readlink "${UCI_CONF}")"
test 'uci set nginx._lan.uci_manage_ssl="self-signed"' 0
"${NGINX_UTIL}" del_ssl "${LAN_NAME}" 2>/dev/null
test_setpoint "/etc/crontabs/root" ""
test_existence "${LAN_NAME}.crt" 1
test_existence "${LAN_NAME}.key" 1
test '"${NGINX_UTIL}" add_ssl '"${UCI_ADDED}"' acme \
    '"${CONF_DIR}${UCI_ADDED}.crt"' '"${CONF_DIR}${UCI_ADDED}.key"'     ' 0
test_setpoint "/etc/crontabs/root" ""
test_existence "${UCI_ADDED}.crt" 1
test_existence "${UCI_ADDED}.key" 1
test '"${NGINX_UTIL}" add_ssl '"${LAN_NAME}" 0
test_setpoint "/etc/crontabs/root" "3 3 12 12 * ${NGINX_UTIL} 'check_ssl'"
test_existence "${LAN_NAME}.crt" 0
test_existence "${LAN_NAME}.key" 0
test '"${NGINX_UTIL}" add_ssl '"${LAN_NAME}" 0
test_setpoint "/etc/crontabs/root" "3 3 12 12 * ${NGINX_UTIL} 'check_ssl'"
test '"${NGINX_UTIL}" add_ssl inexistent' 1
test_setpoint "/etc/crontabs/root" "3 3 12 12 * ${NGINX_UTIL} 'check_ssl'"
test '"${NGINX_UTIL}" init_lan' 0
test_setpoint "${UCI_CONF}" "$(setpoint_init_lan)"
test_setpoint "/etc/crontabs/root" "3 3 12 12 * ${NGINX_UTIL} 'check_ssl'"
test_existence "${UCI_ADDED}.crt" 1
test_existence "${UCI_ADDED}.key" 1
test_existence "${LAN_NAME}.crt" 0
test_existence "${LAN_NAME}.key" 0


[ "$PRINT_PASSED" -gt 0 ] && printf '\n\t-"-\t(legacy) ... \n'

cp different_name.sans different_name.with

cp "/etc/crontabs/root" "cron.setpoint"
for conf in ${CONFS}; do
    name="${conf%:*}"
    [ "${name}" = "acme" ] && continue
    [ "${name}" = "different_name" ] ||
    echo "3 3 12 12 * ${NGINX_UTIL} 'add_ssl' '${name}'" >>"cron.setpoint"
    cp "${name}.sans" "${name}.conf"
    test '"${NGINX_UTIL}" add_ssl '"${name}" "${conf#*:}"
    test_setpoint "${name}.conf" "$(cat "${name}.with")"
    test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"
    [ "${name}" = "different_name" ] || test_existence "${name}.crt" 0
    [ "${name}" = "different_name" ] || test_existence "${name}.key" 0
done

cp acme.sans acme.conf
test '"${NGINX_UTIL}" add_ssl acme acme /path/to/crt /path/to/key' 0
test_setpoint "acme.conf" "$(cat "acme.with")"
test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"
test_existence "acme.crt" 1
test_existence "acme.key" 1



[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting %s del_ssl ...\n" "${NGINX_UTIL}"


sed -E -e 's/443 ssl/80/' -e '/[^2]ssl/d' "/etc/config/nginx" >"config.setpoint"

cp "/etc/crontabs/root" "cron.setpoint"
rm -f "${LAN_NAME}.conf" "_redirect2ssl.conf" "${UCI_ADDED}.conf"
test '"${NGINX_UTIL}" del_ssl '"${LAN_NAME}" 0
test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"
test_existence "${LAN_NAME}.crt" 1
test_existence "${LAN_NAME}.key" 1
test '"${NGINX_UTIL}" del_ssl '"${LAN_NAME}" 1
test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"

rm -f "$(readlink "${UCI_CONF}")"
sed -E "/$(__esc_regex "'check_ssl'")/d" "/etc/crontabs/root" >"cron.setpoint"
test '"${NGINX_UTIL}" init_lan' 0
test_setpoint "${UCI_CONF}" "$(setpoint_init_lan)"
test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"

touch "${UCI_ADDED}.crt" "${UCI_ADDED}.key"
test '"${NGINX_UTIL}" del_ssl "'${UCI_ADDED}'" acme' 0
test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"
test_existence "${UCI_ADDED}.crt" 0
test_existence "${UCI_ADDED}.key" 0

test '"${NGINX_UTIL}" del_ssl inexistent' 1
test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"

test_setpoint "/etc/config/nginx" "$(cat "config.setpoint")"
test '"${NGINX_UTIL}" add_ssl "'${UCI_ADDED}'" acme \
    '"${CONF_DIR}${UCI_ADDED}.crt"' '"${CONF_DIR}${UCI_ADDED}.key"'     ' 0
test '"${NGINX_UTIL}" add_ssl "'$(uci get "nginx.${UCI_ADDED}.server_name")'"' 0
test '"${NGINX_UTIL}" del_ssl "'$(uci get "nginx.${UCI_ADDED}.server_name")'"' 0
rm -f "$(readlink "${UCI_CONF}")"
sed -E "/$(__esc_regex "'check_ssl'")/d" "/etc/crontabs/root" >"cron.setpoint"
test '"${NGINX_UTIL}" init_lan' 0
test_setpoint "${UCI_CONF}" "$(setpoint_init_lan)"
test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"
test_existence "${UCI_ADDED}.crt" 1
test_existence "${UCI_ADDED}.key" 1


[ "$PRINT_PASSED" -gt 0 ] && printf '\n\t-"-\t(legacy) ... \n'

for conf in ${CONFS}; do
    name="${conf%:*}"
    [ "${name}" = "acme" ] && continue
    sed -E "/$(__esc_regex "'${name}'")/d" "/etc/crontabs/root" >"cron.setpoint"
    touch "${name}.crt" "${name}.key"
    cp "${name}.with" "${name}.conf"
    test '"${NGINX_UTIL}" del_ssl '"${name}" "${conf#*:}"
    test_setpoint "${name}.conf" "$(cat "${name}.sans")"
    test_setpoint "/etc/crontabs/root" "$(cat "cron.setpoint")"
    [ "${name}" = "different_name" ] && rm "${name}.crt" "${name}.key"
    test_existence "${name}.crt" 1
    test_existence "${name}.key" 1
done
test_setpoint "/etc/crontabs/root" ""

test '"${NGINX_UTIL}" del_ssl acme acme' 0
test_existence "acme.crt" 1
test_existence "acme.key" 1

cp acme.with acme.conf
touch acme.crt acme.key
echo "3 3 12 12 * ${NGINX_UTIL} 'add_ssl' 'acme'" >>"/etc/crontabs/root"
test '"${NGINX_UTIL}" del_ssl acme acme' 0
test_setpoint "acme.conf" "$(cat "acme.sans")"
test_setpoint "/etc/crontabs/root" "3 3 12 12 * ${NGINX_UTIL} 'add_ssl' 'acme'"
test_existence "acme.crt" 0
test_existence "acme.key" 0
"${NGINX_UTIL}" del_ssl acme 2>/dev/null
test_setpoint "/etc/crontabs/root" ""
test_existence "acme.crt" 1
test_existence "acme.key" 1


[ "$PRINT_PASSED" -gt 0 ] && printf "\nTesting without UCI ... \n"

rm -f "$(readlink "${UCI_CONF}")"

test 'uci set nginx.global.uci_enable=0' 0

test '"${NGINX_UTIL}" init_lan' 0

test '[ -e "$(readlink '"${UCI_CONF}"')" ]' 1

cp "${LAN_NAME}.sans" "${LAN_NAME}.conf"
test '"${NGINX_UTIL}" add_ssl '"${LAN_NAME}" 0
test '"${NGINX_UTIL}" add_ssl '"${LAN_NAME}" 0
test '"${NGINX_UTIL}" del_ssl '"${LAN_NAME}" 0
test '"${NGINX_UTIL}" del_ssl '"${LAN_NAME}" 0

test 'rm "${LAN_NAME}.conf"' 0
test '"${NGINX_UTIL}" add_ssl '"${LAN_NAME}" 1
test '"${NGINX_UTIL}" del_ssl '"${LAN_NAME}" 1



pst_exit 0
