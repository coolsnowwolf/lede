#!/usr/bin/lua

local libubus = require "ubus"
local uloop = require "uloop"

local UBUS_STATUS_OK = 0
local UBUS_STATUS_INVALID_COMMAND = 1
local UBUS_STATUS_INVALID_ARGUMENT = 2
local UBUS_STATUS_METHOD_NOT_FOUND = 3
local UBUS_STATUS_NOT_FOUND = 4
local UBUS_STATUS_NO_DATA = 5
local UBUS_STATUS_PERMISSION_DENIED = 6
local UBUS_STATUS_TIMEOUT = 7
local UBUS_STATUS_NOT_SUPPORTED = 8
local UBUS_STATUS_UNKNOWN_ERROR = 9
local UBUS_STATUS_CONNECTION_FAILED = 10
local UBUS_STATUS_ALREADY_EXISTS = 11

local cfg_file = "/etc/appfilter/feature.cfg"

local cfg = {}
local class = {}
local ubus

cfg.__index = cfg
class.__index = class
function cfg:init(file)
    local f = io.open(file, "r")
    local t = {}
    local t2 = {}
    if f then
        for line in f:lines() do
            table.insert(t, line)
            local tt = line:match("#class (%S+)")
            if tt then 
                table.insert(t2, tt)
            end
        end
        setmetatable(t, self)
        setmetatable(t2, self)
        return t,t2
    end
    return nil
end

function cfg:lookup(o)
    if not o then return UBUS_STATUS_INVALID_ARGUMENT end
	local tab = self
    for _, v in ipairs(tab) do
        if v:match(o) then
            if v:match("#class") then
                local tt = {}
                local t2 = {}
                local found
                for _, t in ipairs(tab) do
                    repeat
                        if t:match(o) then
                            found = true
                            table.insert(tt, t)
                            break
                        end

                        if t:match("#class") then
                            found = false
                            table.insert(t2, t)
                            break
                        end

                        if found then
                            table.insert(tt, t)
                        else
                            table.insert(t2, t)
                        end
                    until true
                end
                setmetatable(tt,  self)
                setmetatable(t2, self)
                return tt, t2
            else
                return v
            end
        end
    end
    return nil
end

function cfg:lookup_class(m)
    if not m then return UBUS_STATUS_INVALID_ARGUMENT end
	local t1, t2 = self:lookup(m)
	if type(t1) ~= "table" then return nil end
	return t1, t2
end

function cfg:add_class(m)
    if not m then return UBUS_STATUS_INVALID_ARGUMENT end
    local f = io.open(cfg_file, "r+")
    local tab = self
    if f then
        io.output(f)
        for _, v in ipairs(tab) do
            io.write(v)
            io.write("\n")
        end
        io.write("#class "..m)
        f:flush()
        f:close()
        return UBUS_STATUS_OK
    else
        return UBUS_STATUS_NOT_FOUND
    end
end

function cfg:add_app(m, name, proto, sport, dport, url, request, dict)
    if not name then return UBUS_STATUS_INVALID_ARGUMENT end
	local id
	local offset
    local f = io.open(cfg_file, "r+")
    io.output(f)
    local t1,t2 = self:lookup_class(m)
    if t1[#t1] == nil or "" then
    	offset = 0
    	id = math.modf(string.match(t1[#t1-1], "(%d+) %S+:") +1)
    else
		offset = 1
		id = math.modf(string.match(t1[#t1], "(%d+) %S+:") +1)
    end
     
    local str = string.format("%d %s:[%s;%s;%s;%s;%s;%s]", id, name, proto, sport or "", dport or "", url or "", request or "", dict or "")
    table.insert(t1, #t1+offset, str)
    if f then
        for _, v in ipairs(t2) do
			if v then
				io.write(v)
				io.write("\n")
            end
        end
        for _, v in ipairs(t1) do
			if v then
				io.write(v)
				io.write("\n")
            end
        end
        f:flush()
        f:close()
    end
    return id
end

function cfg:del_app(id, name)
    local t = self
    local f = io.open(cfg_file, "r+")
    local ret
    if id then
        for i, v in ipairs(t) do
            if v:match(id) then
                table.remove(t, i)
                ret = i
            end
        end

    end

    if name then
        for i, v in ipairs(t) do
            if v:match(name) then
                table.remove(t, i)
                ret = i
            end
        end
    end

    if f then
        io.output(f)
        for _, v in ipairs(t) do
            io.write(v)
            io.write("\n")
        end
        f:flush()
        f:close()
    end
    return ret
end

local methods = {
	["appfilter"] = {
		add_class = {
            function(req, msg)
                if not msg.class then return UBUS_STATUS_INVALID_ARGUMENT end
                local t = cfg:init(cfg_file)
                local ret
                if t:lookup_class(msg.class) then return ubus.reply(req, {ret = UBUS_STATUS_ALREADY_EXISTS}) end
                ret = t:add_class(msg.class)
				ubus.reply(req, {msg = ret})
			end, {class = libubus.STRING}
        },
        add_app = {
            function (req, msg)
                if not msg.class then return UBUS_STATUS_INVALID_ARGUMENT end
                if not msg.name then return UBUS_STATUS_INVALID_ARGUMENT end
				if not msg.proto then return UBUS_STATUS_INVALID_ARGUMENT end
                local t = cfg:init(cfg_file)
                local ret
                if t:lookup(msg.name) then return ubus.reply(req, {ret = UBUS_STATUS_ALREADY_EXISTS}) end
                ret = t:add_app(msg.class, msg.name, msg.proto, msg.sport, msg.dport, msg.url, msg.request, msg.dict)
                ubus.reply(req, {ret = ret})
            end,{class = libubus.STRING, name = libubus.STRING, proto = libubus.STRING, sport = libubus.INT32, dport = libubus.INT32, url = libubus.STRING, request = libubus.STRING, dict = libubus.STRING}
        },
        del_app = {
            function(req, msg)
                local t = cfg:init(cfg_file)
                local ret = t:del_app(msg.id, msg.name)
                ubus.reply(req, {ret = ret})
            end,{id = libubus.INT32, name = libubus.STRING}
        },
        list_class = {
            function (req, msg)
                local _, c = cfg:init(cfg_file)
                ubus.reply(req, {result = c})
            end,{}
        },
        list_app = {
            function (req, msg)
                if not msg.class then return UBUS_STATUS_INVALID_ARGUMENT end
                local t  = cfg:init(cfg_file)
                local ret = {}
                for i, v in ipairs(t:lookup_class(msg.class)) do
					if not v:match("#class") then
						local id, name = v:match("(%d+) (%S+):%[")
						ret[i-1] = {id = id,  name = name}
					end
                end
                ubus.reply(req, {result = ret})
            end,{class = libubus.STRING}
        }
	}
}

function ubus_init()
    local conn = libubus.connect()
    if not conn then
        error("Failed to connect to ubus")
    end

    conn:add(methods)

    return {
        call = function(object, method, params)
            return conn:call(object, method, params or {})
        end,
        reply = function(req, msg)
            conn:reply(req, msg)
        end
    }
end

local function main()
    uloop.init()
    ubus = ubus_init()
    uloop.run()
end

main()