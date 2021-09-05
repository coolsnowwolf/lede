--- Binary operations and conversion.
module "nixio.bin"

--- Return a hexadecimal ASCII represantation of the content of a buffer. 
-- @class function
-- @name hexlify
-- @param buffer	Buffer
-- @return representation using characters [0-9a-f]

--- Return a binary buffer from a hexadecimal ASCII representation. 
-- @class function
-- @name unhexlify
-- @param hexvalue representation using characters [0-9a-f]
-- @return binary data

--- Calculate the CRC32 value of a buffer. 
-- @class function
-- @name crc32
-- @param buffer	Buffer
-- @param initial	Initial CRC32 value (optional)
-- @return crc32 value

--- Base64 encode a given buffer.
-- @class function
-- @name b64encode
-- @param buffer	Buffer
-- @return base64 encoded buffer

--- Base64 decode a given buffer.
-- @class function
-- @name b64decode
-- @param buffer	Base 64 Encoded data
-- @return binary data