
# UDPspeeder + openvpn config guide
![image_vpn](/images/en/udpspeeder+openvpn3.PNG)

# UDPspeeder command

#### run at server side
```
./speederv2 -s -l0.0.0.0:8855 -r 127.0.0.1:7777 -f20:10
```

#### run at client side
assume server ip is 45.66.77.88
```
./speederv2 -c -l0.0.0.0:3333 -r 45.66.77.88:8855 -f20:10
```

# openvpn config

#### client side config
```
client
dev tun100
proto udp

remote 127.0.0.1 3333
resolv-retry infinite 
nobind 
persist-key 
persist-tun  

ca /root/add-on/openvpn/ca.crt
cert /root/add-on/openvpn/client.crt
key /root/add-on/openvpn/client.key

keepalive 3 20
verb 3
mute 20

comp-lzo no

fragment 1200       ##### very important    you can turn it up a bit. but, the lower the safer
mssfix 1200         ##### very important

sndbuf 2000000      ##### important
rcvbuf 2000000      ##### important
txqueuelen 4000     ##### suggested
```


#### server side config
```
local 0.0.0.0
port 7777 
proto udp
dev tun 

ca /etc/openvpn/easy-rsa/2.0/keys/ca.crt
cert /etc/openvpn/easy-rsa/2.0/keys/server.crt
key /etc/openvpn/easy-rsa/2.0/keys/server.key
dh /etc/openvpn/easy-rsa/2.0/keys/dh1024.pem

server 10.222.2.0 255.255.255.0 
ifconfig 10.222.2.1 10.222.2.6

client-to-client
duplicate-cn 
keepalive 10 60 

max-clients 50

persist-key
persist-tun

status /etc/openvpn/openvpn-status.log

verb 3
mute 20  

comp-lzo no

fragment 1200       ##### very important    you can turn it up a bit. but, the lower the safer
mssfix 1200         ##### very important

sndbuf 2000000      ##### important
rcvbuf 2000000      ##### important
txqueuelen 4000     ##### suggested
```

##### Note:
If you use the `redirect-gateway` option of OpenVPN, you may need to add a route exception for your remote server ip at client side.Otherwise OpenVPN may hijack UDPspeeder 's traffic.

For example, depend on your network environment, the command may looks like:
```
ip route add 44.55.66.77 via 44.55.66.1
```
or

```
ip route add 44.55.66.77 dev XXX
```
(run at client side)

##### Other Info
You can also use tinyFecVPN,a lightweight VPN with build-in FEC support:

https://github.com/wangyu-/tinyFecVPN
