#!/bin/sh

if [ ! -f "/tmp/adbyby.updated" ];then
  touch /tmp/adbyby.mem
  wget_ok="0"
  while [ "$wget_ok" = "0" ] 
    do 
    wget-ssl --spider --quiet --tries=1 --timeout=3 www.baidu.com
    if [ "$?" == "0" ]; then
      wget_ok="1"
      touch /tmp/lazy.txt && wget-ssl --no-check-certificate -t 1 -T 10 -O /tmp/lazy.txt https://coding.net/u/adbyby/p/xwhyc-rules/git/raw/master/lazy.txt
      touch /tmp/video.txt && wget-ssl --no-check-certificate -t 1 -T 10 -O /tmp/video.txt https://coding.net/u/adbyby/p/xwhyc-rules/git/raw/master/video.txt
      
      touch /tmp/local-md5.json && md5sum /tmp/lazy.txt /tmp/video.txt > /tmp/local-md5.json
      touch /tmp/md5.json && wget-ssl --no-check-certificate -t 1 -T 10 -O /tmp/md5.json https://coding.net/u/adbyby/p/xwhyc-rules/git/raw/master/md5.json
      
      lazy_local=$(grep 'lazy' /tmp/local-md5.json | awk -F' ' '{print $1}')
      video_local=$(grep 'video' /tmp/local-md5.json | awk -F' ' '{print $1}')  
      lazy_online=$(sed  's/":"/\n/g' /tmp/md5.json  |  sed  's/","/\n/g' | sed -n '2p')
      video_online=$(sed  's/":"/\n/g' /tmp/md5.json  |  sed  's/","/\n/g' | sed -n '4p')
      
      if [ "$lazy_online"x == "$lazy_local"x -a "$video_online"x == "$video_local"x ]; then
          echo "adbyby rules MD5 OK!"
          mv /tmp/lazy.txt /usr/share/adbyby/data/lazy.txt
          mv /tmp/video.txt /usr/share/adbyby/data/video.txt
          echo $(date +%F) > /tmp/adbyby.updated
          logger "adbyby mem mode rules updated!"
      fi    
    else
      sleep 10
    fi
   done
   rm -f /tmp/adbyby.mem /tmp/lazy.txt /tmp/video.txt /tmp/local-md5.json /tmp/md5.json
   rm -f /usr/share/adbyby/data/*.bak
   sleep 10 && /etc/init.d/adbyby restart
fi
