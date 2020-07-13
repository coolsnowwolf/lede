--Alex<1886090@gmail.com>
--Dennis<code@tossp.com>
local fs = require "nixio.fs"
local CONFIG_FILE = "/tmp/dnsforwarder.conf";

function sync_value_to_file(value, file)
	value = value:gsub("\r\n?", "\n")
	local old_value = nixio.fs.readfile(file)
	if value ~= old_value then
		nixio.fs.writefile(file, value)
	end

end
local state_msg = "" 

local dnsforwarder_on = (luci.sys.call("pidof dnsforwarder > /dev/null") == 0)
local resolv_file = luci.sys.exec("uci get dhcp.@dnsmasq[0].resolvfile")
local listen_port = luci.sys.exec("uci get dhcp.@dnsmasq[0].server")

if dnsforwarder_on then	
	state_msg = "<b><font color=\"green\">" .. translate("Running") .. "</font></b>"
else
	state_msg = "<b><font color=\"red\">" .. translate("Not running") .. "</font></b>"
end


if dnsforwarder_on and string.sub(listen_port,1,14) == "127.0.0.1#5053" then 
	state_msg=state_msg .. "，DNSmasq已经将流量定向至本软件"
elseif dnsforwarder_on then
	state_msg=state_msg .. "<b><font color=\"red\"></font></b>"
end

if resolv_file=="" then

else if dnsforwarder_on then
	state_msg=state_msg .. "<b><font color=\"red\"></font></b>"
	end
end
m=Map("dnsforwarder",translate("dnsforwarder"),translate("Dnsforwarder是企业级DNS服务器，可以通过TCP协议进行DNS解析,可以方便的使用iptables进行透明代理，配合ipset、GFWList使用效果更佳。默认上游服务器为114DNS，SSR和Redsocks的GFWList模式要依赖本软件包提供的功能").. "<br><br>状态 - " .. state_msg)
s=m:section(TypedSection,"arguments", translate("通用配置"))
s.addremove=false
s.anonymous=true
	view_enable = s:option(Flag,"enabled",translate("Enable"))
	view_dnsmasq = s:option(Flag,"dnsmasq",translate("设置成DNSmasq的上游服务器"),translate("让DNSMasq从本软件获得解析结果，支持GFWList模式"))
	view_addr = s:option(Value,"addr",translate("转发地址"),translate("请填写dnsforwarder的监听地址,默认127.0.0.1:5053,如果填写<b><font color=\"red\">208.67.222.222:5353</font></b>那么可不通过该软件获得无污染结果"))
	view_addr.default = "127.0.0.1:5053"

-- ---------------------------------------------------
s1=m:section(TypedSection,"config",translate("标准配置"))
s1.addremove=false
s1.anonymous=true
	log_enable = s1:option(Flag,"log",translate("打开文件日志"))
		log_enable.rmempty=false
		log_enable.default="false"
		log_enable.disabled="false"
		log_enable.enabled="true"
	log_size = s1:option(Value,"log_size",translate("单个日志文件大小的临界值（字节）"),translate("当日志文件大小超过这个临界值后，当前的日志文件将会被重命名，然后建立一个新的日志文件，继续记录日志"))
		log_size.datatype="uinteger"
		log_size.default="102400"
		log_size:depends ({log="true"})
	log_path = s1:option(Value,"log_path",translate("设定日志文件所在的文件夹"),translate("日志文件初始的文件名为 `dnsforwarder.log'，当超过临界值之后，将会被重命名为 `dnsforwarder.log.1'、`dnsforwarder.log.2' 等等，然后重新建立一个 dnsforwarder.log' 文件"))
		log_path.datatype="directory"
		log_path.default="/var/log/"
		log_path.readonly=true
		log_path:depends ({log="true"})
	gfw_enable = s1:option(Flag,"gfw",translate("使用GFW列表"))
		gfw_enable.rmempty=false
		gfw_enable.default="true"
		gfw_enable.disabled="false"
		gfw_enable.enabled="true"
	udp_local = s1:option(DynamicList,"udp_local",translate("本地监听"),translate("设置在本地开启的接口的IP地址和端口，可以是本地回环地址 (127.0.0.1) ，本地局域网，以及互联网<br/>如果是 IPv6 地址，请在IP两端加上方括号（不包含端口部分），例如 [::1]:53 (本地回环)、[fe80::699c:f79a:9bb6:1]:5353<br/>如果不指定端口，则默认为 53"))
	tcp_group = s1:option(DynamicList,"tcp_group",translate("解析策略(TCP)"),translate([[格式：&lt;IP1[:PORT],IP2[:PORT],...&gt; &lt;DOMAIN1,DOMAIN2,...&gt; &lt;on|PROXY1[:PORT],PROXY2[:PORT],...&gt;<br/>
			设置一个 TCP 上游服务器组，并指定通过这些上游服务器查询的域名<br/>
			第一部分参数“&lt;IP1[:PORT],IP2[:PORT],...&gt”，用于指定 TCP 上游服务器，不写端口则默认使用 53 端口<br/>
			第二部分参数“&lt;DOMAIN1,DOMAIN2,...&gt”，用于指定通过这些 TCP 上游服务器查询的域名，可以包含通配符<br/>
			第三部分参数“&lt;no|PROXY1[:PORT],PROXY2[:PORT],...&gt”，用于指定查询时使用的 Socks5 代理，不写端口则默认使用 1080 端口。`no' 代表不使用代理]]))
		tcp_group.placeholder = "8.8.8.8 * on"
		tcp_group.default=tcp_group.placeholder
	udp_group = s1:option(DynamicList,"udp_group",translate("解析策略(UDP)"),translate([[格式：&lt;IP1[:PORT],IP2[:PORT],...&gt; &lt;DOMAIN1,DOMAIN2,...&gt; &lt;on|off&gt;<br/>
		设置一个 UDP 上游服务器组，并指定通过这些上游服务器查询的域名<br/>
		第一部分参数“&lt;IP1[:PORT],IP2[:PORT],...&gt”，用于指定 UDP 上游服务器，不写端口则默认使用 53 端口<br/>
		第二部分参数“&lt;DOMAIN1,DOMAIN2,...&gt”，用于指定通过这些 UDP 上游服务器查询的域名，可以包含通配符<br/>
		第三部分参数“&lt;on|off&gt”，用于指定是否开启并发查询，`on' 为开启，`off' 为不开启<br/>
		并发查询指的是，向所有指定的 UDP 服务器发送查询请求，取最先接受到的未屏蔽的回复作为查询结果，并丢弃其余的回复]]))
		udp_group.placeholder = "9.9.9.9,119.29.29.29,223.5.5.5,114.114.114.114 * on"
		udp_group.default=udp_group.placeholder
	group_file = s1:option(DynamicList,"group_file",translate("解析策略(文件)"),translate([[
		从文件加载服务器组，
		服务器组文件的写法<a href="https://github.com/holmium/dnsforwarder/wiki/GroupFile-%E7%9A%84%E5%86%99%E6%B3%95-%7C-How-to-write-GroupFiles" target="_blank">点击这里查看</a>。]]))
		group_file.datatype="file"
	s1:option(DummyValue,"nothing","  ",translate([[
		解析策略优先级：<b>解析策略(TCP)&gt解析策略(UDP)&gt解析策略(文件)</b><br/>
		1.对于没有指定服务器的域名，会随机选择一个服务器组进行查询。<br/>
		2.如果某域名匹配多个服务器组，则选择的顺序如下：<br/>
		2.1 优先选择不含通配符的匹配项，如果仍然有多条匹配，则选择匹配度最高的那个，
		如果匹配度相同的情况下仍然有多条匹配，则选择最后一个。
		（例如 `ipv6.microsoft.com' 比 `microsoft.com' 对于域名 `teredo.ipv6.microsoft.com' 的匹配度更高）<br/>
		2.2 然后选择包含通配符的匹配项，如果有多条匹配，则选择第一个.
	]]))
	block_ip = s1:option(DynamicList,"block_ip",translate("DNS黑名单"),translate("阻挡含有以上 IP 地址的 DNS 数据包,IPv6 地址不用加方括号"))
		block_ip.datatype="ipaddr"		
		block_ip.delimiter = ","
	ip_substituting = s1:option(DynamicList,"ip_substituting",translate("替换 DNS 数据包"),translate("替换 DNS 数据包中的 IP 地址（只支持 IPv4 地址）,例如：<br/><code>127.0.0.1 1.2.0.127</code>的效果是把 DNS 数据包中所有的 127.0.0.1 地址替换为 1.2.0.127,仅替换通过服务器（TCP 和 UDP）查询而来的 DNS 数据包，对于缓存中和 Hosts 中的结果无效"))
		ip_substituting.placeholder = "127.0.0.1 1.2.0.127"
	block_negative_resp = s1:option(Flag,"block_negative_resp",translate("过滤失败请求"),translate("是否过滤来自上游服务器的查询未成功的响应。查询未成功指：格式错误、服务器错误、域名不存在和服务器拒绝请求等，参见 RFC 6895，`2.3.  RCODE Assignment'"))
		block_negative_resp.rmempty=false
		block_negative_resp.default="false"
		block_negative_resp.disabled="false"
		block_negative_resp.enabled="true"
	append_host = s1:option(DynamicList,"append_host",translate("附加hosts"),translate([[
		写法与HOSTS文件中的一样，同样支持通配符,IPv6 地址不用加方括号<br>
		也可以定义 CName 的 Hosts 项，例如：<code>www.google.cn *.google.com</code>,这样所有匹配 *.google.com 的域名都将指向 www.google.cn 的 IP 地址<br>
		如果使某些域名跳过在 hosts 中的查询，可以<code>@@ *.012345.com</code>这样所有匹配 *.012345.com 的域名都不会在 hosts 中查询<br>
		各种 hosts 的优先级从高到低:1.带 `@@' 的禁止项 2.一般 hosts 项 3.CName hosts 项<br>
	]]))
	block_ipv6 = s1:option(Flag,"block_ipv6",translate("拒绝IPv6 地址"),translate("如果一个域名在 Hosts 被指定了一个 IPv4 地址（包括文件 Hosts 和 `AppendHosts' 指定的 Hosts），那么拒绝对这个域名的 IPv6 地址查询"))
		block_ipv6.rmempty=false
		block_ipv6.default="false"
		block_ipv6.disabled="false"
		block_ipv6.enabled="true"

	cache = s1:option(Flag,"cache",translate("使用缓存"),translate("是否使用缓存"))
		cache.rmempty=false
		cache.default="false"
		cache.disabled="false"
		cache.enabled="true"
	cache_size = s1:option(Value,"cache_size",translate("缓存大小 (字节)"),translate("缓存大小不能小于 102400 (100KB)"))
		cache_size.datatype="min(102400)"
		cache_size.default="1048576"
		cache_size:depends ({cache="true"})
	cache_file = s1:option(Value,"cache_file",translate("缓存文件"))
		cache_file.default="/tmp/dnsforwarder.cache"
		cache_file.readonly=true
		cache_file:depends ({cache="true"})
	cache_ignore = s1:option(Flag,"cache_ignore",translate("忽略TTL"),translate("为 `true' 时将忽略 TTL，此时所有的缓存条目将不会被移除,为 `false' 时不忽略 TTL，缓存条目将会按照 TTL 的时间来移除"))
		cache_ignore.rmempty=false
		cache_ignore.default="false"
		cache_ignore.disabled="false"
		cache_ignore.enabled="true"
		cache_ignore:depends ({cache="true"})
	cache_control = s1:option(DynamicList,"cache_control",translate("控制域名的缓存行为"),translate([[
		单条格式：&lt;DOMAIN&gt; [$ [$] ]&lt;TYPE&gt; [ARGUMENT]<br/>
		&lt;DOMAIN&gt; 是域名；[$ [$] ] 是传染设定；&lt;TYPE&gt; 是类型；[ARGUMENT] 是参数<br/>
		&lt;DOMAIN&gt; 支持通配符，支持尾部匹配<br/>
		&lt;TYPE&gt; 有以下可选项（均为小写）：<br>
		&nbsp;&nbsp;&nbsp;&nbsp;orig    : 使用原先的 TTL，即不改变指定域名记录的 TTL，此类型无需 [ARGUMENT] 参数<br>
		&nbsp;&nbsp;&nbsp;&nbsp;nocache : 不缓存指定域名记录，此类型无需 [ARGUMENT] 参数<br>
		&nbsp;&nbsp;&nbsp;fixed   : 将指定域名记录的 TTL 修改为一个预先指定的常数值，这个值通过 [ARGUMENT] 参数指定<br>
		&nbsp;&nbsp;&nbsp;&nbsp;vari    : 将指定域名记录的 TTL 修改为一个预先规定的变量，这个变量通过 [ARGUMENT] 参数指定，此时 [ARGUMENT] 参数的格式为 `ax+b'，其中 a、b 为非负整数（需用户手工指定），x 为记录原先的 TTL 值（不需要用户指定）。此选项必须为 `ax+b' 的形式，不能是 `ax-b' 或者其它的变体形<br>
		<br/>传染机制：<br/>
		&nbsp;&nbsp;&nbsp;&nbsp;主动传染 （此为默认情况）:主动传染 : 所有的域名记录不论是否存在单独指定的控制条目，均使用上级域名控制条目<br>
		&nbsp;&nbsp;&nbsp;&nbsp;被动传染（&lt;TYPE&gt; 前加一个 `$'）: 如果存在单独指定的控制条目，那么使用单独指定的控制条目，否则使用上级域名的控制条目<br>
		&nbsp;&nbsp;&nbsp;&nbsp;不传染（<TYPE> 前加两个 `$'）: 如果存在单独指定的控制条目，那么使用单独指定的控制条目，如果没有则不修改记录的 TTL 值<br>
		只有被查询的域名控制条目才具有传染性质<br/>
		<br/>常见例子：<br/>
		<code>* fixed 300</code>强制使所有缓存的条目的 TTL 为 300<br/>
		<code>* vari 2x+0</code>将所有缓存条目的 TTL 加倍。即原来为300，现在为600<br/>
	]]))
		cache_control:depends ({cache="true"})

	domain_statistic = s1:option(Flag,"domain_statistic",translate("启用域名统计"),translate("域名信息统计会依照模板文件记录域名的查询情况<b>目前只能生成到<code>/root/.dnsforwarder/statistic.html</code>会影响闪存寿命不建议开启</b>"))
		domain_statistic.rmempty=false
		domain_statistic.default="false"
		domain_statistic.disabled="false"
		domain_statistic.enabled="true"

	view_cfg = s1:option(TextValue, "1", nil)
	view_cfg.rmempty = false
	view_cfg.rows = 24
	view_cfg.readonly=true

	function view_cfg.cfgvalue()
		return nixio.fs.readfile(CONFIG_FILE) or ""
	end
	function view_cfg.write(self, section, value)
	end

-- ---------------------------------------------------
local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("/etc/init.d/dnsforwarder makeconfig >/dev/null 2>&1 &")
	os.execute("/etc/init.d/dnsforwarder restart >/dev/null 2>&1 &")
end

return m
