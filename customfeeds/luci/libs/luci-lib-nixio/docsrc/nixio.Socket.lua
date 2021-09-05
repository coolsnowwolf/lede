--- Socket Object.
-- Supports IPv4, IPv6 and UNIX (POSIX only) families.
-- @cstyle	instance
module "nixio.Socket"

--- Get the local address of a socket.
-- @class function
-- @name Socket.getsockname
-- @return	IP-Address
-- @return	Port

--- Get the peer address of a socket.
-- @class function
-- @name Socket.getpeername
-- @return	IP-Address
-- @return	Port

--- Bind the socket to a network address.
-- @class function
-- @name Socket.bind
-- @usage This function calls getaddrinfo() and bind() but NOT listen().
-- @usage If <em>host</em> is a domain name it will be looked up and bind() 
-- tries the IP-Addresses in the order returned by the DNS resolver
-- until the bind succeeds. 
-- @usage UNIX sockets ignore the <em>port</em>,
-- and interpret <em>host</em> as a socket path.
-- @param host	Host (optional, default: all addresses)
-- @param port	Port or service description
-- @return true

--- Connect the socket to a network address.
-- @class function
-- @name Socket.connect
-- @usage This function calls getaddrinfo() and connect().
-- @usage If <em>host</em> is a domain name it will be looked up and connect() 
-- tries the IP-Addresses in the order returned by the DNS resolver
-- until the connect succeeds. 
-- @usage UNIX sockets ignore the <em>port</em>,
-- and interpret <em>host</em> as a socket path.
-- @param host	Hostname or IP-Address (optional, default: localhost)
-- @param port	Port or service description
-- @return true

--- Listen for connections on the socket.
-- @class function
-- @name Socket.listen
-- @param	backlog	Length of queue for pending connections
-- @return	true

--- Accept a connection on the socket.
-- @class function
-- @name Socket.accept
-- @return	Socket Object
-- @return	Peer IP-Address
-- @return  Peer Port

--- Send a message on the socket specifying the destination.
-- @class function
-- @name Socket.sendto
-- @usage <strong>Warning:</strong> It is not guaranteed that all data
-- in the buffer is written at once.
-- You have to check the return value - the number of bytes actually written -
-- or use the safe IO functions in the high-level IO utility module.
-- @usage Unlike standard Lua indexing the lowest offset and default is 0.
-- @param buffer	Buffer holding the data to be written.
-- @param host		Target IP-Address
-- @param port		Target Port
-- @param offset	Offset to start reading the buffer from. (optional)
-- @param length	Length of chunk to read from the buffer. (optional)
-- @return number of bytes written

--- Send a message on the socket.
-- This function is identical to sendto except for the missing destination
-- paramters. See the sendto description for a detailed description.
-- @class function
-- @name Socket.send
-- @param buffer	Buffer holding the data to be written.
-- @param offset	Offset to start reading the buffer from. (optional)
-- @param length	Length of chunk to read from the buffer. (optional)
-- @see	Socket.sendto
-- @return number of bytes written

--- Send a message on the socket (This is an alias for send).
-- See the sendto description for a detailed description.
-- @class function
-- @name Socket.write
-- @param buffer	Buffer holding the data to be written.
-- @param offset	Offset to start reading the buffer from. (optional)
-- @param length	Length of chunk to read from the buffer. (optional)
-- @see	Socket.sendto
-- @return number of bytes written

--- Receive a message on the socket including the senders source address.
-- @class function
-- @name Socket.recvfrom
-- @usage <strong>Warning:</strong> It is not guaranteed that all requested data
-- is read at once.
-- You have to check the return value - the length of the buffer actually read -
-- or use the safe IO functions in the high-level IO utility module.
-- @usage The length of the return buffer is limited by the (compile time) 
-- nixio buffersize which is <em>nixio.const.buffersize</em> (8192 by default).
-- Any read request greater than that will be safely truncated to this value.  
-- @param length	Amount of data to read (in Bytes).
-- @return buffer containing data successfully read
-- @return host		IP-Address of the sender
-- @return port		Port of the sender

--- Receive a message on the socket.
-- This function is identical to recvfrom except that it does not return 
-- the sender's source address. See the recvfrom description for more details.
-- @class function
-- @name Socket.recv  
-- @param length	Amount of data to read (in Bytes).
-- @see Socket.recvfrom
-- @return buffer containing data successfully read

--- Receive a message on the socket (This is an alias for recv).
-- See the recvfrom description for more details.
-- @class function
-- @name Socket.read  
-- @param length	Amount of data to read (in Bytes).
-- @see Socket.recvfrom
-- @return buffer containing data successfully read

--- Close the socket.
-- @class function
-- @name Socket.close
-- @return true

--- Shut down part of a full-duplex connection.
-- @class function
-- @name Socket.shutdown
-- @param how	(optional, default: rdwr) ["rdwr", "rd", "wr"] 
-- @return	true

--- Get the number of the filedescriptor.
-- @class function
-- @name Socket.fileno
-- @return file descriptor number

--- Set the blocking mode of the socket.
-- @class function
-- @name Socket.setblocking
-- @param	blocking	(boolean)
-- @return true

--- Set a socket option.
-- @class function
-- @name Socket.setopt
-- @param level Level ["socket", "tcp", "ip", "ipv6"]
-- @param option Option	["keepalive", "reuseaddr", "sndbuf", "rcvbuf", 
-- "priority", "broadcast", "linger", "sndtimeo", "rcvtimeo", "dontroute", 
-- "bindtodevice", "error", "oobinline", "cork" (TCP),  "nodelay" (TCP),
-- "mtu" (IP, IPv6), "hdrincl" (IP), "multicast_ttl" (IP), "multicast_loop"
-- (IP, IPv6), "multicast_if" (IP, IPv6), "v6only" (IPv6), "multicast_hops"
-- (IPv6), "add_membership" (IP, IPv6), "drop_membership" (IP, IPv6)]
-- @param	value Value
-- @return true

--- Get a socket option.
-- @class function
-- @name Socket.getopt
-- @param level Level ["socket", "tcp", "ip", "ipv6"]
-- @param option Option	["keepalive", "reuseaddr", "sndbuf", "rcvbuf", 
-- "priority", "broadcast", "linger", "sndtimeo", "rcvtimeo", "dontroute", 
-- "bindtodevice", "error", "oobinline", "cork" (TCP),  "nodelay" (TCP),
-- "mtu" (IP, IPv6), "hdrincl" (IP), "multicast_ttl" (IP), "multicast_loop"
-- (IP, IPv6), "multicast_if" (IP, IPv6), "v6only" (IPv6), "multicast_hops"
-- (IPv6), "add_membership" (IP, IPv6), "drop_membership" (IP, IPv6)]
-- @return Value