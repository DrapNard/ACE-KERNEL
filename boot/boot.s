.code16
.global _start

_start:
    # Message de démarrage simple
    mov $0x0E, %ah
    mov $'A', %al
    int $0x10
    mov $'C', %al
    int $0x10
    mov $'E', %al
    int $0x10
    mov $' ', %al
    int $0x10
    mov $'O', %al
    int $0x10
    mov $'K', %al
    int $0x10
    mov $'\r', %al
    int $0x10
    mov $'\n', %al
    int $0x10
    
    # Boucle infinie
halt:
    hlt
    jmp halt
    
# Remplir jusqu'à 510 octets
.org 510
.word 0xAA55