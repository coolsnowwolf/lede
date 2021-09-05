--- TLS Socket Object.
-- TLS Sockets contain the underlying socket and context in the fields
-- "socket" and "context".
-- @cstyle	instance
module "nixio.TLSSocket"

--- Initiate the TLS handshake as client with the server.
-- @class function
-- @name TLSSocket.connect
-- @usage This function calls SSL_connect().
-- @usage You have to call either connect or accept before transmitting data.
-- @see TLSSocket.accept
-- @return true

--- Wait for a TLS handshake from a client.
-- @class function
-- @name TLSSocket.accept
-- @usage This function calls SSL_accept().
-- @usage You have to call either connect or accept before transmitting data.
-- @see TLSSocket.connect
-- @return	true

--- Send a message to the socket.
-- @class function
-- @name TLSSocket.send
-- @usage This function calls SSL_write().
-- @usage <strong>Warning:</strong> It is not guaranteed that all data
-- in the buffer is written at once.
-- You have to check the return value - the number of bytes actually written -
-- or use the safe IO functions in the high-level IO utility module.
-- @usage Unlike standard Lua indexing the lowest offset and default is 0.
-- @param buffer	Buffer holding the data to be written.
-- @param offset	Offset to start reading the buffer from. (optional)
-- @param length	Length of chunk to read from the buffer. (optional)
-- @return number of bytes written

--- Send a message on the socket (This is an alias for send).
-- See the send description for a detailed description.
-- @class function
-- @name TLSSocket.write
-- @param buffer	Buffer holding the data to be written.
-- @param offset	Offset to start reading the buffer from. (optional)
-- @param length	Length of chunk to read from the buffer. (optional)
-- @see	TLSSocket.send
-- @return number of bytes written

--- Receive a message on the socket.
-- @class function
-- @name TLSSocket.recv
-- @usage This function calls SSL_read().
-- @usage <strong>Warning:</strong> It is not guaranteed that all requested data
-- is read at once.
-- You have to check the return value - the length of the buffer actually read -
-- or use the safe IO functions in the high-level IO utility module.
-- @usage The length of the return buffer is limited by the (compile time) 
-- nixio buffersize which is <em>nixio.const.buffersize</em> (8192 by default).
-- Any read request greater than that will be safely truncated to this value.  
-- @param length	Amount of data to read (in Bytes).
-- @return buffer containing data successfully read

--- Receive a message on the socket (This is an alias for recv).
-- See the recv description for more details.
-- @class function
-- @name TLSSocket.read  
-- @param length	Amount of data to read (in Bytes).
-- @see TLSSocket.recv
-- @return buffer containing data successfully read

--- Shut down the TLS connection.
-- @class function
-- @name TLSSocket.shutdown
-- @usage This function calls SSL_shutdown().
-- @return	true