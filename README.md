For Kernel 4.15 ~ 4.20 (Linux Mint or Ubuntu Derivatives)

sudo apt-get install build-essential git dkms linux-headers-$(uname -r)

git clone https://github.com/kelebek333/rtl8188fu

sudo dkms add ./rtl8188fu

sudo dkms install rtl8188fu/1.0

sudo cp ./rtl8188fu/firmware/rtl8188fufw.bin /lib/firmware/rtlwifi/
