#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <stdint.h>

// ================ SPI0 ================
#define RADIO_SPI0_ENABLE   1
#define DEV_NAME_SPI0       "nrf_garden" // YOUR_MODE_NAME

/* ---- SPI0 Hardware connections ---- */
#define SPI0_MISO_PIN       9
#define SPI0_MOSI_PIN       10
#define SPI0_SCLK_PIN       11

/* ---- CHIP SELECT (Choose one) ---- */
#define CS_PIN_SPI0       8   // CS0 (GPIO 8)
// #define CS_PIN_SPI0    7   // CS1 (GPIO 7)

/* ---- Any GPIO pin ---- */
#define CE_PIN_SPI0         24
#define IRQ_PIN_SPI0        23

/* ---- SPI Speed ---- */
#define SPI0_SPEED          8000000  //  8 MHz
// ================ SPI0 ================

// ================ SPI1 ================
#define RADIO_SPI1_ENABLE   1
#define DEV_NAME_SPI1       "nrf_garage" // YOUR_MODE_NAME

/* ---- SPI1 Hardware connections ---- */
#define SPI1_MISO_PIN       19
#define SPI1_MOSI_PIN       20
#define SPI1_SCLK_PIN       21

/* ---- CHIP SELECT (Choose one) ---- */
#define CS_PIN_SPI1       18   // CS0 (GPIO 18)
// #define CS_PIN_SPI1    17   // CS1 (GPIO 17)
// #define CS_PIN_SPI1    16   // CS1 (GPIO 16)

/* ---- Any GPIO pin ---- */
#define CE_PIN_SPI1         5
#define IRQ_PIN_SPI1        6

/* ---- SPI Speed ---- */
#define SPI1_SPEED          8000000  //  8 MHz
// ================ SPI1 ================

#define NRF_MASTER_ADDRESS 0x1122334455ULL

static const uint32_t NRF_NETWORK_KEY[8] = {
    0x04030201, 0x08070605, 0x0C0B0A09, 0x100F0E0D, 
    0x14131211, 0x18171615, 0x1C1B1A19, 0x201F1E1D
};

struct nrf_node_config{
    char *name;
    uint8_t id;
    uint64_t address;
    uint8_t channel;
    uint8_t power;
    uint8_t speed;
};


// ------ My nodes ------
static const struct nrf_node_config nrf_nodes[] = {

    {DEV_NAME_SPI0, 1, 0xAABBCCDD01ULL, 15, 3, 0},
    {DEV_NAME_SPI1, 2, 0xAABBCCDD02ULL, 15, 3, 0}
};


#endif // NETWORK_CONFIG_H
