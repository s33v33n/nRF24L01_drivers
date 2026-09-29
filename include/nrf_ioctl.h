#ifndef NRF_IOCTL_H
#define NRF_IOCTL_H

#include <linux/ioctl.h>

#ifdef __KERNEL__
    #include <linux/types.h> // kernel types
#else
    #include <stdint.h>      // user-space types
#endif

// for pipe addresses
struct nrf_pipe_config {
    uint8_t pipe_num;
    uint64_t address;
};

#define NRF_MAGIC 'N'

// Write 
#define SET_CHANNEL_SEQ_NO  0x01
#define SET_POWER_SEQ_NO    0x02
#define SET_SPEED_SEQ_NO    0x03
#define SET_RX_ADDR_SEQ_NO  0x04
#define SET_TX_ADDR_SEQ_NO  0x05

// Read 
#define GET_RF_CHANNEL      0x06
#define GET_RF_SETUP        0x07
#define GET_STATUS_SEQ_NO   0x08

// Write
#define NRF_IOCTL_SET_CHANNEL       _IOW(NRF_MAGIC, SET_CHANNEL_SEQ_NO, uint8_t)
#define NRF_IOCTL_SET_POWER         _IOW(NRF_MAGIC, SET_POWER_SEQ_NO, uint8_t)
#define NRF_IOCTL_SET_SPEED         _IOW(NRF_MAGIC, SET_SPEED_SEQ_NO, uint8_t)
#define NRF_IOCTL_SET_RX_ADDR       _IOW(NRF_MAGIC, SET_RX_ADDR_SEQ_NO, struct nrf_pipe_config)
#define NRF_IOCTL_SET_TX_ADDR       _IOW(NRF_MAGIC, SET_TX_ADDR_SEQ_NO, uint64_t)

// Read
#define NRF_IOCTL_GET_STATUS        _IOR(NRF_MAGIC, GET_STATUS_SEQ_NO, uint8_t)
#define NRF_IOCTL_GET_RF_CHANNEL    _IOR(NRF_MAGIC, GET_RF_CHANNEL, uint8_t)
#define NRF_IOCTL_GET_RF_SETUP      _IOR(NRF_MAGIC, GET_RF_SETUP, uint8_t)

#endif