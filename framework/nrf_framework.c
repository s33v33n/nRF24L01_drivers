#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <stdlib.h>
#include "network_config.h"
#include "Chacha20-Poly1305.h"
#include "nrf_ioctl.h"

#ifdef NODE_ID
    #define MY_NODE_ID NODE_ID  
#else
    #error "Compilation error! Define node id: -DNODE_ID=X!" 
#endif

#ifdef MASTER_ID
    #define MY_MASTER_ID MASTER_ID  
#else
    #error "Compilation error! Define master id: -DMASTER_ID=X!" 
#endif

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

    //TODO 

    /* IOCTL module configuration */

}

int nrf_send(){

}

int nrf_receive(){

}

int nrf_send_and_receive(){

}

int nrf_close(){

}

