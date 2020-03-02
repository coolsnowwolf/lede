- If you want to support S5 WOL, you have to find

	EXTRA_CFLAGS += -DRTL8152_S5_WOL

  in the Makefile. Then, remove the first character '#", if it exists.


- For Fedora, you may have to run the following command after installing the
  driver.

	# dracut -f

- For Ubuntu, you may have to run the following command after installing the
  driver.

	# sudo depmod -a
	# sudo update-initramfs -u

- Example of setting speed

	# ethtool -s eth0 autoneg on advertise 0x802f (2.5G)
	# ethtool -s eth0 autoneg on advertise 0x002f (1G)
	# ethtool -s eth0 autoneg on advertise 0x000f (100M full)
	# ethtool -s eth0 autoneg on advertise 0x0003 (10M full)

- Disable center tap short

	# make CONFIG_CTAP_SHORT=OFF modules
