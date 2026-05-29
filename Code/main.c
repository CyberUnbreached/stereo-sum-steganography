// Coded by Ian Rohan
// main.c
// This file provides a command-line interface for hiding and extracting messages in WAV files.

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wav_io.h"
#include "extract.h"

// Hiding function declaration
int hide_message(FILE *input_wav, FILE *output_wav, const char *message_file, int bits_per_block, int samples_per_block);

void print_help() {
    printf("Usage: stego.exe [mode] [options]\n");
    printf("Modes:\n");
    printf("  -hide        Hide a message in a WAV file\n");
    printf("  -extract     Extract a hidden message from a WAV file\n");
    printf("\nOptions:\n");
    printf("  -m <message>   Message file to embed (for -hide)\n");
    printf("  -c <cover>     Cover WAV file (for -hide)\n");
    printf("  -s <stego>     Stego WAV file (for -extract)\n");
    printf("  -o <output>    Output file (for both -hide and -extract)\n");
    printf("  -b <bits>      Number of bits to embed per block (1-16)\n");
    printf("  -n <samples>   Number of stereo sample pairs per block (2, 4, 6, 8)\n");
    printf("\nExample:\n");
    printf("  stego.exe -hide -m message.txt -c cover.wav -o output.wav -b 8 -n 4\n");
    printf("  stego.exe -extract -s stego.wav -o recovered.txt -b 8 -n 4\n");
}

int main(int argc, char *argv[]) {
    // Check if the user provided any arguments
    if (argc < 2) {
        printf("Error: Missing mode (-hide or -extract)\n");
        print_help();
        return 1;
    }

    // Mode variables
    int hide_mode = 0;
    int extract_mode = 0;
    char *cover_file = NULL;
    char *stego_file = NULL;
    char *message_file = NULL;
    char *output_file = NULL;
    int bits_per_block = 8;
    int samples_per_block = 4;

    // Parsing command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-hide") == 0) {
            hide_mode = 1;
        }
        else if (strcmp(argv[i], "-extract") == 0) {
            extract_mode = 1;
        }
        else if (strcmp(argv[i], "-m") == 0) {
            if (i + 1 < argc) {
                message_file = argv[i + 1];
                i++; // Skip to next argument
            } else {
                printf("Error: Missing message file after -m\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 < argc) {
                cover_file = argv[i + 1];
                i++; // Skip to next argument
            } else {
                printf("Error: Missing cover file after -c\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                stego_file = argv[i + 1];
                i++; // Skip to next argument
            } else {
                printf("Error: Missing stego file after -s\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 < argc) {
                output_file = argv[i + 1];
                i++; // Skip to next argument
            } else {
                printf("Error: Missing output file after -o\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-b") == 0) {
            if (i + 1 < argc) {
                bits_per_block = atoi(argv[i + 1]);
                if (bits_per_block < 1 || bits_per_block > 16) {
                    printf("Error: Bits per block must be between 1 and 16\n");
                    return 1;
                }
                i++; // Skip to next argument
            } else {
                printf("Error: Missing bits per block after -b\n");
                return 1;
            }
        }
        else if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 < argc) {
                samples_per_block = atoi(argv[i + 1]);
                if (samples_per_block != 2 && samples_per_block != 4 && samples_per_block != 6 && samples_per_block != 8) {
                    printf("Error: Samples per block must be 2, 4, 6, or 8\n");
                    return 1;
                }
                i++; // Skip to next argument
            } else {
                printf("Error: Missing number of samples after -n\n");
                return 1;
            }
        }
        else {
            printf("Error: Unknown argument '%s'\n", argv[i]);
            print_help();
            return 1;
        }
    }

    // If no mode is selected
    if (!hide_mode && !extract_mode) {
        printf("Error: You must specify either -hide or -extract\n");
        print_help();
        return 1;
    }

    // Check the files and validate the WAV file
    FILE *wav_file = NULL;

    if (hide_mode && cover_file != NULL) {
        wav_file = fopen(cover_file, "rb");
        if (wav_file == NULL) {
            printf("Error: Unable to open cover file '%s'\n", cover_file);
            return 1;
        }
        
        // Validate the WAV file (only 16-bit stereo PCM WAV is valid)
        if (!validate_wav_file(wav_file)) {
            fclose(wav_file);
            return 1;
        }
        
        // Read audio data
        int16_t *audioData;
        uint32_t dataSize;
        if (!read_wav_data(wav_file, &audioData, &dataSize)) {
            fclose(wav_file);
            return 1;
        }
        
        printf("Cover file successfully loaded with %u bytes of audio data.\n", dataSize);

        // Open output WAV file to write the stego data
        FILE *output_wav = fopen(output_file, "wb");
        if (output_wav == NULL) {
            printf("Error: Unable to open output file '%s'\n", output_file);
            fclose(wav_file);
            return 1;
        }

        // Call the hide_message function to hide the message in the WAV file
        if (!hide_message(wav_file, output_wav, message_file, bits_per_block, samples_per_block)) {
            printf("Error: Hiding message failed\n");
        }

        // Clean up
        fclose(wav_file);
        fclose(output_wav);
    }
    else if (extract_mode && stego_file != NULL) {
        wav_file = fopen(stego_file, "rb");
        if (wav_file == NULL) {
            printf("Error: Unable to open stego file '%s'\n", stego_file);
            return 1;
        }

        // Validate the WAV file
        if (!validate_wav_file(wav_file)) {
            fclose(wav_file);
            return 1;
        }

        // Read audio data
        int16_t *audioData;
        uint32_t dataSize;
        if (!read_wav_data(wav_file, &audioData, &dataSize)) {
            fclose(wav_file);
            return 1;
        }

        printf("Stego file successfully loaded with %u bytes of audio data.\n", dataSize);
        FILE *output_fp = fopen(output_file, "wb");
        if (!output_fp) {
            printf("Error: Unable to open output file '%s'\n", output_file);
            free(audioData);
            fclose(wav_file);
            return 1;
        }

        if (!extract_message(audioData, output_fp, dataSize, bits_per_block, samples_per_block)) {
            printf("Error: Message extraction failed.\n");
        }

        fclose(output_fp);
        free(audioData);
    }

    // Clean up and close the file
    fclose(wav_file);

    return 0;
}
