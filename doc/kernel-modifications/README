# Linux Kernel Modifications

In order to run and test the flash drive, the default Linux kernel has to be
slightly modified. This is because VUSB for AVR microcontrollers only supports
Low-Speed USB transfers which don't support Bulk endpoints that are required
for mass storage devices according to the USB standard.

The device can be made to work by removing a few lines from the Linux kernel
which check whether a Low-Speed devices uses bulk endpoints and then disables
those endpoints.

An example patch file containing the modifications for the default (Torvalds)
Linux 5.7.5 kernel can be found [here](./patch).

The code for Linux 5.7.5 can be downloaded from [here](wget https://cdn.kernel.org/pub/linux/kernel/v5.x/linux-5.7.5.tar.xz).

And the modified and pre-compiled kernel for Ubuntu 20.10 can be found in this
repository here: [linux-image](./linux-image-5.7.5_5.7.5-1_amd64.deb) and
[linux-headers](./linux-headers-5.7.5_5.7.5-1_amd64.deb).

Simply install both packages using `sudo dpkg -i <filename>.deb` and then reboot
selecting the modified kernel in Grub.
