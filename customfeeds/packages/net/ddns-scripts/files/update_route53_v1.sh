#!/bin/sh
#.Distributed under the terms of the GNU General Public License (GPL) version 2.0
#.based on Yuval Adam's route53.sh found at https://github.com/yuvadm/route53-ddns/blob/master/route53.sh
#.2017 Max Berger <max at berger dot name>
[ -z "$CURL_SSL" ] && write_log 14 "Amazon AWS Route53 communication require cURL with SSL support. Please install"
[ -z "$username" ] && write_log 14 "Service section not configured correctly! Missing key as 'username'"
[ -z "$password" ] && write_log 14 "Service section not configured correctly! Missing secret as 'password'"
[ -z "$domain" ] && write_log 14 "Service section not configured correctly! Missing zone id as 'domain'"

set -euo pipefail
IFS=$'\n\t'

ENDPOINT="route53.amazonaws.com"
RECORD_TTL=300
RECORD_NAME="$lookup_host".
[ $use_ipv6 -eq 0 ] && RECORD_TYPE="A"
[ $use_ipv6 -eq 1 ] && RECORD_TYPE="AAAA"
RECORD_VALUE="$LOCAL_IP"
HOSTED_ZONE_ID="$domain"
API_PATH="/2013-04-01/hostedzone/${HOSTED_ZONE_ID}/rrset/"

AWS_ACCESS_KEY_ID="$username"
AWS_SECRET_ACCESS_KEY="$password"
AWS_REGION='us-east-1'
AWS_SERVICE='route53'

hash() {
    msg=$1
    echo -en "$msg" | openssl dgst -sha256 | sed 's/^.* //'
}

sign_plain() {
    # Sign message using a plaintext key
    key=$1
    msg=$2
    echo -en "$msg" | openssl dgst -hex -sha256 -hmac "$key" | sed 's/^.* //'
}

sign() {
    # Sign message using a hex formatted key
    key=$1
    msg=$2
    echo -en "$msg" | openssl dgst -hex -sha256 -mac HMAC -macopt "hexkey:${key}" | sed 's/^.* //'
}

request_body="<?xml version=\"1.0\" encoding=\"UTF-8\"?> \
<ChangeResourceRecordSetsRequest xmlns=\"https://route53.amazonaws.com/doc/2013-04-01/\"> \
<ChangeBatch> \
   <Changes> \
      <Change> \
         <Action>UPSERT</Action> \
         <ResourceRecordSet> \
            <Name>${RECORD_NAME}</Name> \
            <Type>${RECORD_TYPE}</Type> \
            <TTL>${RECORD_TTL}</TTL> \
            <ResourceRecords> \
               <ResourceRecord> \
                  <Value>${RECORD_VALUE}</Value> \
               </ResourceRecord> \
            </ResourceRecords> \
         </ResourceRecordSet> \
      </Change> \
   </Changes> \
</ChangeBatch> \
</ChangeResourceRecordSetsRequest>"

fulldate=$(date --utc +%Y%m%dT%H%M%SZ)
shortdate=$(date --utc +%Y%m%d)
signed_headers="host;x-amz-date"
request_hash=$(hash "$request_body")
canonical_request="POST\n${API_PATH}\n\nhost:route53.amazonaws.com\nx-amz-date:${fulldate}\n\n${signed_headers}\n${request_hash}"

date_key=$(sign_plain "AWS4${AWS_SECRET_ACCESS_KEY}" "${shortdate}")
region_key=$(sign "$date_key" $AWS_REGION)
service_key=$(sign "$region_key" $AWS_SERVICE)
signing_key=$(sign "$service_key" aws4_request)

credential="${shortdate}/${AWS_REGION}/${AWS_SERVICE}/aws4_request"
sigmsg="AWS4-HMAC-SHA256\n${fulldate}\n${credential}\n$(hash "$canonical_request")"

signature=$(sign "$signing_key" "$sigmsg")

authorization="AWS4-HMAC-SHA256 Credential=${AWS_ACCESS_KEY_ID}/${credential}, SignedHeaders=${signed_headers}, Signature=${signature}"

ANSWER=$(curl \
    -X "POST" \
    -H "Host: route53.amazonaws.com" \
    -H "X-Amz-Date: ${fulldate}" \
    -H "Authorization: ${authorization}" \
    -H "Content-Type: text/xml" \
    -d "$request_body" \
    "https://${ENDPOINT}${API_PATH}")
write_log 7 "${ANSWER}"

echo ${ANSWER} | grep Error >/dev/null && return 1
echo ${ANSWER} | grep ChangeInfo >/dev/null && return 0
return 2
