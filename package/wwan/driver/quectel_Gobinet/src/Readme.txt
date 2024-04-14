Gobi3000 network driver 2011-07-29-1026

This readme covers important information concerning 
the Gobi Net driver.

Table of Contents

1. What's new in this release
2. Known issues
3. Known platform issues


-------------------------------------------------------------------------------

1. WHAT'S NEW

This Release (Gobi3000 network driver 2011-07-29-1026)
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

2. KNOWN ISSUES

No known issues.
         
-------------------------------------------------------------------------------

3. KNOWN PLATFORM ISSUES

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



