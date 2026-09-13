#include <stdint.h>
#include <stddef.h>
#include "Chacha20.h"

void combine_chacha20_block_and_message(
    uint32_t prepared_chacha_block[CHACHA_BLOCKS], 
    unsigned char *message, 
    size_t length,
    unsigned char *encrypted_message    
    ){

        unsigned char *keystream_byte = (unsigned char *)prepared_chacha_block;

        for(int i = 0; i < length; i++){
            encrypted_message[i] = message[i] ^ keystream_byte[i]; 
        }
}

void generate_chacha20_block(
    const uint32_t key[KEY_BLOCKS],  
    const uint32_t counter, 
    uint32_t nonce[NONCE_BLOCKS],
    uint32_t output_block[CHACHA_BLOCKS]
    ){

        // before rounds
        uint32_t chacha_block[CHACHA_BLOCKS] = {0};

        // after rounds
        uint32_t working_chacha_block[CHACHA_BLOCKS] = {0};

        // 1. Constans
        chacha_block[0] = 0x61707865;
        chacha_block[1] = 0x3320646E;
        chacha_block[2] = 0x79622D32;
        chacha_block[3] = 0x6B206574;

        // 2. Key 
        for(int i=4; i < 12; i++){
            chacha_block[i] = key[i - 4];
        }

        // 3. Counter
        chacha_block[12] = counter;

        // 4. Nonce 
        for(int i=13; i < 16; i++){
            chacha_block[i] = nonce[i - 13];
        }

        // 4,5. Copy blocks
        for(int i=0; i < CHACHA_BLOCKS; i++){
            working_chacha_block[i] = chacha_block[i];
        }

        // 5. Rounds
        for(int i=0; i < ROUNDS; i++){
            
            // Column round
            QR(working_chacha_block[0], working_chacha_block[4], working_chacha_block[8], working_chacha_block[12]);
            QR(working_chacha_block[1], working_chacha_block[5], working_chacha_block[9], working_chacha_block[13]);
            QR(working_chacha_block[2], working_chacha_block[6], working_chacha_block[10], working_chacha_block[14]);
            QR(working_chacha_block[3], working_chacha_block[7], working_chacha_block[11], working_chacha_block[15]);
            
            // Diagonal Round
            QR(working_chacha_block[0], working_chacha_block[5], working_chacha_block[10], working_chacha_block[15]);
            QR(working_chacha_block[1], working_chacha_block[6], working_chacha_block[11], working_chacha_block[12]);
            QR(working_chacha_block[2], working_chacha_block[7], working_chacha_block[8], working_chacha_block[13]);
            QR(working_chacha_block[3], working_chacha_block[4], working_chacha_block[9], working_chacha_block[14]);
        }

        // 6. Output 
        for(int i=0; i < CHACHA_BLOCKS; i++){
            output_block[i] = chacha_block[i] + working_chacha_block[i];
        }  
}