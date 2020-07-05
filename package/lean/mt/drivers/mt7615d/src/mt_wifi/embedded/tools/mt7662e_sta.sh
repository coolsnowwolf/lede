ifconfig ra0 down
rmmod mt7662e_sta.ko
insmod ../os/linux/mt7662e_sta.ko mac=00:0c:43:11:22:22
ifconfig ra0 up
