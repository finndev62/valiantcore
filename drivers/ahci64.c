/*
* ValiantCore Kernel
* Copyright (C) 2026 bigpower
* SPDX-License-Identifier: GPL-2.0-only
* By Finn Dev
*/



#include <asm/kernel.h>
#include <stdint.h>

#define AHCI_BASE 0x10000000UL

#define HBA_CAP   0x00
#define HBA_GHC   0x04
#define HBA_IS    0x08
#define HBA_PI    0x0C
#define HBA_VS    0x10


#define HBA_GHC_AE     (1 << 31)
#define HBA_GHC_IE     (1 << 1)
#define HBA_GHC_HR     (1 << 0)


#define HBA_PORT_BASE  0x100
#define HBA_PORT_SIZE  0x80


#define PORT_CLB       0x00
#define PORT_CLBU      0x04
#define PORT_FB        0x08
#define PORT_FBU       0x0C
#define PORT_IS        0x10
#define PORT_IE        0x14
#define PORT_CMD       0x18
#define PORT_TFD       0x20
#define PORT_SIG       0x24
#define PORT_SSTS      0x28
#define PORT_SCTL      0x2C
#define PORT_SERR      0x30
#define PORT_SACT      0x34
#define PORT_CI        0x38

#define PORT_CMD_ST    (1 << 0)
#define PORT_CMD_FRE   (1 << 4)
#define PORT_CMD_FR    (1 << 14)
#define PORT_CMD_CR    (1 << 15)

#define ATA_CMD_READ_DMA_EX  0x25
#define ATA_CMD_WRITE_DMA_EX 0x35
#define ATA_CMD_IDENTIFY     0xEC

#define FIS_TYPE_REG_H2D   0x27

#define AHCI_SECTOR_SZ      512

#define AHCI_MAX_PORTS      32
/* ValiantCore AHCİ Aarch64 definitions End Of Line */

typedef struct __attribute__((packed)) {
    uint8_t cfl      : 5;
    uint8_t a        : 1;
    uint8_t w        : 1;
    uint8_t p        : 1;
    uint8_t r        : 1;
    uint8_t b        : 1;
    uint8_t c        : 1;
    uint8_t  rsvd0   : 1;
    uint8_t  pmp     : 4;
    uint8_t  prdtl;
    uint8_t  prdbc;
    uint32_t ctba;
    uint32_t ctbau;
    uint32_t rsvd1[4];
} ahci_cmd_header_t;


typedef struct __attribute__((packed)) {
    uint8_t fis_type;
    uint8_t pmport  : 4;
    uint8_t rvd0    : 3;
    uint8_t c       : 1;
    uint8_t command;
    uint8_t featurel;

    uint8_t lba0;
    uint8_t lba1;
    uint8_t lba2;
    uint8_t device;

    uint8_t lba3;
    uint8_t lba4;
    uint8_t lba5;
    uint8_t featureh;

    uint8_t countl;
    uint8_t counth;
    uint8_t icc;
    uint8_t control;

    uint8_t  rsvd1[4];
} ahci_fis_h2d_t;


typedef struct __attribute__ ((packed)) {
    uint32_t dba;
    uint32_t dbau;
    uint32_t rsvd;
    uint32_t dbc   : 22;
    uint32_t rsvd2 :  9;
    uint32_t i     : 1;
} ahci_prdt_entry_t;

/**************
* By Finn Dev
****************/
typedef struct __attribute__ ((packed)) {
    uint8_t           cfis[64];
    uint8_t           acmd[16];
    uint8_t           rsvd[48];
    ahci_prdt_entry_t prdt[1];
} ahci_cmd_table_t;

/* Ahci Aort Situation */
typedef struct {
    int       active;
    uint32_t  port_num;

} ahci_port_t;

/* Ahci Driver Situation */
static struct {
    addr_t       base;
    ahci_port_t  ports[AHCI_MAX_PORTS];
    uint32_t     port_count;
    int          initialized;
} ahci;


static ahci_cmd_header_t ahci_cmd_list[32] __attribute__((aligned(1024)));
static ahci_cmd_table_t  ahci_cmd_table    __attribute__((aligned(128)));
static uint8_t           ahci_fis_buf[256] __attribute__ ((aligned(256)));

/* ====== */
static inline uint32_t ahci_read32(addr_t addr) {
     return *(volatile uint32_t *)addr;

}

static inline void ahci_write32(addr_t addr, uint32_t val){
     *(volatile uint32_t *)addr = val;
}

static inline addr_t ahci_port_addr(uint32_t port, uint32_t offset) {
     return ahci.base + HBA_PORT_BASE + (port * HBA_PORT_SIZE) + offset;
}

/*==========================================*/

static void ahci_port_stop(uint32_t port) {
    addr_t cmd_reg = ahci_port_addr(port, PORT_CMD);
    uint32_t cmd   = ahci_read32(cmd_reg);


    cmd &= ~PORT_CMD_ST;
    ahci_write32(cmd_reg, cmd);
    for (int i = 0; i < 500; i++) {
        if (!(ahci_read32(cmd_reg) & PORT_CMD_CR)) break;
        asm volatile ("nop");
    }


    cmd  &= ~PORT_CMD_FRE;
    ahci_write32(cmd_reg, cmd);

    for (int i = 0; i < 500; i++) {
        if (!(ahci_read32(cmd_reg) & PORT_CMD_FR)) break;
        asm volatile ("nop");
    }
}


static void ahci_port_start(uint32_t port) {
    addr_t cmd_reg = ahci_port_addr(port, PORT_CMD);

    while (ahci_read32(cmd_reg) & PORT_CMD_CR)
        asm volatile ("nop");

    uint32_t cmd = ahci_read32(cmd_reg);
    cmd |= PORT_CMD_FRE;
    cmd |= PORT_CMD_ST;
    ahci_write32(cmd_reg, cmd);
}

static void ahci_port_init(uint32_t port) {
    ahci_port_stop(port);

    addr_t clb = (addr_t)ahci_cmd_list;
    addr_t fb  = (addr_t)ahci_fis_buf;

    ahci_write32(ahci_port_addr(port, PORT_CLB),  (uint32_t)(clb & 0xFFFFFFFF));
    ahci_write32(ahci_port_addr(port, PORT_CLBU), (uint32_t)(clb >> 32));
    ahci_write32(ahci_port_addr(port, PORT_FB),   (uint32_t)(fb & 0xFFFFFFFF));
    ahci_write32(ahci_port_addr(port, PORT_FBU),  (uint32_t)(fb >> 32));

    ahci_write32(ahci_port_addr(port, PORT_SERR), 0xFFFFFFFF);
    ahci_write32(ahci_port_addr(port, PORT_IS),   0xFFFFFFFF);

    ahci_port_start(port);
}

/* =============================================*/
static int ahci_send_cmd(uint32_t port, uint64_t lba, uint16_t count,
                          void *buf, int write) {
    if (!buf || count == 0) return -1;

    ahci_cmd_header_t *hdr = &ahci_cmd_list[0];
    __builtin_memset(hdr, 0, sizeof(ahci_cmd_header_t));

    hdr->cfl    = sizeof(ahci_fis_h2d_t) / 4;
    hdr->w      = write ? 1 : 0;
    hdr->prdtl  = 1;

    addr_t ctba = (addr_t)&ahci_cmd_table;
    hdr->ctba   = (uint32_t)(ctba & 0xFFFFFFFF);
    hdr->ctbau  = (uint32_t)(ctba >> 32);



    __builtin_memset(&ahci_cmd_table, 0, sizeof(ahci_cmd_table_t));


    ahci_fis_h2d_t *fis = (ahci_fis_h2d_t *)ahci_cmd_table.cfis;
    fis->fis_type = FIS_TYPE_REG_H2D;
    fis->c        = 1;
    fis->command  = write ? ATA_CMD_WRITE_DMA_EX : ATA_CMD_READ_DMA_EX;

    fis->lba0     = (uint8_t)(lba & 0xFF);
    fis->lba1     = (uint8_t)((lba >> 8) & 0xFF);
    fis->lba2     = (uint8_t)((lba >> 16) & 0xFF);
    fis->device   = 0x40;   /* LBA MODE */
    fis->lba3     = (uint8_t)((lba >> 24) & 0xFF);
    fis->lba4     = (uint8_t)((lba >> 32) & 0xFF);
    fis->lba5     = (uint8_t)((lba >> 40) & 0xFF);


    fis->countl   = (uint8_t)(count & 0xFF);
    fis->counth   = (uint8_t)((count >> 8) & 0xFF);


    addr_t buf_addr = (addr_t)buf;
    ahci_cmd_table.prdt[0].dba  = (uint32_t)(buf_addr & 0xFFFFFFFF);
    ahci_cmd_table.prdt[0].dbau = (uint32_t)(buf_addr >> 32);
    ahci_cmd_table.prdt[0].dbc  = (count * AHCI_SECTOR_SZ) - 1;
    ahci_cmd_table.prdt[0].i    = 1;


    addr_t tfd_reg = ahci_port_addr(port, PORT_TFD);
    for (int i = 0; i < 1000; i++) {
        uint32_t tfd = ahci_read32(tfd_reg);
        if (!(tfd & 0x88)) break;
        asm volatile ("nop");
    }

    ahci_write32(ahci_port_addr(port, PORT_CI), 1);

    for (int i = 0; i < 100000; i++) {
        if (!(ahci_read32(ahci_port_addr(port, PORT_CI)) & 1)) break;
        if (ahci_read32(ahci_port_addr(port, PORT_IS)) & (1 << 30)) {
            uart_print("[AHCI] Task file error\n");
            return -1;
        }
        asm volatile ("nop");
      }

      return 0;
}


/* AHCI INIT */
int ahci_init(void) {
    ahci.base        = AHCI_BASE;
    ahci.port_count  = 0;
    ahci.initialized = 0;


    ahci_write32(ahci.base + HBA_GHC, HBA_GHC_AE);


    uint32_t pi = ahci_read32(ahci.base + HBA_PI);


    for (uint32_t i = 0; i < AHCI_MAX_PORTS; i++) {
        if (!(pi & (1 << i))) continue;


        uint32_t ssts = ahci_read32(ahci_port_addr(i, PORT_SSTS));
        uint8_t det   = ssts & 0x0F;
        uint8_t ipm   = (ssts >> 8) & 0x0F;


        if (det != 3 || ipm != 1) continue;


        ahci_port_init(i);
        ahci.ports[ahci.port_count].active  = 1;
        ahci.ports[ahci.port_count].port_num = i;
        ahci.port_count++;

        uart_print("[AHCI] Port found and initialized\n");
     }

     if (ahci.port_count == 0) {
        uart_print("[AHCI] No drives found\n");
        return -1;
     }

     ahci.initialized = 1;
     uart_print("[AHCI] Initialized successfully\n");
     return 0;
  }


  /* AHCI READ */
  int ahci_read(uint64_t lba, uint16_t count, void *buf) {
      if (!ahci.initialized) return -1;
      if (!buf || count == 0) return -1;
      if (ahci.port_count == 0) return -1;


      return ahci_send_cmd(ahci.ports[0].port_num, lba, count, buf, 0);
 }

 /* AHCI WRITE */
 int ahci_write(uint64_t lba, uint16_t count, const void *buf) {
    if (!ahci.initialized) return -1;
    if (!buf || count == 0) return -1;
    if (ahci.port_count == 0) return -1;

    return ahci_send_cmd(ahci.ports[0].port_num, lba, count, (void*)buf, 1);
 }

 /* The End */
  

  
    
   

