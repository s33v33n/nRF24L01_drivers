#ifndef NRF_HAL_H
#define NRF_HAL_H

#include <linux/spi/spi.h>
#include "nrf_device.h"

// HAL functions 
int nrf_read_reg(struct nrf24l01_dev *dev, u8 reg, u8 *val);
int nrf_write_reg(struct nrf24l01_dev *dev, u8 reg, u8 val);
int nrf_write_payload(struct nrf24l01_dev *dev, const u8 *buf, size_t len);

#endif // NRF_HAL_H
