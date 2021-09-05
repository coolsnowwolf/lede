-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

module("luci.i18n", package.seeall)
require("luci.util")

local tparser = require "luci.template.parser"

table   = {}
i18ndir = luci.util.libpath() .. "/i18n/"
loaded  = {}
context = luci.util.threadlocal()
default = "en"

function clear()
end

function load(file, lang, force)
end

-- Alternatively load the translation of the fallback language.
function loadc(file, force)
end

function setlanguage(lang)
	context.lang   = lang:gsub("_", "-")
	context.parent = (context.lang:match("^([a-z][a-z])_"))
	if not tparser.load_catalog(context.lang, i18ndir) then
		if context.parent then
			tparser.load_catalog(context.parent, i18ndir)
			return context.parent
		end
	end
	return context.lang
end

function translate(key)
	return tparser.translate(key) or key
end

function translatef(key, ...)
	return tostring(translate(key)):format(...)
end

-- and ensure that the returned value is a Lua string value.
-- This is the same as calling <code>tostring(translate(...))</code>
function string(key)
	return tostring(translate(key))
end

-- Ensure that the returned value is a Lua string value.
-- This is the same as calling <code>tostring(translatef(...))</code>
function stringf(key, ...)
	return tostring(translate(key)):format(...)
end
