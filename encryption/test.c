#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "Chacha20-Poly1305.h"

int main(){

    const uint32_t key[KEY_BLOCKS] = {0x7F2A9C81, 0x4B3D5E6F, 0x1A2B3C4D, 0x9E8F7A6B, 0x5C4D3E2F, 0x18293A4B, 0xD4C3B2A1, 0xF5E4D3C2};  
    const uint32_t counter = 0; 
    uint32_t nonce[NONCE_BLOCKS] = {0x11111111, 0x22222222, 0x33333333};
    uint32_t output_block[CHACHA_BLOCKS] = {0};
    const unsigned char *my_message = "Encrypted data";

    size_t len = strlen(my_message);

    printf("Original message:");
    for(size_t i=0; i < len; i++){
        printf("%c",my_message[i]);
    }

    uint32_t prepared_block[CHACHA_BLOCKS] = {0};

    unsigned char *encrypted_message = malloc(len + 1);
    encrypted_message[len] = '\0';
    memset(encrypted_message, 0, len);

    generate_chacha20_block(key, counter, nonce, prepared_block);
    combine_chacha20_block_and_message(prepared_block, my_message, len, encrypted_message);

    printf("\nEncrypted message: ");
    for(size_t i=0; i < len; i++){
        printf("%c",encrypted_message[i]);
    }

    unsigned char *decrypted_message = malloc(len + 1);
    decrypted_message[len] = '\0';
    memset(decrypted_message, 0, len);

    combine_chacha20_block_and_message(prepared_block, encrypted_message, len, decrypted_message);
    
    printf("\nDecrypted message: ");
    for(size_t i=0; i < len; i++){
        printf("%c",decrypted_message[i]);
    }

    printf("\n");

    return 0;
}