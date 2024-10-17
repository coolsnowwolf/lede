#!/bin/sh
# https://github.com/isecret/leigod-helper/blob/main/leigod-helper.sh

USERNAME="${USERNAME:-phone}"
PASSWORD="${PASSWORD:-password}"

if ! command -v "jq" > /dev/null; then
    echo "缺失 jq 依赖";
    exit;
fi

if command -v md5sum > /dev/null; then
    password_hash=$(echo -n "$PASSWORD" | md5sum | awk '{print $1}')
elif command -v md5 > /dev/null; then
    password_hash=$(echo -n "$PASSWORD" | md5)
else
    echo "缺失 md5 或 md5sum 依赖，请手动安装"
    exit 1
fi

login=$(curl -Lks -X POST https://webapi.leigod.com/api/auth/login \
    -H 'content-type: application/json' \
    -d "{
        \"account_token\": null,
        \"country_code\": 86,
        \"lang\": \"zh_CN\",
        \"password\": \"$password_hash\",
        \"region_code\": 1,
        \"src_channel\": \"guanwang\",
        \"user_type\": \"0\",
        \"username\": \"$USERNAME\"
    }")

login_code=$(echo $login | jq -r ".code" 2>&1)
login_msg=$(echo $login | jq -r ".msg" 2>&1)

if [ "$login_code" -eq 0 ]; then
    account_token=$(echo $login | jq -r ".data.login_info.account_token" 2>&1)
    pause=$(curl -Lks -X POST https://webapi.leigod.com/api/user/pause \
        -H 'content-type: application/json' \
        -d "{
                \"account_token\": \"$account_token\",
                \"lang\": \"zh_CN\"
            }")
    pause_code=$(echo $pause | jq -r ".code" 2>&1)
    pause_msg=$(echo $pause | jq -r ".msg" 2>&1)

    echo "暂停结果: $pause_msg"
else
    echo "登录失败...原因: $login_msg"
fi
