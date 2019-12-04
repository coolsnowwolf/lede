local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()
local server_table = {}
local encrypt_methods = {
	"none",
	"table",
	"rc4",
	"rc4-md5-6",
	"rc4-md5",
	"aes-128-cfb",
	"aes-192-cfb",
	"aes-256-cfb",
	"aes-128-ctr",
	"aes-192-ctr",
	"aes-256-ctr",	
	"bf-cfb",
	"camellia-128-cfb",
	"camellia-192-cfb",
	"camellia-256-cfb",
	"cast5-cfb",
	"des-cfb",
	"idea-cfb",
	"rc2-cfb",
	"seed-cfb",
	"salsa20",
	"chacha20",
	"chacha20-ietf",
}

local protocol = {
	"origin",
	"verify_deflate",		
	"auth_sha1_v4",
	"auth_aes128_sha1",
	"auth_aes128_md5",
	"auth_chain_a",
	"auth_chain_b",
	"auth_chain_c",
	"auth_chain_d",
	"auth_chain_e",
	"auth_chain_f",
}

obfs = {
	"plain",
	"http_simple",
	"http_post",
	"random_head",	
	"tls1.2_ticket_auth",
}

local raw_mode = {
	"faketcp",
	"udp",
	"icmp",
}

local seq_mode = {
	"0",
	"1",
	"2",
	"3",
	"4",
}

local cipher_mode = {
	"none",
	"xor",
	"aes128cbc",
}

local auth_mode = {
	"none",
	"simple",
	"md5",
	"crc32",
}

local speeder_mode = {
	"0",
	"1",
}

uci:foreach(shadowsocksr, "servers", function(s)
	if s.alias then
		server_table[s[".name"]] = "[%s]:%s" %{string.upper(s.type), s.alias}
	elseif s.server and s.server_port then
		server_table[s[".name"]] = "[%s]:%s:%s" %{string.upper(s.type), s.server, s.server_port}
	end
end)

local key_table = {}   
for key,_ in pairs(server_table) do  
    table.insert(key_table,key)  
end 

table.sort(key_table)

m = Map(shadowsocksr)



-- [[ haProxy ]]--

s = m:section(TypedSection, "global_haproxy", translate("haProxy settings"))
s.anonymous = true

o = s:option(Flag, "admin_enable", translate("Enabling the Management Console"))
o.rmempty = false
o.default = 1

o = s:option(Value, "admin_port", translate("Service Port"))
o.datatype = "uinteger"
o.default = 1111

o = s:option(Value, "admin_user", translate("User name"))
o.default = "admin"

o = s:option(Value, "admin_password", translate("Password"))
o.default = "root"

-- [[ SOCKS5 Proxy ]]--
if nixio.fs.access("/usr/bin/ssr-local") then
s = m:section(TypedSection, "socks5_proxy", translate("SOCKS5 Proxy"))
s.anonymous = true

o = s:option(ListValue, "server", translate("Server"))
o:value("nil", translate("Disable"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end
o.default = "nil"
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.default = 1080
o.rmempty = false

-- [[ HTTP Proxy ]]--
if nixio.fs.access("/usr/sbin/privoxy") then
o = s:option(Flag, "http_enable", translate("Enable HTTP Proxy"))
o.rmempty = false

o = s:option(Value, "http_port", translate("HTTP Port"))
o.datatype = "port"
o.default = 1081
o.rmempty = false

end
end



-- [[ udp2raw ]]--
if nixio.fs.access("/usr/bin/udp2raw") then

s = m:section(TypedSection, "udp2raw", translate("udp2raw tunnel"))
s.anonymous = true

o = s:option(Flag, "udp2raw_enable", translate("Enable udp2raw"))
o.default = 0
o.rmempty = false

o = s:option(Value, "server", translate("Server Address"))
o.datatype = "host"
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "key", translate("Password"))
o.password = true
o.rmempty = false

o = s:option(ListValue, "raw_mode", translate("Raw Mode"))
for _, v in ipairs(raw_mode) do o:value(v) end
o.default = "faketcp"
o.rmempty = false

o = s:option(ListValue, "seq_mode", translate("Seq Mode"))
for _, v in ipairs(seq_mode) do o:value(v) end
o.default = "3"
o.rmempty = false

o = s:option(ListValue, "cipher_mode", translate("Cipher Mode"))
for _, v in ipairs(cipher_mode) do o:value(v) end
o.default = "xor"
o.rmempty = false

o = s:option(ListValue, "auth_mode", translate("Auth Mode"))
for _, v in ipairs(auth_mode) do o:value(v) end
o.default = "simple"
o.rmempty = false

end

-- [[ udpspeeder ]]--
if nixio.fs.access("/usr/bin/udpspeeder") then

s = m:section(TypedSection, "udpspeeder", translate("UDPspeeder"))
s.anonymous = true

o = s:option(Flag, "udpspeeder_enable", translate("Enable UDPspeeder"))
o.default = 0
o.rmempty = false

o = s:option(Value, "server", translate("Server Address"))
o.datatype = "host"
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.rmempty = false

o = s:option(Value, "key", translate("Password"))
o.password = true
o.rmempty = false

o = s:option(ListValue, "speeder_mode", translate("Speeder Mode"))
for _, v in ipairs(speeder_mode) do o:value(v) end
o.default = "0"
o.rmempty = false

o = s:option(Value, "fec", translate("Fec"))
o.default = "20:10"
o.rmempty = false

o = s:option(Value, "mtu", translate("Mtu"))
o.datatype = "uinteger"
o.default = 1250
o.rmempty = false

o = s:option(Value, "queue_len", translate("Queue Len"))
o.datatype = "uinteger"
o.default = 200
o.rmempty = false

o = s:option(Value, "timeout", translate("Fec Timeout"))
o.datatype = "uinteger"
o.default = 8
o.rmempty = false

end



return m
