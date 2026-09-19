#ifndef NETWORK_CONFIG_H
#define NETWORK_CONFIG_H

#include <stdint.h>

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

    {"Main door", 1, 0xAABBCCDD01ULL, 15, 3, 0},
    {"Garage", 2, 0xAABBCCDD02ULL, 15, 3, 0},
    {"Outdoor lights", 3, 0xAABBCCDD03ULL, 15, 3, 0}
};


#endif // NETWORK_CONFIG_H
