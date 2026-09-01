#ifndef NRF24L01_H
#define NRF24L01_H

#include <linux/cdev.h>

struct nrf24l01_dev
{
    struct spi_device *spi;
    struct gpio_desc *ce_gpio;
    int irq;

    struct cdev cdev;
    int minor;
};

// --- NRF24L01 SPI COMMANDS ---
#define NRF_CMD_R_REGISTER    0x00 // read command 000A AAAA, AAAAA = 5 bit register map 
#define NRF_CMD_W_REGISTER    0x20 // write command 001A AAAA, AAAAA = 5 bit register map 
#define NRF_CMD_R_RX_PAYLOAD  0x61 // read RX FIFO payload
#define NRF_CMD_W_TX_PAYLOAD  0xA0 // write TX FIFO payload
#define NRF_CMD_FLUSH_TX      0xE1 // flush TX FIFO 
#define NRF_CMD_FLUSH_RX      0xE2 // flush RX FIFO 

// --- NRF24L01 REGISTERS ---
#define NRF_REG_CONFIG        0x00 // Config (Power, RX/TX, CRC)
#define NRF_REG_EN_AA         0x01 // Enable Auto Acknowledgment
#define NRF_REG_EN_RXADDR     0x02 // Enable RX addresses (pipes)
#define NRF_REG_SETUP_AW      0x03 // Address width 
#define NRF_REG_SETUP_RETR    0x04 // Auto retransmition 
#define NRF_REG_RF_CH         0x05 // RF channel 
#define NRF_REG_RF_SETUP      0x06 // RF setup (Power, speed)
#define NRF_REG_STATUS        0x07 // Status register
#define NRF_REG_RX_PW_P0      0x11 // Payload width for pipe 0
#define NRF_REG_FIFO_STATUS   0x17 // FIFO status 


#endif // NRF24L01_H
