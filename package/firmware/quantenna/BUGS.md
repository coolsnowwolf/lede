# Known Quantenna firmware bugs #

This is an attempt to document Quantenna firmware bugs, explaining strange
implemtation details in this OpenWrt package and also hopefully helping
anyone else trying to get this mess to work...

## Quantenna switch and VLANs ##

Mapping WiFi VIFs to VLANs has a horrible side effect: Traffic from the
Quantenna SoC, like the RPC responses, is tagged with the last used VLAN.

The tag depends on Wireless client activity! I.e it is unpredictable.

And even if it were predictable: The module does not accept tagged RPC
calls or other management traffic.  The RPC requests must be sent
untagged even if the module responds with an arbitrary tag...

The switch state is not reset on either Quantenna module reboot by RPC
call, or reset by GPIO.  The VLAN issue affects the bootloader,
making the module fail to load the second stage bootloader and OS over
tftp because of the VLAN tag mismatch.

Full power cycling is necessary to reset the Quantenna switch to
non-tagging.  There is no known software controllable reset method.

### Workaround ###

Playing with static and gratuitous ARPs, in combination with VLAN
interfaces accepting local traffic works pretty well.

But this does not solve the reboot issue, caused by the switch bug
being persistent over reset.  The two U-Boot stages will switch mac
addresses up to 3 times during module reboot.  The final address is
often dynamically allocated, changing for each boot.  This makes it
hard to manage a static ARP entry.

Using "tc" to pop the VLAN tag of any tagged IP and ARP packets for
our management address works perfectly.


## Using bootloader environment ##

The Quantenna bootloader environment is ignored by the firmware unless
/scripts/build_config contains STATELESS=Y.  This is a build time
setting, which is not set by the Netgear firmware we use.

The implications is that the module will use different mac adresses
etc compared to an OEM firmware with STATELESS=Y.

This should not be considered a bug, but is an issue caused by our
choice of a common Quantenna firmware image for all hosts with such a
module.

