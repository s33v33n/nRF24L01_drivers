#ifndef NRF_FRAMEWORK_H
#define NRF_FRAMEWORK_H

#include <stdint.h>
#include <stddef.h>

// file
int nrf_init(void);
int nrf_close(void);

// RF communication
int nrf_master_send(uint8_t target_node_id, char *message);
int nrf_node_send(char *message);
int nrf_receive(uint8_t *sender_id, char *out_message, size_t max_len);

// dynamic configuration
int nrf_set_channel(uint8_t channel);
int nrf_set_power(uint8_t power);
int nrf_set_speed(uint8_t speed);

// diagnostics
int nrf_get_status(uint8_t *status);
int nrf_get_channel(uint8_t *channel);
int nrf_get_rf_setup(uint8_t *setup);

#endif // NRF_FRAMEWORK_H