--- General POSIX IO library.
module "nixio"

--- Look up a hostname and service via DNS.
-- @class function
-- @name nixio.getaddrinfo
-- @param host		hostname to lookup	(optional)
-- @param family 	address family [<strong>"any"</strong>, "inet", "inet6"]  
-- @param service	service name or port (optional)
-- @return			Table containing one or more tables containing: <ul>
-- <li>family = ["inet", "inet6"]</li>
-- <li>socktype = ["stream", "dgram", "raw"]</li>
-- <li>address = Resolved IP-Address</li>
-- <li>port = Resolved Port (if service was given)</li>
-- </ul>

--- Reverse look up an IP-Address via DNS.
-- @class function
-- @name nixio.getnameinfo
-- @param ipaddr	IPv4 or IPv6-Address
-- @return	FQDN

--- (Linux, BSD) Get a list of available network interfaces and their addresses.
-- @class function
-- @name nixio.getifaddrs
-- @return			Table containing one or more tables containing: <ul>
-- <li>name = Interface Name</li>
-- <li>family = ["inet", "inet6", "packet"]</li>
-- <li>addr = Interface Address (IPv4, IPv6, MAC, ...)</li>
-- <li>broadaddr = Broadcast Address</li>
-- <li>dstaddr = Destination Address (Point-to-Point)</li>
-- <li>netmask = Netmask (if available)</li>
-- <li>prefix = Prefix (if available)</li>
-- <li>flags = Table of interface flags (up, multicast, loopback, ...)</li>
-- <li>data = Statistics (Linux, "packet"-family)</li>
-- <li>hatype = Hardware Type Identifier (Linix, "packet"-family)</li>
-- <li>ifindex = Interface Index (Linux, "packet"-family)</li>
-- </ul>

--- Get protocol entry by name.
-- @usage This function returns nil if the given protocol is unknown.
-- @class function
-- @name nixio.getprotobyname
-- @param name		protocol name to lookup
-- @return			Table containing the following fields: <ul>
-- <li>name = Protocol Name</li>
-- <li>proto = Protocol Number</li>
-- <li>aliases = Table of alias names</li>
-- </ul>

--- Get protocol entry by number.
-- @usage This function returns nil if the given protocol is unknown.
-- @class function
-- @name nixio.getprotobynumber
-- @param proto		protocol number to lookup
-- @return			Table containing the following fields: <ul>
-- <li>name = Protocol Name</li>
-- <li>proto = Protocol Number</li>
-- <li>aliases = Table of alias names</li>
-- </ul>

--- Get all or a specifc proto entry.
-- @class function
-- @name nixio.getproto
-- @param proto		protocol number or name to lookup (optional)
-- @return			Table (or if no parameter is given, a table of tables)
--                  containing the following fields: <ul>
-- <li>name = Protocol Name</li>
-- <li>proto = Protocol Number</li>
-- <li>aliases = Table of alias names</li>
-- </ul>

--- Create a new socket and bind it to a network address.
-- This function is a shortcut for calling nixio.socket and then bind()
-- on the socket object.
-- @usage This functions calls getaddrinfo(), socket(),
-- setsockopt() and bind() but NOT listen().
-- @usage The <em>reuseaddr</em>-option is automatically set before binding.
-- @class function
-- @name nixio.bind
-- @param host	Hostname or IP-Address (optional, default: all addresses)
-- @param port	Port or service description
-- @param family Address family [<strong>"any"</strong>, "inet", "inet6"]
-- @param socktype Socket Type [<strong>"stream"</strong>, "dgram"]
-- @return	Socket Object

--- Create a new socket and connect to a network address.
-- This function is a shortcut for calling nixio.socket and then connect()
-- on the socket object.
-- @usage This functions calls getaddrinfo(), socket() and connect().
-- @class function
-- @name nixio.connect
-- @param host	Hostname or IP-Address (optional, default: localhost)
-- @param port	Port or service description
-- @param family Address family [<strong>"any"</strong>, "inet", "inet6"]
-- @param socktype Socket Type [<strong>"stream"</strong>, "dgram"]
-- @return	Socket Object

--- Open a file.
-- @class function
-- @name nixio.open
-- @usage Although this function also supports the traditional fopen() 
-- file flags it does not create a file stream but uses the open() syscall.
-- @param path	Filesystem path to open
-- @param flags	Flag string or number (see open_flags).
-- [<strong>"r"</strong>, "r+", "w", "w+", "a", "a+"]
-- @param  mode	File mode for newly created files (see chmod, umask).
-- @see	nixio.umask
-- @see nixio.open_flags
-- @return	File Object

--- Generate flags for a call to open().
-- @class function
-- @name nixio.open_flags
-- @usage This function cannot fail and will never return nil.
-- @usage The "nonblock" and "ndelay" flags are aliases.
-- @usage The "nonblock", "ndelay" and "sync" flags are no-ops on Windows.
-- @param flag1 First Flag ["append", "creat", "excl", "nonblock", "ndelay",
-- "sync", "trunc", "rdonly", "wronly", "rdwr"]
-- @param ...	More Flags [-"-]
-- @return flag to be used as second paramter to open

--- Duplicate a file descriptor.
-- @class function
-- @name nixio.dup
-- @usage This funcation calls dup2() if <em>newfd</em> is set, otherwise dup().
-- @param oldfd	Old descriptor [File Object, Socket Object (POSIX only)]
-- @param newfd New descriptor to serve as copy (optional)
-- @return	File Object of new descriptor

--- Create a pipe.
-- @class function
-- @name nixio.pipe
-- @return	File Object of the read end
-- @return	File Object of the write end

--- Get the last system error code.
-- @class function
-- @name nixio.errno
-- @return	Error code

--- Get the error message for the corresponding error code.
-- @class function
-- @name nixio.strerror
-- @param errno System error code
-- @return	Error message

--- Sleep for a specified amount of time.
-- @class function
-- @usage Not all systems support nanosecond precision but you can expect
-- to have at least maillisecond precision.
-- @usage This function is not signal-protected and may fail with EINTR.
-- @param seconds Seconds to wait (optional)
-- @param nanoseconds Nanoseconds to wait (optional)
-- @name nixio.nanosleep
-- @return true

--- Generate events-bitfield or parse revents-bitfield for poll.
-- @class function
-- @name nixio.poll_flags
-- @param	mode1	revents-Flag bitfield returned from poll to parse OR 
-- ["in", "out", "err", "pri" (POSIX), "hup" (POSIX), "nval" (POSIX)]
-- @param	...		More mode strings for generating the flag [-"-]
-- @see nixio.poll
-- @return table with boolean fields reflecting the mode parameter 
-- <strong>OR</strong> bitfield to use for the events-Flag field

--- Wait for some event on a file descriptor.
-- poll() sets the revents-field of the tables provided by fds to a bitfield
-- indicating the events that occured.
-- @class function
-- @usage This function works in-place on the provided table and only
-- writes the revents field, you can use other fields on your demand.
-- @usage All metamethods on the tables provided as fds are ignored.
-- @usage The revents-fields are not reset when the call times out.
-- You have to check the first return value to be 0 to handle this case.
-- @usage If you want to wait on a TLS-Socket you have to use the underlying
-- socket instead.
-- @usage On Windows poll is emulated through select(), can only be used
-- on socket descriptors and cannot take more than 64 descriptors per call.
-- @usage This function is not signal-protected and may fail with EINTR.
-- @param fds	Table containing one or more tables containing <ul>
-- <li> fd = I/O Descriptor [Socket Object, File Object (POSIX)]</li>
-- <li> events = events to wait for (bitfield generated with poll_flags)</li>
-- </ul>
-- @param timeout Timeout in milliseconds
-- @name nixio.poll
-- @see nixio.poll_flags
-- @return number of ready IO descriptors
-- @return the fds-table with revents-fields set

--- (POSIX) Clone the current process.
-- @class function
-- @name nixio.fork
-- @return	the child process id for the parent process, 0 for the child process

--- (POSIX) Send a signal to one or more processes.
-- @class function
-- @name nixio.kill
-- @param	target	Target process of process group.
-- @param	signal	Signal to send
-- @return	true

--- (POSIX) Get the parent process id of the current process.
-- @class function
-- @name nixio.getppid
-- @return	parent process id

--- (POSIX) Get the user id of the current process.
-- @class function
-- @name nixio.getuid
-- @return	process user id

--- (POSIX) Get the group id of the current process.
-- @class function
-- @name nixio.getgid
-- @return	process group id

--- (POSIX) Set the group id of the current process.
-- @class function
-- @name nixio.setgid
-- @param gid New Group ID
-- @return	true

--- (POSIX) Set the user id of the current process.
-- @class function
-- @name nixio.setuid
-- @param gid New User ID
-- @return	true

--- (POSIX) Change priority of current process.
-- @class function
-- @name nixio.nice
-- @param nice Nice Value
-- @return	true

--- (POSIX) Create a new session and set the process group ID.
-- @class function
-- @name nixio.setsid
-- @return	session id

--- (POSIX) Wait for a process to change state.
-- @class function
-- @name nixio.waitpid
-- @usage	If the "nohang" is given this function becomes non-blocking.
-- @param pid Process ID	(optional, default: any childprocess)
-- @param flag1 Flag	(optional) ["nohang", "untraced", "continued"]
-- @param ...	More Flags [-"-]
-- @return	process id of child or 0 if no child has changed state
-- @return  ["exited", "signaled", "stopped"]
-- @return  [exit code, terminate signal, stop signal]

--- (POSIX) Get process times.
-- @class function
-- @name nixio.times
-- @return Table containing: <ul>
-- <li>utime = user time</li>
-- <li>utime = system time</li>
-- <li>cutime = children user time</li>
-- <li>cstime = children system time</li>
-- </ul>

--- (POSIX) Get information about current system and kernel.
-- @class function
-- @name nixio.uname
-- @return Table containing: <ul>
-- <li>sysname = operating system</li>
-- <li>nodename = network name (usually hostname)</li>
-- <li>release = OS release</li>
-- <li>version = OS version</li>
-- <li>machine = hardware identifier</li>
-- </ul>

--- Change the working directory.
-- @class function
-- @name nixio.chdir
-- @param	path New working directory
-- @return true

--- Ignore or use set the default handler for a signal.
-- @class function
-- @name nixio.signal
-- @param signal	Signal
-- @param handler	["ign", "dfl"]
-- @return true

--- Get the ID of the current process.
-- @class function
-- @name nixio.getpid
-- @return process id

--- Get the current working directory.
-- @class function
-- @name nixio.getcwd
-- @return workign directory

--- Get the current environment table or a specific environment variable.
-- @class function
-- @name nixio.getenv
-- @param variable Variable (optional)
-- @return environment table or single environment variable

--- Set or unset a environment variable.
-- @class function
-- @name nixio.setenv
-- @usage The environment variable will be unset if value is ommited.
-- @param variable	Variable
-- @param value		Value (optional)
-- @return true

--- Execute a file to replace the current process.
-- @class function
-- @name nixio.exec
-- @usage The name of the executable is automatically passed as argv[0]
-- @usage This function does not return on success.
-- @param executable Executable
-- @param ... Parameters

--- Invoke the shell and execute a file to replace the current process.
-- @class function
-- @name nixio.execp
-- @usage The name of the executable is automatically passed as argv[0]
-- @usage This function does not return on success.
-- @param executable Executable
-- @param ... Parameters

--- Execute a file with a custom environment to replace the current process.
-- @class function
-- @name nixio.exece
-- @usage The name of the executable is automatically passed as argv[0]
-- @usage This function does not return on success.
-- @param executable Executable
-- @param arguments Argument Table
-- @param environment Environment Table (optional)

--- Sets the file mode creation mask.
-- @class function
-- @name nixio.umask
-- @param mask	New creation mask (see chmod for format specifications)
-- @return the old umask as decimal mode number
-- @return the old umask as mode string

--- (Linux) Get overall system statistics.
-- @class function
-- @name nixio.sysinfo
-- @return Table containing: <ul>
-- <li>uptime = system uptime in seconds</li>
-- <li>loads = {loadavg1, loadavg5, loadavg15}</li>
-- <li>totalram = total RAM</li>
-- <li>freeram = free RAM</li>
-- <li>sharedram = shared RAM</li>
-- <li>bufferram = buffered RAM</li>
-- <li>totalswap = total SWAP</li>
-- <li>freeswap = free SWAP</li>
-- <li>procs = number of running processes</li>
-- </ul>

--- Create a new socket.
-- @class function
-- @name nixio.socket
-- @param domain	Domain ["inet", "inet6", "unix"]
-- @param type		Type   ["stream", "dgram", "raw"]
-- @return	Socket Object

--- (POSIX) Send data from a file to a socket in kernel-space.
-- @class function
-- @name nixio.sendfile
-- @param socket Socket Object
-- @param file	 File Object
-- @param length Amount of data to send (in Bytes).
-- @return bytes sent

--- (Linux) Send data from / to a pipe in kernel-space.
-- @class function
-- @name nixio.splice
-- @param fdin	Input I/O descriptor 
-- @param fdout	Output I/O descriptor
-- @param length Amount of data to send (in Bytes).
-- @param flags (optional, bitfield generated by splice_flags)
-- @see nixio.splice_flags
-- @return bytes sent

--- (Linux) Generate a flag bitfield for a call to splice.
-- @class function
-- @name nixio.splice_flags
-- @param flag1	First Flag	["move", "nonblock", "more"]
-- @param ...	More flags	[-"-]
-- @see nixio.splice
-- @return Flag bitfield

--- (POSIX) Open a connection to the system logger.
-- @class function
-- @name nixio.openlog
-- @param ident	Identifier
-- @param flag1	Flag 1 ["cons", "nowait", "pid", "perror", "ndelay", "odelay"]
-- @param ...	More flags [-"-]

--- (POSIX) Close the connection to the system logger.
-- @class function
-- @name nixio.closelog

--- (POSIX) Write a message to the system logger.
-- @class function
-- @name nixio.syslog
-- @param priority Priority ["emerg", "alert", "crit", "err", "warning",
-- "notice", "info", "debug"]
-- @param message

--- (POSIX) Set the logmask of the system logger for current process.
-- @class function
-- @name nixio.setlogmask
-- @param priority Priority ["emerg", "alert", "crit", "err", "warning",
-- "notice", "info", "debug"]

--- (POSIX) Encrypt a user password.
-- @class function
-- @name nixio.crypt
-- @param key Key
-- @param salt Salt
-- @return password hash

--- (POSIX) Get all or a specific user group.
-- @class function
-- @name nixio.getgr
-- @param group Group ID or groupname (optional)
-- @return Table containing: <ul>
-- <li>name = Group Name</li>
-- <li>gid = Group ID</li>
-- <li>passwd = Password</li>
-- <li>mem = {Member #1, Member #2, ...}</li>
-- </ul>

--- (POSIX) Get all or a specific user account.
-- @class function
-- @name nixio.getpw
-- @param user User ID or username (optional)
-- @return Table containing: <ul>
-- <li>name = Name</li>
-- <li>uid = ID</li>
-- <li>gid = Group ID</li>
-- <li>passwd = Password</li>
-- <li>dir = Home directory</li>
-- <li>gecos = Information</li>
-- <li>shell = Shell</li>
-- </ul>

--- (Linux, Solaris) Get all or a specific shadow password entry.
-- @class function
-- @name nixio.getsp
-- @param user username (optional)
-- @return Table containing: <ul>
-- <li>namp = Name</li>
-- <li>expire = Expiration Date</li>
-- <li>flag = Flags</li>
-- <li>inact = Inactivity Date</li>
-- <li>lstchg = Last change</li>
-- <li>max = Maximum</li>
-- <li>min = Minimum</li>
-- <li>warn = Warning</li>
-- <li>pwdp = Password Hash</li>
-- </ul>

--- Create a new TLS context.
-- @class function
-- @name nixio.tls
-- @param mode TLS-Mode ["client", "server"]
-- @return TLSContext Object
