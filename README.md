rtl8188fu for linux
===================


**PSA** public service announcement  
-------------------------------------
This driver will only work with kernels <= v5.10  
.. there no use for a new kernel  
.. sorry for that  

the purpose of this repository is to rip/strip unneeded parts from the vendor driver  
because of ..  

rtl8188fu linux driver for wireless bgn device

end of PSA
----------

Note:
This is an USB2 only adapter,
Source is ripped out of firefly source tree found at
https://gitlab.com/TeeFirefly/linux-kernel

<u>If one USB-ID is missing, please mail me.</u>

Build/load/function tested with v4.19, v5.8

Building and install driver
---------------------------

### The easiest way: with `dkms`

First install `dkms` from your distribution. Then execute this command at terminal:

```
sudo dkms add .
```

Then install it with:

```
sudo dkms install rtl8188fu
```

If you ever update your kernel, you need to execute the above command again.

To load the drivers, execute this:

```
sudo modprobe rtl8188fu
```

To load it automatically on boot, add `rtl8188fu` to `/etc/modules` file.

### Manual way

For building execute:

```bash
make
```

For load the driver

```bash
sudo modprobe cfg80211
sudo insmod rtl8188fu.ko
```

You need to install the needed fw with:

```bash
sudo make installfw
```

If you need to cross-compile, append these variable to `make` command:

```bash
ARCH={architecture} CROSS_COMPILE={target} KSRC={linux-source-or-header-directory}
```

Example:

```bash
make ARCH="arm" CROSS_COMPILE=armv5tel-softfloat-linux-gnueabi- KSRC=/home/linux-master modules
```

Please use prefix **rtl8188fu** if you want to mail me.
But please please don't, I have enough to do.
TIA

Hans Ulli Kroll <linux@ulli-kroll.de>
