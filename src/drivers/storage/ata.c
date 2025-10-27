#include "drivers/storage/ata.h"
#include "arch/x86/io.h"
#include "drivers/vga.h"

#define ATA_PRIMARY_CMD_BASE    0x1F0
#define ATA_PRIMARY_CTL_BASE    0x3F6
#define ATA_SECONDARY_CMD_BASE  0x170
#define ATA_SECONDARY_CTL_BASE  0x376

#define ATA_REG_DATA        0x00
#define ATA_REG_ERROR       0x01
#define ATA_REG_FEATURES    0x01
#define ATA_REG_SECCOUNT0   0x02
#define ATA_REG_LBA0        0x03
#define ATA_REG_LBA1        0x04
#define ATA_REG_LBA2        0x05
#define ATA_REG_HDDEVSEL    0x06
#define ATA_REG_COMMAND     0x07
#define ATA_REG_STATUS      0x07
#define ATA_REG_SECCOUNT1   0x08
#define ATA_REG_LBA3        0x09
#define ATA_REG_LBA4        0x0A
#define ATA_REG_LBA5        0x0B
#define ATA_REG_CONTROL     0x0C
#define ATA_REG_ALTSTATUS   0x0C
#define ATA_REG_DEVADDRESS  0x0D

// ATA Commands
#define ATA_CMD_READ_PIO        0x20
#define ATA_CMD_READ_PIO_EXT    0x24
#define ATA_CMD_READ_DMA        0xC8
#define ATA_CMD_READ_DMA_EXT    0x25
#define ATA_CMD_WRITE_PIO       0x30
#define ATA_CMD_WRITE_PIO_EXT   0x34
#define ATA_CMD_WRITE_DMA       0xCA
#define ATA_CMD_WRITE_DMA_EXT   0x35
#define ATA_CMD_CACHE_FLUSH     0xE7
#define ATA_CMD_CACHE_FLUSH_EXT 0xEA
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY        0xEC

// Status flags
#define ATA_SR_BSY     0x80    // Busy
#define ATA_SR_DRDY    0x40    // Drive ready
#define ATA_SR_DRQ     0x08    // Data request ready
#define ATA_SR_ERR     0x01    // Error

// Device flags
#define ATA_DEV_BUSY 0x80
#define ATA_DEV_DRQ  0x08

// Sector size definition
#define SECTOR_SIZE 512

// Missing functions for 16-bit I/O
static inline void outw(u16 port, u16 val) {
    asm volatile ("outw %0, %1" : : "a"(val), "Nd"(port));
}

static inline u16 inw(u16 port) {
    u16 ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static bool ata_primary_initialized = false;
static bool ata_secondary_initialized = false;

static void ata_wait_bsy(u16 base) {
    while (inb(base + ATA_REG_STATUS) & ATA_SR_BSY);
}

static void ata_wait_drq(u16 base) {
    while (!(inb(base + ATA_REG_STATUS) & ATA_SR_DRQ));
}

static bool ata_wait_status(u16 base, u8 flags, u8 invert) {
    u32 timeout = 1000000; // 1 second timeout
    while (timeout--) {
        u8 status = inb(base + ATA_REG_ALTSTATUS);
        if (invert) {
            if (!(status & flags)) return true;
        } else {
            if ((status & flags)) return true;
        }
        if (status == 0xFF) return false; // No device
    }
    return false;
}

bool ata_init(void) {
    vga_print("[ata] Initializing ATA storage...\n");
    
    // Test primary channel
    outb(ATA_PRIMARY_CTL_BASE, 0x02); // Reset
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    outb(ATA_PRIMARY_CTL_BASE, 0x00); // Un-reset
    
    // Wait for devices to become ready
    if (ata_wait_status(ATA_PRIMARY_CMD_BASE, ATA_SR_BSY, 1)) {
        if (ata_wait_status(ATA_PRIMARY_CMD_BASE, ATA_SR_DRDY, 0)) {
            ata_primary_initialized = true;
            vga_print("[ata] Primary channel initialized\n");
        }
    }
    
    // Test secondary channel
    outb(ATA_SECONDARY_CTL_BASE, 0x02); // Reset
    __asm__ volatile("nop");
    __asm__ volatile("nop");
    outb(ATA_SECONDARY_CTL_BASE, 0x00); // Un-reset
    
    if (ata_wait_status(ATA_SECONDARY_CMD_BASE, ATA_SR_BSY, 1)) {
        if (ata_wait_status(ATA_SECONDARY_CMD_BASE, ATA_SR_DRDY, 0)) {
            ata_secondary_initialized = true;
            vga_print("[ata] Secondary channel initialized\n");
        }
    }
    
    return ata_primary_initialized || ata_secondary_initialized;
}

bool ata_read_sector(u16 base, u32 lba, u8* buffer) {
    if (!buffer) return false;
    
    // Wait for not busy
    if (!ata_wait_status(base, ATA_SR_BSY, 1)) return false;
    
    // Select device (master)
    outb(base + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    
    // Wait for DRQ to be ready
    if (!ata_wait_status(base, ATA_SR_DRQ, 1)) return false;
    
    // Set sector count and LBA
    outb(base + ATA_REG_SECCOUNT0, 1);
    outb(base + ATA_REG_LBA0, (u8)lba);
    outb(base + ATA_REG_LBA1, (u8)(lba >> 8));
    outb(base + ATA_REG_LBA2, (u8)(lba >> 16));
    
    // Send read command
    outb(base + ATA_REG_COMMAND, ATA_CMD_READ_PIO);
    
    // Wait for DRQ
    if (!ata_wait_status(base, ATA_SR_DRQ, 0)) return false;
    
    // Read 256 words (512 bytes)
    for (int i = 0; i < 256; i++) {
        u16 data = inw(base + ATA_REG_DATA);
        buffer[i * 2] = data & 0xFF;
        buffer[i * 2 + 1] = (data >> 8) & 0xFF;
    }
    
    // Wait for operation to complete
    if (!ata_wait_status(base, ATA_SR_BSY, 1)) return false;
    
    return true;
}

bool ata_write_sector(u16 base, u32 lba, u8* buffer) {
    if (!buffer) return false;
    
    // Wait for not busy
    if (!ata_wait_status(base, ATA_SR_BSY, 1)) return false;
    
    // Select device (master)
    outb(base + ATA_REG_HDDEVSEL, 0xE0 | ((lba >> 24) & 0x0F));
    
    // Wait for DRQ to be ready
    if (!ata_wait_status(base, ATA_SR_DRQ, 1)) return false;
    
    // Set sector count and LBA
    outb(base + ATA_REG_SECCOUNT0, 1);
    outb(base + ATA_REG_LBA0, (u8)lba);
    outb(base + ATA_REG_LBA1, (u8)(lba >> 8));
    outb(base + ATA_REG_LBA2, (u8)(lba >> 16));
    
    // Send write command
    outb(base + ATA_REG_COMMAND, ATA_CMD_WRITE_PIO);
    
    // Wait for DRQ
    if (!ata_wait_status(base, ATA_SR_DRQ, 0)) return false;
    
    // Write 256 words (512 bytes)
    for (int i = 0; i < 256; i++) {
        u16 data = buffer[i * 2] | (buffer[i * 2 + 1] << 8);
        outw(base + ATA_REG_DATA, data);
    }
    
    // Wait for operation to complete
    if (!ata_wait_status(base, ATA_SR_BSY, 1)) return false;
    
    // Flush cache
    outb(base + ATA_REG_COMMAND, ATA_CMD_CACHE_FLUSH);
    if (!ata_wait_status(base, ATA_SR_BSY, 1)) return false;
    
    return true;
}

// Use primary channel by default
bool disk_init(void) {
    if (ata_init()) {
        return true;
    }
    return false;
}

bool disk_read_sector(u32 sector, u8* buffer) {
    if (ata_primary_initialized) {
        return ata_read_sector(ATA_PRIMARY_CMD_BASE, sector, buffer);
    }
    return false;
}

bool disk_write_sector(u32 sector, u8* buffer) {
    if (ata_primary_initialized) {
        return ata_write_sector(ATA_PRIMARY_CMD_BASE, sector, buffer);
    }
    return false;
}

bool disk_read_blocks(u32 start_sector, u32 count, u8* buffer) {
    for (u32 i = 0; i < count; i++) {
        if (!disk_read_sector(start_sector + i, &buffer[i * SECTOR_SIZE])) {
            return false;
        }
    }
    return true;
}

bool disk_write_blocks(u32 start_sector, u32 count, u8* buffer) {
    for (u32 i = 0; i < count; i++) {
        if (!disk_write_sector(start_sector + i, &buffer[i * SECTOR_SIZE])) {
            return false;
        }
    }
    return true;
}