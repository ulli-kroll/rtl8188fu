for reboot fix problem run this command(for ubuntu like distro)
write 'sudo visudo'
replace this line 
'# Allow members of group sudo to execute any command
%sudo   ALL=(ALL:ALL) ALL'
with
'# Allow members of group sudo to execute any command
%sudo   ALL=(ALL:ALL) ALL
username ALL=(ALL) NOPASSWD: /sbin/modprobe,/sbin/insmod'
where username is your username
this basically remove sudo privilage for the above command so you dont need to write password with these two command
then make a new file and write this in it 'sudo modprobe cfg80211  && sudo insmod /home/username/rtl8188fu/rtl8188fu.ko'
then save this file with 'yourchoice-anyname.sh'and make it executable with 'sudo chmod +x yourchoice-anyname.sh' 
and make this file read and write for root and your username respectively by going to its file property. 
now open startup applications =>add custom command=> name 'wifi' and browse 'yourchoice-anyname.sh' file and click add
now reboot and it should enable wifi with boot automatically
enjoy !!
