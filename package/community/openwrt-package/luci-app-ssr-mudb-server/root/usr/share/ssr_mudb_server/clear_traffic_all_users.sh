#!/bin/sh

cd /usr/share/ssr_mudb_server
user_total=$(python3 mujson_mgr.py -l | wc -l)
[ $user_total -eq 0 ] && echo -e "没有发现用户，请检查 !" && exit 1
for i in $(seq 1 $user_total)
do
	port=$(python3 mujson_mgr.py -l | sed -n ${i}p | awk '{print $4}')
	match_clear=$(python3 mujson_mgr.py -c -p "${port}")
done
exit
