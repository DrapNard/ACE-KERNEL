#ifndef DRIVERS_STORAGE_DISK_H
#define DRIVERS_STORAGE_DISK_H

#include "ace/types.h"

#define SECTOR_SIZE 512
#define DISK_SIZE (1024 * 1024)  // 1MB disk
#define DISK_SECTORS (DISK_SIZE / SECTOR_SIZE)

bool disk_init(void);
bool disk_read_sector(u32 sector, u8* buffer);
bool disk_write_sector(u32 sector, u8* buffer);
bool disk_read_blocks(u32 start_sector, u32 count, u8* buffer);
bool disk_write_blocks(u32 start_sector, u32 count, u8* buffer);

#endif /* DRIVERS_STORAGE_DISK_H */