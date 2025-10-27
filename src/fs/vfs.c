#include "fs/vfs.h"
#include "drivers/vga.h"
#include "libk/mem.h"
#include "mm/heap.h"

static vfs_node_t* root_node = 0;
static file_descriptor_t file_descriptors[MAX_FILES];
static filesystem_t* filesystems[MAX_FILESYSTEMS];
static u32 fs_count = 0;
static u32 next_inode = 1;

static void copy_string(char* dest, const char* src, size_t max_len) {
    if (!dest || !max_len) {
        return;
    }
    size_t i = 0;
    if (src) {
        for (; i + 1 < max_len && src[i]; ++i) {
            dest[i] = src[i];
        }
    }
    dest[i] = '\0';
}

static int strings_equal(const char* a, const char* b) {
    if (!a || !b) {
        return 0;
    }
    size_t i = 0;
    while (a[i] || b[i]) {
        if (a[i] != b[i]) {
            return 0;
        }
        ++i;
    }
    return 1;
}

void vfs_init() {
    k_memset(file_descriptors, 0, sizeof(file_descriptors));
    k_memset(filesystems, 0, sizeof(filesystems));
    fs_count = 0;
    next_inode = 1;

    root_node = vfs_create_node("/", FILE_TYPE_DIRECTORY);
    if (root_node) {
        root_node->parent = root_node;
    }

    vga_printf("[fs] vfs initialised\n");
}

vfs_node_t* vfs_create_node(char* name, file_type_t type) {
    vfs_node_t* node = (vfs_node_t*)kmalloc(sizeof(vfs_node_t));
    if (!node) {
        return 0;
    }

    k_memset(node, 0, sizeof(*node));

    int i = 0;
    while (name && name[i] && i < MAX_FILENAME - 1) {
        node->name[i] = name[i];
        ++i;
    }
    node->name[i] = 0;

    node->inode = next_inode++;
    node->permissions = (type == FILE_TYPE_DIRECTORY) ? 0755 : 0644;
    node->type = type;

    return node;
}

void vfs_destroy_node(vfs_node_t* node) {
    if (!node) return;
    
    if (node->children) {
        vfs_node_t* child = node->children;
        while (child) {
            vfs_node_t* next = child->next;
            vfs_destroy_node(child);
            child = next;
        }
    }

    kfree(node);
}

int vfs_mount(filesystem_t* fs, char* mountpoint) {
    if (fs_count >= MAX_FILESYSTEMS) return -1;
    
    filesystems[fs_count] = fs;
    fs_count++;
    
    vga_printf("[fs] mounted at %s\n", mountpoint);
    
    return 0;
}

int vfs_unmount(char* mountpoint) {
    (void)mountpoint;
    vga_print("Unmount not implemented\n");
    return -1;
}

vfs_node_t* vfs_resolve_path(char* path) {
    if (!path || path[0] != '/') return 0;
    
    if (path[1] == 0) return root_node;
    
    vfs_node_t* current = root_node;
    char* token = path + 1;
    
    while (*token) {
        char name[MAX_FILENAME];
        int i = 0;
        
        while (*token && *token != '/' && i < MAX_FILENAME - 1) {
            name[i++] = *token++;
        }
        name[i] = 0;
        
        if (*token == '/') token++;
        
        vfs_node_t* child = current->children;
        vfs_node_t* found = 0;
        
        while (child) {
            if (strings_equal(name, child->name)) {
                found = child;
                break;
            }
            child = child->next;
        }
        
        if (!found) return 0;
        current = found;
    }
    
    return current;
}

int vfs_open(char* path, u32 flags) {
    if (!path || path[0] == '\0') {
        return -1;
    }

    char path_copy[MAX_PATH];
    copy_string(path_copy, path, MAX_PATH);

    vfs_node_t* node = vfs_resolve_path(path_copy);

    if (!node && (flags & O_CREAT)) {
        char parent_path[MAX_PATH];
        copy_string(parent_path, path_copy, MAX_PATH);

        char* last_slash = 0;
        for (char* p = parent_path; *p; ++p) {
            if (*p == '/') {
                last_slash = p;
            }
        }

        if (last_slash && *(last_slash + 1)) {
            *last_slash = '\0';
            const char* child_name = last_slash + 1;
            vfs_node_t* parent = (last_slash == parent_path)
                ? root_node
                : vfs_resolve_path(parent_path);

            if (parent) {
                node = vfs_create_node((char*)child_name, FILE_TYPE_REGULAR);
                if (node) {
                    node->parent = parent;
                    node->next = parent->children;
                    parent->children = node;
                }
            }
        }
    }

    if (!node) {
        return -1;
    }

    for (int i = 3; i < MAX_FILES; ++i) {
        if (file_descriptors[i].node == 0) {
            file_descriptors[i].node = node;
            file_descriptors[i].position = 0;
            file_descriptors[i].flags = flags;
            file_descriptors[i].ref_count = 1;
            return i;
        }
    }

    return -1;
}

int vfs_close(int fd) {
    if (fd < 0 || fd >= MAX_FILES) return -1;
    if (file_descriptors[fd].node == 0) return -1;
    
    file_descriptors[fd].ref_count--;
    if (file_descriptors[fd].ref_count == 0) {
        file_descriptors[fd].node = 0;
        file_descriptors[fd].position = 0;
        file_descriptors[fd].flags = 0;
    }
    
    return 0;
}

int vfs_read(int fd, void* buffer, u32 count) {
    if (fd < 0 || fd >= MAX_FILES) return -1;
    if (file_descriptors[fd].node == 0) return -1;
    
    vfs_node_t* node = file_descriptors[fd].node;
    
    if (node->fs && node->fs->read) {
        return node->fs->read(node, file_descriptors[fd].position, count, (u8*)buffer);
    }
    
    u8* buf = (u8*)buffer;
    for (u32 i = 0; i < count; i++) {
        buf[i] = 'A' + (i % 26);
    }
    
    file_descriptors[fd].position += count;
    return count;
}

int vfs_write(int fd, void* buffer, u32 count) {
    if (fd < 0 || fd >= MAX_FILES) return -1;
    if (file_descriptors[fd].node == 0) return -1;
    
    vfs_node_t* node = file_descriptors[fd].node;
    
    if (node->fs && node->fs->write) {
        return node->fs->write(node, file_descriptors[fd].position, count, (u8*)buffer);
    }
    
    file_descriptors[fd].position += count;
    if (file_descriptors[fd].position > node->size) {
        node->size = file_descriptors[fd].position;
    }
    return count;
}

int vfs_seek(int fd, u32 offset, int whence) {
    if (fd < 0 || fd >= MAX_FILES) return -1;
    if (file_descriptors[fd].node == 0) return -1;
    
    switch (whence) {
        case 0:
            file_descriptors[fd].position = offset;
            break;
        case 1:
            file_descriptors[fd].position += offset;
            break;
        case 2:
            file_descriptors[fd].position = file_descriptors[fd].node->size + offset;
            break;
        default:
            return -1;
    }
    
    return file_descriptors[fd].position;
}

int vfs_stat(char* path, vfs_node_t* stat_buf) {
    vfs_node_t* node = vfs_resolve_path(path);
    if (!node || !stat_buf) return -1;
    
    *stat_buf = *node;
    return 0;
}

int vfs_mkdir(char* path) {
    if (!path || path[0] != '/') {
        return -1;
    }

    char path_copy[MAX_PATH];
    copy_string(path_copy, path, MAX_PATH);

    char* last_slash = 0;
    for (char* p = path_copy; *p; ++p) {
        if (*p == '/') {
            last_slash = p;
        }
    }

    if (!last_slash || !*(last_slash + 1)) {
        return -1;
    }

    *last_slash = '\0';
    const char* name = last_slash + 1;
    vfs_node_t* parent = (last_slash == path_copy)
        ? root_node
        : vfs_resolve_path(path_copy);

    if (!parent || parent->type != FILE_TYPE_DIRECTORY) {
        return -1;
    }

    vfs_node_t* new_dir = vfs_create_node((char*)name, FILE_TYPE_DIRECTORY);
    if (!new_dir) {
        return -1;
    }

    new_dir->parent = parent;
    new_dir->next = parent->children;
    parent->children = new_dir;

    return 0;
}

int vfs_rmdir(char* path) {
    vfs_node_t* node = vfs_resolve_path(path);
    if (!node || node->type != FILE_TYPE_DIRECTORY) return -1;
    if (node->children) return -1;
    
    return vfs_unlink(path);
}

int vfs_unlink(char* path) {
    vfs_node_t* node = vfs_resolve_path(path);
    if (!node) return -1;
    
    if (node->parent) {
        vfs_node_t* parent = node->parent;
        if (parent->children == node) {
            parent->children = node->next;
        } else {
            vfs_node_t* prev = parent->children;
            while (prev && prev->next != node) {
                prev = prev->next;
            }
            if (prev) {
                prev->next = node->next;
            }
        }
    }
    
    vfs_destroy_node(node);
    return 0;
}

vfs_node_t* vfs_get_root() {
    return root_node;
}

file_descriptor_t* vfs_get_fd(int fd) {
    if (fd < 0 || fd >= MAX_FILES) return 0;
    if (file_descriptors[fd].node == 0) return 0;
    return &file_descriptors[fd];
}

