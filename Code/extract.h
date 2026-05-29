// Coded by Ian Rohan
// extract.h

#ifndef EXTRACT_H
#define EXTRACT_H

#include <stdint.h>
#include <stdio.h>

int extract_message(int16_t *audioData, FILE *output_file, uint32_t dataSize, int bits_per_block, int samples_per_block);

#endif
