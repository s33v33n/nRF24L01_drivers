#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <poll.h>
#include <stdbool.h>
#include <unistd.h>
#include "nrf_ioctl.h" 

#define MAX_PAYLOAD 32

unsigned long long public_pin = 0;
unsigned long long my_secret = 0;
unsigned long long shared_key = 0;

bool sent_key = false;
bool receive_key = false;
bool use_encryption = true;

void encrypt_decrypt(char *data, int len, unsigned long long key) {
    unsigned char *key_bytes = (unsigned char *)&key;
    for (int i = 0; i < len; i++) {
        data[i] ^= key_bytes[i % sizeof(unsigned long long)];
    }
}

int main(int argc, char *argv[]) {

    // 1. Open device 
    if (argc < 4) {
        printf("Error, use script + device loction + public_key + private_key: ./secure_chat /dev/nrf24l01_n XXXX XXXX\n");
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

    int speed = 1; // Mbps
    if (ioctl(nrf_file, NRF_IOCTL_SET_SPEED, &speed) < 0) {
        printf("Channel setup failed");
        close(nrf_file); 
        return -1;
    }

    int power = 3; // dBm | 0 - min , 3 - max 
    if (ioctl(nrf_file, NRF_IOCTL_SET_POWER, &power) < 0) {
        printf("Power setup failed");
        close(nrf_file); 
        return -1;
    }

    public_pin = strtoull(argv[2], NULL, 10);
    my_secret = strtoull(argv[3], NULL, 10);

    printf("--- Channel: %d | Speed: %d Mbps --- Power: %d dBm ---\n", channel, speed, ((power * 6) - 18));
    printf("--- Device: %s ---\n", dev_name);
    printf("--- public_key: %s | private_key: %s ---\n", argv[2], argv[3]);


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

                if(strncmp(buffer,"/connect",8) == 0 && sent_key == false){

                    unsigned long long my_public = public_pin * my_secret;

                    char msg[32];
                    sprintf(msg, "P_KEY:%llu", my_public); 

                    if(write(nrf_file, msg, strlen(msg)) < 0){
                        printf("Key was not received.\n");
                    }
                    else{
                        printf("Key was received\n");
                        sent_key = true;
                    }

                    
                }

                // power setup
                else if (strncmp(buffer, "/power", 6) == 0) {
                    printf("Enter power from range <0,3> only int, corresponds 0 dBm, -6 dBm, -12 dBm, -18 dBm\n");
                    
                    //print value
                    fflush(stdout); 
                    
                    int val = -1;
                    if (scanf("%d", &val) == 1) {
                        
                        // clear ENTER
                        int c; 
                        while ((c = getchar()) != '\n' && c != EOF);
                        
                        if (val >= 0 && val <= 3) {
                            if (ioctl(nrf_file, NRF_IOCTL_SET_POWER, &val) < 0) {
                                perror("IOCTL error setup");
                            } 
                            else {
                                printf("New power: %d dBm.\n", ((val * 6) - 18));
                            }
                        } 
                        else {
                            printf("Input is out of range\n");
                        }
                    }
                }

                // chennel setup
                else if (strncmp(buffer, "/channel", 8) == 0) {
                    printf("New channel: 2.4GHz + digit from range <1,124> only int\n");
                    fflush(stdout);
                    
                    int val = -1;
                    if (scanf("%d", &val) == 1) {
                        int c; while ((c = getchar()) != '\n' && c != EOF);
                        
                        if (val >= 1 && val <= 124) {
                            if (ioctl(nrf_file, NRF_IOCTL_SET_CHANNEL, &val) < 0) {
                                perror("IOCTL error setup");
                            } 
                            else {
                                printf("New channel %.3f GHz.\n", (float)val / 1000.0 + 2.4);
                            }
                        } 
                        else {
                            printf("Input is out of range\n");
                        }
                    }
                }

                // speed setup
                else if (strncmp(buffer, "/speed", 6) == 0) {
                
                    printf("New speed (Mbps) only allowed 1 or 2\n");
                    fflush(stdout);
                    
                    int val = -1;
                    if (scanf("%d", &val) == 1) {
                        int c; while ((c = getchar()) != '\n' && c != EOF);
                        
                        if (val == 1 || val == 2) {
                            if (ioctl(nrf_file, NRF_IOCTL_SET_SPEED, &val) < 0) {
                                perror("IOCTL error setup");
                            } 
                            else {
                                printf("New speed: %d Mbps.\n", val);
                            }
                        } 
                        else {
                            printf("Input is out of range.\n");
                        }
                    }
                }

                // read rf channel
                else if (strncmp(buffer, "/rrfchannel", 11) == 0) {
                
                    fflush(stdout);
                    
                    unsigned int reg = 0;

                    if (ioctl(nrf_file, NRF_IOCTL_GET_RF_CHANNEL, &reg) < 0) {
                        perror("RF channel read error");
                    } 
                    else {
                        printf("My RF channel: 0x%02X.\n", reg);
                    }
                }


                // read rf setup
                else if (strncmp(buffer, "/rrfsetup", 9) == 0) {
                
                    fflush(stdout);
                    
                    unsigned char reg = 0x00;

                    if (ioctl(nrf_file, NRF_IOCTL_GET_RF_SETUP, &reg) < 0) {
                        perror("RF setup read error");
                    } 
                    else {
                        printf("My RF setup: 0x%02X.\n", reg);
                    }
                }

                // turn off / on encryption
                else if (strncmp(buffer, "/encryption", 11) == 0) {
                    printf("0 - turn off, 1 - turn on, deafult is 1 :\n");
                    fflush(stdout);
                    
                    int val = -1;
                    if (scanf("%d", &val) == 1) {

                        int c; 
                        while ((c = getchar()) != '\n' && c != EOF);
                        
                        if (val == 1) {
                            use_encryption = true;
                            printf("Encryption ON.\n");
                        } 
                        else if (val == 0) {
                            use_encryption = false;
                            printf("Encryptio OFF.\n");
                        } 
                        else {
                            printf("Input is out of range.\n");
                        }
                    }
                }

                // Over The Air power change 
                else if (strncmp(buffer, "/otapower", 9) == 0) {
                    printf("OTA power change. Enter power from range <0,3> only int, corresponds 0 dBm, -6 dBm, -12 dBm, -18 dBm\n");
                    fflush(stdout);
                    
                    int val = -1;
                    if (scanf("%d", &val) == 1) {
                        
                        int c; 
                        while ((c = getchar()) != '\n' && c != EOF);
                        
                        if (val >= 0 && val <= 3) {
                            if (receive_key == true && sent_key == true) {
                                char cmd_msg[MAX_PAYLOAD];
                                memset(cmd_msg, 0, MAX_PAYLOAD);
                                
                                // create command message
                                sprintf(cmd_msg, "OTA_PWR:%d", val);
                                
                                if (use_encryption == true) {
                                    encrypt_decrypt(cmd_msg, MAX_PAYLOAD, shared_key);
                                }
                                
                                if (write(nrf_file, cmd_msg, MAX_PAYLOAD) < 0) {
                                    perror("Cannot send OTA command");
                                } 
                                else {
                                    printf("OTA command sent.\n");
                                }
                            } 
                            else {
                                printf("Send keys first!\n");
                            }
                        } 
                        else {
                            printf("Input is out of range\n");
                        }
                    }
                }

                // write to device (send message)
                else{

                    if(receive_key == true && sent_key == true){

                        int len = strlen(buffer);

                        if (use_encryption == true) {
                            encrypt_decrypt(buffer, MAX_PAYLOAD, shared_key);
                        }
                        
                        int sent_bytes = write(nrf_file, buffer, MAX_PAYLOAD);
                    
                        if(sent_bytes < 0){
                            perror("Cannot send to device");
                        }
                        
                    }
                    else{
                        printf("Keys were not shared - communication is blocked, only setup is availiable\n");
                    }
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
 
                if(strncmp(buffer, "P_KEY:",6) == 0 && receive_key == false){

                    if(receive_key == false){

                        unsigned long long received_public = strtoull(buffer + 6, NULL, 10);

                        if (received_public == 0 || received_public % public_pin != 0) {
                            printf("\rReceived wrong public key\n");
                        }
                        else{

                            shared_key = received_public * my_secret;
                            receive_key = true;

                            printf("\rReceived shared_key: %llu\n", shared_key);
                        }
                    }  

                    if (sent_key == false) {
                        printf("\rType /connect to send your key and start communication\n");
                    }
                }

                else if (receive_key == true && sent_key == true) {
                    
                    if (use_encryption == true) {
                        encrypt_decrypt(buffer, MAX_PAYLOAD, shared_key);
                    }

                    if (strncmp(buffer, "OTA_PWR:", 8) == 0) {

                        int ota_val = -1;
            
                        if (sscanf(buffer + 8, "%d", &ota_val) == 1) {

                            if (ota_val >= 0 && ota_val <= 3) {

                                if (ioctl(nrf_file, NRF_IOCTL_SET_POWER, &ota_val) < 0) {
                                    perror("OTA power setup IOCTL error");
                                } 
                                else {
                                    printf("\r[OTA]: Power changed: %d dBm.\n", ((ota_val * 6) - 18));
                                }
                            }
                        }
                    }
                    else{
                        printf("\rReceived: %s\n", buffer);
                    }

                    
                }

                else{

                    if (sent_key == false) {
                        printf("\rGarbage received. Type /connect to send your key and start communication\n");
                    } 
                    else {
                        printf("\rGarbage received. Key was not received from another side\n");
                    }
                }
            }
        }

    }


    close(nrf_file);
    return 0;
}