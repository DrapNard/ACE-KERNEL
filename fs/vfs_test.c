#include "../include/vfs.h"
#include "../include/vga.h"

void test_vfs() {
    vga_print("Testing VFS...\n");
    
    vfs_init();
    
    vga_print("Creating directories...\n");
    if (vfs_mkdir("/home") == 0) {
        vga_print("Created /home\n");
    }
    
    if (vfs_mkdir("/home/user") == 0) {
        vga_print("Created /home/user\n");
    }
    
    if (vfs_mkdir("/tmp") == 0) {
        vga_print("Created /tmp\n");
    }
    
    vga_print("Testing file operations...\n");
    
    int fd = vfs_open("/home/user/test.txt", O_CREAT | O_WRONLY);
    if (fd >= 0) {
        vga_print("Created file /home/user/test.txt\n");
        
        char* data = "Hello VFS World";
        int written = vfs_write(fd, data, 15);
        if (written > 0) {
            vga_print("Written data to file\n");
        }
        
        vfs_close(fd);
        vga_print("Closed file\n");
    }
    
    fd = vfs_open("/home/user/test.txt", O_RDONLY);
    if (fd >= 0) {
        vga_print("Opened file for reading\n");
        
        char buffer[32];
        int read_bytes = vfs_read(fd, buffer, 15);
        if (read_bytes > 0) {
            vga_print("Read data from file\n");
        }
        
        vfs_close(fd);
        vga_print("Closed file\n");
    }
    
    vga_print("Testing file stats...\n");
    vfs_node_t stat_buf;
    if (vfs_stat("/home/user/test.txt", &stat_buf) == 0) {
        vga_print("File stats retrieved\n");
        vga_print("File type: ");
        if (stat_buf.type == FILE_TYPE_REGULAR) {
            vga_print("Regular file\n");
        }
    }
    
    if (vfs_stat("/home", &stat_buf) == 0) {
        vga_print("Directory stats retrieved\n");
        vga_print("File type: ");
        if (stat_buf.type == FILE_TYPE_DIRECTORY) {
            vga_print("Directory\n");
        }
    }
    
    vga_print("Testing path resolution...\n");
    vfs_node_t* root = vfs_get_root();
    if (root) {
        vga_print("Root directory found\n");
    }
    
    vfs_node_t* home = vfs_resolve_path("/home");
    if (home) {
        vga_print("Resolved /home\n");
    }
    
    vfs_node_t* user_dir = vfs_resolve_path("/home/user");
    if (user_dir) {
        vga_print("Resolved /home/user\n");
    }
    
    vfs_node_t* test_file = vfs_resolve_path("/home/user/test.txt");
    if (test_file) {
        vga_print("Resolved /home/user/test.txt\n");
    }
    
    vga_print("VFS test completed\n");
}