#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "nrf_ioctl.h" 

int main() {

    //nrf24l01_0
    int fd = open("/dev/nrf24l01_0", O_RDWR);
    if (fd < 0) {
        perror("Error opening device");
        return -1;
    }
    else{
        printf("---/dev/nrf24l01_0 opened---\n" );
    }

    // Channel
    int new_channel = 77;
    if (ioctl(fd, NRF_IOCTL_SET_CHANNEL, &new_channel) == 0){
        printf("Channel set to %d\n", new_channel);
        ioctl(fd, NRF_IOCTL_GET_RF_CHANNEL, &new_channel);
        printf("new channel: %d\n", new_channel);
    }
        
    // Speed
    int new_speed = 3;
    if (ioctl(fd, NRF_IOCTL_SET_SPEED, &new_speed) == 0){
        printf("Speed set to %d Mbps\n", new_speed);
    }
    else{
        printf("Encountered error while speed changing\n");
    }

    // Status 
    unsigned char status;
    if (ioctl(fd, NRF_IOCTL_GET_STATUS, &status) == 0){
        printf("nrf24l01_0 STATUS register: 0x%02X\n", status);
    } 
    close(fd);

    //nrf24l01_1
    fd = open("/dev/nrf24l01_1", O_RDWR);
    if (fd < 0) {
        perror("Error opening device");
        return -1;
    }
    else{
        printf("---/dev/nrf24l01_1 opened---\n" );
    }

    // Channel
    new_channel = 77;
    if (ioctl(fd, NRF_IOCTL_SET_CHANNEL, &new_channel) == 0){
        printf("Channel set to %d\n", new_channel);
        ioctl(fd, NRF_IOCTL_GET_RF_CHANNEL, &new_channel);
        printf("new channel: %d\n", new_channel);
    }
        
    // Speed
    if (ioctl(fd, NRF_IOCTL_SET_SPEED, &new_speed) == 0){
        printf("Speed set to %d Mbps\n", new_speed);
    }
    else{
        printf("Encountered error while speed changing\n");
    }

    // Status 
    if (ioctl(fd, NRF_IOCTL_GET_STATUS, &status) == 0){
        printf("nrf24l01_1 STATUS register: 0x%02X\n", status);
    } 
    close(fd);

    return 0;
}