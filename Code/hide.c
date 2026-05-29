// Coded by Ian Rohan
// hide.c
// This file hides a message in a WAV file.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "wav_io.h"
#include <string.h>

// Helper function: Adjust a block of X left and X right samples to achieve the required delta
void adjust_block_samples(int16_t *audioData, uint32_t block_start, int samples_per_block, int32_t delta) {
    // Distribute delta across left/right, avoid crossing zero, handle min/max, compensate if needed
    int total_samples = samples_per_block * 2;
    int32_t remaining = delta;

    // Try to distribute delta by increasing left and decreasing right (or vice versa)
    // Alternate between left and right, and between samples
    int direction = (delta >= 0) ? 1 : -1;
    int abs_delta = delta >= 0 ? delta : -delta;

    // Try to adjust left and right in pairs to avoid crossing zero and min/max
    for (int d = 0; d < abs_delta; d++) {
        int left_idx = block_start + (d % samples_per_block);
        int right_idx = block_start + samples_per_block + (d % samples_per_block);

        // For positive delta, prefer increasing left and decreasing right
        // For negative delta, prefer decreasing left and increasing right
        if (direction > 0) {
            // Try to increase left if not at max
            if (audioData[left_idx] < 32767) {
                audioData[left_idx]++;
                remaining -= 1;
            }
            // Try to decrease right if not at min
            else if (audioData[right_idx] > -32768) {
                audioData[right_idx]--;
                remaining -= 1;
            }
        } else {
            // Try to decrease left if not at min
            if (audioData[left_idx] > -32768) {
                audioData[left_idx]--;
                remaining += 1;
            }
            // Try to increase right if not at max
            else if (audioData[right_idx] < 32767) {
                audioData[right_idx]++;
                remaining += 1;
            }
        }
        // If neither can be adjusted, skip
        if (remaining == 0) break;
    }

    // If some delta remains, distribute across all samples, avoiding overflow/underflow and zero crossing
    if (remaining != 0) {
        for (int i = 0; i < total_samples && remaining != 0; i++) {
            int idx = block_start + i;
            int16_t val = audioData[idx];
            // Avoid crossing zero if possible
            if (direction > 0 && val < 32767 && !(val < 0 && val + 1 > 0)) {
                audioData[idx]++;
                remaining--;
            } else if (direction < 0 && val > -32768 && !(val > 0 && val - 1 < 0)) {
                audioData[idx]--;
                remaining++;
            }
        }
    }
    // If still not zero, try to adjust any sample within range (This is my last resort to fix this)
    if (remaining != 0) {
        for (int i = 0; i < total_samples && remaining != 0; i++) {
            int idx = block_start + i;
            if (direction > 0 && audioData[idx] < 32767) {
                audioData[idx]++;
                remaining--;
            } else if (direction < 0 && audioData[idx] > -32768) {
                audioData[idx]--;
                remaining++;
            }
        }
    }
}

int hide_message(FILE *input_wav, FILE *output_wav, const char *message_file, int bits_per_block, int samples_per_block) {
    int16_t *audioData;
    uint32_t dataSize;

    // Load audio data
    if (!read_wav_data(input_wav, &audioData, &dataSize)) {
        printf("Error: Failed to read audio data\n");
        return 0;
    }

    // Load message
    FILE *msg_file = fopen(message_file, "rb");
    if (!msg_file) {
        printf("Error: Failed to open message file\n");
        free(audioData);
        return 0;
    }

    fseek(msg_file, 0, SEEK_END);
    long msg_size = ftell(msg_file);
    fseek(msg_file, 0, SEEK_SET);

    uint32_t msg_len = (uint32_t)msg_size;
    unsigned char *message = (unsigned char *)malloc(msg_size + 4);
    if (!message) {
        printf("Error: Failed to allocate memory for message\n");
        free(audioData);
        fclose(msg_file);
        return 0;
    }

    // Store length in first 4 bytes (using little-endian)
    memcpy(message, &msg_len, 4);

    // Store the actual message
    fread(message + 4, 1, msg_size, msg_file);
    msg_size += 4;  // account for length header

    fclose(msg_file);

    int bit_index = 0;  // Bit offset into message

    // Step 3: Modify blocks
    uint32_t total_samples = dataSize / sizeof(int16_t);
    uint32_t blocks = total_samples / (samples_per_block * 2);

    for (uint32_t i = 0; i < blocks; i++) {
        uint32_t block_start = i * samples_per_block * 2;

        // Extract bits to embed
        uint8_t message_bits = 0;
        for (int b = 0; b < bits_per_block; b++) {
            if (bit_index < msg_size * 8) {
                uint8_t bit = (message[bit_index / 8] >> (bit_index % 8)) & 1;
                message_bits |= (bit << b);
                bit_index++;
            } else {
                break;  // No more bits to embed
            }
        }

        // Compute block sum: X left + X right
        int32_t current_sum = 0;
        for (int j = 0; j < samples_per_block; j++) {
            current_sum += audioData[block_start + j]; // left
        }
        for (int j = 0; j < samples_per_block; j++) {
            current_sum += audioData[block_start + samples_per_block + j]; // right
        }

        // Calculate the desired sum for the message bits
        int32_t target_sum = (current_sum - (current_sum % (1 << bits_per_block))) + message_bits;
        if (target_sum > current_sum + (1 << bits_per_block) / 2) target_sum -= (1 << bits_per_block);

        // Compute actual min/max possible sum for this block based on current values
        int32_t max_increase = 0, max_decrease = 0;
        for (int j = 0; j < samples_per_block * 2; j++) {
            int idx = block_start + j;
            max_increase += (32767 - audioData[idx]);
            max_decrease += (audioData[idx] - (-32768));
        }
        int32_t min_possible_sum = current_sum - max_decrease;
        int32_t max_possible_sum = current_sum + max_increase;

        // Adjust target_sum to be within range (to avoid overflow/underflow)
        while (target_sum > max_possible_sum) target_sum -= (1 << bits_per_block);
        while (target_sum < min_possible_sum) target_sum += (1 << bits_per_block);

        // Check if this is actually achievable
        if (target_sum < min_possible_sum || target_sum > max_possible_sum) {
            printf("[WARNING] Block %u: Cannot embed %d bits (target sum %d out of achievable range [%d, %d])\n",
                i, bits_per_block, target_sum, min_possible_sum, max_possible_sum);
            continue;
        }

        int32_t delta = target_sum - current_sum;
        if (delta != 0) {
            adjust_block_samples(audioData, block_start, samples_per_block, delta);
        }
    }

    // Step 4: Write modified audio
    printf("Writing WAV output to file...\n");
    if (!write_wav_data(output_wav, audioData, dataSize)) {
        printf("Error: Failed to write modified WAV data to output file.\n");
        free(audioData);
        free(message);
        return 0;
    }

    printf("Message successfully hidden in WAV file.\n");

    free(audioData);
    free(message);
    return 1;
}