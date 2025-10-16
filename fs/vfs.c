#include "../include/vfs.h"
#include "../include/memory.h"
#include "../include/vga.h"

static vfs_node_t* root_node = 0;
static file_descriptor_t file_descriptors[MAX_FILES];
static filesystem_t* filesystems[MAX_FILESYSTEMS];
static u32 next_fd = 3;
static u32 fs_count = 0;

void vfs_init() {
    for (int i = 0; i < MAX_FILES; i++) {
        file_descriptors[i].node = 0;
        file_descriptors[i].position = 0;
        file_descriptors[i].flags = 0;
        file_descriptors[i].ref_count = 0;
    }
    
    for (int i = 0; i < MAX_FILESYSTEMS; i++) {
        filesystems[i] = 0;
    }
    
    root_node = vfs_create_node("/", FILE_TYPE_DIRECTORY);
    if (root_node) {
        root_node->parent = 0;
        root_node->permissions = 0755;
    }
    
    vga_print("VFS initialized\n");
}

vfs_node_t* vfs_create_node(char* name, file_type_t type) {
    vfs_node_t* node = (vfs_node_t*)0x400000;
    static u32 node_offset = 0;
    node = (vfs_node_t*)(0x400000 + node_offset);
    node_offset += sizeof(vfs_node_t);
    
    if (!node) return 0;
    
    int i = 0;
    while (name[i] && i < MAX_FILENAME - 1) {
        node->name[i] = name[i];
        i++;
    }
    node->name[i] = 0;
    
    static u32 next_inode = 1;
    node->inode = next_inode++;
    node->size = 0;
    node->uid = 0;
    node->gid = 0;
    node->permissions = 0644;
    node->type = type;
    node->atime = 0;
    node->mtime = 0;
    node->ctime = 0;
    node->parent = 0;
    node->children = 0;
    node->next = 0;
    node->fs_data = 0;
    node->fs = 0;
    
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
}

int vfs_mount(filesystem_t* fs, char* mountpoint) {
    if (fs_count >= MAX_FILESYSTEMS) return -1;
    
    filesystems[fs_count] = fs;
    fs_count++;
    
    vga_print("Filesystem mounted at ");
    vga_print(mountpoint);
    vga_print("\n");
    
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
            int match = 1;
            for (int j = 0; name[j] || child->name[j]; j++) {
                if (name[j] != child->name[j]) {
                    match = 0;
                    break;
                }
            }
            if (match) {
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
    vfs_node_t* node = vfs_resolve_path(path);
    
    if (!node && (flags & O_CREAT)) {
        char* last_slash = 0;
        for (char* p = path; *p; p++) {
            if (*p == '/') last_slash = p;
        }
        
        if (last_slash) {
            *last_slash = 0;
            vfs_node_t* parent = vfs_resolve_path(path);
            *last_slash = '/';
            
            if (parent) {
                node = vfs_create_node(last_slash + 1, FILE_TYPE_REGULAR);
                if (node) {
                    node->parent = parent;
                    node->next = parent->children;
                    parent->children = node;
                }
            }
        }
    }
    
    if (!node) return -1;
    
    for (int i = 3; i < MAX_FILES; i++) {
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
    char* last_slash = 0;
    for (char* p = path; *p; p++) {
        if (*p == '/') last_slash = p;
    }
    
    if (!last_slash) return -1;
    
    *last_slash = 0;
    vfs_node_t* parent = vfs_resolve_path(path);
    *last_slash = '/';
    
    if (!parent) return -1;
    
    vfs_node_t* new_dir = vfs_create_node(last_slash + 1, FILE_TYPE_DIRECTORY);
    if (!new_dir) return -1;
    
    new_dir->parent = parent;
    new_dir->next = parent->children;
    parent->children = new_dir;
    new_dir->permissions = 0755;
    
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