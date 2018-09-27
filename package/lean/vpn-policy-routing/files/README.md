# VPN Policy-Based Routing

## Description

This service allows you to define rules (policies) for routing traffic via WAN or your L2TP, Openconnect, OpenVPN, PPTP or Wireguard tunnels. Policies can be set based on any combination of local/remote ports, local/remote IPv4 or IPv6 addresses/subnets or domains. This service supersedes the [VPN Bypass](https://github.com/openwrt/packages/blob/master/net/vpnbypass/files/README.md) service, by supporting IPv6 and by allowing you to set explicit rules not just for WAN interface (bypassing OpenVPN tunnel), but for L2TP, Openconnect, OpenVPN, PPTP and Wireguard tunnels as well.

## Features

### Gateways/Tunnels

- Any policy can target either WAN or a VPN tunnel interface.
- L2TP tunnels supported (with protocol names l2tp\*).
- Openconnect tunnels supported (with protocol names openconnect\*).
- OpenVPN tunnels supported (with device names tun\* or tap\*).
- PPTP tunnels supported (with protocol names pptp\*).
- Wireguard tunnels supported (with protocol names wireguard\*).

### IPv4/IPv6/Port-Based Policies

- Policies based on local names, IPs or subnets. You can specify a single IP (as in ```192.168.1.70```) or a local subnet (as in ```192.168.1.81/29```) or a local device name (as in ```nexusplayer```). IPv6 addresses are also supported.
- Policies based on local ports numbers. Can be set as an individual port number (```32400```), a range (```5060-5061```), a space-separated list (```80 8080```) or a combination of the above (```80 8080 5060-5061```). Limited to 15 space-separated entries per policy.
- Policies based on remote IPs/subnets or domain names. Same format/syntax as local IPs/subnets.
- Policies based on remote ports numbers. Same format/syntax and restrictions as local ports.
- You can mix the IP addresses/subnets and device (or domain) names in one field separating them by space (like this: ```66.220.2.74 he.net tunnelbroker.net```).

### DSCP-tag Based Policies

You can also set policies for traffic with specific DSCP tag. On Windows 10, for example, you can mark traffic from specific apps with DSCP tags (instructions for tagging specific app traffic in Windows 10 can be found [here](http://serverfault.com/questions/769843/cannot-set-dscp-on-windows-10-pro-via-group-policy)).

### Strict enforcement

- Supports strict policy enforcement, even if the policy interface is down -- resulting in network being unreachable for specific policy (enabled by default).

### Use DNSMASQ

- Service can be set to utilize ```dnsmasq```'s ```ipset``` support. This requires the ```dnsmasq-full``` to be installed (see [How to install dnsmasq-full](#how-to-install-dnsmasq-full)) and it significantly improves the start up time because ```dnsmasq``` resolves the domain names and adds them to appropriate ```ipset``` in background. Another benefit of using ```dnsmasq```'s ```ipset``` is that it also automatically adds third-level domains to the ```ipset```: if ```domain.com``` is added to the policy, this policy will affect all ```*.domain.com``` subdomains. This also works for top-level domains as well, a policy targeting the ```at``` for example, will affect all the ```*.at``` domains.

### Customization

- Can be fully configured with ```uci``` commands or by editing ```/etc/config/vpn-policy-routing``` file.
- Has a companion package (```luci-app-vpn-policy-routing```) so policies can be configured with Web UI.

### Other Features

- Doesn't stay in memory, creates the routing tables and ```iptables``` rules/```ipset``` entries which are automatically updated when supported/monitored interface changes.
- Proudly made in Canada, using locally-sourced electrons.

## Screenshot (luci-app-vpn-policy-routing)

Basic Settings
![screenshot](https://raw.githubusercontent.com/stangri/openwrt_packages/master/screenshots/vpn-policy-routing/screenshot03-basic.png "screenshot")
Advanced Settings
![screenshot](https://raw.githubusercontent.com/stangri/openwrt_packages/master/screenshots/vpn-policy-routing/screenshot03-advanced.png "screenshot")

## How it works

On start, this service creates routing tables for each supported interface (WAN/WAN6 and VPN tunnels) which are used to route specially marked packets. Service adds new ```VPR_PREROUTING``` chain in the ```mangle``` table's ```PREROUTING``` chain (can be optionally set to create chains in the ```FORWARD```, ```INPUT``` and ```OUTPUT``` chains of ```mangle``` table, see [Additional settings](#additional-settings) for details). Evaluation and marking of packets happens in the ```VPR_PREROUTING``` (and if enabled, also in ```VPR_FORWARD```, ```VPR_INPUT``` and ```VPR_OUTPUT```) chains. If enabled, the service also creates the ```ipset``` per each supported interface and the corresponding ```iptables``` rule for marking packets matching the ```ipset```. The service then processes the user-created policies.

### Processing Policies

Each policy can result in either a new ```iptables``` rule or, if ```ipset``` or use of ```dnsmasq``` are enabled, an ```ipset``` or a ```dnsmasq```'s ```ipset``` entry.

- Policies with local IP addresses or local device names are always created as ```iptables``` rules.
- Policies with local or remote ports are always created as ```iptables``` rules.
- Policies with local or remote netmasks are always created as ```iptables``` rules.
- Policies with **only** remote IP address or a domain name are created as ```dnsmasq```'s ```ipset``` or an ```ipset``` (if enabled).

### Policies Priorities

- If support for ```dnsmasq```'s ```ipset``` and ```ipset``` is disabled, then only ```iptables``` rules are created. The policy priority is the same as its order as listed in Web UI and ```/etc/config/vpn-policy-routing```. The higher the policy is in the Web UI and configuration file, the higher its priority is.
- If support for ```dnsmasq```'s ```ipset``` and ```ipset``` is enabled, then the ```ipset``` entries have the highest priority (irrelevant of their position in the policies list) and the other policies are processed in the same order as they are listed in Web UI and ```/etc/config/vpn-policy-routing```.
- If there are conflicting ```ipset``` entries for different interfaces, the priority is given to the interface which is listed first in the ```/etc/config/network``` file.
- If set, the ```DSCP``` policies trump all other policies, including ```ipset``` ones.

## Requirements

This service requires the following packages to be installed on your router: ```ipset```, ```resolveip```, ```ip-full``` (or a ```busybox``` built with ```ip``` support), ```kmod-ipt-ipset``` and ```iptables```.

To satisfy the requirements, connect to your router via ssh and run the following commands:

```sh
opkg update; opkg install ipset resolveip ip-full kmod-ipt-ipset iptables
```

### How to install dnsmasq-full

If you want to use ```dnsmasq```'s ```ipset``` support, you will need to install ```dnsmasq-full``` instead of the ```dnsmasq```. To do that, connect to your router via ssh and run the following command:

```sh
opkg update; opkg remove dnsmasq; opkg install dnsmasq-full
```

### Unmet dependencies

If you are running a development (trunk/snapshot) build of OpenWrt/LEDE Project on your router and your build is outdated (meaning that packages of the same revision/commit hash are no longer available and when you try to satisfy the [requirements](#requirements) you get errors), please flash either current LEDE release image or current development/snapshot image.

## How to install

Please make sure that the [requirements](#requirements) are satisfied and install ```vpn-policy-routing``` and ```luci-app-vpn-policy-routing``` from Web UI or connect to your router via ssh and run the following commands:

```sh
opkg update
opkg install vpn-policy-routing luci-app-vpn-policy-routing
```

If these packages are not found in the official feed/repo for your version of OpenWrt/LEDE Project, you will need to [add a custom repo to your router](#add-custom-repo-to-your-router) first.

### Add custom repo to your router

If your router is not set up with the access to repository containing these packages you will need to add custom repository to your router by connecting to your router via ssh and running the following commands:

#### OpenWrt 15.05.1 Instructions

```sh
opkg update; opkg install ca-certificates wget libopenssl
echo -e -n 'untrusted comment: LEDE usign key of Stan Grishin\nRWR//HUXxMwMVnx7fESOKO7x8XoW4/dRidJPjt91hAAU2L59mYvHy0Fa\n' > /tmp/stangri-repo.pub && opkg-key add /tmp/stangri-repo.pub
! grep -q 'stangri_repo' /etc/opkg/customfeeds.conf && echo 'src/gz stangri_repo https://raw.githubusercontent.com/stangri/openwrt-repo/master' >> /etc/opkg/customfeeds.conf
opkg update
opkg install vpn-policy-routing luci-app-vpn-policy-routing
```

#### LEDE Project 17.01.x and OpenWrt 18.06.x Instructions

```sh
opkg update
opkg list-installed | grep -q uclient-fetch || opkg install uclient-fetch
opkg list-installed | grep -q libustream || opkg install libustream-mbedtls
echo -e -n 'untrusted comment: LEDE usign key of Stan Grishin\nRWR//HUXxMwMVnx7fESOKO7x8XoW4/dRidJPjt91hAAU2L59mYvHy0Fa\n' > /tmp/stangri-repo.pub && opkg-key add /tmp/stangri-repo.pub
! grep -q 'stangri_repo' /etc/opkg/customfeeds.conf && echo 'src/gz stangri_repo https://raw.githubusercontent.com/stangri/openwrt-repo/master' >> /etc/opkg/customfeeds.conf
opkg update
opkg install vpn-policy-routing luci-app-vpn-policy-routing
```

## Default Settings

Default configuration has service disabled (use Web UI to enable/start service or run ```uci set vpn-policy-routing.config.enabled=1; uci commit vpn-policy-routing;```).

## Additional settings

The ```vpn-policy-routing``` settings are split into ```basic``` and ```advanced``` settings. The full list of configuration parameters of ```vpn-policy-routing.config``` section is:

|Web UI Section|Parameter|Type|Default|Comment|
| --- | --- | --- | --- | --- |
|Basic|enabled|boolean|0|Enable/disable the ```vpn-policy-routing``` service.|
|Basic|verbosity|integer|2|Can be set to 0, 1 or 2 to control the console and system log output verbosity of the ```vpn-policy-routing``` service.|
|Basic|strict_enforcement|boolean|1|Enforce policies when their interface is down. See [Strict enforcement](#strict-enforcement) for more details.|
|Basic|dnsmasq_enabled|boolean|1|Enable/disable use of ```dnsmasq``` for ```ipset``` entries. See [Use DNSMASQ](#use-dnsmasq) for more details. Assumes ```ipset_enabled=1```. Make sure the [requirements](#requirements) are met.|
|Basic|ipset_enabled|boolean|1|Enable/disable use of ```ipset``` entries for compatible policies. This speeds up service start-up and operation. Make sure the [requirements](#requirements) are met. This setting is hidden in Web UI unless ```Use DNSMASQ for domain policies``` is disabled.|
|Basic|ipv6_enabled|boolean|1|Enable/disable IPv6 support.|
|Advanced|supported_interface|list/string||Allows to specify the list of interface names (in lower case) to be explicitly supported by the ```vpn-policy-routing``` service. Can be useful if your OpenVPN tunnels have dev option other than tun\* or tap\*.|
|Advanced|ignored_interface|list/string||Allows to specify the list of interface names (in lower case) to be ignored by the ```vpn-policy-routing``` service. Can be useful if running both VPN server and VPN client on the router.|
|Advanced|iprule_enabled|boolean|0|Add an ```ip rule```, not an ```iptables``` entry for policies with just the local address. Use with caution to manipulate policies priorities.|
|Advanced|udp_proto_enabled|boolean|0|Add ```UDP``` protocol ```iptables``` rules for protocol policies with unset local addresses and either local or remote port set. By default (unless this variable is set to 1) only ```TCP``` protocol iptables rules are added.|
|Advanced|forward_chain_enabled|boolean|0|Create and use a ```FORWARD``` chain in the mangle table. By default the ```vpn-policy-routing``` only creates and uses the ```PREROUTING``` chain. Use with caution.|
|Advanced|input_chain_enabled|boolean|0|Create and use an ```INPUT``` chain in the mangle table. By default the ```vpn-policy-routing``` only creates and uses the ```PREROUTING``` chain. Use with caution.|
|Advanced|output_chain_enabled|boolean|0|Create and use an ```OUTPUT``` chain in the mangle table. By default the ```vpn-policy-routing``` only creates and uses the ```PREROUTING``` chain. Policies in the ```OUTPUT``` chain will affect traffic from the router itself. All policies with unset local address will be duplicated in the ```OUTPUT``` chain. Use with caution.|
|Advanced|icmp_interface|string||Set the default ICMP protocol interface (interface name in lower case). Requires ```output_chain_enabled=1```. This setting is hidden in Web UI unless ```Create OUTPUT Chain``` is enabled. Use with caution.|
|Advanced|wan_tid|integer|201|Starting (WAN) Table ID number for tables created by the ```vpn-policy-routing``` service.|
|Advanced|wan_mark|hexadecimal|0x010000|Starting (WAN) fw mark for marks used by the ```vpn-policy-routing``` service. High starting mark is used to avoid conflict with SQM/QoS, this can be changed by user. Change with caution together with ```fw_mask```.|
|Advanced|fw_mask|hexadecimal|0xff0000|FW Mask used by the ```vpn-policy-routing``` service. High mask is used to avoid conflict with SQM/QoS, this can be changed by user. Change with caution together with ```wan_mark```.|
||wan_dscp|integer||Allows use of [DSCP-tag based policies](#dscp-tag-based-policies) for WAN interface.|
||{interface_name}_dscp|integer||Allows use of [DSCP-tag based policies](#dscp-tag-based-policies) for a VPN interface.|

### Example Policies

```text
config policy
	option name 'Plex Local Server'
	option interface 'wan'
	option local_ports '32400'

config policy
	option name 'Plex Remote Servers'
	option interface 'wan'
	option remote_addresses 'plex.tv my.plexapp.com'

config policy
	option name 'LogmeIn Hamachi'
	option interface 'wan'
	option remote_addresses '25.0.0.0/8 hamachi.cc hamachi.com logmein.com'

config policy
	option name 'Local Subnet'
	option interface 'wan'
	option local_addresses '192.168.1.81/29'

config policy
	option name 'Local IP'
	option interface 'wan'
	option local_addresses '192.168.1.70'
```

### Multiple OpenVPN Clients

If you use multiple OpenVPN clients on your router, the order in which their devices are named (tun0, tun1, etc) is not guaranteed by OpenWrt/LEDE Project. The following settings are recommended in this case.

For ```/etc/config/network```:

```text
config interface 'vpnclient0'
	option proto 'none'
	option ifname 'ovpnc0'

config interface 'vpnclient1'
	option proto 'none'
	option ifname 'ovpnc1'
```

For ```/etc/config/openvpn```:

```text
config openvpn 'vpnclient0'
	option client '1'
	option dev_type 'tun'
	option dev 'ovpnc0'
	...

config openvpn 'vpnclient1'
	option client '1'
	option dev_type 'tun'
	option dev 'ovpnc1'
	...
```

For ```/etc/config/vpn-policy-routing```:

```text
config vpn-policy-routing 'config'
	list supported_interface 'vpnclient0 vpnclient1'
	...
```

## Discussion

Please head to [LEDE Project Forum](https://forum.lede-project.org/t/vpn-policy-based-routing-web-ui-discussion/10389) for discussions of this service.

## Getting help

If things are not working as intended, please include the following in your post:

- content of ```/etc/config/vpn-policy-routing```
- the output of ```/etc/init.d/vpn-policy-routing status```
- the output of ```/etc/init.d/vpn-policy-routing reload``` with verbosity setting set to 2

If you don't want to post the ```/etc/init.d/vpn-policy-routing status``` output in a public forum, there's a way to have the support details automatically uploaded to my account at paste.ee by running: ```/etc/init.d/vpn-policy-routing status -p```. You need to have the following packages installed to enable paste.ee upload functionality: ```curl libopenssl ca-bundle```. WARNING: while paste.ee uploads are unlisted, they are still publicly available.

## Notes/Known Issues

- While you can select some down/inactive VPN tunnel in Web UI, the appropriate tunnel must be up/active for the policies to properly work without errors on service start.
- If your ```OpenVPN``` interface has the device name different from tun\* or tap\*, please make sure that the tunnel is up before trying to assign it policies in Web UI.
- Service does not alter the default routing. Depending on your VPN tunnel settings (and settings of the VPN server you are connecting to), the default routing might be set to go via WAN or via VPN tunnel. This service affects only routing of the traffic matching the policies. If you want to override default routing, consider adding the following to your OpenVPN tunnel config:

  ```text
  option route_nopull '1'
  ```

  <!-- option route '0.0.0.0 0.0.0.0'  -->
  or set the following option for your Wireguard tunnel config:

  ```text
  option route_allowed_ips '0'
  ```

## Thanks

I'd like to thank everyone who helped create, test and troubleshoot this service. Without contributions from [@hnyman](https://github.com/hnyman), [@dibdot](https://github.com/dibdot), [@danrl](https://github.com/danrl), [@tohojo](https://github.com/tohojo), [@cybrnook](https://github.com/cybrnook), [@nidstigator](https://github.com/nidstigator), [@AndreBL](https://github.com/AndreBL) and [@dz0ny](https://github.com/dz0ny) and rigorous testing by [@dziny](https://github.com/dziny), [@bluenote73](https://github.com/bluenote73), [@buckaroo](https://github.com/pgera) and [@Alexander-r](https://github.com/Alexander-r) it wouldn't have been possible. Wireguard support is courtesy of [Mullvad](https://www.mullvad.net).
