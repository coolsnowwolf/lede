TEXT_START   := 0x800000

# It seems that the TP-Link TL-WDR4900 U-Boot does not flush the cache before
# jumping into the uImage. Padding the image to 32k seems make the boot work
# reliably.
#
# We leave 0x40 for the uImage header, which allows us to append the kernel
# uImage after the loader and read with an added offset 0x8000 from the
# SPI flash.
PAD_TO       := 0x807fc0

# Kernel begins at 0x60200 in flash, loader adds 0x8000
IMAGE_OFFSET := 0x68200
