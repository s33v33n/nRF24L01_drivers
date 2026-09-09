#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include "nrf_ioctl.h" 

#define MAX_PAYLOAD 32

int main(int argc, char *argv[]) {

    // 1. Open device 
    if (argc < 2) {
        printf("Error, use script + device loction: ./secure_chat /dev/nrf24l01_n\n");
        return -1;
    }

    const char *dev_name = argv[1];
    int nrf_file = open(dev_name, O_RDWR);
    if (nrf_file < 0) {
        printf("Error while opening device, use sudo");
        return -1;
    }

    // 2. Config
    int channel = 15; 
    if (ioctl(nrf_file, NRF_IOCTL_SET_CHANNEL, &channel) < 0) {
        printf("Channel setup failed");
        close(nrf_file); 
        return -1;
    }

    int speed = 1; // Mbs
    if (ioctl(nrf_file, NRF_IOCTL_SET_SPEED, &speed) < 0) {
        printf("Channel setup failed");
        close(nrf_file); 
        return -1;
    }

    int power = 0; // dBm | 0 - min , 3 - max 
    if (ioctl(nrf_file, NRF_IOCTL_SET_POWER, &power) < 0) {
        printf("Power setup failed");
        close(nrf_file); 
        return -1;
    }

    printf("--- Device: %s ---\n", dev_name);
    printf("--- Channel: %d | Speed: %d Mbps --- Power: %d dBm ---\n", channel, speed, ((power * 6) - 18));

    // 3. Structs for kernel and poll 
    struct pollfd kernel_files[2];
    
    // keyboard
    kernel_files[0].fd = STDIN_FILENO;
    kernel_files[0].events = POLLIN;

    // nrfdevice as char device 
    kernel_files[1].fd = nrf_file;
    kernel_files[1].events = POLLIN;

    // payload + end of line
    char buffer[MAX_PAYLOAD + 1];

    // 4. loop 
    while (1) {

        // sleep kernel files
        int ret = poll(kernel_files, 2, -1); 
        if (ret < 0) {
            perror("Sleeping kernel files failed\n");
            break;
        }

        // user command
        if (kernel_files[0].revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            
            if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
                
                // command check (remove \n)
                buffer[strcspn(buffer, "\n")] = '\0';

                // write to device
                if (write(nrf_file, buffer, strlen(buffer)) < 0) {
                        perror("Cannot send to device");
                } 
            }
        }

        // radio received message 
        if (kernel_files[1].revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            
            // read data 
            int bytes_read = read(nrf_file, buffer, MAX_PAYLOAD); 
            
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0'; 
                printf("\rReceived: %s\n", buffer); 
            }
        }
    }

    close(nrf_file);
    return 0;
}