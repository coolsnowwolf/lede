NSS Physical Interface Ingress Driver
=====================================

This driver redirect NSS physical interface (namely GMACs) ingress traffic to itself
and sends it back to the Linux network stack (as the source GMACs packets) as it's
egress traffic.

This allows the NSS QDISC drivers to manage the egress traffic of this driver's
NSS virtual interface.

This driver will create a single network interface named 'nssifb'.  The default
source interface is defined as 'eth0'.  It can be changed using the following module
parameter path:

/sys/module/nss-ifb/parameter/nss_src_dev

To change the source NSS physical interface to 'eth1', use the following command:

printf eth1 > /sys/module/nss-ifb/parameter/nss_src_dev

You need to change the source interface first before bringing up the 'nssifb'
interface.  Changing it after the interface is up will have no effect.  You need
to bring down the interface and bring it back up to have the changes take effect.

CPU load imposed on the Krait CPUs appears negligible with this driver intercepting
the physical interface's ingress traffic.  Full line speed of the GMAC interface
could still be achieved.

The commands below shows an example to shape ingress traffic to 500 Mbps and egress
to 200 Mbps for the 'eth0' interface.

# Load the module if it's not loaded
modprobe nss-ifb

# Bring up the nssifb interface to active ingress redirect
ip link set up nssifb

# Shape ingress traffic to 500 Mbit with chained NSSFQ_CODEL
tc qdisc add dev nssifb root handle 1: nsstbl rate 500Mbit burst 1Mb
tc qdisc add dev nssifb parent 1: handle 10: nssfq_codel limit 10240 flows 1024 quantum 1514 target 5ms interval 100ms set_default

# Shape egress traffic to 200 Mbit with chained NSSFQ_CODEL
tc qdisc add dev eth0 root handle 1: nsstbl rate 200Mbit burst 1Mb
tc qdisc add dev eth0 parent 1: handle 10: nssfq_codel limit 10240 flows 1024 quantum 1514 target 5ms interval 100ms set_default

