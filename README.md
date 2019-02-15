For Kernel 4.15 ~ 4.20 (Linux Mint or Ubuntu Derivatives)

------------------

sudo apt-get install build-essential git dkms linux-headers-$(uname -r)

git clone https://github.com/kelebek333/rtl8188fu

sudo dkms add ./rtl8188fu

sudo dkms build rtl8188fu/1.0

sudo dkms install rtl8188fu/1.0

sudo cp ./rtl8188fu/firmware/rtl8188fufw.bin /lib/firmware/rtlwifi/

------------------

Alternativly, you can download and install rtl8188fu-dkms and rtl8188fufw-firmware deb files.

https://github.com/kelebek333/rtl8188fu/raw/master/rtl8188fu-dkms_1.0.2_amd64.deb

https://github.com/kelebek333/rtl8188fu/raw/master/rtl8188fufw-firmware_1.0.1.deb


------------------

Run following for disable power mangement and plugging/replugging issues.
Güç yönetimini pasifleştirmek ve takmak/yeniden takma sorunları için aşağıdaki komutları çalıştırınız.

sudo mkdir -p /etc/modprobe.d/

sudo touch /etc/modprobe.d/rtl8188fu.conf

echo "options rtl8188fu rtw_power_mgnt=0 rtw_enusbss=0" | sudo tee /etc/modprobe.d/rtl8188fu.conf
