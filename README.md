rtl8188fu for linux
===================

rtl8188fu linux driver for wireless bgn device

Note:
This is an USB2 only adapter,  
Source is ripped out of firefly source tree found at  
https://gitlab.com/TeeFirefly/linux-kernel 

<u>If one USB-ID is missing, please mail me.</u>  

build/load/function tested with v4.19  

Building and install driver
---------------------------

for building type  
`make`  

for load the driver  
`sudo modprobe cfg80211`  
`sudo insmod rtl8188fu.ko`  

You need to install the needed fw with  
`sudo make installfw`  

If you need to crosscompile use  
`ARCH= CROSS_COMPILE= KSRC=`  
while calling `make` i.e.  

`make ARCH="arm" CROSS_COMPILE=armv5tel-softfloat-linux-gnueabi- KSRC=/home/linux-master modules`  

Please use prefix **rtl8188fu** if you want to mail me  
But please please don't, I have enough to do.  
TIA  

Hans Ulli Kroll <ulli.kroll@googlemail.com>
