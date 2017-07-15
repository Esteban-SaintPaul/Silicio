#mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o Silicio.iso iso
xorrisofs -R -b boot/grub/stage2_eltorito -no-emul-boot -boot-load-size 4 -boot-info-table -o Silicio.iso iso
echo ""
echo "Imagen de cd ceada en el directorio \"iso-Silicio\" "
echo "nombre de la imagen : Silicio.img"
