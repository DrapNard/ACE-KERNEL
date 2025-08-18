# ACE Micro-Kernel

Un micro-kernel fonctionnel basé sur une architecture x86, développé en C et assembleur.

## Caractéristiques

### Architecture
- **Micro-kernel** : Architecture modulaire avec composants séparés
- **Mode protégé x86** : Fonctionnement en 32 bits
- **Gestion mémoire** : Allocateur dynamique avec heap
- **Ordonnanceur** : Round-robin avec gestion des processus
- **Interruptions** : Gestion complète des IRQ et exceptions
- **Drivers** : VGA, clavier, timer

### Composants principaux

#### Kernel Core (`kernel/`)
- `main.c` : Point d'entrée et initialisation
- `scheduler.c` : Ordonnanceur de processus
- `interrupts.c` : Gestion des interruptions

#### Gestion Mémoire (`mm/`)
- `memory.c` : Allocateur dynamique (kmalloc/kfree)
- Heap de 1MB avec fusion automatique des blocs

#### Drivers (`drivers/`)
- `vga.c` : Driver d'affichage VGA couleur
- Support clavier avec table de conversion
- Timer système pour l'ordonnanceur

#### Boot (`boot/`)
- `boot.s` : Bootloader minimal
- Passage en mode protégé
- Chargement de la GDT

## Structure du projet

```
ACE/
├── boot/           # Bootloader
│   └── boot.s
├── kernel/         # Noyau principal
│   ├── main.c
│   ├── scheduler.c
│   └── interrupts.c
├── drivers/        # Pilotes
│   └── vga.c
├── mm/            # Gestion mémoire
│   └── memory.c
├── include/       # Fichiers d'en-tête
│   ├── kernel.h
│   ├── memory.h
│   ├── scheduler.h
│   ├── interrupts.h
│   └── vga.h
├── build/         # Fichiers compilés
├── Makefile       # Script de compilation
├── linker.ld      # Script de liaison
└── README.md      # Cette documentation
```

## Compilation

### Prérequis
- GCC avec support 32 bits
- GNU Binutils (as, ld, objcopy)
- Make
- QEMU (optionnel, pour l'exécution)

### Installation des dépendances

**Ubuntu/Debian :**
```bash
sudo apt-get install gcc-multilib binutils make qemu-system-x86
```

**macOS :**
```bash
brew install i686-elf-gcc qemu
```

### Compilation

```bash
# Compiler tout le projet
make all

# Créer une image disque bootable
make iso

# Exécuter avec QEMU
make run

# Nettoyer les fichiers générés
make clean
```

## Utilisation

### Exécution avec QEMU
```bash
make run
```

### Exécution avec VirtualBox
1. Créer une nouvelle VM x86
2. Utiliser `build/ace.img` comme disquette de boot
3. Démarrer la VM

### Fonctionnalités disponibles
- **Affichage** : Messages du kernel en couleur
- **Clavier** : Saisie de texte basique
- **Processus** : Ordonnancement automatique
- **Mémoire** : Allocation dynamique

## Architecture technique

### Initialisation
1. **Bootloader** : Passage en mode protégé
2. **Kernel** : Initialisation des sous-systèmes
3. **VGA** : Configuration de l'affichage
4. **Mémoire** : Initialisation du heap
5. **Interruptions** : Configuration IDT et PIC
6. **Ordonnanceur** : Création du processus idle

### Gestion des processus
- **États** : READY, RUNNING, BLOCKED, TERMINATED
- **Algorithme** : Round-robin avec tranches de temps
- **Contexte** : Sauvegarde des registres x86
- **Pile** : 4KB par processus

### Gestion mémoire
- **Heap** : 1MB à partir de 0x100000
- **Allocation** : First-fit avec division des blocs
- **Libération** : Fusion automatique des blocs adjacents
- **Alignement** : 4 bytes pour les performances

### Interruptions
- **IDT** : 256 entrées configurées
- **PIC** : Remapping des IRQ (32-47)
- **Timer** : IRQ0 pour l'ordonnanceur
- **Clavier** : IRQ1 avec table de conversion

## Développement

### Ajouter un nouveau driver
1. Créer le fichier dans `drivers/`
2. Ajouter l'en-tête dans `include/`
3. Initialiser dans `kernel_main()`
4. Le Makefile détecte automatiquement

### Ajouter un appel système
1. Définir dans `include/syscalls.h`
2. Implémenter dans `kernel/syscalls.c`
3. Configurer l'interruption 0x80

### Debug
- Messages avec `vga_print()`
- `kernel_panic()` pour les erreurs critiques
- QEMU monitor pour l'inspection

## Limitations actuelles

- **Architecture** : x86 32 bits uniquement
- **Système de fichiers** : Non implémenté
- **Réseau** : Non supporté
- **Mode utilisateur** : Tous les processus en mode kernel
- **Pagination** : Gestion mémoire simplifiée

## Roadmap

- [ ] Système de fichiers VFS
- [ ] Mode utilisateur et protection
- [ ] Pagination mémoire
- [ ] Drivers réseau
- [ ] Shell interactif
- [ ] Support multiprocesseur

## Licence

Ce projet est développé à des fins éducatives. Libre d'utilisation et de modification.

## Contribution

Les contributions sont les bienvenues ! Merci de :
1. Fork le projet
2. Créer une branche pour votre fonctionnalité
3. Tester vos modifications
4. Soumettre une pull request

---

**ACE Micro-Kernel** - Un kernel simple mais fonctionnel pour l'apprentissage des systèmes d'exploitation.# ACE-KERNEL
