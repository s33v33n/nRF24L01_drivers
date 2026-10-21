#ifndef USER_CONFIG_H
#define USER_CONFIG_H

#include <stdint.h>
#include "hardware_config.h"

#define NRF_MASTER_ADDRESS 0x1122334455ULL

static const uint32_t NRF_NETWORK_KEY[8] = {
    0x04030201, 0x08070605, 0x0C0B0A09, 0x100F0E0D, 
    0x14131211, 0x18171615, 0x1C1B1A19, 0x201F1E1D
};

struct nrf_node_config{
    char *name;             // must be uniqe
    uint64_t tx_address;    // master address
    uint64_t rx_pipe_addr;  // must be uniqe
    uint8_t rx_pipe_num;    // only 0 allowed
    uint8_t id;             // must be uniqe
    uint8_t channel;
    uint8_t power;
    uint8_t speed;
};


// ------ My nodes ------
static const struct nrf_node_config nrf_nodes[] = {

    {DEV_NAME_SPI0, 0x00, 0xAABBCCDD01ULL, 0, 0, 15, 3, 1},
    {DEV_NAME_SPI1, 0x00, 0xAABBCCDD02ULL, 0, 1, 15, 3, 1}
};

#endif // USER_CONFIG_H