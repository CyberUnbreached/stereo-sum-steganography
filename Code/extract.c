// Coded by Ian Rohan
// extract.c
// This file extracts a hidden message from a WAV file.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "wav_io.h"

// Extract hidden message from WAV file
int extract_message(int16_t *audioData, FILE *output_file, uint32_t dataSize, int bits_per_block, int samples_per_block) {
    // Estimate maximum message size and allocate buffer
    uint32_t max_bytes = dataSize / 8;
    unsigned char *bit_buffer = (unsigned char *)calloc(max_bytes, 1);  // Zero-initialized
    if (!bit_buffer) {
        printf("Error: Memory allocation failed.\n");
        return 0;
    }

    int bit_index = 0;
    uint32_t extracted_length = 0;
    int length_known = 0;

    uint32_t total_pairs = dataSize / sizeof(int16_t) / 2;
    uint32_t blocks = total_pairs / samples_per_block;

    for (uint32_t i = 0; i < blocks; i++) {
        int32_t block_sum = 0;

        for (int j = 0; j < samples_per_block; j++) {
            int index = (i * samples_per_block + j) * 2;
            int16_t left = audioData[index];
            int16_t right = audioData[index + 1];
            block_sum += left + right;
        }

        unsigned char message_bits = (unsigned char)(block_sum & ((1 << bits_per_block) - 1));

        for (int b = 0; b < bits_per_block && (bit_index / 8) < max_bytes; b++) {
            int bit = (message_bits >> b) & 1;
            bit_buffer[bit_index / 8] |= (bit << (bit_index % 8));
            bit_index++;
        }

        // Decode message length after first 32 bits
        if (!length_known && bit_index >= 32) {
            memcpy(&extracted_length, bit_buffer, 4);  // First 4 bytes = length
            length_known = 1;

            // Sanity check
            if (extracted_length == 0 || extracted_length > max_bytes - 4) {
                printf("Error: Invalid or corrupted message length (%u bytes).\n", extracted_length);
                free(bit_buffer);
                return 0;
            }
        }

        // Stop early if full message extracted
        if (length_known && bit_index >= (extracted_length + 4) * 8) {
            break;
        }
    }

    if (!length_known) {
        printf("Error: Could not extract message length.\n");
        free(bit_buffer);
        return 0;
    }

    printf("Decoded message length: %u bytes\n", extracted_length);
    fwrite(bit_buffer + 4, 1, extracted_length, output_file);
    printf("Message successfully extracted (%u bytes written).\n", extracted_length);

    free(bit_buffer);
    return 1;
}

