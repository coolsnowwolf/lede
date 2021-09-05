-- Copyright 2008 Steven Barth <steven@midlink.org>
-- Licensed to the public under the Apache License 2.0.

local util = require "luci.util"
local config = require "luci.config"
local tparser = require "luci.template.parser"

local tostring, pairs, loadstring = tostring, pairs, loadstring
local setmetatable, loadfile = setmetatable, loadfile
local getfenv, setfenv, rawget = getfenv, setfenv, rawget
local assert, type, error = assert, type, error

--- LuCI template library.
module "luci.template"

config.template = config.template or {}
viewdir = config.template.viewdir or util.libpath() .. "/view"


-- Define the namespace for template modules
context = util.threadlocal()

--- Render a certain template.
-- @param name		Template name
-- @param scope		Scope to assign to template (optional)
function render(name, scope)
	return Template(name):render(scope or getfenv(2))
end

--- Render a template from a string.
-- @param template	Template string
-- @param scope		Scope to assign to template (optional)
function render_string(template, scope)
	return Template(nil, template):render(scope or getfenv(2))
end


-- Template class
Template = util.class()

-- Shared template cache to store templates in to avoid unnecessary reloading
Template.cache = setmetatable({}, {__mode = "v"})


-- Constructor - Reads and compiles the template on-demand
function Template.__init__(self, name, template)
	if name then
		self.template = self.cache[name]
		self.name = name
	else
		self.name = "[string]"
	end

	-- Create a new namespace for this template
	self.viewns = context.viewns
	
	-- If we have a cached template, skip compiling and loading
	if not self.template then

		-- Compile template
		local err
		local sourcefile

		if name then
			sourcefile = viewdir .. "/" .. name .. ".htm"
			self.template, _, err = tparser.parse(sourcefile)
		else
			sourcefile = "[string]"
			self.template, _, err = tparser.parse_string(template)
		end

		-- If we have no valid template throw error, otherwise cache the template
		if not self.template then
			error("Failed to load template '" .. name .. "'.\n" ..
			      "Error while parsing template '" .. sourcefile .. "':\n" ..
			      (err or "Unknown syntax error"))
		elseif name then
			self.cache[name] = self.template
		end
	end
end


-- Renders a template
function Template.render(self, scope)
	scope = scope or getfenv(2)
	
	-- Put our predefined objects in the scope of the template
	setfenv(self.template, setmetatable({}, {__index =
		function(tbl, key)
			return rawget(tbl, key) or self.viewns[key] or scope[key]
		end}))
	
	-- Now finally render the thing
	local stat, err = util.copcall(self.template)
	if not stat then
		error("Failed to execute template '" .. self.name .. "'.\n" ..
		      "A runtime error occured: " .. tostring(err or "(nil)"))
	end
end
