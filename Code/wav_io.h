// Coded by Ian Rohan
// wav_io.h

#include <stdint.h>

#ifndef WAV_IO_H
#define WAV_IO_H

int validate_wav_file(FILE *file);
int read_wav_data(FILE *file, int16_t **audioData, uint32_t *dataSize);
int write_wav_data(FILE *output_wav, int16_t *audioData, uint32_t dataSize);

#endif
