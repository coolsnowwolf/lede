-- Copyright 2008 Freifunk Leipzig / Jo-Philipp Wich <jow@openwrt.org>
-- Licensed to the public under the Apache License 2.0.

-- This class provides basic ETag handling and implements most of the
-- conditional HTTP/1.1 headers specified in RFC2616 Sct. 14.24 - 14.28 .
module("luci.http.conditionals", package.seeall)

local date = require("luci.http.date")


function mk_etag( stat )
	if stat ~= nil then
		return string.format( '"%x-%x-%x"', stat.ino, stat.size, stat.mtime )
	end
end

-- Test whether the given message object contains an "If-Match" header and
-- compare it against the given stat object.
function if_match( req, stat )
	local h    = req.headers
	local etag = mk_etag( stat )

	-- Check for matching resource
	if type(h['If-Match']) == "string" then
		for ent in h['If-Match']:gmatch("([^, ]+)") do
			if ( ent == '*' or ent == etag ) and stat ~= nil then
				return true
			end
		end

		return false, 412
	end

	return true
end

-- Test whether the given message object contains an "If-Modified-Since" header
-- and compare it against the given stat object.
function if_modified_since( req, stat )
	local h = req.headers

	-- Compare mtimes
	if type(h['If-Modified-Since']) == "string" then
		local since = date.to_unix( h['If-Modified-Since'] )

		if stat == nil or since < stat.mtime then
			return true
		end

		return false, 304, {
			["ETag"]          = mk_etag( stat );
			["Date"]          = date.to_http( os.time() );
			["Last-Modified"] = date.to_http( stat.mtime )
		}
	end

	return true
end

-- Test whether the given message object contains an "If-None-Match" header and
-- compare it against the given stat object.
function if_none_match( req, stat )
	local h      = req.headers
	local etag   = mk_etag( stat )
	local method = req.env and req.env.REQUEST_METHOD or "GET"

	-- Check for matching resource
	if type(h['If-None-Match']) == "string" then
		for ent in h['If-None-Match']:gmatch("([^, ]+)") do
			if ( ent == '*' or ent == etag ) and stat ~= nil then
				if method == "GET" or method == "HEAD" then
					return false, 304, {
						["ETag"]          = etag;
						["Date"]          = date.to_http( os.time() );
						["Last-Modified"] = date.to_http( stat.mtime )
					}
				else
					return false, 412
				end
			end
		end
	end

	return true
end

-- The If-Range header is currently not implemented due to the lack of general
-- byte range stuff in luci.http.protocol . This function will always return
-- false, 412 to indicate a failed precondition.
function if_range( req, stat )
	-- Sorry, no subranges (yet)
	return false, 412
end

-- Test whether the given message object contains an "If-Unmodified-Since"
-- header and compare it against the given stat object.
function if_unmodified_since( req, stat )
	local h = req.headers

	-- Compare mtimes
	if type(h['If-Unmodified-Since']) == "string" then
		local since = date.to_unix( h['If-Unmodified-Since'] )

		if stat ~= nil and since <= stat.mtime then
			return false, 412
		end
	end

	return true
end
