--- Cryptographical library.
module "nixio.crypto"

--- Create a hash object. 
-- @class function
-- @name nixio.crypto.hash
-- @param algo	Algorithm ["sha1", "md5"]
-- @return CryptoHash Object

--- Create a HMAC object. 
-- @class function
-- @name nixio.crypto.hmac
-- @param algo	Algorithm ["sha1", "md5"]
-- @param key	HMAC-Key
-- @return CryptoHash Object