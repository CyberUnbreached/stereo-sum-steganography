Stereo Sum Embedding for Audio Steganography

A command-line steganography utility developed for CS 4463 (Summer 2025) that hides and extracts arbitrary files within 16-bit stereo WAV audio files using a custom stereo sum embedding technique.

Unlike traditional LSB methods that directly modify sample bits, this project embeds information in the least significant bits of the sum of stereo sample blocks, providing configurable tradeoffs between payload capacity and audio transparency.

Features
Hide arbitrary files inside WAV audio
Extract hidden files from stego WAV files
Supports:
Text files (.txt)
Bitmap images (.bmp)
PNG images (.png)
WAV audio files (.wav)
Compressed archives (.zip)
Adjustable embedding parameters:
Bits per block (-b)
Stereo sample pairs per block (-n)
WAV validation and format checking
SHA256 verification support
Command-line interface
Requirements

Input WAV files must be:

16-bit PCM
Stereo (2 channels)
44.1 kHz or 48 kHz sample rate
Repository Contents
main.c
hide.c
extract.c
wav_io.c
wav_io.h
hide.h
extract.h

stego.exe      # Steganography tool
Sha256.exe     # Hash verification utility
wbh.exe        # Histogram / analysis utility

Documentation.txt
Steg.BlockDiagram.pdf
Project_Report.pdf
Compilation
gcc -o stego.exe main.c hide.c extract.c wav_io.c
Usage
Hide a File
stego.exe -hide -m Secret.txt -c Cover.wav -o Stego.wav -b 8 -n 4

Parameters:

Option	Description
-m	File to embed
-c	Cover WAV file
-o	Output WAV file
-b	Bits per block
-n	Stereo sample pairs per block
Extract a File
stego.exe -extract -s Stego.wav -o Recovered.txt -b 8 -n 4

Parameters:

Option	Description
-s	Stego WAV file
-o	Recovered output file
-b	Bits per block used during embedding
-n	Stereo sample pairs per block used during embedding
Recommended Settings
-b 8 -n 4

Testing showed this configuration provides the best balance between payload capacity, audio transparency, and extraction reliability.

Known Limitations
Large payloads may exceed cover file capacity.
WAV-in-WAV embedding can result in truncated recovery.
Embedding depths greater than 8 bits per block may introduce audible distortion and increase extraction errors.
Block adjustments can occasionally fail when sample values reach 16-bit limits.
Verification

You can verify extraction integrity using SHA256 hashes:

Sha256.exe OriginalFile.txt
Sha256.exe RecoveredFile.txt

Matching hashes indicate successful recovery.

Authors

Team 03

Ian Rohan — Primary developer and implementation
Khristian Neal — Testing and audio sample preparation
Quinn Westerberg — Testing and block diagram development
Academic Context

Created for CS 4463 – Steganography (Summer 2025).
