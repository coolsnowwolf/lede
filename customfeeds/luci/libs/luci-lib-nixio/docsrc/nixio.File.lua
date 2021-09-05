--- Large File Object.
-- Large file operations are supported up to 52 bits if the Lua number type is
-- double (default).
-- @cstyle	instance
module "nixio.File"

--- Write to the file descriptor.
-- @class function
-- @name File.write
-- @usage <strong>Warning:</strong> It is not guaranteed that all data
-- in the buffer is written at once especially when dealing with pipes.
-- You have to check the return value - the number of bytes actually written -
-- or use the safe IO functions in the high-level IO utility module.
-- @usage Unlike standard Lua indexing the lowest offset and default is 0.
-- @param buffer	Buffer holding the data to be written.
-- @param offset	Offset to start reading the buffer from. (optional)
-- @param length	Length of chunk to read from the buffer. (optional)
-- @return number of bytes written

--- Read from a file descriptor.
-- @class function
-- @name File.read
-- @usage <strong>Warning:</strong> It is not guaranteed that all requested data
-- is read at once especially when dealing with pipes.
-- You have to check the return value - the length of the buffer actually read -
-- or use the safe IO functions in the high-level IO utility module.
-- @usage The length of the return buffer is limited by the (compile time) 
-- nixio buffersize which is <em>nixio.const.buffersize</em> (8192 by default).
-- Any read request greater than that will be safely truncated to this value.  
-- @param length	Amount of data to read (in Bytes).
-- @return buffer containing data successfully read

--- Reposition read / write offset of the file descriptor.
-- The seek will be done either from the beginning of the file or relative
-- to the current position or relative to the end.
-- @class function
-- @name File.seek
-- @usage This function calls lseek().
-- @param offset	File Offset
-- @param whence	Starting point [<strong>"set"</strong>, "cur", "end"]
-- @return new (absolute) offset position

--- Return the current read / write offset of the file descriptor.
-- @class function
-- @name File.tell
-- @usage This function calls lseek() with offset 0 from the current position.
-- @return offset position

--- Synchronizes the file with the storage device.
-- Returns when the file is successfully written to the disk.
-- @class function
-- @name File.sync
-- @usage This function calls fsync() when data_only equals false
-- otherwise fdatasync(), on Windows _commit() is used instead.
-- @usage fdatasync() is only supported by Linux and Solaris. For other systems
-- the <em>data_only</em> parameter is ignored and fsync() is always called.
-- @param data_only		Do not synchronize the metadata. (optional, boolean)
-- @return true

--- Apply or test a lock on the file.
-- @class function
-- @name File.lock
-- @usage This function calls lockf() on POSIX and _locking() on Windows.
-- @usage The "lock" command is blocking, "tlock" is non-blocking,
-- "ulock" unlocks and "test" only tests for the lock.
-- @usage The "test" command is not available on Windows.
-- @usage Locks are by default advisory on POSIX, but mandatory on Windows.
-- @param command	Locking Command ["lock", "tlock", "ulock", "test"]
-- @param length	Amount of Bytes to lock from current offset (optional)
-- @return true

--- Get file status and attributes.
-- @class function
-- @name File.stat
-- @param	field	Only return a specific field, not the whole table (optional)
-- @usage This function calls fstat().
-- @return	Table containing: <ul>
-- <li>atime = Last access timestamp</li>
-- <li>blksize = Blocksize (POSIX only)</li>
-- <li>blocks = Blocks used (POSIX only)</li>
-- <li>ctime = Creation timestamp</li>
-- <li>dev = Device ID</li>
-- <li>gid = Group ID</li>
-- <li>ino = Inode</li>
-- <li>modedec = Mode converted into a decimal number</li>
-- <li>modestr = Mode as string as returned by `ls -l`</li>
-- <li>mtime = Last modification timestamp</li>
-- <li>nlink = Number of links</li>
-- <li>rdev = Device ID (if special file)</li>
-- <li>size = Size in bytes</li>
-- <li>type = ["reg", "dir", "chr", "blk", "fifo", "lnk", "sock"]</li>
-- <li>uid = User ID</li>
-- </ul>

--- Close the file descriptor.
-- @class function
-- @name File.close
-- @return true

--- Get the number of the filedescriptor.
-- @class function
-- @name File.fileno
-- @return file descriptor number

--- (POSIX) Set the blocking mode of the file descriptor.
-- @class function
-- @name File.setblocking
-- @param	blocking	(boolean)
-- @return true