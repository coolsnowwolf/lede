#!/bin/sh
#kysdm(gxk7231@gmail.com) mod by mj for adbyby plus+ luci
alias echo_date="echo 【$(date +%Y年%m月%d日\ %X)】:"
export ADBYBY=/usr/share/adbyby
separated="—————————————————————"
coding_rules="https://coding.net/u/adbyby/p/xwhyc-rules/git/raw/master"
github_rules="https://raw.githubusercontent.com/adbyby/xwhyc-rules/master"
#hiboy_rules="http://opt.cn2qq.com/opt-file"

restart_ad(){
  echo $(date) > /tmp/adbyby.updated
  [ ! -f "/tmp/adbyby.mem" ] && /etc/init.d/adbyby restart
}
rm_cache(){
  cd /tmp
  rm -f lazy.txt video.txt local-md5.json md5.json
  rm -f /usr/share/adbyby/data/*.bak
}
judge_update(){
    if [ "$lazy_online"x == "$lazy_local"x ]; then
      echo_date "本地lazy规则已经最新，无需更新" 
        if [ "$video_online"x == "$video_local"x ]; then
          echo_date "本地video规则已经最新，无需更新"
	        echo_date "$separated检测规则更新结束$separated" 
          rm_cache && exit 0
        else
          echo_date "检测到video规则更新，下载规则中..."
          download_video;restart_ad
          echo_date "$separated检测规则更新结束$separated"
          rm_cache && exit 0
        fi
    else
      echo_date "检测到lazy规则更新，下载规则中..."
        if [ "$video_online"x == "$video_local"x ]; then
          echo_date "本地video规则已经最新，无需更新"
          download_lazy;restart_ad
          echo_date "$separated检测规则更新结束$separated"
          rm_cache && exit 0
        else
          echo_date "检测到video规则更新，下载规则中..."
          download_lazy;download_video;restart_ad
          echo_date "$separated检测规则更新结束$separated"
          rm_cache && exit 0
        fi
    fi
}

download_lazy(){
    wget-ssl --no-check-certificate -O /tmp/lazy.txt $coding_rules/lazy.txt
      if [ "$?"x != "0"x ]; then
        echo_date "【lazy】下载coding中的规则失败，尝试下载github中的规则"
        wget-ssl --no-check-certificate -O /tmp/lazy.txt $github_rules/lazy.txt
          if [ "$?"x != "0"x ]; then
            echo_date "【lazy】双双失败GG，请检查网络"
          else
            echo_date "【lazy】下载成功，正在应用..."
            cp -f /tmp/lazy.txt $ADBYBY/data/lazy.txt
          fi  
      else
        echo_date "【lazy】下载成功，正在应用..."
        cp -f /tmp/lazy.txt $ADBYBY/data/lazy.txt
      fi  
}
download_video(){
    wget-ssl --no-check-certificate -O /tmp/video.txt $coding_rules/video.txt
      if [ "$?"x != "0"x ]; then
        echo_date "【video】下载Coding中的规则失败，尝试下载Github中的规则"
        wget-ssl --no-check-certificate -O /tmp/video.txt $github_rules/video.txt
          if [ "$?"x != "0"x ]; then           
            echo_date "【video】双双失败GG，请检查网络"
          else
            echo_date "【video】下载成功，正在应用..."
            cp -f /tmp/video.txt $ADBYBY/data/video.txt
          fi       
      else
        echo_date "【video】下载成功，正在应用..."
        cp -f /tmp/video.txt $ADBYBY/data/video.txt
      fi  
}

# check_rules()
    echo_date "$separated检测规则更新中$separated" && cd /tmp
    md5sum /usr/share/adbyby/data/lazy.txt /usr/share/adbyby/data/video.txt > local-md5.json
    wget-ssl --no-check-certificate https://coding.net/u/adbyby/p/xwhyc-rules/git/raw/master/md5.json
      if [ "$?"x != "0"x ]; then
         echo_date "获取在线规则时间失败" && exit 0       
      else
         lazy_local=$(grep 'lazy' local-md5.json | awk -F' ' '{print $1}')
         video_local=$(grep 'video' local-md5.json | awk -F' ' '{print $1}')  
         lazy_online=$(sed  's/":"/\n/g' md5.json  |  sed  's/","/\n/g' | sed -n '2p')
         video_online=$(sed  's/":"/\n/g' md5.json  |  sed  's/","/\n/g' | sed -n '4p')
         echo_date "获取在线规则MD5成功，正在判断是否有更新中"
         judge_update
      fi  
