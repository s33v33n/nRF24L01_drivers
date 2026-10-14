#ifndef NRF_NETWORK_USER_CONFIG_H
#define NRF_NETWORK_USER_CONFIG_H

#include <stdint.h>
#include "../network_config.h"

#define NRF_MASTER_ADDRESS 0x892E76AA36ULL

static const uint32_t NRF_NETWORK_KEY[8] = {
    0x92541e9e, 0xa3c22474, 0x1bf3eb1e, 0x73ec748c,
    0x4817d09f, 0x1d208862, 0x3f6b0e5f, 0x755b6beb
};

struct nrf_node_config{
    char *name;             // must be uniqe
    uint64_t tx_address;
    uint64_t rx_pipe_addr;  // must be uniqe
    uint8_t rx_pipe_num;    // only 0 allowed
    uint8_t id;             // must be uniqe and starts from 0. 0 - is only for master
    uint8_t channel;
    uint8_t power;
    uint8_t speed;
};


// ------ My nodes ------
static const struct nrf_node_config nrf_nodes[] = {
    {"master", 0x00, NRF_MASTER_ADDRESS, 0, 0, 1, 1, 1},
    {"nrf_dev_1", 0x00, 0x0000000001ULL, 0, 1, 1, 1, 1}
};

#endif // NRF_NETWORK_USER_CONFIG_H
