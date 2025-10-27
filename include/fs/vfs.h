#ifndef FS_VFS_H
#define FS_VFS_H

#include "ace/types.h"

#define MAX_FILENAME 256
#define MAX_PATH 1024
#define MAX_FILES 128
#define MAX_FILESYSTEMS 8

#define O_RDONLY 0x00
#define O_WRONLY 0x01
#define O_RDWR   0x02
#define O_CREAT  0x40
#define O_TRUNC  0x200
#define O_APPEND 0x400

typedef enum {
    FILE_TYPE_REGULAR,
    FILE_TYPE_DIRECTORY,
    FILE_TYPE_DEVICE,
    FILE_TYPE_SYMLINK
} file_type_t;

typedef struct vfs_node {
    char name[MAX_FILENAME];
    u32 inode;
    u32 size;
    u32 uid;
    u32 gid;
    u32 permissions;
    file_type_t type;
    u32 atime;
    u32 mtime;
    u32 ctime;
    
    struct vfs_node* parent;
    struct vfs_node* children;
    struct vfs_node* next;
    
    void* fs_data;
    struct filesystem* fs;
} vfs_node_t;

typedef struct file_descriptor {
    vfs_node_t* node;
    u32 position;
    u32 flags;
    u32 ref_count;
} file_descriptor_t;

typedef struct filesystem {
    char name[32];
    vfs_node_t* root;
    
    int (*read)(vfs_node_t* node, u32 offset, u32 size, u8* buffer);
    int (*write)(vfs_node_t* node, u32 offset, u32 size, u8* buffer);
    vfs_node_t* (*finddir)(vfs_node_t* node, char* name);
    vfs_node_t* (*readdir)(vfs_node_t* node, u32 index);
    int (*create)(vfs_node_t* parent, char* name, file_type_t type);
    int (*unlink)(vfs_node_t* node);
} filesystem_t;

void vfs_init();
int vfs_mount(filesystem_t* fs, char* mountpoint);
int vfs_unmount(char* mountpoint);

vfs_node_t* vfs_resolve_path(char* path);
vfs_node_t* vfs_create_node(char* name, file_type_t type);
void vfs_destroy_node(vfs_node_t* node);

int vfs_open(char* path, u32 flags);
int vfs_close(int fd);
int vfs_read(int fd, void* buffer, u32 count);
int vfs_write(int fd, void* buffer, u32 count);
int vfs_seek(int fd, u32 offset, int whence);
int vfs_stat(char* path, vfs_node_t* stat_buf);

int vfs_mkdir(char* path);
int vfs_rmdir(char* path);
int vfs_unlink(char* path);

vfs_node_t* vfs_get_root();
file_descriptor_t* vfs_get_fd(int fd);
#endif /* FS_VFS_H */
