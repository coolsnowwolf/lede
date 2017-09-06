#!/usr/bin/env python
#
# Copyright 2008, 2009 (C) Jose Vasconcellos <jvasco@verizon.net>
#
# A script that can communicate with jungo-based routers
# (such as MI424-WR, USR8200 and WRV54G) to backup the installed
# firmware and replace the boot loader.
#
# Tested with Python 2.5 on Linux and Windows
#
"""Usage: %s [options] <IP_address> [image.bin | url]
Valid options:
\t-h | --help: usage statement
\t-d | --dump: create a flash dump
\t-f | --file: use <filename> to store dump contents
\t-u | --user: provide username (default admin)
\t-p | --pass: provide password (default password1)
\t     --port: set port for http (default 8080)
\t-q | --quiet: don't display unnecessary information
\t-r | --reboot: reboot target on successful transfer
\t-V | --version: display version information

If no image (or url) is given, a flash dump is created.
A built-in http server is used when an image file is provided.
"""

import os
import sys
import getopt
import getpass
import telnetlib
import string
import binascii
import socket
import thread
import SocketServer
import SimpleHTTPServer

reboot = 0
HOST = "192.168.1.1"
PORT = 8080
user = "admin"
#password = getpass.getpass()
password = "password1"
proto = "http"
url = ""
imagefile = ""
dumpfile = ""
verbose = 1
do_dump = 0
dumplen = 0x10000
flashsize=4*1024*1024
#device="br0"
device="ixp0"

####################

def start_server(server):
    httpd = SocketServer.TCPServer((server,PORT),SimpleHTTPServer.SimpleHTTPRequestHandler)
    thread.start_new_thread(httpd.serve_forever,())

####################

def get_flash_size():
    # make sure we don't have an A0 stepping
    tn.write("cat /proc/cpuinfo\n")
    buf = tn.read_until("Returned 0", 3)
    if not buf:
        print "Unable to obtain CPU information; make sure to not use A0 stepping!"
    elif buf.find('rev 0') > 0:
        print "Warning: IXP42x stepping A0 detected!"
        if imagefile or url:
            print "Error: No linux support for A0 stepping!"
            sys.exit(2)

    # now get flash size
    tn.write("cat /proc/mtd\n")
    buf = tn.read_until("Returned 0", 3)
    if buf:
        i = buf.find('mtd0:')
        if i > 0:
            return int(buf[i+6:].split()[0],16)
        # use different command
        tn.write("flash_layout\n")
        buf = tn.read_until("Returned 0", 3)
        i = buf.rfind('Range ')
        if i > 0:
            return int(buf[i+17:].split()[0],16)
        print "Can't determine flash size!"
    else:
        print "Unable to obtain flash size!"
    sys.exit(2)

def image_dump(tn, dumpfile):
    if not dumpfile:
        tn.write("ver\n");
        buf = tn.read_until("Returned 0",2)
        i = buf.find("Platform:")
        if i < 0:
	    platform="jungo"
	else:
	    line=buf[i+9:]
	    i=line.find('\n')
	    platform=line[:i].split()[-1]

        tn.write("rg_conf_print /dev/%s/mac\n" % device);
        buf = tn.read_until("Returned 0",3)

	i = buf.find("mac(")
	if i > 0:
	    i += 4
	else:
	    print "No MAC address found! (use -f option)"
	    sys.exit(1)
        dumpfile = "%s-%s.bin" % (platform, buf[i:i+17].replace(':',''))
    else:
        tn.write("\n")

    print "Dumping flash contents (%dMB) to %s" % (flashsize/1048576, dumpfile)
    f = open(dumpfile, "wb")

    t=flashsize/dumplen
    for addr in range(t):
	if verbose:
	    sys.stdout.write('\r%d%%'%(100*addr/t))
	    sys.stdout.flush()

        tn.write("flash_dump -r 0x%x -l %d -4\n" % (addr*dumplen, dumplen))
	tn.read_until("\n")

	count = addr*dumplen
        while 1:
            buf = tn.read_until("\n")
            if buf.strip() == "Returned 0":
                break
            s = buf.split()
            if s and s[0][-1] == ':':
		a=int(s[0][:-1],16)
		if a != count:
		    print "Format error: %x != %x"%(a,count)
		    sys.exit(2)
	    	count += 16
		f.write(binascii.a2b_hex(string.join(s[1:],'')))
	tn.read_until(">",1)

    f.close()
    if verbose:
	print ""

def telnet_option(sock,cmd,option):
    #print "Option: %d %d" % (ord(cmd), ord(option))
    if cmd == telnetlib.DO:
        c=telnetlib.WILL
    elif cmd == telnetlib.WILL:
        c=telnetlib.DO
    sock.sendall(telnetlib.IAC + c + option)

def telnet_timeout():
    print "Fatal error: telnet timeout!"
    sys.exit(1)

def usage():
    print __doc__ % os.path.basename(sys.argv[0])

####################

try:
    opts, args = getopt.getopt(sys.argv[1:], "hdf:qp:P:rvV", \
	["help", "dump", "file=", "user=", "pass=", "port=",
	 "quiet=", "reboot", "verbose", "version"])
except getopt.GetoptError:
    # print help information and exit:
    usage()
    sys.exit(1)

for o, a in opts:
    if o in ("-h", "--help"):
	usage()
	sys.exit(1)
    elif o in ("-V", "--version"):
	print "%s: 0.11" % sys.argv[0]
	sys.exit(1)
    elif o in ("-d", "--no-dump"):
	do_dump = 1
    elif o in ("-f", "--file"):
	dumpfile = a
    elif o in ("-u", "--user"):
	user = a
    elif o in ("-p", "--pass"):
	password = a
    elif o == "--port":
	PORT = int(a)
    elif o in ("-q", "--quiet"):
	verbose = 0
    elif o in ("-r", "--reboot"):
	reboot = 1
    elif o in ("-v", "--verbose"):
	verbose = 1

# make sure we have enough arguments
if len(args) > 0:
    HOST = args[0]

if len(args) == 2:
    if args[1].split(':')[0] in ("tftp", "http", "ftp"):
        url = args[1]
    else:
        imagefile = args[1]
else:
    do_dump = 1;

####################
# create a telnet session to the router
try:
    tn = telnetlib.Telnet(HOST)
except socket.error, msg:
    print "Unable to establish telnet session to %s: %s" % (HOST, msg)
    sys.exit(1)

tn.set_option_negotiation_callback(telnet_option)

buf = tn.read_until("Username: ", 3)
if not buf:
    telnet_timeout()
tn.write(user+"\n")
if password:
    buf = tn.read_until("Password: ", 3)
    if not buf:
        telnet_timeout()
    tn.write(password+"\n")

# wait for prompt
buf = tn.read_until("> ", 3)
if not buf:
    telnet_timeout()

flashsize = get_flash_size()

if do_dump:
    image_dump(tn, dumpfile)

if imagefile or url:
    splitpath = os.path.split(imagefile)

    # create load command
    if url:
        cmd = "load -u %s -r 0\n" % (url)
    else:
        server = tn.get_socket().getsockname()[0]
        cmd = "load -u http://%s:%d/%s -r 0\n" % (server, PORT, splitpath[1])

        if not os.access(imagefile, os.R_OK):
            print "File access error: %s" % (imagefile)
            sys.exit(3)

        # make sure we're in the directory where the image is located
        if splitpath[0]:
            os.chdir(splitpath[0])

        start_server(server)

    if verbose:
	print "Unlocking flash..."
    tn.write("unlock 0 0x%x\n" % flashsize)
    buf = tn.read_until("Returned 0",5)

    if verbose:
	print "Writing new image..."
    print cmd,
    tn.write(cmd)
    buf = tn.read_until("Returned 0",10)

    # wait till the transfer completed
    buf = tn.read_until("Download completed successfully",20)
    if buf:
	print "Flash update complete!"
        if reboot:
            tn.write("reboot\n")
            print "Rebooting..."

tn.write("exit\n")
tn.close()

