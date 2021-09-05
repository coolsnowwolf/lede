-------------------------------------------------------------------------------
-- @release $Id: standard.lua,v 1.39 2007/12/21 17:50:48 tomas Exp $
-------------------------------------------------------------------------------

local assert, pairs, tostring, type = assert, pairs, tostring, type
local io = require "io"
local posix = require "nixio.fs"
local luadoc = require "luadoc"
local util = require "luadoc.util"
local tags = require "luadoc.taglet.standard.tags"
local string = require "string"
local table = require "table"

module 'luadoc.taglet.standard'

-------------------------------------------------------------------------------
-- Creates an iterator for an array base on a class type.
-- @param t array to iterate over
-- @param class name of the class to iterate over

function class_iterator (t, class)
	return function ()
		local i = 1
		return function ()
			while t[i] and t[i].class ~= class do
				i = i + 1
			end
			local v = t[i]
			i = i + 1
			return v
		end
	end
end

-- Patterns for function recognition
local identifiers_list_pattern = "%s*(.-)%s*"
local identifier_pattern = "[^%(%s]+"
local function_patterns = {
	"^()%s*function%s*("..identifier_pattern..")%s*%("..identifiers_list_pattern.."%)",
	"^%s*(local%s)%s*function%s*("..identifier_pattern..")%s*%("..identifiers_list_pattern.."%)",
	"^()%s*("..identifier_pattern..")%s*%=%s*function%s*%("..identifiers_list_pattern.."%)",
}

-------------------------------------------------------------------------------
-- Checks if the line contains a function definition
-- @param line string with line text
-- @return function information or nil if no function definition found

local function check_function (line)
	line = util.trim(line)

	local info = table.foreachi(function_patterns, function (_, pattern)
		local r, _, l, id, param = string.find(line, pattern)
		if r ~= nil then
			return {
				name = id,
				private = (l == "local"),
				param = { } --util.split("%s*,%s*", param),
			}
		end
	end)

	-- TODO: remove these assert's?
	if info ~= nil then
		assert(info.name, "function name undefined")
		assert(info.param, string.format("undefined parameter list for function `%s'", info.name))
	end

	return info
end

-------------------------------------------------------------------------------
-- Checks if the line contains a module definition.
-- @param line string with line text
-- @param currentmodule module already found, if any
-- @return the name of the defined module, or nil if there is no module
-- definition

local function check_module (line, currentmodule)
	line = util.trim(line)

	-- module"x.y"
	-- module'x.y'
	-- module[[x.y]]
	-- module("x.y")
	-- module('x.y')
	-- module([[x.y]])
	-- module(...)

	local r, _, modulename = string.find(line, "^module%s*[%s\"'(%[]+([^,\"')%]]+)")
	if r then
		-- found module definition
		logger:debug(string.format("found module `%s'", modulename))
		return modulename
	end
	return currentmodule
end

-- Patterns for constant recognition
local constant_patterns = {
	"^()%s*([A-Z][A-Z0-9_]*)%s*=",
	"^%s*(local%s)%s*([A-Z][A-Z0-9_]*)%s*=",
}

-------------------------------------------------------------------------------
-- Checks if the line contains a constant definition
-- @param line string with line text
-- @return constant information or nil if no constant definition found

local function check_constant (line)
	line = util.trim(line)

	local info = table.foreachi(constant_patterns, function (_, pattern)
		local r, _, l, id = string.find(line, pattern)
		if r ~= nil then
			return {
				name = id,
				private = (l == "local"),
			}
		end
	end)

	-- TODO: remove these assert's?
	if info ~= nil then
		assert(info.name, "constant name undefined")
	end

	return info
end

-------------------------------------------------------------------------------
-- Extracts summary information from a description. The first sentence of each
-- doc comment should be a summary sentence, containing a concise but complete
-- description of the item. It is important to write crisp and informative
-- initial sentences that can stand on their own
-- @param description text with item description
-- @return summary string or nil if description is nil

local function parse_summary (description)
	-- summary is never nil...
	description = description or ""

	-- append an " " at the end to make the pattern work in all cases
	description = description.." "

	-- read until the first period followed by a space or tab
	local summary = string.match(description, "(.-%.)[%s\t]")

	-- if pattern did not find the first sentence, summary is the whole description
	summary = summary or description

	return summary
end

-------------------------------------------------------------------------------
-- @param f file handle
-- @param line current line being parsed
-- @param modulename module already found, if any
-- @return current line
-- @return code block
-- @return modulename if found

local function parse_code (f, line, modulename)
	local code = {}
	while line ~= nil do
		if string.find(line, "^[\t ]*%-%-%-") then
			-- reached another luadoc block, end this parsing
			return line, code, modulename
		else
			-- look for a module definition
			modulename = check_module(line, modulename)

			table.insert(code, line)
			line = f:read()
		end
	end
	-- reached end of file
	return line, code, modulename
end

-------------------------------------------------------------------------------
-- Parses the information inside a block comment
-- @param block block with comment field
-- @return block parameter

local function parse_comment (block, first_line, modulename)

	-- get the first non-empty line of code
	local code = table.foreachi(block.code, function(_, line)
		if not util.line_empty(line) then
			-- `local' declarations are ignored in two cases:
			-- when the `nolocals' option is turned on; and
			-- when the first block of a file is parsed (this is
			--	necessary to avoid confusion between the top
			--	local declarations and the `module' definition.
			if (options.nolocals or first_line) and line:find"^%s*local" then
				return
			end
			return line
		end
	end)

	-- parse first line of code
	if code ~= nil then
		local func_info = check_function(code)
		local module_name = check_module(code)
		local const_info = check_constant(code)
		if func_info then
			block.class = "function"
			block.name = func_info.name
			block.param = func_info.param
			block.private = func_info.private
		elseif const_info then
			block.class = "constant"
			block.name = const_info.name
			block.private = const_info.private
		elseif module_name then
			block.class = "module"
			block.name = module_name
			block.param = {}
		else
			block.param = {}
		end
	else
		-- TODO: comment without any code. Does this means we are dealing
		-- with a file comment?
	end

	-- parse @ tags
	local currenttag = "description"
	local currenttext

	table.foreachi(block.comment, function (_, line)
		line = util.trim_comment(line)

		local r, _, tag, text = string.find(line, "@([_%w%.]+)%s+(.*)")
		if r ~= nil then
			-- found new tag, add previous one, and start a new one
			-- TODO: what to do with invalid tags? issue an error? or log a warning?
			tags.handle(currenttag, block, currenttext)

			currenttag = tag
			currenttext = text
		else
			currenttext = util.concat(currenttext, "\n" .. line)
			assert(string.sub(currenttext, 1, 1) ~= " ", string.format("`%s', `%s'", currenttext, line))
		end
	end)
	tags.handle(currenttag, block, currenttext)

	-- extracts summary information from the description
	block.summary = parse_summary(block.description)
	assert(string.sub(block.description, 1, 1) ~= " ", string.format("`%s'", block.description))

	if block.name and block.class == "module" then
		modulename = block.name
	end

	return block, modulename
end

-------------------------------------------------------------------------------
-- Parses a block of comment, started with ---. Read until the next block of
-- comment.
-- @param f file handle
-- @param line being parsed
-- @param modulename module already found, if any
-- @return line
-- @return block parsed
-- @return modulename if found

local function parse_block (f, line, modulename, first)
	local multiline = not not (line and line:match("%[%["))
	local block = {
		comment = {},
		code = {},
	}

	while line ~= nil do
		if (multiline == true and string.find(line, "%]%]") ~= nil) or
		   (multiline == false and string.find(line, "^[\t ]*%-%-") == nil) then
			-- reached end of comment, read the code below it
			-- TODO: allow empty lines
			line, block.code, modulename = parse_code(f, line, modulename)

			-- parse information in block comment
			block, modulename = parse_comment(block, first, modulename)

			return line, block, modulename
		else
			table.insert(block.comment, line)
			line = f:read()
		end
	end
	-- reached end of file

	-- parse information in block comment
	block, modulename = parse_comment(block, first, modulename)

	return line, block, modulename
end

-------------------------------------------------------------------------------
-- Parses a file documented following luadoc format.
-- @param filepath full path of file to parse
-- @param doc table with documentation
-- @return table with documentation

function parse_file (filepath, doc, handle, prev_line, prev_block, prev_modname)
	local blocks = { prev_block }
	local modulename = prev_modname

	-- read each line
	local f = handle or io.open(filepath, "r")
	local i = 1
	local line = prev_line or f:read()
	local first = true
	while line ~= nil do

		if string.find(line, "^[\t ]*%-%-%-") then
			-- reached a luadoc block
			local block, newmodname
			line, block, newmodname = parse_block(f, line, modulename, first)

			if modulename and newmodname and newmodname ~= modulename then
				doc = parse_file( nil, doc, f, line, block, newmodname )
			else
				table.insert(blocks, block)
				modulename = newmodname
			end
		else
			-- look for a module definition
			local newmodname = check_module(line, modulename)

			if modulename and newmodname and newmodname ~= modulename then
				parse_file( nil, doc, f )
			else
				modulename = newmodname
			end

			-- TODO: keep beginning of file somewhere

			line = f:read()
		end
		first = false
		i = i + 1
	end

	if not handle then
		f:close()
	end

	if filepath then
		-- store blocks in file hierarchy
		assert(doc.files[filepath] == nil, string.format("doc for file `%s' already defined", filepath))
		table.insert(doc.files, filepath)
		doc.files[filepath] = {
			type = "file",
			name = filepath,
			doc = blocks,
	--		functions = class_iterator(blocks, "function"),
	--		tables = class_iterator(blocks, "table"),
		}
	--
		local first = doc.files[filepath].doc[1]
		if first and modulename then
			doc.files[filepath].author = first.author
			doc.files[filepath].copyright = first.copyright
			doc.files[filepath].description = first.description
			doc.files[filepath].release = first.release
			doc.files[filepath].summary = first.summary
		end
	end

	-- if module definition is found, store in module hierarchy
	if modulename ~= nil then
		if modulename == "..." then
			assert( filepath, "Can't determine name for virtual module from filepatch" )
			modulename = string.gsub (filepath, "%.lua$", "")
			modulename = string.gsub (modulename, "/", ".")
		end
		if doc.modules[modulename] ~= nil then
			-- module is already defined, just add the blocks
			table.foreachi(blocks, function (_, v)
				table.insert(doc.modules[modulename].doc, v)
			end)
		else
			-- TODO: put this in a different module
			table.insert(doc.modules, modulename)
			doc.modules[modulename] = {
				type = "module",
				name = modulename,
				doc = blocks,
--				functions = class_iterator(blocks, "function"),
--				tables = class_iterator(blocks, "table"),
				author = first and first.author,
				copyright = first and first.copyright,
				description = "",
				release = first and first.release,
				summary = "",
			}

			-- find module description
			for m in class_iterator(blocks, "module")() do
				doc.modules[modulename].description = util.concat(
					doc.modules[modulename].description,
					m.description)
				doc.modules[modulename].summary = util.concat(
					doc.modules[modulename].summary,
					m.summary)
				if m.author then
					doc.modules[modulename].author = m.author
				end
				if m.copyright then
					doc.modules[modulename].copyright = m.copyright
				end
				if m.release then
					doc.modules[modulename].release = m.release
				end
				if m.name then
					doc.modules[modulename].name = m.name
				end
			end
			doc.modules[modulename].description = doc.modules[modulename].description or (first and first.description) or ""
			doc.modules[modulename].summary = doc.modules[modulename].summary or (first and first.summary) or ""
		end

		-- make functions table
		doc.modules[modulename].functions = {}
		for f in class_iterator(blocks, "function")() do
			if f and f.name then
				table.insert(doc.modules[modulename].functions, f.name)
				doc.modules[modulename].functions[f.name] = f
			end
		end

		-- make tables table
		doc.modules[modulename].tables = {}
		for t in class_iterator(blocks, "table")() do
			if t and t.name then
				table.insert(doc.modules[modulename].tables, t.name)
				doc.modules[modulename].tables[t.name] = t
			end
		end

		-- make constants table
		doc.modules[modulename].constants = {}
		for c in class_iterator(blocks, "constant")() do
			if c and c.name then
				table.insert(doc.modules[modulename].constants, c.name)
				doc.modules[modulename].constants[c.name] = c
			end
		end
	end

	if filepath then
		-- make functions table
		doc.files[filepath].functions = {}
		for f in class_iterator(blocks, "function")() do
			if f and f.name then
				table.insert(doc.files[filepath].functions, f.name)
				doc.files[filepath].functions[f.name] = f
			end
		end

		-- make tables table
		doc.files[filepath].tables = {}
		for t in class_iterator(blocks, "table")() do
			if t and t.name then
				table.insert(doc.files[filepath].tables, t.name)
				doc.files[filepath].tables[t.name] = t
			end
		end
	end

	return doc
end

-------------------------------------------------------------------------------
-- Checks if the file is terminated by ".lua" or ".luadoc" and calls the
-- function that does the actual parsing
-- @param filepath full path of the file to parse
-- @param doc table with documentation
-- @return table with documentation
-- @see parse_file

function file (filepath, doc)
	local patterns = { "%.lua$", "%.luadoc$" }
	local valid = table.foreachi(patterns, function (_, pattern)
		if string.find(filepath, pattern) ~= nil then
			return true
		end
	end)

	if valid then
		logger:info(string.format("processing file `%s'", filepath))
		doc = parse_file(filepath, doc)
	end

	return doc
end

-------------------------------------------------------------------------------
-- Recursively iterates through a directory, parsing each file
-- @param path directory to search
-- @param doc table with documentation
-- @return table with documentation

function directory (path, doc)
	for f in posix.dir(path) do
		local fullpath = path .. "/" .. f
		local attr = posix.stat(fullpath)
		assert(attr, string.format("error stating file `%s'", fullpath))

		if attr.type == "reg" then
			doc = file(fullpath, doc)
		elseif attr.type == "dir" and f ~= "." and f ~= ".." then
			doc = directory(fullpath, doc)
		end
	end
	return doc
end

-- Recursively sorts the documentation table
local function recsort (tab)
	table.sort (tab)
	-- sort list of functions by name alphabetically
	for f, doc in pairs(tab) do
		if doc.functions then
			table.sort(doc.functions)
		end
		if doc.tables then
			table.sort(doc.tables)
		end
	end
end

-------------------------------------------------------------------------------

function start (files, doc)
	assert(files, "file list not specified")

	-- Create an empty document, or use the given one
	doc = doc or {
		files = {},
		modules = {},
	}
	assert(doc.files, "undefined `files' field")
	assert(doc.modules, "undefined `modules' field")

	table.foreachi(files, function (_, path)
		local attr = posix.stat(path)
		assert(attr, string.format("error stating path `%s'", path))

		if attr.type == "reg" then
			doc = file(path, doc)
		elseif attr.type == "dir" then
			doc = directory(path, doc)
		end
	end)

	-- order arrays alphabetically
	recsort(doc.files)
	recsort(doc.modules)

	return doc
end
