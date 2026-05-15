#include <io.h>
#include <stdint.h>

#ifdef __x86_64__
    typedef uint64_t addr_t;
#else
    typedef uint32_t addr_t;
#endif

#define ATA_DATA      0x1F0
#define ATA_ERROR     0x1F1
#define ATA_SECCOUNT  0x1F2
#define ATA_LBA_LOW   0x1F3
#define ATA_LBA_MID   0x1F4
#define ATA_LBA_HIGH  0x1F5
#define ATA_DRIVE_SEL 0x1F6
#define ATA_COMMAND   0x1F7
#define ATA_CONTROL   0x3F6

#define CMD_READ_PIO_EXT  0x24
#define CMD_WRITE_PIO_EXT 0x34
#define CMD_CACHE_FLUSH   0xE7
#define STATUS_BSY        0x80
#define STATUS_DRQ        0x08
#define STATUS_DF         0x20
#define STATUS_ERR        0x01

static void ata_io_wait() {
    inb(ATA_CONTROL);
    inb(ATA_CONTROL);
    inb(ATA_CONTROL);
    inb(ATA_CONTROL);
}

int ata_status_wait(int bit, addr_t timeout) {
    for (addr_t i = 0; i < timeout; i++) {
        uint8_t status = inb(ATA_COMMAND);
        if (bit && (status & bit))  return 1;
        if (!bit && !(status & STATUS_BSY)) return 1;
        ata_io_wait();
    }
    return 0;
}

int ata_write_250gb(uint64_t lba, uint16_t count, uint16_t *buffer) {

    outb(ATA_DRIVE_SEL, 0x40);
    ata_io_wait();

    outb(ATA_SECCOUNT, (uint8_t)(count >> 8));
    outb(ATA_LBA_LOW,  (uint8_t)(lba >> 24));
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 32));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 40));

    outb(ATA_SECCOUNT, (uint8_t)count);
    outb(ATA_LBA_LOW,  (uint8_t)lba);
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outb(ATA_COMMAND, CMD_WRITE_PIO_EXT);

    for (uint16_t s = 0; s < count; s++) {
        if (!ata_status_wait(STATUS_DRQ, 10000))
            return -1;

        for (int i = 0; i < 256; i++) {
            outw(ATA_DATA, buffer[s * 256 + i]);
        }
    }

    outb(ATA_COMMAND, CMD_CACHE_FLUSH);
    ata_status_wait(0, 100000);
    return 0;
}

int ata_read_250gb(uint64_t lba, uint16_t count, uint16_t *buffer) {

    outb(ATA_DRIVE_SEL, 0x40);
    ata_io_wait();

    outb(ATA_SECCOUNT, (uint8_t)(count >> 8));
    outb(ATA_LBA_LOW,  (uint8_t)(lba >> 24));
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 32));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 40));

    outb(ATA_SECCOUNT, (uint8_t)count);
    outb(ATA_LBA_LOW,  (uint8_t)lba);
    outb(ATA_LBA_MID,  (uint8_t)(lba >> 8));
    outb(ATA_LBA_HIGH, (uint8_t)(lba >> 16));

    outb(ATA_COMMAND, CMD_READ_PIO_EXT);

    for (uint16_t s = 0; s < count; s++) {
        if (!ata_status_wait(STATUS_DRQ, 10000))
            return -1;

        for (int i = 0; i < 256; i++) {
            buffer[s * 256 + i] = inw(ATA_DATA);
        }
    }

    return 0;
}
