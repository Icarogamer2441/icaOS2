#ifndef ATA_H
#define ATA_H

// Portas de I/O do disco primário
#define ATA_DATA        0x1F0
#define ATA_FEATURES    0x1F1
#define ATA_SECTOR_CNT  0x1F2
#define ATA_LBA_LOW     0x1F3
#define ATA_LBA_MID     0x1F4
#define ATA_LBA_HIGH    0x1F5
#define ATA_DRIVE       0x1F6
#define ATA_COMMAND     0x1F7
#define ATA_STATUS      0x1F7

// Comandos ATA
#define ATA_CMD_READ  0x20
#define ATA_CMD_WRITE 0x30

// Funções do driver
void ata_init();
int ata_read_sectors(unsigned int lba, unsigned char sectors, void* buffer);
int ata_write_sectors(unsigned int lba, unsigned char sectors, const void* buffer);

#endif 