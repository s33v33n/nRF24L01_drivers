#ifndef NRF_PACKET_H
#define NRF_PACKET_H

#include <stdint.h>
#include <stddef.h>

int send_packet(int fd, uint8_t sender_id, const char *message);
int receive_packet(int fd, uint8_t *out_sender_id, char *out_message, size_t max_len);

#endif // NRF_PACKET_H