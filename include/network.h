#ifndef NETWORK_H
#define NETWORK_H

#include <stdint.h>
#include <kernel.h>

typedef struct {
    uint8_t dest_mac[6];
    uint8_t source_mac[6];
    uint16_t ethertype;
    uint8_t data[1500];
    uint32_t length;
} __attribute__((packed)) net_packet_t;

#define PCI_CONFIG_ADDR 0xCF8
#define PCI_CONFIG_DATA 0xCFC

#define  RTL_BASE       0x000
#define  RTL_MACO       0x00
#define  RTL_CMD        0x37
#define  RTL_IMR        0xC3
#define  RTL_ISR        0x3E
#define  RTL_TCR        0x40
#define  RTL_RCR        0x44
#define  RTL_CONFIG1    0x52
#define  RTL_CMD_RESET  0x10
#define  RTL_CMD_RX_EN  0x08
#define  RTL_CMD_TX_EN  0x04

void net_init();
void net_send_packet(net_packet_t *packet);
int  net_receive_packet(net_packet_t *packet);

extern int8_t rust_validate_packet(net_packet_t *packet);
extern void   rust_encrypt_payload(net_packet_t *packet);

#endif
