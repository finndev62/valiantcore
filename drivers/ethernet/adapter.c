#include "../../include/network.h"

static uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t addr = (uint32_t)((bus << 16) | (slot << 11) |
                   (func << 8) | (offset & 0xFC) | 0x80000000);
    outl(PCI_CONFIG_ADDR, addr);
    return inl(PCI_CONFIG_DATA);
}

static addr_t rtl_find_base() {
    for (uint8_t bus = 0; bus < 255; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id = pci_read(bus, slot, 0, 0);

            if (id == 0x813910EC)
                return (addr_t)pci_read(bus, slot, 0, 0x10) & ~0xF;
        }
    }
    return 0;
}

static addr_t base = 0;

void net_init() {
    base = rtl_find_base();
    if (!base) {
        kprint("NET: RTL8139 bulunamadi!\n");
        return;
    }

    outb(base + RTL_CONFIG1, 0x00);

    outb(base + RTL_CMD, RTL_CMD_RESET);
    while (inb(base + RTL_CMD) & RTL_CMD_RESET);

    outb(base + RTL_CMD, RTL_CMD_TX_EN | RTL_CMD_RX_EN);

    kprint("NET: RTL8139 hazir!\n");
}

void net_send_packet(net_packet_t *packet) {
    if (!base) return;

    if (!rust_validate_packet(packet)) {
        kprint("NET: Paket reddedildi!\n");
        return;
    }

    rust_encrypt_payload(packet);

    outl(base + 0x20, (uint32_t)(addr_t)packet->data);
    outl(base + 0x10, packet->length);
}

int net_receive_packet(net_packet_t *packet) {
    if (!base) return -1;

    uint16_t isr = inw(base + RTL_ISR);
    if (!(isr & 0x01)) return 0;  

    outw(base + RTL_ISR, 0x01);

    return 1;
}
