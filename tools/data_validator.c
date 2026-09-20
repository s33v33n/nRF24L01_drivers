#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../network_config.h"


int main(){

    if(sizeof(nrf_nodes) == 0){
        printf("Nodes were not added to the network\n");
        return -1;
    }

    int nodes_counter = sizeof(nrf_nodes) / sizeof(nrf_nodes[0]);

    for(int i=0; i < nodes_counter; i++){

        // check name
        char *uniqe_name = nrf_nodes[i].name;
        for(int j=i+1; j < nodes_counter; j++){
            if(strcmp(uniqe_name, nrf_nodes[j].name) == 0){
                printf("The same name for nodes %d and %d\n", i, j);
                return -1;
            }
        }

        // check ID 
        uint8_t uniqe_id = nrf_nodes[i].id;
        for(int j=i+1; j < nodes_counter; j++){
            if(uniqe_id == nrf_nodes[j].id){
                printf("The same ID for nodes %d and %d\n", i, j);
                return -1;
            }
        }

        // check address 
        uint64_t uniqe_addr = nrf_nodes[i].address;
        for(int j=i+1; j < nodes_counter; j++){
            if(uniqe_addr == nrf_nodes[j].address){
                printf("The same address for nodes %d and %d\n", i, j);
                return -1;
            }
        }

        // check channel 
        uint8_t channel = nrf_nodes[i].channel;
        if(channel < 1 || 124 < channel){
            printf("Node: %d error - enter channel as uint from range <1, 124>, current channel: %d\n",i, channel);
            return -1;
        }

        // check power 
        uint8_t power = nrf_nodes[i].power;
        if(3 < power){
            printf("Node: %d error - enter power as uint from range <0, 3>, current power: %d\n",i, power);
            return -1;
        }

        // check speed
        uint8_t speed = nrf_nodes[i].speed;
        if(speed != 0 && speed != 1){
            printf("Node: %d error - enter speed as uint from range <0, 1>, current speed: %d\n",i, speed);
            return -1;
        }
    }

    printf("User's data are correct\n");
    return 0;
}