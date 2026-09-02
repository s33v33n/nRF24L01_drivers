#include <linux/spi/spi.h>
#include "nrf_device.h"
#include "nrf_hal.h"

// Read 1 byte from register
int nrf_read_reg(struct nrf24l01_dev *dev, u8 reg, u8 *val)
{
    u8 tx_buf[2] = { NRF_CMD_R_REGISTER | reg, 0xFF }; // 0xFF - dummy byte for reading
    u8 rx_buf[2] = { 0x00, 0x00 };
    
    struct spi_transfer t = {
        .tx_buf = tx_buf,
        .rx_buf = rx_buf,
        .len = 2,
    };
    struct spi_message m;
    int ret;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);

    ret = spi_sync(dev->spi, &m);
    if (ret < 0)
        return ret;

    *val = rx_buf[1];
    return 0;
}

// Write 1 byte to register
int nrf_write_reg(struct nrf24l01_dev *dev, u8 reg, u8 val)
{
    u8 tx_buf[2] = { NRF_CMD_W_REGISTER | reg, val };
    
    struct spi_transfer t = {
        .tx_buf = tx_buf,
        .len = 2,
    };
    struct spi_message m;

    spi_message_init(&m);
    spi_message_add_tail(&t, &m);

    return spi_sync(dev->spi, &m);
}

// Write payload to TX FIFO
int nrf_write_payload(struct nrf24l01_dev *dev, const u8 *buf, size_t len)
{
    u8 tx_cmd = NRF_CMD_W_TX_PAYLOAD;

    struct spi_transfer t[2] = {
        [0] = {
            .tx_buf = &tx_cmd,
            .len = 1,
        },
        [1] = {
            .tx_buf = buf,
            .len = len,
        }
    };

    struct spi_message m;

    spi_message_init(&m);
    spi_message_add_tail(&t[0], &m);
    spi_message_add_tail(&t[1], &m);

    return spi_sync(dev->spi, &m);
}
