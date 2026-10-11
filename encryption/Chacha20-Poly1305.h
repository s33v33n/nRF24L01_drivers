#ifndef CHACHA20_POLY1305_H
#define CHACHA20_POLY1305_H

#include <stdint.h>
#include <stddef.h>

#define NONCE_BLOCKS    3
#define KEY_BLOCKS      8
#define ROUNDS          10
#define CHACHA_BLOCKS   16
#define POLY_BLOCKS     16
#define MESSAGE_LENGTH  32

#define ROTATE_INT_32(val, n) (((val) << (n)) | ((val) >> (32 - (n))))

#define QR(a, b, c, d) \
    a += b; d ^= a; d = ROTATE_INT_32(d, 16); \
    c += d; b ^= c; b = ROTATE_INT_32(b, 12); \
    a += b; d ^= a; d = ROTATE_INT_32(d, 8);  \
    c += d; b ^= c; b = ROTATE_INT_32(b, 7); 

void combine_chacha20_block_and_message(
    uint32_t prepared_chacha_block[CHACHA_BLOCKS], 
    const unsigned char *message, 
    size_t length,
    unsigned char *encrypted_message   
);   

void generate_chacha20_block(
    const uint32_t key[KEY_BLOCKS],  
    const uint32_t counter, 
    const uint32_t nonce[NONCE_BLOCKS],
    uint32_t output_block[CHACHA_BLOCKS]
);

void poly1305(
    uint32_t chacha_block[CHACHA_BLOCKS],
    const unsigned char *message,
    size_t length,
    unsigned char authenticate[POLY_BLOCKS]
);

int verify_tag(const unsigned char *tag1, const unsigned char *tag2);

#endif // CHACHA20_POLY1305_H
