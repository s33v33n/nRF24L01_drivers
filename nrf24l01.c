#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/of.h>

#define DRIVER_NAME "nrf24l01"

struct nrf24l01_dev
{
    struct spi_device *spi;
    struct gpio_desc *ce_gpio;
    int irq;
};

// 1. Device declararion (compativility using SPI and DeviceTree)

// SPI
static const struct spi_device_id nrf24l01_spi_id[] = {
    {"nrf24l01", 0},
    {},
};

// DeviceTree
static const struct of_device_id nrf24l01_of_match[] = {
    {.compatible = "nordic,nrf24l01"},
    {},
};

// 2. Register module in the kernel (during the compilation)
MODULE_DEVICE_TABLE(spi, nrf24l01_spi_id);
MODULE_DEVICE_TABLE(of, nrf24l01_of_match);

//  3a. Probe and Remove functions
static int nrf24l01_probe(struct spi_device *spi)
{

    struct nrf24l01_dev *dev;
    int ret;

    // memory allocation for device (private data define)
    dev = devm_kzalloc(&spi->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    // spi cinfiguration
    spi->mode = SPI_MODE_0;
    spi->bits_per_word = 8;
    spi->max_speed_hz = 8000000; // 8 MHz

    ret = spi_setup(spi);
    if (ret < 0)
    {
        dev_err(&spi->dev, "Failed to setup SPI: %d\n", ret);
        return ret;
    }

    // device configuration
    dev->ce_gpio = devm_gpiod_get(&spi->dev, "ce", GPIOD_OUT_LOW);
    if (IS_ERR(dev->ce_gpio))
    {
        dev_err(&spi->dev, "Failed to get CE GPIO\n");
        return PTR_ERR(dev->ce_gpio);
    }

    // define and saving private data
    dev->spi = spi;
    spi_set_drvdata(spi, dev);

    // SPI hardware verification: read CONFIG register (0x00)
    // nRF24L01 default value after reset = 0x08
    // 0x00 or 0xFF = wiring problem (MISO/MOSI/CS issue)
    {
        // spi = (spi_transfer + spi_message) -> spi_sync

        u8 tx[2] = {0x00, 0xFF}; // command R_REGISTER|CONFIG + dummy byte
        u8 rx[2] = {0x00, 0x00};
        struct spi_transfer t = {
            .tx_buf = tx,
            .rx_buf = rx,
            .len = 2,
        };

        struct spi_message m;
        spi_message_init(&m);
        spi_message_add_tail(&t, &m);

        ret = spi_sync(spi, &m);
        if (ret < 0)
        {
            dev_err(&spi->dev, "SPI transfer failed: %d\n", ret);
            return ret;
        }

        dev_info(&spi->dev, "STATUS = 0x%02X, CONFIG = 0x%02X\n", rx[0], rx[1]);

        if (rx[1] == 0x00 || rx[1] == 0xFF)
            dev_warn(&spi->dev, "Unexpected CONFIG value - check wiring!\n");
    }

    {
        const char *model_name = "default - safety mechnism"; // safety mechnism - if not defined in dts

        if (spi->dev.of_node)
        {
            ret = of_property_read_string(spi->dev.of_node, "model", &model_name);
        }

        dev_info(&spi->dev, "Device [%s] probed successfully\n", model_name);
    }

    return 0;
}

// 3b. Remove function
static void nrf24l01_remove(struct spi_device *spi)
{

    // get private data
    struct nrf24l01_dev *dev = spi_get_drvdata(spi);
    int ret;

    // set CE to LOW
    gpiod_set_value(dev->ce_gpio, 0);

    {
        const char *model_name = "default - safety mechnism"; // safety mechnism - if not defined in dts

        if (spi->dev.of_node)
        {
            ret = of_property_read_string(spi->dev.of_node, "model", &model_name);
        }

        dev_info(&spi->dev, "Device [%s] removed successfully\n", model_name);
    }
}

// 4. Struct declaration
static struct spi_driver nrf24l01_driver = {

    .driver = {

        .name = DRIVER_NAME,
        .of_match_table = of_match_ptr(nrf24l01_of_match),
    },

    .probe = nrf24l01_probe,
    .remove = nrf24l01_remove,
    .id_table = nrf24l01_spi_id,
};

// 5. Registration in kernel
module_spi_driver(nrf24l01_driver);

MODULE_DESCRIPTION("nrf24l01 SPI driver");
MODULE_LICENSE("GPL");
