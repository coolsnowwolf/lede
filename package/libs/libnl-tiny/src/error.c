/*
 * lib/error.c		Error Handling
 *
 *	This library is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU Lesser General Public
 *	License as published by the Free Software Foundation version 2.1
 *	of the License.
 *
 * Copyright (c) 2008 Thomas Graf <tgraf@suug.ch>
 */

#include <netlink-local.h>
#include <netlink/netlink.h>

static const char *errmsg[NLE_MAX+1] = {
[NLE_SUCCESS]		= "Success",
[NLE_FAILURE]		= "Unspecific failure",
[NLE_INTR]		= "Interrupted system call",
[NLE_BAD_SOCK]		= "Bad socket",
[NLE_AGAIN]		= "Try again",
[NLE_NOMEM]		= "Out of memory",
[NLE_EXIST]		= "Object exists",
[NLE_INVAL]		= "Invalid input data or parameter",
[NLE_RANGE]		= "Input data out of range",
[NLE_MSGSIZE]		= "Message size not sufficient",
[NLE_OPNOTSUPP]		= "Operation not supported",
[NLE_AF_NOSUPPORT]	= "Address family not supported",
[NLE_OBJ_NOTFOUND]	= "Object not found",
[NLE_NOATTR]		= "Attribute not available",
[NLE_MISSING_ATTR]	= "Missing attribute",
[NLE_AF_MISMATCH]	= "Address family mismatch",
[NLE_SEQ_MISMATCH]	= "Message sequence number mismatch",
[NLE_MSG_OVERFLOW]	= "Kernel reported message overflow",
[NLE_MSG_TRUNC]		= "Kernel reported truncated message",
[NLE_NOADDR]		= "Invalid address for specified address family",
[NLE_SRCRT_NOSUPPORT]	= "Source based routing not supported",
[NLE_MSG_TOOSHORT]	= "Netlink message is too short",
[NLE_MSGTYPE_NOSUPPORT]	= "Netlink message type is not supported",
[NLE_OBJ_MISMATCH]	= "Object type does not match cache",
[NLE_NOCACHE]		= "Unknown or invalid cache type",
[NLE_BUSY]		= "Object busy",
[NLE_PROTO_MISMATCH]	= "Protocol mismatch",
[NLE_NOACCESS]		= "No Access",
[NLE_PERM]		= "Operation not permitted",
[NLE_PKTLOC_FILE]	= "Unable to open packet location file",
[NLE_PARSE_ERR]		= "Unable to parse object",
[NLE_NODEV]		= "No such device",
[NLE_IMMUTABLE]		= "Immutable attribute",
[NLE_DUMP_INTR]		= "Dump inconsistency detected, interrupted",
};

/**
 * Return error message for an error code
 * @return error message
 */
const char *nl_geterror(int error)
{
	error = abs(error);

	if (error > NLE_MAX)
		error = NLE_FAILURE;

	return errmsg[error];
}

/**
 * Print a libnl error message
 * @arg s		error message prefix
 *
 * Prints the error message of the call that failed last.
 *
 * If s is not NULL and *s is not a null byte the argument
 * string is printed, followed by a colon and a blank. Then
 * the error message and a new-line.
 */
void nl_perror(int error, const char *s)
{
	if (s && *s)
		fprintf(stderr, "%s: %s\n", s, nl_geterror(error));
	else
		fprintf(stderr, "%s\n", nl_geterror(error));
}

int nl_syserr2nlerr(int error)
{
	error = abs(error);

	switch (error) {
	case EBADF:		return NLE_BAD_SOCK;
	case EADDRINUSE:	return NLE_EXIST;
	case EEXIST:		return NLE_EXIST;
	case EADDRNOTAVAIL:	return NLE_NOADDR;
	case ESRCH:		/* fall through */
	case ENOENT:		return NLE_OBJ_NOTFOUND;
	case EINTR:		return NLE_INTR;
	case EAGAIN:		return NLE_AGAIN;
	case ENOTSOCK:		return NLE_BAD_SOCK;
	case ENOPROTOOPT:	return NLE_INVAL;
	case EFAULT:		return NLE_INVAL;
	case EACCES:		return NLE_NOACCESS;
	case EINVAL:		return NLE_INVAL;
	case ENOBUFS:		return NLE_NOMEM;
	case ENOMEM:		return NLE_NOMEM;
	case EAFNOSUPPORT:	return NLE_AF_NOSUPPORT;
	case EPROTONOSUPPORT:	return NLE_PROTO_MISMATCH;
	case EOPNOTSUPP:	return NLE_OPNOTSUPP;
	case EPERM:		return NLE_PERM;
	case EBUSY:		return NLE_BUSY;
	case ERANGE:		return NLE_RANGE;
	case ENODEV:		return NLE_NODEV;
	default:		return NLE_FAILURE;
	}
}

/** @} */

