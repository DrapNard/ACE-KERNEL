#include "shell/modules/fs_module.h"
#include "drivers/vga.h"
#include "fs/vfs.h"
#include "shell/string_utils.h"

static char current_path[MAX_PATH] = "/";

int fs_module_init(void) {
    vga_print("[fs_module] Initialisation\n");
    copy_string(current_path, "/", sizeof(current_path));
    return 0;
}

int fs_module_cleanup(void) {
    vga_print("[fs_module] Nettoyage\n");
    return 0;
}

int cmd_cd(int argc, char* argv[]) {
    if (argc < 2) {
        vga_print("Usage: cd <repertoire>\n");
        return -1;
    }

    char new_path[MAX_PATH];
    if (argv[1][0] == '/') {
        // Absolute path
        // Simple normalize_path implementation
        if (argv[1][1] == 0) { // Just "/"
            copy_string(new_path, "/", sizeof(new_path));
        } else {
            copy_string(new_path, argv[1], sizeof(new_path));
        }
    } else {
        // Relative path
        if (strings_equal(argv[1], "..")) {
            // Go up one directory
            u32 len = strlen(current_path);
            if (len > 1) { // Not already at root
                while (len > 1 && current_path[len - 1] == '/') len--;
                while (len > 1 && current_path[len - 1] != '/') len--;
                if (len > 1) len--; // Remove trailing slash
                new_path[0] = 0;
                if (len > 0) {
                    strncat(new_path, current_path, len);
                } else {
                    new_path[0] = 0;
                }
                if (strlen(new_path) == 0) {
                    new_path[0] = '/';
                    new_path[1] = 0;
                }
            } else {
                copy_string(new_path, "/", sizeof(new_path));
            }
        } else if (strings_equal(argv[1], ".")) {
            // Stay in current directory
            copy_string(new_path, current_path, sizeof(new_path));
        } else {
            // Append to current path
            copy_string(new_path, current_path, sizeof(new_path));
            if (new_path[strlen(new_path) - 1] != '/') {
                strcat(new_path, "/");
            }
            strcat(new_path, argv[1]);
        }
    }

    // Validate the new path exists
    vfs_node_t* node = vfs_resolve_path(new_path);
    if (!node || node->type != FILE_TYPE_DIRECTORY) {
        vga_printf("Repertoire introuvable: %s\n", new_path);
        return -1;
    }

    copy_string(current_path, new_path, sizeof(current_path));
    vga_printf("Repertoire courant: %s\n", current_path);
    return 0;
}

int cmd_pwd(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    vga_print(current_path);
    vga_print("\n");
    return 0;
}

int cmd_cp(int argc, char* argv[]) {
    if (argc < 3) {
        vga_print("Usage: cp <source> <destination>\n");
        return -1;
    }

    char src_path[MAX_PATH], dst_path[MAX_PATH];
    // Simple path normalization
    if (argv[1][0] == '/') {
        copy_string(src_path, argv[1], sizeof(src_path));
    } else {
        copy_string(src_path, current_path, sizeof(src_path));
        if (src_path[strlen(src_path) - 1] != '/') {
            strcat(src_path, "/");
        }
        strcat(src_path, argv[1]);
    }
    
    if (argv[2][0] == '/') {
        copy_string(dst_path, argv[2], sizeof(dst_path));
    } else {
        copy_string(dst_path, current_path, sizeof(dst_path));
        if (dst_path[strlen(dst_path) - 1] != '/') {
            strcat(dst_path, "/");
        }
        strcat(dst_path, argv[2]);
    }

    int src_fd = vfs_open(src_path, O_RDONLY);
    if (src_fd < 0) {
        vga_printf("Impossible d'ouvrir source: %s\n", src_path);
        return -1;
    }

    int dst_fd = vfs_open(dst_path, O_CREAT | O_WRONLY);
    if (dst_fd < 0) {
        vga_printf("Impossible de creer destination: %s\n", dst_path);
        vfs_close(src_fd);
        return -1;
    }

    char buffer[512];
    int bytes_read;
    int total_copied = 0;

    while ((bytes_read = vfs_read(src_fd, buffer, sizeof(buffer))) > 0) {
        int bytes_written = vfs_write(dst_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            vga_print("Erreur lors de l'ecriture\n");
            vfs_close(src_fd);
            vfs_close(dst_fd);
            return -1;
        }
        total_copied += bytes_read;
    }

    vfs_close(src_fd);
    vfs_close(dst_fd);

    vga_printf("Copie terminee: %d octets\n", total_copied);
    return 0;
}

int cmd_mv(int argc, char* argv[]) {
    if (argc < 3) {
        vga_print("Usage: mv <source> <destination>\n");
        return -1;
    }

    vga_print("Deplacement non implemente dans cette version\n");
    return -1; // Not implemented yet
}

// Module structure
shell_module_t fs_shell_module = {
    .name = "fs",
    .description = "Commandes de gestion de fichiers",
    .init = fs_module_init,
    .cleanup = fs_module_cleanup,
    .next = NULL
};