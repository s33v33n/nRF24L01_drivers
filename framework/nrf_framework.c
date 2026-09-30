#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>

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

static struct nrf_node_config master_config = {0};
static struct nrf_node_config node_config = {0};
int nrf_fd = -1;

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
    for(int i=0; i < (sizeof(nrf_nodes)/sizeof(nrf_nodes[0])); i++){
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

int nrf_send(){

}

int nrf_receive(){

}

int nrf_send_and_receive(){

}

int nrf_close(){

}

