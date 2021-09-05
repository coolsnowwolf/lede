-------------------------------------------------------------------------------
-- General utilities.
-- @release $Id: util.lua,v 1.16 2008/02/17 06:42:51 jasonsantos Exp $
-------------------------------------------------------------------------------

local posix = require "nixio.fs"
local type, table, string, io, assert, tostring, setmetatable, pcall = type, table, string, io, assert, tostring, setmetatable, pcall

-------------------------------------------------------------------------------
-- Module with several utilities that could not fit in a specific module

module "luadoc.util"

-------------------------------------------------------------------------------
-- Removes spaces from the beginning and end of a given string
-- @param s string to be trimmed
-- @return trimmed string

function trim (s)
	return (string.gsub(s, "^%s*(.-)%s*$", "%1"))
end

-------------------------------------------------------------------------------
-- Removes spaces from the beginning and end of a given string, considering the
-- string is inside a lua comment.
-- @param s string to be trimmed
-- @return trimmed string
-- @see trim
-- @see string.gsub

function trim_comment (s)
	s = string.gsub(s, "^%s*%-%-+%[%[(.*)$", "%1")
	s = string.gsub(s, "^%s*%-%-+(.*)$", "%1")
	return s
end

-------------------------------------------------------------------------------
-- Checks if a given line is empty
-- @param line string with a line
-- @return true if line is empty, false otherwise

function line_empty (line)
	return (string.len(trim(line)) == 0)
end

-------------------------------------------------------------------------------
-- Appends two string, but if the first one is nil, use to second one
-- @param str1 first string, can be nil
-- @param str2 second string
-- @return str1 .. " " .. str2, or str2 if str1 is nil

function concat (str1, str2)
	if str1 == nil or string.len(str1) == 0 then
		return str2
	else
		return str1 .. " " .. str2
	end
end

-------------------------------------------------------------------------------
-- Split text into a list consisting of the strings in text,
-- separated by strings matching delim (which may be a pattern).
-- @param delim if delim is "" then action is the same as %s+ except that
-- field 1 may be preceded by leading whitespace
-- @usage split(",%s*", "Anna, Bob, Charlie,Dolores")
-- @usage split(""," x y") gives {"x","y"}
-- @usage split("%s+"," x y") gives {"", "x","y"}
-- @return array with strings
-- @see table.concat

function split(delim, text)
	local list = {}
	if string.len(text) > 0 then
		delim = delim or ""
		local pos = 1
		-- if delim matches empty string then it would give an endless loop
		if string.find("", delim, 1) and delim ~= "" then
			error("delim matches empty string!")
		end
		local first, last
		while 1 do
			if delim ~= "" then
				first, last = string.find(text, delim, pos)
			else
				first, last = string.find(text, "%s+", pos)
				if first == 1 then
					pos = last+1
					first, last = string.find(text, "%s+", pos)
				end
			end
			if first then -- found?
				table.insert(list, string.sub(text, pos, first-1))
				pos = last+1
			else
				table.insert(list, string.sub(text, pos))
				break
			end
		end
	end
	return list
end

-------------------------------------------------------------------------------
-- Comments a paragraph.
-- @param text text to comment with "--", may contain several lines
-- @return commented text

function comment (text)
	text = string.gsub(text, "\n", "\n-- ")
	return "-- " .. text
end

-------------------------------------------------------------------------------
-- Wrap a string into a paragraph.
-- @param s string to wrap
-- @param w width to wrap to [80]
-- @param i1 indent of first line [0]
-- @param i2 indent of subsequent lines [0]
-- @return wrapped paragraph

function wrap(s, w, i1, i2)
	w = w or 80
	i1 = i1 or 0
	i2 = i2 or 0
	assert(i1 < w and i2 < w, "the indents must be less than the line width")
	s = string.rep(" ", i1) .. s
	local lstart, len = 1, string.len(s)
	while len - lstart > w do
		local i = lstart + w
		while i > lstart and string.sub(s, i, i) ~= " " do i = i - 1 end
		local j = i
		while j > lstart and string.sub(s, j, j) == " " do j = j - 1 end
		s = string.sub(s, 1, j) .. "\n" .. string.rep(" ", i2) ..
			string.sub(s, i + 1, -1)
		local change = i2 + 1 - (i - j)
		lstart = j + change
		len = len + change
	end
	return s
end

-------------------------------------------------------------------------------
-- Opens a file, creating the directories if necessary
-- @param filename full path of the file to open (or create)
-- @param mode mode of opening
-- @return file handle

function posix.open (filename, mode)
	local f = io.open(filename, mode)
	if f == nil then
		filename = string.gsub(filename, "\\", "/")
		local dir = ""
		for d in string.gfind(filename, ".-/") do
			dir = dir .. d
			posix.mkdir(dir)
		end
		f = io.open(filename, mode)
	end
	return f
end


----------------------------------------------------------------------------------
-- Creates a Logger with LuaLogging, if present. Otherwise, creates a mock logger.
-- @param options a table with options for the logging mechanism
-- @return logger object that will implement log methods

function loadlogengine(options)
	local logenabled = pcall(function()
		require "logging"
		require "logging.console"
	end)

	local logging = logenabled and logging

	if logenabled then
		if options.filelog then
			logger = logging.file("luadoc.log") -- use this to get a file log
		else
			logger = logging.console("[%level] %message\n")
		end

		if options.verbose then
			logger:setLevel(logging.INFO)
		else
			logger:setLevel(logging.WARN)
		end

	else
		noop = {__index=function(...)
			return function(...)
				-- noop
			end
		end}

		logger = {}
		setmetatable(logger, noop)
	end

	return logger
end
