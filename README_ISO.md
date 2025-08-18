# ACE Kernel - Image ISO Bootable

##  Image ISO Créée

L'image ISO bootable du kernel ACE a été générée avec succès :
- **Fichier** : `build/ace_kernel.iso`
- **Taille** : ~8 MB
- **Bootloader** : GRUB Legacy compatible

##  Utilisation

### 1. Test avec QEMU
```bash
# Tester l'ISO avec QEMU
make qemu-iso

# Ou directement :
qemu-system-i386 -cdrom build/ace_kernel.iso -nographic
```

### 2. Création d'une clé USB bootable

#### Sur macOS :
```bash
# Identifier votre clé USB
diskutil list

# Démonter la clé USB (remplacez diskX par votre clé)
diskutil unmountDisk /dev/diskX

# Copier l'ISO sur la clé USB
sudo dd if=build/ace_kernel.iso of=/dev/rdiskX bs=1m

# Éjecter la clé USB
diskutil eject /dev/diskX
```

#### Sur Linux :
```bash
# Identifier votre clé USB
lsblk

# Copier l'ISO sur la clé USB (remplacez sdX par votre clé)
sudo dd if=build/ace_kernel.iso of=/dev/sdX bs=1M status=progress

# Synchroniser
sync
```

### 3. Boot sur ordinateur physique

1. **Insérer la clé USB** dans votre ordinateur
2. **Redémarrer** et accéder au BIOS/UEFI (généralement F2, F12, DEL, ou ESC)
3. **Configurer le boot** :
   - Activer le "Legacy Boot" ou "CSM" si disponible
   - Désactiver le "Secure Boot" si nécessaire
   - Mettre la clé USB en premier dans l'ordre de boot
4. **Sauvegarder et redémarrer**

##  Fonctionnalités du Kernel

Le kernel ACE inclut :
-  Bootloader Multiboot compatible
-  Gestionnaire de mémoire basique
-  Ordonnanceur de processus
-  Drivers VGA, clavier, timer
-  Appels système essentiels
-  Système de fichiers VFS
-  Shell interactif avec commandes

### Commandes disponibles dans le shell :
- `help` - Affiche l'aide
- `clear` - Efface l'écran
- `ls` - Liste les fichiers
- `cat <fichier>` - Affiche le contenu d'un fichier
- `mkdir <dir>` - Crée un répertoire
- `ps` - Liste les processus
- `mem` - Affiche l'utilisation mémoire
- `uptime` - Temps de fonctionnement

## 🔧 Compilation

Pour recompiler l'ISO :
```bash
# Nettoyer et recompiler
make clean

# Créer l'image ISO
make iso

# Tester avec QEMU
make qemu-iso
```

## ⚠️ Notes importantes

- **Compatibilité** : Testé sur x86 32-bit
- **BIOS Legacy** : Fonctionne en mode BIOS Legacy (pas UEFI natif)
- **Matériel** : Compatible avec la plupart des PC x86
- **Sécurité** : Kernel éducatif, ne pas utiliser en production

##  Dépannage

Si le boot échoue :
1. Vérifier que le "Legacy Boot" est activé
2. Désactiver le "Secure Boot"
3. Essayer différents ports USB
4. Vérifier l'intégrité de l'ISO avec `md5sum`

##  Développement

Pour modifier le kernel :
1. Éditer les fichiers sources dans `kernel/`, `drivers/`, etc.
2. Recompiler avec `make clean && make iso`
3. Tester avec `make qemu-iso`

---

**ACE Kernel** - Micro-kernel éducatif développé pour l'apprentissage des systèmes d'exploitation.
