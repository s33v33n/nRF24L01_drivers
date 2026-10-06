#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <linux/poll.h>
#include "nrf_device.h"
#include "nrf_hal.h"
#include "nrf_ioctl.h"
#include "../network_config.h"

static dev_t nrf24l01_devt;
static struct class *nrf24l01_class;
static int nrf24l01_minor_counter = 0;


/* START OF FILE_OPERATIONS */

static int nrf24l01_open(struct inode *inode, struct file *file)
{
    // retrieve hardware data and store it in current file session
    struct nrf24l01_dev *dev = container_of(inode->i_cdev, struct nrf24l01_dev, cdev);
    file->private_data = dev;

    mutex_lock(&dev->priv_mutex);
    
    // turn radio on in RX mode
    nrf_write_reg(dev, NRF_REG_CONFIG, 0x0F);
    
    // wait for hardware
    msleep(2); 
    
    // antenna up
    gpiod_set_value(dev->ce_gpio, 1);
    
    mutex_unlock(&dev->priv_mutex);

    dev_info(&dev->spi->dev, "Device opened - Radio turned on in RX mode\n");
    return 0;
}

static int nrf24l01_release(struct inode *inode, struct file *file)
{
    // reverse open actions
    struct nrf24l01_dev *dev = file->private_data;

    mutex_lock(&dev->priv_mutex);
    
    // antenna down
    gpiod_set_value(dev->ce_gpio, 0);
    
    // turn off radio
    nrf_write_reg(dev, NRF_REG_CONFIG, 0x0D);
    
    mutex_unlock(&dev->priv_mutex);

    dev_info(&dev->spi->dev, "Device closed - Radio turned off\n");
    return 0;
}

static ssize_t nrf24l01_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
    struct nrf24l01_dev *dev = file->private_data;
    u8 rx_buf[NRF_MAX_PAYLOAD_SIZE];
    size_t payload_len = count;

    if (payload_len > NRF_MAX_PAYLOAD_SIZE){
        payload_len = NRF_MAX_PAYLOAD_SIZE;
    }
        
    if (wait_event_interruptible(dev->rx_waitqueue, dev->rx_data_ready)) {
        return -ERESTARTSYS; // restart system call
    }

    mutex_lock(&dev -> priv_mutex);

    nrf_read_payload(dev, rx_buf, payload_len);
    
    u8 fifo_status = 0;
    nrf_read_reg(dev, NRF_REG_FIFO_STATUS, &fifo_status);
    if (fifo_status & 0x01) {
        dev->rx_data_ready = false;
    }

    mutex_unlock(&dev -> priv_mutex);

    if (copy_to_user(buf, rx_buf, payload_len)) {
        return -EFAULT;
    }

    dev_info(&dev->spi->dev, "Received %zu bytes\n", payload_len);

    return payload_len; 
}

static ssize_t nrf24l01_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
    struct nrf24l01_dev *dev = file->private_data;
    u8 tx_buf[NRF_MAX_PAYLOAD_SIZE] = {0}; 

    size_t payload_len = count;
    if (payload_len > NRF_MAX_PAYLOAD_SIZE){
        payload_len = NRF_MAX_PAYLOAD_SIZE;
    }
    
    // Copy data from user space to kernel space
    if (copy_from_user(tx_buf, buf, payload_len)) {
        return -EFAULT;
    }

    mutex_lock(&dev -> priv_mutex);

    // sleep tx waiting queue
    dev->tx_done = false; 
    dev->tx_success = false;

    // turn off RX mode (antena off)
    gpiod_set_value(dev->ce_gpio, 0);

    // change to TX mode
    nrf_write_reg(dev, NRF_REG_CONFIG, 0x0E);

    // clear buffer and flags
    nrf_send_spi_command(dev, NRF_CMD_FLUSH_TX);
    nrf_write_reg(dev, NRF_REG_STATUS, 0x70);

    // Write data to device
    nrf_write_payload(dev, tx_buf, NRF_MAX_PAYLOAD_SIZE);

    // CE up
    gpiod_set_value(dev->ce_gpio, 1);

    udelay(15);
    
    // CE down 
    gpiod_set_value(dev->ce_gpio, 0); 

    // wait for radio (proccess sleep without cpu delay)
    wait_event_interruptible_timeout(dev->tx_waitqueue, dev->tx_done, msecs_to_jiffies(100));

    // return to RX mode 
    nrf_write_reg(dev, NRF_REG_CONFIG, 0x0F);
    gpiod_set_value(dev->ce_gpio, 1);

    mutex_unlock(&dev -> priv_mutex);

    // ensure data was received
    if (!dev->tx_success) {
        dev_info(&dev->spi->dev, "lack of ACK\n");
        return -EIO; 
    }

    dev_info(&dev->spi->dev, "Sent %zu bytes!\n", payload_len);
    
    return payload_len; 
}

static __poll_t nrf24l01_poll(struct file *file, poll_table *wait)
{
    struct nrf24l01_dev *dev = file->private_data;
    __poll_t mask = 0;

    poll_wait(file, &dev->rx_waitqueue, wait);
    poll_wait(file, &dev->tx_waitqueue, wait);
    
    if (dev->rx_data_ready) {
        mask |= (EPOLLIN | EPOLLRDNORM);
    }
    
    if (dev->tx_done) {
        mask |= (EPOLLOUT | EPOLLWRNORM);
    }

    return mask;
}

static long nrf24l01_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct nrf24l01_dev *dev = file->private_data;
    size_t bytes_to_write;

    u8 val;
    u8 status_reg;
    u8 rf_setup;
    
    // nrf pipes
    struct nrf_pipe_config pipe_config;
    u64 tx_addr;
    u8 enable_rxaddr;
    u8 reg_tx_addr;
    u8 reg_rx_addr;


    switch(cmd) { 

        case NRF_IOCTL_SET_CHANNEL:
        
            if (copy_from_user(&val, (u8 *)arg, sizeof(u8))){
                return -EFAULT;
            } 
            if (val < 1 || val > 124){
                return -EINVAL;
            }  

            mutex_lock(&dev -> priv_mutex);
            
            // device off
            gpiod_set_value(dev->ce_gpio, 0);

            // channel change
            nrf_write_reg(dev, NRF_REG_RF_CH, val);

            // device on
            gpiod_set_value(dev->ce_gpio, 1);

            mutex_unlock(&dev -> priv_mutex);

            dev_info(&dev->spi->dev, "IOCTL: Channel set to %d\n", val);
            break;

        case NRF_IOCTL_SET_POWER:
            
            if (copy_from_user(&val, (u8 *)arg, sizeof(u8))){
                return -EFAULT;
            } 
            if (val < 0 || val > 3){
                return -EINVAL;
            } 

            mutex_lock(&dev -> priv_mutex);
            
            // device off
            gpiod_set_value(dev->ce_gpio, 0);

            // power change
            nrf_read_reg(dev, NRF_REG_RF_SETUP, &rf_setup); // read setup
            rf_setup = (rf_setup & ~0x06) | (val << 1);     // clear and set bits 1-2
            nrf_write_reg(dev, NRF_REG_RF_SETUP, rf_setup); // save setup

            // device on
            gpiod_set_value(dev->ce_gpio, 1);

            mutex_unlock(&dev -> priv_mutex);

            dev_info(&dev->spi->dev, "IOCTL: Power set to level %d\n", val);
            break;

        case NRF_IOCTL_SET_SPEED:
            
            if (copy_from_user(&val, (u8 *)arg, sizeof(u8))){
                return -EFAULT;
            } 
            if (val != 1 && val != 2){
                return -EINVAL;
            }

            mutex_lock(&dev -> priv_mutex);
            
            nrf_read_reg(dev, NRF_REG_RF_SETUP, &rf_setup);
            if (val == 1) {
                rf_setup &= ~0x08;
            } 
            else if (val == 2) {
                rf_setup |= 0x08;       
            }  

            // device off
            gpiod_set_value(dev->ce_gpio, 0);

            // speed change 
            nrf_write_reg(dev, NRF_REG_RF_SETUP, rf_setup);

            // device on
            gpiod_set_value(dev->ce_gpio, 1);

            mutex_unlock(&dev -> priv_mutex);

            dev_info(&dev->spi->dev, "IOCTL: Speed set to %d\n", val);
            break;

        case NRF_IOCTL_SET_RX_ADDR:

            if(copy_from_user(&pipe_config, (struct nrf_pipe_config *)arg, sizeof(struct nrf_pipe_config))){
                return -EFAULT;
            }

            if(pipe_config.pipe_num > 5){
                return -EINVAL;
            }

            // pipe register 
            reg_rx_addr = NRF_REG_RX_ADDR_P0 + pipe_config.pipe_num;

            // set pipe address for pipe 0, 1
            if(pipe_config.pipe_num <= 1){
                bytes_to_write = 5;
            }
            //shared bytes for pipes 2-5
            else{
                bytes_to_write = 1;
            }

            mutex_lock(&dev->priv_mutex);
            gpiod_set_value(dev->ce_gpio, 0);
            
            nrf_write_pipe_register(dev, reg_rx_addr, (const u8 *)&pipe_config.address, bytes_to_write);
            
            // enable this pipe register
            nrf_read_reg(dev, NRF_REG_EN_RXADDR, &enable_rxaddr);
            enable_rxaddr |= (1 << pipe_config.pipe_num);
            nrf_write_reg(dev, NRF_REG_EN_RXADDR, enable_rxaddr);

            gpiod_set_value(dev->ce_gpio, 1);
            mutex_unlock(&dev->priv_mutex);

            dev_info(&dev->spi->dev, "RX pipe enabled: address: %llu number: %d\n", pipe_config.address, pipe_config.pipe_num);
            break;

        case NRF_IOCTL_SET_TX_ADDR:

            reg_tx_addr = NRF_REG_TX_ADDR;
            reg_rx_addr = NRF_REG_RX_ADDR_P0;
            bytes_to_write = 5;

            if(copy_from_user(&tx_addr, (u64 *)arg, sizeof(u64))){
                return -EFAULT;
            }

            mutex_lock(&dev->priv_mutex);
            gpiod_set_value(dev->ce_gpio, 0);

            nrf_write_pipe_register(dev, reg_tx_addr, (const u8 *)&tx_addr, bytes_to_write);
            
            // when transmitting pipe 0 must be the same as tx_addr for ACK
            nrf_write_pipe_register(dev, reg_rx_addr, (const u8 *)&tx_addr, bytes_to_write);
                        
            gpiod_set_value(dev->ce_gpio, 1);
            mutex_unlock(&dev->priv_mutex);

            dev_info(&dev->spi->dev, "TX pipe enabled: address: %llu\n", tx_addr);
            break;

        case NRF_IOCTL_GET_STATUS:

            mutex_lock(&dev -> priv_mutex);
            
            nrf_read_reg(dev, NRF_REG_STATUS, &status_reg);
            
            mutex_unlock(&dev -> priv_mutex);

            if (copy_to_user((u8 *)arg, &status_reg, sizeof(u8))){
                return -EFAULT;
            } 
            break;

        case NRF_IOCTL_GET_RF_CHANNEL:
        
            mutex_lock(&dev->priv_mutex);
            
            nrf_read_reg(dev, NRF_REG_RF_CH, &status_reg);
            
            mutex_unlock(&dev->priv_mutex);

            val = status_reg; 

            if (copy_to_user((u8*)arg, &val, sizeof(u8))){
                return -EFAULT;
            } 
            break; 

        case NRF_IOCTL_GET_RF_SETUP:

            mutex_lock(&dev->priv_mutex);
    
            nrf_read_reg(dev, NRF_REG_RF_SETUP, &rf_setup);

            mutex_unlock(&dev->priv_mutex);

            if (copy_to_user((u8*)arg, &rf_setup, sizeof(u8))){
                return -EFAULT;
            } 
            break;

        default:
            return -ENOTTY; // Unknown command - error not a typewriter
    }
    return 0;
}

static const struct file_operations nrf24l01_fops = {
    .owner = THIS_MODULE,
    .open = nrf24l01_open,
    .release = nrf24l01_release,
    .read = nrf24l01_read,
    .write = nrf24l01_write,
    .poll = nrf24l01_poll,
    .unlocked_ioctl = nrf24l01_ioctl,
};
/* END OF FILE_OPERATIONS   */

/* START OF DRIVER DESCRIPTION */
// 1. Device declararion (compatibility using SPI and DeviceTree)

// SPI
static const struct spi_device_id nrf24l01_spi_id[] = {
    {DRIVER_NAME, 0},
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

    // memory allocation for device (private data define), structure is filled with zeros
    dev = devm_kzalloc(&spi->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev){
        return -ENOMEM; // error no memory 
    }
        
    mutex_init(&dev -> priv_mutex);

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
        u8 config_val = 0;
        u8 status_val = 0;

        ret = nrf_read_reg(dev, NRF_REG_CONFIG, &config_val);
        if (ret < 0) {
            dev_err(&spi->dev, "SPI transfer failed: %d\n", ret);
            return ret;
        }

        nrf_read_reg(dev, NRF_REG_STATUS, &status_val);
        dev_info(&spi->dev, "STATUS = 0x%02X, CONFIG = 0x%02X\n", status_val, config_val);

        if (config_val == 0x00 || config_val == 0xFF) {
            dev_warn(&spi->dev, "Unexpected CONFIG value - check wiring!\n");
        }     
    }

    const char *model_name = "default - safety mechnism, model name not defined"; // safety mechnism - if not defined in dts

    if (spi->dev.of_node)
    {
        ret = of_property_read_string(spi->dev.of_node, "model", &model_name);
    }

    dev_info(&spi->dev, "Device [%s] probed successfully\n", model_name);

    /* START OF NRF24 HARDWARE INIT */
    // optionally - if device is working less than 15ms   
    msleep(15);

    // CONFIG
    nrf_write_reg(dev, NRF_REG_CONFIG, 0x0D);   // radio is off (device is on power), rx mode, enable crc 2 bytes, force crc 

    // payload size for pipes
    for (u8 i = 0; i <= 5; i++) {
        nrf_write_reg(dev, NRF_REG_RX_PW_P0 + i, NRF_MAX_PAYLOAD_SIZE);
    }
    
    // ack active on every pipe
    nrf_write_reg(dev, NRF_REG_EN_AA, 0x3F);

    // enable only rx on pipe 0
    nrf_write_reg(dev, NRF_REG_EN_RXADDR, 0x01);

    // address width - 5 bytes
    nrf_write_reg(dev, NRF_REG_SETUP_AW, 0x03);

    // 15 retries every after 1ms delay
    nrf_write_reg(dev, NRF_REG_SETUP_RETR, 0x3F); 
    
    // RF setup: Channel 
    nrf_write_reg(dev, NRF_REG_RF_CH, 0x0F);       // f0 = 2400 MHz, channel 15 = 2425 MHz
    
    // RF setup: Speed + TX power
    nrf_write_reg(dev, NRF_REG_RF_SETUP, 0x01); // Power = -18 dBm, Speed = 1Mbit/s

    // STATUS
    nrf_write_reg(dev, NRF_REG_STATUS, 0x70);

    //set my own default addresses
    u8 default_addr[5] = {0x77, 0x88, 0x99, 0xAA, 0xBB};
    nrf_write_pipe_register(dev, NRF_REG_TX_ADDR, default_addr, 5);
    nrf_write_pipe_register(dev, NRF_REG_RX_ADDR_P0, default_addr, 5);

    msleep(2); // start up wait 1.5ms 
    /* END OF NRF24 HARDWARE INIT */
    
    // clear FIFO queue 
    nrf_send_spi_command(dev, NRF_CMD_FLUSH_RX);
    nrf_send_spi_command(dev, NRF_CMD_FLUSH_TX);

    // Init waitqueue for RX
    init_waitqueue_head(&dev->rx_waitqueue);

    // Init waitqueue for TX
    init_waitqueue_head(&dev->tx_waitqueue);
    dev->tx_done = true; 
    
    // Request IRQ from Device Tree
    dev->irq = spi->irq;

    //                            device,    irq pin,  bottom-half isr, isr function, isr name, device
    ret = devm_request_threaded_irq(&spi->dev, dev->irq, NULL, nrf24l01_isr, IRQF_TRIGGER_FALLING | IRQF_ONESHOT, "nrf24l01", dev);
    
    if (ret) {
        dev_err(&spi->dev, "Failed to request IRQ %d\n", dev->irq);
        return ret;
    }

    // register character device 
    dev -> minor = nrf24l01_minor_counter++;
    
    // attach file_operations and cdev_add
    cdev_init(&dev->cdev, &nrf24l01_fops);
    dev->cdev.owner = THIS_MODULE;

    // add Major & minor number 
    ret = cdev_add(&dev->cdev, nrf24l01_devt + dev->minor, 1);

    if (ret < 0) {
        pr_err("nrf24l01: Failed to add character device\n");
        return ret;
    }

    // create device file in /dev/nrf24l01
    device_create(nrf24l01_class, &spi->dev, MKDEV(MAJOR(nrf24l01_devt), dev->minor), NULL, "%s", model_name);
    
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
        const char *model_name = "default - safety mechnism, model name not defined"; // safety mechnism - if not defined in dts

        if (spi->dev.of_node)
        {
            ret = of_property_read_string(spi->dev.of_node, "model", &model_name);
        }

        dev_info(&spi->dev, "Device [%s] removed successfully\n", model_name);
    }

    // remove file
    device_destroy(nrf24l01_class, MKDEV(MAJOR(nrf24l01_devt), dev->minor));
    
    // release char number
    cdev_del(&dev->cdev);
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

// 5. Registration in the kernel
static int __init nrf24l01_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&nrf24l01_devt, 0, NRF_NDEVICES, DRIVER_NAME);
    if (ret < 0) {
        pr_err("nrf24l01: Failed to allocate character device region\n");
        return ret;
    }

    // device class visible in /sys/class
    nrf24l01_class = class_create(DRIVER_NAME);
    
    if (IS_ERR(nrf24l01_class)) {
        pr_err("nrf24l01: Failed to create class\n");
        unregister_chrdev_region(nrf24l01_devt, NRF_NDEVICES);
        return PTR_ERR(nrf24l01_class);
    }

    // SPI driver registration
    ret = spi_register_driver(&nrf24l01_driver);
    if (ret < 0) {
        pr_err("nrf24l01: Failed to register SPI driver\n");
        class_destroy(nrf24l01_class);
        unregister_chrdev_region(nrf24l01_devt, NRF_NDEVICES);
        return ret;
    }

    pr_info("nrf24l01: Init successful\n");
    return 0;
}

static void __exit nrf24l01_exit(void)
{
    // unregister device from Kernel
    spi_unregister_driver(&nrf24l01_driver);
    class_destroy(nrf24l01_class);
    unregister_chrdev_region(nrf24l01_devt, NRF_NDEVICES);
    pr_info("nrf24l01: Exit successful\n");
}

module_init(nrf24l01_init);
module_exit(nrf24l01_exit);

/* END OF DRIVER DESCRIPTION */


MODULE_DESCRIPTION("nrf24l01 SPI driver");
MODULE_LICENSE("GPL");