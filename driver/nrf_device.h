#ifndef NRF24L01_H
#define NRF24L01_H

#include <linux/cdev.h>
#include <linux/wait.h> 
#include <linux/mutex.h>

struct nrf24l01_dev
{
    // nrf device 
    struct spi_device *spi;
    struct gpio_desc *ce_gpio;
    int irq;

    // char device 
    struct cdev cdev;
    int minor;

    // reveice data 
    wait_queue_head_t rx_waitqueue;
    bool rx_data_ready;

    // transmit data 
    wait_queue_head_t tx_waitqueue;
    bool tx_done;  
    bool tx_success;  

    // synchronization
    struct mutex priv_mutex;
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

// pipes
#define NRF_REG_RX_ADDR_P0    0x0A // RX address pipe 0
#define NRF_REG_RX_ADDR_P1    0x0B // RX address pipe 1 
#define NRF_REG_RX_ADDR_P2    0x0C // RX address pipe 2
#define NRF_REG_RX_ADDR_P3    0x0D // RX address pipe 3
#define NRF_REG_RX_ADDR_P4    0x0E // RX address pipe 4
#define NRF_REG_RX_ADDR_P5    0x0F // RX address pipe 5
#define NRF_REG_TX_ADDR       0x10 // TX address


#endif // NRF24L01_H
