// Coded by Ian Rohan
// wav_io.c
// This file contains functions for reading and writing WAV files.

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define WAV_HEADER_SIZE 44  // Standard WAV header size

// WAV header structure for the first 44 bytes (simplified)
typedef struct {
    char riff[4];         // "RIFF" string
    uint32_t chunkSize;   // Size of the remaining chunk (excluding the first 8 bytes)
    char wave[4];         // "WAVE" string
    char fmt[4];          // "fmt " string (note the space after 'fmt')
    uint32_t subChunk1Size;
    uint16_t audioFormat; // Should be 1 for PCM
    uint16_t numChannels; // Should be 2 for stereo
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample; // Should be 16 for 16-bit samples
    char data[4];          // "data" string
    uint32_t dataSize;     // Size of the audio data
} WavHeader;

// Function to validate if a file is a valid 16-bit stereo WAV file
int validate_wav_file(FILE *file) {
    WavHeader header;
    
    // Read the first 44 bytes (WAV header)
    size_t bytesRead = fread(&header, 1, WAV_HEADER_SIZE, file);
    if (bytesRead < WAV_HEADER_SIZE) {
        printf("Error: Failed to read WAV header.\n");
        return 0;  // Invalid WAV file
    }
    
    // Check "RIFF" header
    if (strncmp(header.riff, "RIFF", 4) != 0) {
        printf("Error: Not a valid RIFF file.\n");
        return 0;
    }
    
    // Check "WAVE" format
    if (strncmp(header.wave, "WAVE", 4) != 0) {
        printf("Error: Not a valid WAVE file.\n");
        return 0;
    }
    
    // Check "fmt " chunk
    if (strncmp(header.fmt, "fmt ", 4) != 0) {
        printf("Error: Invalid format chunk.\n");
        return 0;
    }
    
    // Check audio format (should be PCM, which is 1)
    if (header.audioFormat != 1) {
        printf("Error: Only PCM audio is supported.\n");
        return 0;
    }
    
    // Check number of channels (should be 2 for stereo)
    if (header.numChannels != 2) {
        printf("Error: Only stereo (2 channels) is supported.\n");
        return 0;
    }
    
    // Check sample rate (common values are 44100 or 48000, for example)
    if (header.sampleRate != 44100 && header.sampleRate != 48000) {
        printf("Error: Unsupported sample rate.\n");
        return 0;
    }
    
    // Check bits per sample (should be 16-bit)
    if (header.bitsPerSample != 16) {
        printf("Error: Only 16-bit audio is supported.\n");
        return 0;
    }
    
    // If all checks passed, it's a valid 16-bit stereo WAV file
    return 1;
}

// Function to read audio data from the WAV file
int read_wav_data(FILE *file, int16_t **audioData, uint32_t *dataSize) {
    char chunk_id[4];
    uint32_t chunk_size;

    // Start at byte 12: skip RIFF header + WAVE
    fseek(file, 12, SEEK_SET);

    while (fread(chunk_id, 1, 4, file) == 4) {
        if (fread(&chunk_size, sizeof(uint32_t), 1, file) != 1) {
            printf("Error: Failed to read chunk size.\n");
            return 0;
        }

        if (strncmp(chunk_id, "data", 4) == 0) {
            *dataSize = chunk_size;
            *audioData = (int16_t *)malloc(chunk_size);
            if (*audioData == NULL) {
                printf("Error: Memory allocation failed.\n");
                return 0;
            }

            if (fread(*audioData, 1, chunk_size, file) != chunk_size) {
                printf("Error: Failed to read audio data.\n");
                free(*audioData);
                return 0;
            }

            return 1;
        } else {
            // Skip over this chunk
            fseek(file, chunk_size, SEEK_CUR);
        }
    }

    printf("Error: 'data' chunk not found in WAV file.\n");
    return 0;
}


// Function to write audio data to the WAV file (new function)
int write_wav_data(FILE *output_wav, int16_t *audioData, uint32_t dataSize) {
    // Create and write the header
    WavHeader header = {0};
    strncpy(header.riff, "RIFF", 4);
    strncpy(header.wave, "WAVE", 4);
    strncpy(header.fmt, "fmt ", 4);
    header.subChunk1Size = 16;  // PCM format
    header.audioFormat = 1;     // PCM format
    header.numChannels = 2;     // Stereo
    header.sampleRate = 44100;  // Standard rate
    header.byteRate = header.sampleRate * header.numChannels * 2; // 16-bit samples
    header.blockAlign = header.numChannels * 2; // 16-bit samples
    header.bitsPerSample = 16; // 16-bit samples
    strncpy(header.data, "data", 4);
    header.dataSize = dataSize;

    // Write header
    if (fwrite(&header, sizeof(WavHeader), 1, output_wav) != 1) {
        printf("Error: Failed to write WAV header.\n");
        return 0;
    }

    // Write audio data
    if (fwrite(audioData, sizeof(int16_t), dataSize / sizeof(int16_t), output_wav) != dataSize / sizeof(int16_t)) {
        printf("Error: Failed to write audio data.\n");
        return 0;
    }

    return 1;
}
