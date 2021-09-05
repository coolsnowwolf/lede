--- Transport Layer Security Context Object.
-- @cstyle	instance
module "nixio.TLSContext"

--- Create a TLS Socket from a socket descriptor.
-- @class function
-- @name TLSContext.create
-- @param socket Socket Object
-- @return TLSSocket Object

--- Assign a PEM certificate to this context.
-- @class function
-- @name TLSContext.set_cert
-- @usage This function calls SSL_CTX_use_certificate_chain_file().
-- @param path	Certificate File path
-- @return true

--- Assign a PEM private key to this context.
-- @class function
-- @name TLSContext.set_key
-- @usage This function calls SSL_CTX_use_PrivateKey_file().
-- @param path	Private Key File path
-- @return true

--- Set the available ciphers for this context.
-- @class function
-- @name TLSContext.set_ciphers
-- @usage This function calls SSL_CTX_set_cipher_list().
-- @param cipherlist	String containing a list of ciphers
-- @return true

--- Set the verification depth of this context.
-- @class function
-- @name TLSContext.set_verify_depth
-- @usage This function calls SSL_CTX_set_verify_depth().
-- @param depth	Depth
-- @return true

--- Set the verification flags of this context.
-- @class function
-- @name TLSContext.set_verify
-- @usage This function calls SSL_CTX_set_verify().
-- @param flag1	First Flag	["none", "peer", "verify_fail_if_no_peer_cert", 
-- "client_once"]
-- @param ...	More Flags	[-"-]
-- @return true