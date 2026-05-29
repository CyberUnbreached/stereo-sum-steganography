# Stereo Sum Embedding for Audio Steganography

A command-line steganography utility developed for **CS 4463 - Steganography (Summer 2025)** that hides and extracts arbitrary files within 16-bit stereo WAV audio files using a custom **Stereo Sum Embedding** technique.

Unlike traditional Least Significant Bit (LSB) steganography, which directly modifies sample bits, this project embeds information into the least significant bits of the **sum of stereo sample blocks**, allowing for configurable tradeoffs between payload capacity, audio transparency, and extraction reliability.

---

## Features

* Hide arbitrary files inside stereo WAV audio files
* Extract hidden files from stego WAV files
* Supports multiple payload types:

  * Text files (`.txt`)
  * Bitmap images (`.bmp`)
  * PNG images (`.png`)
  * WAV audio files (`.wav`)
  * Compressed archives (`.zip`)
* Adjustable embedding parameters:

  * Bits per block (`-b`)
  * Stereo sample pairs per block (`-n`)
* WAV file validation
* SHA256 integrity verification support
* Command-line interface
* Configurable balance between capacity and audio quality

---

## Repository Structure

```text
.
├── main.c
├── hide.c
├── extract.c
├── wav_io.c
├── wav_io.h
├── hide.h
├── extract.h
│
├── stego.exe
├── Sha256.exe
├── wbh.exe
│
├── Documentation.txt
├── Steg.BlockDiagram.pdf
├── 2025_08_CS4463_Team_03_Project_M3.pdf
└── README.md
```

---

## How It Works

### Embedding

1. Validate the input WAV file.
2. Read the message file into memory.
3. Prepend a 4-byte message length header.
4. Divide stereo audio samples into blocks.
5. Compute the sum of samples in each block.
6. Embed message bits into the least significant bits of the block sum.
7. Make minimal sample adjustments when necessary.
8. Write the modified audio to a new WAV file.

### Extraction

1. Validate the stego WAV file.
2. Read audio sample blocks.
3. Compute each block's sample sum.
4. Extract embedded bits from the least significant bits of the sum.
5. Recover the original message length from the header.
6. Reconstruct and write the extracted file.

---

## WAV Requirements

Input cover files must be:

* PCM WAV format
* Stereo (2 channels)
* 16-bit samples
* 44.1 kHz or 48 kHz sample rate

Unsupported WAV formats will be rejected.

---

## Compilation

Compile using GCC:

```bash
gcc -o stego.exe main.c hide.c extract.c wav_io.c
```

You may use another compatible C compiler if preferred.

---

## Usage

### Hide a File

```bash
stego.exe -hide -m Secret.txt -c Cover.wav -o Stego.wav -b 8 -n 4
```

#### Parameters

| Option | Description                   |
| ------ | ----------------------------- |
| `-m`   | Message file to embed         |
| `-c`   | Cover WAV file                |
| `-o`   | Output WAV file               |
| `-b`   | Bits per block (1–16)         |
| `-n`   | Stereo sample pairs per block |

---

### Extract a File

```bash
stego.exe -extract -s Stego.wav -o Recovered.txt -b 8 -n 4
```

#### Parameters

| Option | Description                                         |
| ------ | --------------------------------------------------- |
| `-s`   | Stego WAV file                                      |
| `-o`   | Recovered output file                               |
| `-b`   | Bits per block used during embedding                |
| `-n`   | Stereo sample pairs per block used during embedding |

---

## Recommended Settings

### Best Overall Configuration

```bash
-b 8 -n 4
```

This configuration consistently provided the best balance between:

* Payload capacity
* Audio transparency
* Recovery accuracy

During testing, embedded files were recovered successfully while audio distortion remained largely imperceptible.

---

## Example Workflow

### Embed a Secret Text File

```bash
stego.exe -hide -m Secret.txt -c Music.wav -o SecretMusic.wav -b 8 -n 4
```

### Recover the File

```bash
stego.exe -extract -s SecretMusic.wav -o Recovered.txt -b 8 -n 4
```

### Verify Integrity

```bash
Sha256.exe Secret.txt
Sha256.exe Recovered.txt
```

If both hashes match, the embedded file was recovered successfully without corruption.

---

## Known Limitations

### Capacity Constraints

The cover WAV file must contain enough blocks to store the payload. Large files may exceed available capacity.

### WAV-in-WAV Embedding

Embedding WAV files inside another WAV file is supported but may result in truncated recovery if the cover file lacks sufficient capacity.

### High Embedding Depths

Embedding beyond 8 bits per block can introduce:

* Audible distortion
* Extraction failures
* Corrupted payloads

### Sample Range Constraints

If the required block adjustment exceeds the legal 16-bit audio range (-32768 to 32767), some blocks may not be modified successfully.

---

## Testing Results Summary

| Configuration | Audio Quality          | Recovery Success |
| ------------- | ---------------------- | ---------------- |
| 4 bits/block  | Excellent              | Excellent        |
| 8 bits/block  | Excellent              | Excellent        |
| 10 bits/block | Noticeable distortion  | Mixed            |
| 12 bits/block | Significant distortion | Poor             |
| 16 bits/block | Severe distortion      | Poor             |

Based on testing, 8 bits per block was found to be the practical upper limit for reliable and transparent embedding.

---

## Future Improvements

Potential future enhancements include:

* Automatic capacity checking before embedding
* Graphical User Interface (GUI)
* Batch embedding and extraction support
* Payload encryption
* Adaptive embedding strategies
* Support for additional audio formats
* Improved robustness against audio processing operations

---

## Authors

### Team 03

**Ian Rohan**

* Primary software developer
* Implemented embedding and extraction functionality
* Created and tested payload files

**Khristian Neal**

* Project lead
* Testing
* Audio sample preparation
* Report contributions

**Quinn Westerberg**

* Testing
* Functional block diagram creation
* Report contributions

---

## Academic Information

Created for:

**CS 4463 - Steganography**
Summer 2025

University of Texas at San Antonio
