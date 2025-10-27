#ifndef DRIVERS_STORAGE_ATA_H
#define DRIVERS_STORAGE_ATA_H

#include "ace/types.h"

bool ata_init(void);
bool ata_read_sector(u16 base, u32 lba, u8* buffer);
bool ata_write_sector(u16 base, u32 lba, u8* buffer);

// Disk interface (for SimpleFS compatibility)
bool disk_init(void);
bool disk_read_sector(u32 sector, u8* buffer);
bool disk_write_sector(u32 sector, u8* buffer);
bool disk_read_blocks(u32 start_sector, u32 count, u8* buffer);
bool disk_write_blocks(u32 start_sector, u32 count, u8* buffer);

#endif /* DRIVERS_STORAGE_ATA_H */