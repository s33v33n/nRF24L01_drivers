#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/nrf_ioctl.h"
#include "../include/nrf_network_user_config.h"
#include "../encryption/Chacha20-Poly1305.h"

#ifndef NODE_ID
    #define NODE_ID 0      
#endif
#define MY_NODE_ID NODE_ID  

#ifndef MASTER_ID
    #define MASTER_ID 0    
#endif
#define MY_MASTER_ID MASTER_ID

#define ALL_DEVICES (sizeof(nrf_nodes) / sizeof((nrf_nodes)[0]))

static struct nrf_node_config master_config = {0};
static struct nrf_node_config node_config = {0};
int nrf_fd = -1;

static uint32_t my_tx_counter = 1;
static uint32_t last_rx_counter[ALL_DEVICES] = {0};

int nrf_init(){

    // find master config
    int ret = 1;
    for(int i=0; i < (sizeof(nrf_nodes)/sizeof(nrf_nodes[0])); i++){
        if(nrf_nodes[i].id == MY_MASTER_ID){
            master_config = nrf_nodes[i];
            ret = 0;
        }
    }

    if(ret){
        fprintf(stderr, "ERROR: Master ID=%d does not exist in network_config!\n", MY_MASTER_ID);
        return -1;
    }

    //find node number for init
    ret = 1;
    for(int i=0; i < ALL_DEVICES; i++){
        if(nrf_nodes[i].id == MY_NODE_ID){
            node_config = nrf_nodes[i];
            ret = 0;
        }
    }
    if(ret){
        fprintf(stderr, "ERROR: Node ID=%d does not exist in network_config!\n", MY_NODE_ID);
        return -1;
    }

    //------ configure node (master or slave) ------

    char *dev_file = NULL;
    if (asprintf(&dev_file, "/dev/%s", node_config.name) == -1) {
        fprintf(stderr, "Not enough memory for device allocation!\n");
        return -1;
    }

    // device open
    nrf_fd = open(dev_file, O_RDWR);
    if (nrf_fd < 0) {
        fprintf(stderr, "ERROR: cannot open%s.\n", dev_file);
        free(dev_file);
        return -1;
    }

    free(dev_file);

    /* IOCTL module configuration */

    struct nrf_pipe_config rx_pipe_config ={
        .pipe_num = node_config.rx_pipe_num,
        .address = node_config.rx_pipe_addr
    };

    // RX address
    if(ioctl(nrf_fd, NRF_IOCTL_SET_RX_ADDR, &rx_pipe_config) < 0){
        perror("IOCTL: cannot write rx pipe config");
        return -1;
    }

    // TX address
    if(ioctl(nrf_fd, NRF_IOCTL_SET_TX_ADDR, &master_config.rx_pipe_addr) < 0){
        perror("IOCTL: cannot write tx adderss");
        return -1;
    }

    // RF channel
    if (ioctl(nrf_fd, NRF_IOCTL_SET_CHANNEL, &node_config.channel) < 0) {
        perror("IOCTL: cannot set rf channel");
        return -1;
    }
    
    // RF power
    if (ioctl(nrf_fd, NRF_IOCTL_SET_POWER, &node_config.power) < 0) {
        perror("IOCTL: cannot set power level");
        return -1;
    }
    
    // RF speed
    if (ioctl(nrf_fd, NRF_IOCTL_SET_SPEED, &node_config.speed) < 0) {
        perror("IOCTL: cannot set speed level");
        return -1;
    }

    printf("Module %s init successfully\n",node_config.name);
    
    return 0;
}

int nrf_master_send(uint8_t target_node_id, char *message){

    if(nrf_fd < 0 || message == NULL){
        return -1;
    }

    // find node ID
    uint64_t target_addr = 0;
    uint8_t target_channel = 0;
    uint8_t target_speed = 0;
    bool found = false;
    for(size_t i=0; i < ALL_DEVICES; i++){
        if(nrf_nodes[i].id == target_node_id){
            target_addr = nrf_nodes[i].rx_pipe_addr;
            target_channel = nrf_nodes[i].channel;
            target_speed = nrf_nodes[i].speed;
            found = true;
            break;
        }
    }

    if(!found) {
        fprintf(stderr, "ERROR: Target node ID=%d not found in config!\n", target_node_id);
        return -1;
    }

    if(ioctl(nrf_fd, NRF_IOCTL_SET_TX_ADDR, &target_addr) < 0) {
        perror("IOCTL: Failed to set TX address");
        return -1;
    }

    if(ioctl(nrf_fd, NRF_IOCTL_SET_CHANNEL, &target_channel) < 0) {
        perror("IOCTL: Failed to set TX RF channel");
        return -1;
    }

    if(ioctl(nrf_fd, NRF_IOCTL_SET_SPEED, &target_speed) < 0) {
        perror("IOCTL: Failed to set TX speed");
        return -1;
    }

    // Send packet
    uint8_t package_to_send[32] = {0};
    uint32_t current_counter = my_tx_counter++;

    // counter value
    package_to_send[0] = (uint8_t)(current_counter);
    package_to_send[1] = (uint8_t)(current_counter >> 8);
    package_to_send[2] = (uint8_t)(current_counter >> 16);
    package_to_send[3] = (uint8_t)(current_counter >> 24);

    // node ID
    package_to_send[4] = master_config.id;

    size_t msg_len = strlen(message);
    if (msg_len > 11) {
        msg_len = 11;
        fprintf(stderr, "Warning: Message exceeds 11 chars (got %zu), truncating!\n", msg_len);
    }

    const uint32_t nonce[NONCE_BLOCKS] = {0, 0, current_counter};
    uint32_t chacha_block[CHACHA_BLOCKS] = {0};

    generate_chacha20_block(NRF_NETWORK_KEY, 0, nonce, chacha_block);

    uint8_t *keystream = (uint8_t *)chacha_block;
    for (size_t i = 0; i < msg_len; i++) {
        package_to_send[5 + i] = (uint8_t)message[i] ^ keystream[32 + i];
    }

    uint8_t tag[POLY_BLOCKS] = {0};
    poly1305(chacha_block, package_to_send, 16, tag);
    memcpy(&package_to_send[16], tag, POLY_BLOCKS);

    ssize_t bytes_sent = write(nrf_fd, package_to_send, sizeof(package_to_send));
    if (bytes_sent != (ssize_t)sizeof(package_to_send)) {
        perror("write: Failed to send packet");
        return -1;
    }

    return 0;
}

int nrf_node_send(char *message){

}

int nrf_receive(){

}

int nrf_close() {
    if (nrf_fd >= 0) {
        close(nrf_fd); 
        nrf_fd = -1;
    }

    return 0;
}

