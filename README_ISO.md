# Image ISO ACE (à venir)

La génération d'une image ISO bootable n'est pas encore intégrée à la nouvelle
infrastructure de build. La pile a été restructurée autour d'un noyau
Multiboot (`src/arch/x86/boot/multiboot_entry.s`) et d'un `Makefile` qui produit
`build/kernel.bin`. L'exécution standard sous QEMU se fait désormais via :

```bash
make            # compile le noyau
make run        # lance qemu-system-i386 -serial stdio
```

## Roadmap ISO

- Préparer un `grub.cfg` minimal et une hiérarchie `isofiles/`.
- Ajouter une cible `make iso` qui assemble l'image via `grub-mkrescue`.
- Prévoir `make run-iso` pour tester l'image avec `qemu-system-i386 -cdrom`.

## Fabriquer une ISO manuellement (expérimental)

1. Créer l'arborescence :

   ```bash
   mkdir -p isofiles/boot/grub
   cp build/kernel.bin isofiles/boot/kernel.bin
   cat > isofiles/boot/grub/grub.cfg <<'EOF'
   set timeout=0
   set default=0
   menuentry "ACE Kernel" {
       multiboot /boot/kernel.bin
       boot
   }
   EOF
   ```

2. Générer l'ISO (nécessite `grub-mkrescue`) :

   ```bash
   grub-mkrescue -o build/ace.iso isofiles
   ```

3. Tester :

   ```bash
   qemu-system-i386 -cdrom build/ace.iso -serial stdio -display none
   ```

4. Pour une clé USB, utiliser `dd` comme décrit précédemment.

---

Ce fichier sera mis à jour dès que la cible `make iso` sera réintroduite dans le
`Makefile` principal.
