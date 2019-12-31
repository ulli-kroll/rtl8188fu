RTL8188EUS for linux
===================

RTL8188EUS linux driver for wireless bgn device

Note:
This is an USB2 only adapter,  
Source is ripped out of firefly source tree found at  
<https://gitlab.com/TeeFirefly/linux-kernel> 

<u>If one USB-ID is missing, please submit a issue<br/>and specify the branch name.</u>  

Tested with v4.19 and v5.0  

Building and install driver
---------------------------

for building type  
`make`  

You need to install the needed fw with  
`sudo make installfw`  
`sudo make install`

To load driver 
`sudo modprobe cfg80211`  
`sudo modprobe rtl8188fu`  

To load on boot add these line to /etc/modules  
`cfg80211`  
`rtl8188fu`

If you need to crosscompile use  
`ARCH= CROSS_COMPILE= KSRC=`  
while calling `make` i.e.  

`make ARCH="arm" CROSS_COMPILE=armv5tel-softfloat-linux-gnueabi- KSRC=/home/linux-master modules`  

Please submit a issue if you encounter a problem  
and also mention the branch name.  
(Note: I am not a professional C and C++ programmer,  
I only know the basics and I am still learning.)

Based on rtl8188fu <https://github.com/ulli-kroll/rtl8188fu>  
developed by Hans Ulli Kroll <https://github.com/ulli-kroll/>.
