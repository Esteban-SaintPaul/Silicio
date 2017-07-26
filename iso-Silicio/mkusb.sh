cp grub-usb.cfg iso/boot/grub/grub.cfg
cp -r iso/* /media/esteban/9310-A6B3/
grub-install --recheck --debug --boot-directory=/media/esteban/9310-A6B3/boot/ /dev/sdb
