#include "ata.h"
#include "screen.h"

void ata_wait_bsy() {
    while(inb(ATA_STATUS) & 0x80);
}

void ata_wait_drq() {
    while(!(inb(ATA_STATUS) & 0x08));
}

void ata_init() {
    // Espera o disco ficar pronto
    ata_wait_bsy();
}

void ata_select_drive(unsigned char slave) {
    outb(ATA_DRIVE, 0xE0 | (slave << 4));
}

int ata_read_sectors(unsigned int lba, unsigned char sectors, void* buffer) {
    unsigned char* buf = (unsigned char*)buffer;
    
    ata_wait_bsy();
    
    // Seleciona o drive e envia os bits mais significativos do LBA
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    
    // Envia número de setores e LBA
    outb(ATA_SECTOR_CNT, sectors);
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Envia comando de leitura
    outb(ATA_COMMAND, ATA_CMD_READ);
    
    // Lê os dados
    for(int i = 0; i < sectors; i++) {
        ata_wait_bsy();
        ata_wait_drq();
        
        // Lê 256 words (512 bytes) por setor
        for(int j = 0; j < 256; j++) {
            unsigned short data = inw(ATA_DATA);
            *buf++ = data & 0xFF;
            *buf++ = (data >> 8) & 0xFF;
        }
    }
    
    return 0;
}

int ata_write_sectors(unsigned int lba, unsigned char sectors, const void* buffer) {
    const unsigned char* buf = (const unsigned char*)buffer;
    
    ata_wait_bsy();
    
    // Seleciona o drive e envia os bits mais significativos do LBA
    outb(ATA_DRIVE, 0xE0 | ((lba >> 24) & 0x0F));
    
    // Envia número de setores e LBA
    outb(ATA_SECTOR_CNT, sectors);
    outb(ATA_LBA_LOW, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HIGH, (lba >> 16) & 0xFF);
    
    // Envia comando de escrita
    outb(ATA_COMMAND, ATA_CMD_WRITE);
    
    // Escreve os dados
    for(int i = 0; i < sectors; i++) {
        ata_wait_bsy();
        ata_wait_drq();
        
        // Escreve 256 words (512 bytes) por setor
        for(int j = 0; j < 256; j++) {
            unsigned short data = *buf++;
            data |= (*buf++) << 8;
            outw(ATA_DATA, data);
        }
    }
    
    return 0;
} 