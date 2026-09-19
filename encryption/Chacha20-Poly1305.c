#include <stdint.h>
#include <stddef.h>
#include "Chacha20-Poly1305.h"

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
        for (int i = 0; i < 3; i++) {
            chacha_block[13 + i] = nonce[i];
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

void poly1305(
    uint32_t chacha_block[CHACHA_BLOCKS],
    unsigned char *message, 
    size_t length,
    unsigned char authenticate[POLY_BLOCKS]
    ){

        uint32_t r[4] = {0};
        uint32_t s[4] = {0};

        // 1. Init value 
        for(int i=0; i < 4; i++){
            r[i] = chacha_block[i];
            s[i] = chacha_block[i + 4];
        }

        // 2. 16-byte block 
        unsigned char block[17] = {0};
        size_t bytes = 0;
        for(bytes; bytes < length; bytes++){
            block[bytes] = message[bytes];
        }
        block[bytes] = 0x01;

        // 3. Clamp function
        r[0] &= 0x0fffffff;
        r[1] &= 0x0ffffffc;
        r[2] &= 0x0ffffffc;
        r[3] &= 0x0ffffffc;

        // 4. poly init
        uint32_t h[5] = {0};

        // 5. My message init
        uint32_t m[5] = {0};

        m[0] = (uint32_t)block[0] | ((uint32_t)block[1] << 8) | ((uint32_t)block[2] << 16) | ((uint32_t)block[3] << 24);
        m[1] = (uint32_t)block[4] | ((uint32_t)block[5] << 8) | ((uint32_t)block[6] << 16) | ((uint32_t)block[7] << 24);
        m[2] = (uint32_t)block[8] | ((uint32_t)block[9] << 8) | ((uint32_t)block[10] << 16) | ((uint32_t)block[11] << 24);
        m[3] = (uint32_t)block[12] | ((uint32_t)block[13] << 8) | ((uint32_t)block[14] << 16) | ((uint32_t)block[15] << 24);
        m[4] = (uint32_t)block[16];

        // 6. h = h + m (with carry)
        uint64_t carry = 0;
        for(int i=0; i < 4;i++){
            uint64_t tmp = (uint64_t)h[i] + m[i] + carry;
            h[i] = (uint32_t)tmp; 
            carry = tmp >> 32;
        }
        h[4] = m[4] + (uint32_t)carry;

        // 7. (h * r) % (2^130 - 5)
        uint32_t product[9] = {0};

        for(int i = 0; i < 5; i++){

            uint64_t carry_mul = 0;
            for(int j = 0; j < 4; j++){

                uint64_t tmp = (uint64_t)h[i] * r[j] + product[i + j] + carry_mul;
                product[i + j] = (uint32_t)tmp;
                carry_mul = tmp >> 32;
            }

            int k = i + 4;
            while(carry_mul != 0){

                uint64_t tmp = (uint64_t)product[k] + carry_mul;
                product[k] = (uint32_t)tmp;
                carry_mul = tmp >> 32;
                k++;
            }
        }

        uint32_t low[5];
        uint32_t high[5];
        uint32_t reduced[5];

        // Low 130 bits
        low[0] = product[0];
        low[1] = product[1];
        low[2] = product[2];
        low[3] = product[3];
        low[4] = product[4] & 0x00000003;

        // product >> 130
        high[0] = (product[4] >> 2) | (product[5] << 30);
        high[1] = (product[5] >> 2) | (product[6] << 30);
        high[2] = (product[6] >> 2) | (product[7] << 30);
        high[3] = (product[7] >> 2) | (product[8] << 30);
        high[4] = product[8] >> 2;

        // low + high * 5
        carry = 0;
        for(int i = 0; i < 5; i++){

            uint64_t tmp = (uint64_t)low[i] + (uint64_t)high[i] * 5 + carry;
            reduced[i] = (uint32_t)tmp;
            carry = tmp >> 32;
        }

        h[0] = reduced[0];
        h[1] = reduced[1];
        h[2] = reduced[2];
        h[3] = reduced[3];
        h[4] = reduced[4];


        // 8. Reduce h modulo p = 2^130 - 5
        uint32_t h_test[5];
        h_test[0] = h[0] + 5;
        h_test[1] = h[1] + (h_test[0] < h[0] ? 1 : 0);
        h_test[2] = h[2] + (h_test[1] < h[1] ? 1 : 0);
        h_test[3] = h[3] + (h_test[2] < h[2] ? 1 : 0);
        h_test[4] = h[4] + (h_test[3] < h[3] ? 1 : 0);
        
        // if digit > 2^130
        if (h_test[4] >> 2) { 
            h[0] = h_test[0]; 
            h[1] = h_test[1]; 
            h[2] = h_test[2]; 
            h[3] = h_test[3]; 
            h[4] = h_test[4] & 3;
        }

        // 9. Generate final authentication tag (T = h + s) 
        uint64_t tag_sum = (uint64_t)h[0] + s[0];

        authenticate[0] = (uint8_t)tag_sum;
        authenticate[1] = (uint8_t)(tag_sum >> 8);
        authenticate[2] = (uint8_t)(tag_sum >> 16);
        authenticate[3] = (uint8_t)(tag_sum >> 24);
        tag_sum >>= 32;

        tag_sum += (uint64_t)h[1] + s[1];
        authenticate[4] = (uint8_t)tag_sum;
        authenticate[5] = (uint8_t)(tag_sum >> 8);
        authenticate[6] = (uint8_t)(tag_sum >> 16);
        authenticate[7] = (uint8_t)(tag_sum >> 24);
        tag_sum >>= 32;

        tag_sum += (uint64_t)h[2] + s[2];
        authenticate[8] = (uint8_t)tag_sum;
        authenticate[9] = (uint8_t)(tag_sum >> 8);
        authenticate[10] = (uint8_t)(tag_sum >> 16);
        authenticate[11] = (uint8_t)(tag_sum >> 24);
        tag_sum >>= 32;

        tag_sum += (uint64_t)h[3] + s[3];
        authenticate[12] = (uint8_t)tag_sum;
        authenticate[13] = (uint8_t)(tag_sum >> 8);
        authenticate[14] = (uint8_t)(tag_sum >> 16);
        authenticate[15] = (uint8_t)(tag_sum >> 24);
}