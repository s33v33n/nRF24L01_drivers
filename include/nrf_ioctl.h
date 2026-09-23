#ifndef NRF_IOCTL_H
#define NRF_IOCTL_H

#include <linux/ioctl.h>

// for pipe addresses
struct nrf_pipe_config {
    unsigned char pipe_num;
    unsigned long long address;
};

#define NRF_MAGIC 'N'
#define GET_STATUS_SEQ_NO   0x01
#define SET_CHANNEL_SEQ_NO  0x02
#define SET_POWER_SEQ_NO    0x03
#define SET_SPEED_SEQ_NO    0x04
#define GET_RF_CHANNEL      0x05
#define GET_RF_SETUP        0x06
#define SET_RX_ADDR_SEQ_NO  0x07
#define SET_TX_ADDR_SEQ_NO  0x08

#define NRF_IOCTL_GET_STATUS        _IOR(NRF_MAGIC, GET_STATUS_SEQ_NO, unsigned char)
#define NRF_IOCTL_SET_CHANNEL       _IOW(NRF_MAGIC, SET_CHANNEL_SEQ_NO, int)
#define NRF_IOCTL_SET_POWER         _IOW(NRF_MAGIC, SET_POWER_SEQ_NO, int)
#define NRF_IOCTL_SET_SPEED         _IOW(NRF_MAGIC, SET_SPEED_SEQ_NO, int)
#define NRF_IOCTL_GET_RF_CHANNEL    _IOR(NRF_MAGIC, GET_RF_CHANNEL, unsigned int)
#define NRF_IOCTL_GET_RF_SETUP      _IOR(NRF_MAGIC, GET_RF_SETUP, unsigned char)
#define NRF_IOCTL_SET_RX_ADDR       _IOW(NRF_MAGIC, SET_RX_ADDR_SEQ_NO, struct nrf_pipe_config)
#define NRF_IOCTL_SET_TX_ADDR       _IOW(NRF_MAGIC, SET_TX_ADDR_SEQ_NO, unsigned long long)

#endif