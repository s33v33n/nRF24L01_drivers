#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include "nrf_ioctl.h" 

#define MAX_PAYLOAD 32

int main() {

    const char *dev_name = "/dev/nrf24l01_0";
    int nrf_file = open(dev_name, O_RDWR);

    if (nrf_file < 0) {
        perror("Failed to open nRF24L01 device");
        return -1;
    }

    printf("--- SNIFFER MODE ---\n");

    struct pollfd my_file;
    my_file.fd = nrf_file;
    my_file.events = POLLIN;

    unsigned char buffer[MAX_PAYLOAD];

    while (1) {
        int ret = poll(&my_file, 1, -1);
        if (ret < 0) {
            perror("Poll failed");
            break;
        }

        if (my_file.revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_read = read(nrf_file, buffer, MAX_PAYLOAD);

            if (bytes_read > 0) {
                printf("[INTERCEPTED PACKET]: ");
                for (int i = 0; i < bytes_read; i++) {
                    printf("%02X ", buffer[i]);
                }
                
                printf(" | ASCII: ");
                for (int i = 0; i < bytes_read; i++) {
                    printf("%c", buffer[i]);
                }
                printf("\n");
            }
        }
    }

    close(nrf_file);
    return 0;
}