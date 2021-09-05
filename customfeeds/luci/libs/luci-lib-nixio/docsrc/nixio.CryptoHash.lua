--- Cryptographical Hash and HMAC object.
-- @cstyle	instance
module "nixio.CryptoHash"

--- Add another chunk of data to be hashed.
-- @class function
-- @name CryptoHash.update
-- @param chunk Chunk of data
-- @return CryptoHash object (self)

--- Finalize the hash and return the digest.
-- @class function
-- @name CryptoHash.final
-- @usage You cannot call update on a hash object that was already finalized
-- you can however call final multiple times to get the digest.
-- @return	hexdigest
-- @return  buffer containing binary digest