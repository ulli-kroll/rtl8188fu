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

You need to install the needed fw with  
`sudo make installfw`  
`sudo make install`

for load the driver  
`sudo modprobe cfg80211`  
`sudo modprobe rtl8188fu`  

To load on boot add these line to /etc/modules
`cfg80211`  
`rtl8188fu`

If you need to crosscompile use  
`ARCH= CROSS_COMPILE= KSRC=`  
while calling `make` i.e.  

`make ARCH="arm" CROSS_COMPILE=armv5tel-softfloat-linux-gnueabi- KSRC=/home/linux-master modules`  

Please submit a issue if you encounter a problem.  
(Note: I am not a professional C and C++ programmer,  
I only know the basics and I am still learning.)

Mainly developed by Hans Ulli Kroll <https://github.com/ulli-kroll/>.
