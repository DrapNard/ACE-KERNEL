#include "fs/simplefs.h"
#include "drivers/storage/ata.h"
#include "drivers/vga.h"
#include "libk/mem.h"
#include "mm/heap.h"
#include "fs/vfs.h"
#include "ace/types.h"

// Define SECTOR_SIZE if not already defined
#ifndef SECTOR_SIZE
#define SECTOR_SIZE 512
#endif

// Define DISK_SECTORS if not already defined
#ifndef DISK_SECTORS
#define DISK_SECTORS (1024 * 1024 / SECTOR_SIZE)  // 1MB disk
#endif

// SimpleFS superblock structure
typedef struct {
    u32 magic;          // Magic number to identify SimpleFS
    u32 block_size;     // Block size (should be SECTOR_SIZE)
    u32 total_blocks;   // Total number of blocks
    u32 free_blocks;    // Number of free blocks
    u32 root_inode;     // Inode number of root directory
    char name[32];      // File system name
} simplefs_superblock_t;

// Inode structure for SimpleFS
typedef struct {
    u32 size;           // File size in bytes
    u32 type;           // File type (regular, directory, etc.)
    u32 block_count;    // Number of blocks used
    u32 blocks[32];     // Direct block pointers (for small files)
    u32 permissions;    // File permissions
    u32 uid;            // User ID
    u32 gid;            // Group ID
    u32 atime;          // Access time
    u32 mtime;          // Modification time
    u32 ctime;          // Creation time
} simplefs_inode_t;

// Directory entry structure
typedef struct {
    u32 inode;          // Inode number
    char name[256];     // File name
} simplefs_direntry_t;

// SimpleFS private data structure
typedef struct {
    simplefs_superblock_t superblock;
    u8* block_bitmap;   // Block allocation bitmap
    u32 bitmap_size;    // Size of bitmap in bytes
    u32 first_inode_block;  // First block where inodes are stored
    u32 first_data_block;   // First block where data is stored
} simplefs_private_t;

// SimpleFS magic number
#define SIMPLEFS_MAGIC 0x53494D50  // "SIMP" in hex

// File types
#define SIMPLEFS_TYPE_REGULAR 0
#define SIMPLEFS_TYPE_DIRECTORY 1
#define SIMPLEFS_TYPE_SYMLINK 2

// Global SimpleFS instance
static simplefs_private_t* simplefs = NULL;

// Helper functions (define them here since they're not exported from vfs.c)
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

// Helper functions
static u32 simplefs_get_free_block(void) {
    if (!simplefs) return 0;
    
    for (u32 i = 0; i < simplefs->bitmap_size * 8; i++) {
        u32 byte_idx = i / 8;
        u32 bit_idx = i % 8;
        
        if (byte_idx < simplefs->bitmap_size && 
            !(simplefs->block_bitmap[byte_idx] & (1 << bit_idx))) {
            
            simplefs->block_bitmap[byte_idx] |= (1 << bit_idx);
            simplefs->superblock.free_blocks--;
            return i + simplefs->first_data_block;
        }
    }
    return 0; // No free blocks
}

static void simplefs_free_block(u32 block) {
    if (!simplefs || block < simplefs->first_data_block) return;
    
    u32 relative_block = block - simplefs->first_data_block;
    u32 byte_idx = relative_block / 8;
    u32 bit_idx = relative_block % 8;
    
    if (byte_idx < simplefs->bitmap_size) {
        simplefs->block_bitmap[byte_idx] &= ~(1 << bit_idx);
        simplefs->superblock.free_blocks++;
    }
}

static bool simplefs_read_block(u32 block_num, void* buffer) {
    return disk_read_sector(block_num, (u8*)buffer);
}

static bool simplefs_write_block(u32 block_num, void* buffer) {
    return disk_write_sector(block_num, (u8*)buffer);
}

static simplefs_inode_t* simplefs_get_inode(u32 inode_num) {
    if (!simplefs || inode_num == 0) return NULL;
    
    // Calculate which block contains this inode
    u32 inodes_per_block = SECTOR_SIZE / sizeof(simplefs_inode_t);
    u32 block_idx = simplefs->first_inode_block + (inode_num - 1) / inodes_per_block;
    u32 inode_idx = (inode_num - 1) % inodes_per_block;
    
    u8 block_data[SECTOR_SIZE];
    if (!simplefs_read_block(block_idx, block_data)) {
        return NULL;
    }
    
    simplefs_inode_t* inodes = (simplefs_inode_t*)block_data;
    return &inodes[inode_idx];
}

static bool simplefs_write_inode(u32 inode_num, simplefs_inode_t* inode) {
    if (!simplefs || inode_num == 0) return false;
    
    u32 inodes_per_block = SECTOR_SIZE / sizeof(simplefs_inode_t);
    u32 block_idx = simplefs->first_inode_block + (inode_num - 1) / inodes_per_block;
    u32 inode_idx = (inode_num - 1) % inodes_per_block;
    
    u8 block_data[SECTOR_SIZE];
    if (!simplefs_read_block(block_idx, block_data)) {
        return false;
    }
    
    simplefs_inode_t* inodes = (simplefs_inode_t*)block_data;
    inodes[inode_idx] = *inode;
    
    return simplefs_write_block(block_idx, block_data);
}

// SimpleFS implementation functions
static int simplefs_read(vfs_node_t* node, u32 offset, u32 size, u8* buffer) {
    if (!node || !buffer || !simplefs) return -1;
    
    simplefs_inode_t* inode = simplefs_get_inode(node->inode);
    if (!inode) return -1;
    
    // Calculate how much we can actually read
    u32 remaining = inode->size - offset;
    if (size > remaining) size = remaining;
    if (offset >= inode->size) return 0;
    
    // Read data from blocks
    u32 bytes_read = 0;
    u32 current_offset = offset;
    
    while (bytes_read < size && current_offset < inode->size) {
        u32 block_idx = current_offset / SECTOR_SIZE;
        u32 block_offset = current_offset % SECTOR_SIZE;
        
        if (block_idx >= 32) break; // SimpleFS only supports 32 direct blocks
        
        u32 block_num = inode->blocks[block_idx];
        if (block_num == 0) break;
        
        u8 block_data[SECTOR_SIZE];
        if (!simplefs_read_block(block_num, block_data)) {
            break;
        }
        
        u32 to_copy = SECTOR_SIZE - block_offset;
        if (to_copy > (size - bytes_read)) {
            to_copy = size - bytes_read;
        }
        
        k_memcpy(&buffer[bytes_read], &block_data[block_offset], to_copy);
        bytes_read += to_copy;
        current_offset += to_copy;
    }
    
    return bytes_read;
}

static int simplefs_write(vfs_node_t* node, u32 offset, u32 size, u8* buffer) {
    if (!node || !buffer || !simplefs) return -1;
    
    simplefs_inode_t* inode = simplefs_get_inode(node->inode);
    if (!inode) return -1;
    
    // If we're writing beyond the current file size, we may need to allocate blocks
    u32 required_blocks = (offset + size + SECTOR_SIZE - 1) / SECTOR_SIZE;
    
    // Allocate more blocks if needed
    for (u32 i = inode->block_count; i < required_blocks; i++) {
        if (i >= 32) break; // SimpleFS limit
        u32 new_block = simplefs_get_free_block();
        if (new_block == 0) break; // No more space
        inode->blocks[i] = new_block;
        inode->block_count++;
    }
    
    // Write data to blocks
    u32 bytes_written = 0;
    u32 current_offset = offset;
    
    while (bytes_written < size) {
        u32 block_idx = current_offset / SECTOR_SIZE;
        u32 block_offset = current_offset % SECTOR_SIZE;
        
        if (block_idx >= inode->block_count) break;
        
        u32 block_num = inode->blocks[block_idx];
        if (block_num == 0) break;
        
        u8 block_data[SECTOR_SIZE];
        if (current_offset < inode->size) {
            simplefs_read_block(block_num, block_data);
        } else {
            k_memset(block_data, 0, SECTOR_SIZE);
        }
        
        u32 to_write = SECTOR_SIZE - block_offset;
        if (to_write > (size - bytes_written)) {
            to_write = size - bytes_written;
        }
        
        k_memcpy(&block_data[block_offset], &buffer[bytes_written], to_write);
        simplefs_write_block(block_num, block_data);
        
        bytes_written += to_write;
        current_offset += to_write;
    }
    
    // Update file size if necessary
    if (offset + bytes_written > inode->size) {
        inode->size = offset + bytes_written;
    }
    
    // Write the updated inode back
    simplefs_write_inode(node->inode, inode);
    
    return bytes_written;
}

static vfs_node_t* simplefs_finddir(vfs_node_t* node, char* name) {
    if (!node || !name || node->type != FILE_TYPE_DIRECTORY) return NULL;
    
    simplefs_inode_t* inode = simplefs_get_inode(node->inode);
    if (!inode) return NULL;
    
    // Read directory entries
    for (u32 i = 0; i < inode->block_count && i < 32; i++) {
        u8 block_data[SECTOR_SIZE];
        if (!simplefs_read_block(inode->blocks[i], block_data)) {
            continue;
        }
        
        simplefs_direntry_t* entries = (simplefs_direntry_t*)block_data;
        u32 entries_per_block = SECTOR_SIZE / sizeof(simplefs_direntry_t);
        
        for (u32 j = 0; j < entries_per_block; j++) {
            if (entries[j].inode != 0 && strings_equal(entries[j].name, name)) {
                // Create a VFS node for this entry
                vfs_node_t* result = vfs_create_node(entries[j].name, FILE_TYPE_REGULAR);
                if (result) {
                    result->inode = entries[j].inode;
                    // Read the actual inode to get file info
                    simplefs_inode_t* file_inode = simplefs_get_inode(result->inode);
                    if (file_inode) {
                        result->size = file_inode->size;
                        result->type = (file_inode->type == SIMPLEFS_TYPE_DIRECTORY) ? 
                                      FILE_TYPE_DIRECTORY : FILE_TYPE_REGULAR;
                    }
                }
                return result;
            }
        }
    }
    
    return NULL;
}

static vfs_node_t* simplefs_readdir(vfs_node_t* node, u32 index) {
    if (!node || node->type != FILE_TYPE_DIRECTORY) return NULL;
    
    simplefs_inode_t* inode = simplefs_get_inode(node->inode);
    if (!inode) return NULL;
    
    u32 current_idx = 0;
    
    // Read directory entries
    for (u32 i = 0; i < inode->block_count && i < 32; i++) {
        u8 block_data[SECTOR_SIZE];
        if (!simplefs_read_block(inode->blocks[i], block_data)) {
            continue;
        }
        
        simplefs_direntry_t* entries = (simplefs_direntry_t*)block_data;
        u32 entries_per_block = SECTOR_SIZE / sizeof(simplefs_direntry_t);
        
        for (u32 j = 0; j < entries_per_block; j++) {
            if (entries[j].inode != 0) {
                if (current_idx == index) {
                    // Create a VFS node for this entry
                    vfs_node_t* result = vfs_create_node(entries[j].name, FILE_TYPE_REGULAR);
                    if (result) {
                        result->inode = entries[j].inode;
                        // Read the actual inode to get file info
                        simplefs_inode_t* file_inode = simplefs_get_inode(result->inode);
                        if (file_inode) {
                            result->size = file_inode->size;
                            result->type = (file_inode->type == SIMPLEFS_TYPE_DIRECTORY) ? 
                                          FILE_TYPE_DIRECTORY : FILE_TYPE_REGULAR;
                        }
                    }
                    return result;
                }
                current_idx++;
            }
        }
    }
    
    return NULL;
}

static int simplefs_create(vfs_node_t* parent, char* name, file_type_t type) {
    if (!parent || !name) return -1;
    
    // Find a free inode
    // This is a simplified implementation - in a real system, you'd have an inode bitmap
    u32 new_inode_num = parent->inode + 100; // Simple allocation strategy
    
    // Create new inode
    simplefs_inode_t new_inode;
    k_memset(&new_inode, 0, sizeof(new_inode));
    new_inode.type = (type == FILE_TYPE_DIRECTORY) ? SIMPLEFS_TYPE_DIRECTORY : SIMPLEFS_TYPE_REGULAR;
    new_inode.permissions = (type == FILE_TYPE_DIRECTORY) ? 0755 : 0644;
    new_inode.uid = 0;
    new_inode.gid = 0;
    
    // Write the new inode
    simplefs_write_inode(new_inode_num, &new_inode);
    
    // Add entry to parent directory
    simplefs_inode_t* parent_inode = simplefs_get_inode(parent->inode);
    if (!parent_inode) return -1;
    
    // Find space in parent directory for the new entry
    for (u32 i = 0; i < parent_inode->block_count && i < 32; i++) {
        u8 block_data[SECTOR_SIZE];
        if (!simplefs_read_block(parent_inode->blocks[i], block_data)) {
            continue;
        }
        
        simplefs_direntry_t* entries = (simplefs_direntry_t*)block_data;
        u32 entries_per_block = SECTOR_SIZE / sizeof(simplefs_direntry_t);
        
        for (u32 j = 0; j < entries_per_block; j++) {
            if (entries[j].inode == 0) {
                // Found free entry
                entries[j].inode = new_inode_num;
                copy_string(entries[j].name, name, sizeof(entries[j].name));
                simplefs_write_block(parent_inode->blocks[i], block_data);
                return 0;
            }
        }
    }
    
    // If no free entry found, allocate a new block for the directory
    u32 new_block = simplefs_get_free_block();
    if (new_block == 0) return -1;
    
    u8 new_block_data[SECTOR_SIZE];
    k_memset(new_block_data, 0, SECTOR_SIZE);
    simplefs_direntry_t* entries = (simplefs_direntry_t*)new_block_data;
    entries[0].inode = new_inode_num;
    copy_string(entries[0].name, name, sizeof(entries[0].name));
    simplefs_write_block(new_block, new_block_data);
    
    // Update parent inode
    parent_inode->blocks[parent_inode->block_count] = new_block;
    parent_inode->block_count++;
    simplefs_write_inode(parent->inode, parent_inode);
    
    return 0;
}

static int simplefs_unlink(vfs_node_t* node) {
    if (!node) return -1;
    
    // In a real implementation, this would:
    // 1. Remove the directory entry from parent
    // 2. Free all data blocks
    // 3. Mark the inode as free
    // For now, just return success
    return 0;
}

// File system operations structure
static filesystem_t simplefs_ops = {
    .name = "SimpleFS",
    .read = simplefs_read,
    .write = simplefs_write,
    .finddir = simplefs_finddir,
    .readdir = simplefs_readdir,
    .create = simplefs_create,
    .unlink = simplefs_unlink,
};

bool simplefs_init(void) {
    if (simplefs) return true; // Already initialized
    
    // Allocate private data
    simplefs = (simplefs_private_t*)kmalloc(sizeof(simplefs_private_t));
    if (!simplefs) {
        vga_print("[simplefs] Failed to allocate memory\n");
        return false;
    }
    
    k_memset(simplefs, 0, sizeof(simplefs_private_t));
    
    // Initialize disk
    if (!disk_init()) {
        kfree(simplefs);
        simplefs = NULL;
        return false;
    }
    
    // Try to read superblock
    simplefs_superblock_t superblock;
    if (!simplefs_read_block(0, &superblock) || superblock.magic != SIMPLEFS_MAGIC) {
        // Format disk with new SimpleFS
        vga_print("[simplefs] Formatting disk...\n");
        
        k_memset(&superblock, 0, sizeof(superblock));
        superblock.magic = SIMPLEFS_MAGIC;
        superblock.block_size = SECTOR_SIZE;
        superblock.total_blocks = DISK_SECTORS;
        superblock.free_blocks = DISK_SECTORS - 2; // Reserve superblock and inode block
        superblock.root_inode = 1;
        copy_string(superblock.name, "SimpleFS", sizeof(superblock.name));
        
        // Write superblock
        if (!simplefs_write_block(0, &superblock)) {
            kfree(simplefs);
            simplefs = NULL;
            return false;
        }
        
        // Initialize root directory
        simplefs_inode_t root_inode;
        k_memset(&root_inode, 0, sizeof(root_inode));
        root_inode.type = SIMPLEFS_TYPE_DIRECTORY;
        root_inode.permissions = 0755;
        
        // Allocate block for root directory
        u32 root_block = simplefs_get_free_block();
        if (root_block == 0) {
            kfree(simplefs);
            simplefs = NULL;
            return false;
        }
        
        root_inode.blocks[0] = root_block;
        root_inode.block_count = 1;
        root_inode.size = sizeof(simplefs_direntry_t); // "." entry
        
        // Write root inode
        simplefs_write_inode(1, &root_inode);
        
        // Create "." entry in root directory
        simplefs_direntry_t dot_entry;
        dot_entry.inode = 1;
        copy_string(dot_entry.name, ".", sizeof(dot_entry.name));
        simplefs_write_block(root_block, &dot_entry);
    }
    
    // Copy superblock to our structure
    simplefs->superblock = superblock;
    simplefs->first_inode_block = 1;  // After superblock
    simplefs->first_data_block = 2;   // After superblock and inode block
    
    vga_print("[simplefs] File system initialized\n");
    
    // Mount the file system
    return vfs_mount(&simplefs_ops, "/");
}