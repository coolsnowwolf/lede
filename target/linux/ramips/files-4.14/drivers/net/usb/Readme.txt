GobiNet network driver 2016-10-14

This readme covers important information concerning 
the Gobi Net driver.

Table of Contents

1. What's new in this release
2. Build and Installation.
3. Known issues
4. Known platform issues


-------------------------------------------------------------------------------

1. WHAT'S NEW

This Release (GobiNet network driver 2016-10-14)
a. Added QMAP MUXing support.

Prior Release (GobiNet network driver 2016-10-04)
a. Added QMAP aggregation support.

Prior Release (Gobi3000 network driver 2011-07-29-1026)
a. Signal the device to leave low power mode on enumeration
b. Add "txQueueLength" parameter, which will set the Tx Queue Length
c. Send SetControlLineState message during driver/device removal
d. Change to new date-based versioning scheme

Prior Release (Gobi3000 network driver 1.0.60) 06/29/2011
a. Add UserspacePoll() function, to support select()
b. Fix possible deadlock on GobiUSBNetTXTimeout()
c. Fix memory leak on data transmission

Prior Release (Gobi3000 network driver 1.0.50) 05/18/2011
a. Add support for kernels up to 2.6.38
b. Add support for dynamic interface binding

Prior Release (Gobi3000 network driver 1.0.40) 02/28/2011
a. In cases of QMI read errors, discard the error and continue reading.
b. Add "interruptible" parameter, which may be disabled for debugging purposes.

Prior Release (Gobi3000 network driver 1.0.30) 01/05/2011
a. Fix rare kernel PANIC if a process terminates while file handle close 
   or device removal is in progress.

Prior Release (Gobi3000 network driver 1.0.20) 11/01/2010
a. Fix possible kernel WARNING if device removed before QCWWANDisconnect().
b. Fix multiple memory leaks in error cases.

Prior Release (Gobi3000 network driver 1.0.10) 09/17/2010
a. Initial release

-------------------------------------------------------------------------------

2. Build and Installation

Build
-----

1. Run Make from the extracted folder, it should build GobiNet.ko

Installation
------------
1. GobiNet.ko is dependent on usbnet.ko and mii.ko kernel modules
   > insmod mii.ko (found in /lib/modules/<kernel-version>/kernel/drivers/net)
   > insmod usbnet.ko (found in /lib/modules/<kernel-version>/kernel/drivers/net/usb)
   > insmod GobiNet.ko

Test
----

Since App support is not yet supported we have tested the MUX support with loopback calls with file attributes.

MUX feature:
------------

The header file Structs.h has a defination called MAX_MUX_DEVICES, this defines number od MUX adapters support.
Currently it is defined to be 1. For each MUX adapter a new file attribure is created.
/sys/testgobi/gobi0
/sys/testgobi/gobi1
/sys/testgobi/gobi2
...
...
...

TO make a call with MUX adapter 1:
>echo 1 > /sys/testgobi/gobi0, this will initiate a data call.
 The IP address gets printed in dmesg, please assing the IP address statically for the usb0 interface.
 ifconfig usb0 <ipaddress> up

TO make a call with MUX adapter 2:
>echo 1 > /sys/testgobi/gobi1, this will initiate a data call.
 The IP address gets printed in dmesg, please assing the IP address statically for the usb0:0 interface.
 ifconfig usb0:0 <ipaddress> up

TO make a call with MUX adapter 3:
>echo 1 > /sys/testgobi/gobi2, this will initiate a data call.
 The IP address gets printed in dmesg, please assing the IP address statically for the usb0:1 interface.
 ifconfig usb0:1 <ipaddress> up

TO disconnect a call:
>echo 0 > /sys/testgobi/gobi0, this will bring down the data call on MUX1.
>echo 0 > /sys/testgobi/gobi1, this will bring down the data call on MUX2.
>echo 0 > /sys/testgobi/gobi2, this will bring down the data call on MUX3.
    
-------------------------------------------------------------------------------

3. KNOWN ISSUES

No known issues.
         
-------------------------------------------------------------------------------

4. KNOWN PLATFORM ISSUES

a. Enabling autosuspend:
   Autosuspend is supported by the Gobi3000 module and its drivers, 
   but by default it is not enabled by the open source kernel. As such,
   the Gobi3000 module will not enter autosuspend unless the
   user specifically turns on autosuspend with the command:
      echo auto > /sys/bus/usb/devices/.../power/level
b. Ksoftirq using 100% CPU:
   There is a known issue with the open source usbnet driver that can 
   result in infinite software interrupts. The fix for this is to test 
   (in the usbnet_bh() function) if the usb_device can submit URBs before 
   attempting to submit the response URB buffers.
c. NetworkManager does not recognize connection after resume:
   After resuming from sleep/hibernate, NetworkManager may not recognize new
   network connections by the Gobi device. This is a system issue not specific
   to the Gobi device, which may result in dhcp not being run and the default
   route not being updated.  One way to fix this is to simply restart the 
   NetworkManager service.

-------------------------------------------------------------------------------




