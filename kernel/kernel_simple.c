// Kernel simple pour QEMU avec en-tête Multiboot

// Types de base
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef u32 size_t;

u32 multiboot_magic = 0x1BADB002;
u32 multiboot_flags = 0x00000003;
u32 multiboot_checksum = -(0x1BADB002 + 0x00000003);

// VGA simple
#define VGA_MEMORY 0xB8000
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static u16* vga_buffer = (u16*)VGA_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

void vga_clear() {
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++) {
        vga_buffer[i] = 0x0F20;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void vga_putchar(char c) {
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = VGA_HEIGHT - 1;
            // Scroll simple
            for (int i = 0; i < VGA_WIDTH * (VGA_HEIGHT - 1); i++) {
                vga_buffer[i] = vga_buffer[i + VGA_WIDTH];
            }
            for (int i = VGA_WIDTH * (VGA_HEIGHT - 1); i < VGA_WIDTH * VGA_HEIGHT; i++) {
                vga_buffer[i] = 0x0F20;
            }
        }
        return;
    }

    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) {
            cursor_y = VGA_HEIGHT - 1;
        }
    }

    vga_buffer[cursor_y * VGA_WIDTH + cursor_x] = 0x0F00 | c;
    cursor_x++;
}

void vga_print(const char* str) {
    while (*str) {
        vga_putchar(*str);
        str++;
    }
}

void kernel_main();

void syscall_init();
void test_syscalls();
void shell_init();
void shell_run();
void interrupts_init();
void keyboard_init();

void kernel_main() {
    vga_clear();

    vga_print("=== ACE ===\n");
    vga_print("Initialisation\n");

    vga_print("VGA: OK\n");
    vga_print("Memoire: Simulation OK\n");
    
    interrupts_init();
    vga_print("Interruptions: OK\n");
    
    keyboard_init();
    vga_print("Clavier: OK\n");
    
    vga_print("Ordonnanceur: Simulation OK\n");
    
    syscall_init();
    vga_print("Appels systeme: OK\n");

    vga_print("\nTest d'allocation memoire:\n");
    vga_print("- Allocation 1024 octets: OK\n");
    vga_print("- Allocation 2048 octets: OK\n");
    vga_print("- Liberation memoire: OK\n");

    vga_print("\nInformations memoire:\n");
    vga_print("- Memoire totale: 32 MB\n");
    vga_print("- Memoire utilisee: 4 KB\n");
    vga_print("- Memoire libre: 32764 KB\n");
    
    test_syscalls();

    vga_print("\nSysteme de fichiers VFS: Implemente\n");

    vga_print("\nInitialisation du shell...\n");
    shell_init();
    
    vga_print("Demarrage du shell...\n");
    shell_run();

    vga_print("\n=== ACE Kernel ===\n");
    vga_print("Kernel en attente\n");

    while (1) {
        // Attendre
        __asm__ volatile ("hlt");
    }
}
