#!/usr/bin/python

from ftplib import FTP
from sys import argv
from os import stat

assert len(argv) == 3
ip = argv[1]
image = argv[2]

size = stat(image).st_size
# arbitrary size limit, to prevent the address calculations from overflows etc.
assert size < 0x2000000

# We need to align the address. A page boundary seems to be sufficient on 7362sl
# and 7412
addr = ((0x8000000 - size) & ~0xfff)
haddr = 0x80000000 + addr
img = open(image, "rb")

ftp = FTP(ip, 'adam2', 'adam2')

def adam(cmd):
	print("> %s"%(cmd))
	resp = ftp.sendcmd(cmd)
	print("< %s"%(resp))
	assert resp[0:3] == "200"

ftp.set_pasv(True)
# The following parameters allow booting the avm recovery system with this
# script.
adam('SETENV memsize 0x%08x'%(addr))
adam('SETENV kernel_args_tmp mtdram1=0x%08x,0x88000000'%(haddr))
adam('MEDIA SDRAM')
ftp.storbinary('STOR 0x%08x 0x88000000'%(haddr), img)
img.close()
ftp.close()
