--- Low-level and high-level filesystem manipulation library.
module "nixio.fs"


--- Check user's permission on a file.
-- @class function
-- @name nixio.fs.access
-- @param path		Path
-- @param mode1		First Mode to check ["f", "r", "w", "x"]
-- @param ...		More Modes to check	[-"-]
-- @return	true

--- Strip the directory part from a path.
-- @class function
-- @name nixio.fs.basename
-- @usage	This function cannot fail and will never return nil.
-- @param	path Path
-- @return	basename

--- Strip the base from a path.
-- @class function
-- @name nixio.fs.dirname
-- @usage	This function cannot fail and will never return nil.
-- @param	path Path
-- @return	dirname

--- Return the cannonicalized absolute pathname.
-- @class function
-- @name nixio.fs.realpath
-- @param	path Path
-- @return	absolute path

--- Remove a file or directory.
-- @class function
-- @name nixio.fs.remove
-- @param	path Path
-- @return	true

--- Delete a name and - if no links are left - the associated file.
-- @class function
-- @name nixio.fs.unlink
-- @param	path Path
-- @return	true

--- Renames a file or directory.
-- @class function
-- @name nixio.fs.rename
-- @param	src	Source path
-- @param 	dest Destination path
-- @usage	It is normally not possible to rename files accross fileystems.
-- @return	true

--- Remove an empty directory.
-- @class function
-- @name nixio.fs.rmdir
-- @param	path Path
-- @return	true

--- Create a new directory.
-- @class function
-- @name nixio.fs.mkdir
-- @param	path Path
-- @param	mode File mode (optional, see chmod and umask)
-- @see	nixio.fs.chmod
-- @see	nixio.umask
-- @return	true

--- Change the file mode.
-- @class function
-- @name	nixio.fs.chmod
-- @usage	Windows only supports setting the write-protection through the
-- "Writable to others" bit.
-- @usage	<strong>Notice:</strong> The mode-flag for the functions
-- open, mkdir, mkfifo are affected by the umask. 
-- @param	path Path
-- @param	mode File mode	
-- [decimal mode number, "[-r][-w][-xsS][-r][-w][-xsS][-r][-w][-xtT]"]
-- @see	nixio.umask
-- @return true

--- Iterate over the entries of a directory.
-- @class function
-- @name nixio.fs.dir
-- @usage	The special entries "." and ".." are omitted.
-- @param	path Path
-- @return	directory iterator returning one entry per call

--- Create a hard link.
-- @class function
-- @name nixio.fs.link
-- @usage This function calls link() on POSIX and CreateHardLink() on Windows.
-- @param	oldpath Path
-- @param	newpath Path
-- @return	true

--- Change file last access and last modification time.
-- @class function
-- @name nixio.fs.utimes
-- @param	path Path
-- @param	actime Last access timestamp	(optional, default: current time)
-- @param	mtime Last modification timestamp (optional, default: actime)
-- @return	true

--- Get file status and attributes.
-- @class function
-- @name nixio.fs.stat
-- @param	path	Path
-- @param	field	Only return a specific field, not the whole table (optional)
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

--- Get file status and attributes and do not resolve if target is a symlink.
-- @class function
-- @name nixio.fs.lstat
-- @param	path	Path
-- @param	field	Only return a specific field, not the whole table (optional)
-- @see	nixio.fs.stat
-- @return Table containing attributes (see stat for a detailed description)

--- (POSIX) Change owner and group of a file.
-- @class function
-- @name nixio.fs.chown
-- @param	path	Path
-- @param	user	User ID or Username		(optional)
-- @param 	group	Group ID or Groupname	(optional)
-- @return true

--- (POSIX) Change owner and group of a file and do not resolve
-- if target is a symlink.
-- @class function
-- @name nixio.fs.lchown
-- @param	path	Path
-- @param	user	User ID or Username		(optional)
-- @param 	group	Group ID or Groupname	(optional)
-- @return true

--- (POSIX) Create a FIFO (named pipe).
-- @class function
-- @name nixio.fs.mkfifo
-- @param	path Path
-- @param	mode File mode (optional, see chmod and umask)
-- @see	nixio.fs.chmod
-- @see	nixio.umask
-- @return	true

--- (POSIX) Create a symbolic link.
-- @class function
-- @name nixio.fs.symlink
-- @param	oldpath Path
-- @param	newpath Path
-- @return	true

--- (POSIX) Read the target of a symbolic link.
-- @class function
-- @name nixio.fs.readlink
-- @param	path Path
-- @return	target path

--- (POSIX) Find pathnames matching a pattern.
-- @class function
-- @name nixio.fs.glob
-- @param	pattern Pattern
-- @return	path iterator
-- @return	number of matches

--- (POSIX) Get filesystem statistics.
-- @class function
-- @name nixio.fs.statvfs
-- @param	path Path to any file within the filesystem.
-- @return	Table containing: <ul>
-- <li>bavail = available blocks</li>
-- <li>bfree = free blocks</li>
-- <li>blocks = number of fragments</li>
-- <li>frsize = fragment size</li>
-- <li>favail = available inodes</li>
-- <li>ffree = free inodes</li>
-- <li>files = inodes</li>
-- <li>flag = flags</li>
-- <li>fsid = filesystem ID</li>
-- <li>namemax = maximum filename length</li>
-- </ul>

--- Read the contents of a file into a buffer.
-- @class function
-- @name nixio.fs.readfile
-- @param	path Path
-- @param	limit	Maximum bytes to read (optional)
-- @return	file contents

--- Write a buffer into a file truncating the file first.
-- @class function
-- @name nixio.fs.writefile
-- @param	path Path
-- @param	data Data to write
-- @return	true

--- Copy data between files.
-- @class function
-- @name nixio.fs.datacopy
-- @param	src	Source file path
-- @param 	dest Destination file path
-- @param	limit	Maximum bytes to copy (optional)
-- @return	true

--- Copy a file, directory or symlink non-recursively preserving file mode,
-- timestamps, owner and group.
-- @class function
-- @name nixio.fs.copy
-- @usage	The destination must always be a full destination path e.g. do not
-- omit the basename even if source and destination basename are equal. 
-- @param	src	Source path
-- @param 	dest Destination path
-- @return	true

--- Rename a file, directory or symlink non-recursively across filesystems.
-- @class function
-- @name nixio.fs.move
-- @usage	The destination must always be a full destination path e.g. do not
-- omit the basename even if source and destination basename are equal. 
-- @param	src	Source path
-- @param 	dest Destination path
-- @return	true

--- Create a directory and all needed parent directories recursively. 
-- @class function
-- @name nixio.fs.mkdirr
-- @param 	dest Destination path
-- @param	mode File mode (optional, see chmod and umask)
-- @see	nixio.fs.chmod
-- @see	nixio.umask
-- @return	true

--- Rename a file, directory or symlink recursively across filesystems.
-- @class function
-- @name nixio.fs.mover
-- @usage	The destination must always be a full destination path e.g. do not
-- omit the basename even if source and destination basename are equal. 
-- @param	src	Source path
-- @param 	dest Destination path
-- @return	true

--- Copy a file, directory or symlink recursively preserving file mode,
-- timestamps, owner and group.
-- @class function
-- @name nixio.fs.copyr
-- @usage	The destination must always be a full destination path e.g. do not
-- omit the basename even if source and destination basename are equal. 
-- @param	src	Source path
-- @param 	dest Destination path
-- @return	true