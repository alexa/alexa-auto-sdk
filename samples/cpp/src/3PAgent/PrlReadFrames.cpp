/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

//------ Include files -------------------------------------------------------

#include <stdio.h>
#include <limits.h>
#include <cstdint>
#include <cstring>

#include "SampleApp/3PAgent/PrlAudioFile.h"
//------ Private Definitions -------------------------------------------------

/// Examples of sampling rates in AudioFile_format:
///     8000 = Narrowband
///     16000 = Wideband
///     44100 = CD sampling rate
///     48000 = "super-wideband"

/// Examples of samples per frame in AudioFile_format.
///     1   = mono
///     2   = stereo
///     > 2 = somewhat non-standard, and most editors will not handle this
///         format.

/// Examples of bits per sample in AudioFile_format
#define AUDIOFILE_16_BITS 16  ///< 16 bits per sample
#define AUDIOFILE_32_BITS 32  ///< 32 bits per sample

/// Additional internal choice for sampleType in AudioFile_format
#define AUDIOFILE_WAVE_FORMAT_UNKNOWN 0  ///< Defined by Microsoft spec

/// Bit masks for flags used in AudioFile handle structure
#define AUDIOFILE_IS_WAV (1 << 0)    ///< Bit mask for flag, file has RIFF WAVE file header
#define AUDIOFILE_IS_WRITE (1 << 1)  ///< Bit mask for flag, file is open for writing
#define AUDIOFILE_IS_LOOP (1 << 2)   ///< Bit mask for flag, looped read mode

// ===========================================
//  Bytes 0-3   4   chunk ID = "RIFF"
//  Bytes 4-7   4   chunk size, file size - 8
#define WAVFILE_RIFF_HDR_CHUNK_SIZE_OFFSET (4)  ///< Offset for RIFF chunk size
//  ------------------------------------------
//      Bytes 8-11   4  "WAVE"
//      --------------------------------------
#define WAVFILE_RIFF_DATA_OFFSET (8)  ///< Offset for RIFF data
//      Bytes 12-15  4  chunk ID = "fmt "
//      Bytes 16-19  4  fmt chunk size = 16 (or more if pad bytes)
//          ----------------------------------
//          | Bytes 20-21  2    AudioFormat
//          |       22-23  2    NumChannels
//          | Bytes 24-27  4    SampleRate
//          | Bytes 28-31  4    Byte Rate = (Sample Rate * BitsPerSample * Channels) / 8
//          | Bytes 32-33  2    BlockAlign
//          | Bytes 34-35  2    BitsPerSample
#define WAVFILE_FMT_CHUNK_SIZE (16)  ///< (nominal) size of format chunk
//          | *** Possible pad bytes here ***
//      --------------------------------------
// ===========================================

/// Acceptable RIFF file type choices
#define FILE_TYPE_WAVE 0x45564157u  ///< Little-endian, 'E', 'V', 'A', W'

/// Choices for RIFF chunk ID
#define CHUNK_ID_RIFF 0x46464952u  ///< Little-endian, 'F', 'F', I', 'R'
#define CHUNK_ID_FMT 0x20746D66u   ///< Little-endian, ' ', 't', 'm', f'
#define CHUNK_ID_DATA 0x61746164u  ///< Little-endian, 'a', 't', 'a', d'

// Predefined fields size in bytes
#define SIZE_1BYTE (1)   ///< 1 byte field size
#define SIZE_2BYTES (2)  ///< 2 bytes field size
#define SIZE_4BYTES (4)  ///< 4 bytes field size

#define BYTES_PER_CHAR 1
#define MAX_U4BYTES (4294967295UL)

/// @brief RIFF chunk definition.
typedef struct {
    long id;        ///< FourCC chunk ID
    uint32_t size;  ///< chunk size, bytes. Stored in buffer as a uint32
} RiffChunk;

#define ERROR_FILE_NAME_TOO_SHORT (1)                        ///< A three-letter extension is expected.
#define ERROR_FILE_OPEN_FAILED_RB (2)                        ///< File open for reading failed.
#define ERROR_FORMAT_UNSUPPORTED_SAMPLE_TYPE (5)             ///< We only support a very small subset of WAVE formats.
#define ERROR_FORMAT_INVALID_SAMPLING_RATE (6)               ///< Sampling rates must be positive
#define ERROR_FORMAT_INVALID_SAMPLES_PER_FRAME (7)           ///< Number of channels per frame must be positive.
#define ERROR_FORMAT_INVALID_BITS_PER_SAMPLE (8)             ///< Number of bits per sample is invalid
#define ERROR_WAVE_HEADER_UPDATE_INCONSISTENT_FILE_SIZE (9)  ///< Data chunk size is not consistent with total file size
#define ERROR_WAVE_HEADER_UPDATE_DATA_CHUNK_SEEK_FAILED (10)   ///< fseek failed during wave header update (data chunk)
#define ERROR_WAVE_HEADER_UPDATE_DATA_CHUNK_WRITE_FAILED (11)  ///< write failed during wave header update (data chunk)
#define ERROR_WAVE_HEADER_UPDATE_RIFF_CHUNK_SEEK_FAILED (12)   ///< fseek failed during wave header update (RIFF chunk)
#define ERROR_WAVE_HEADER_UPDATE_RIFF_CHUNK_WRITE_FAILED (13)  ///< write failed during wave header update (RIFF chunk)
#define ERROR_WAVE_HEADER_READ_RIFF_CHUNK (24)                 ///< read failed during wave header parsing (RIFF chunk)
#define ERROR_WAVE_HEADER_READ_RIFF_FILE_TYPE (25)     ///< read failed during wave header parsing (RIFF file type)
#define ERROR_WAVE_HEADER_READ_FORMAT_CHUNK (26)       ///< read failed during wave header parsing (format chunk)
#define ERROR_WAVE_HEADER_READ_SAMPLE_TYPE (27)        ///< read failed during wave header parsing (sample type)
#define ERROR_WAVE_HEADER_READ_SAMPLES_PER_FRAME (28)  ///< read failed during wave header parsing (number of channels)
#define ERROR_WAVE_HEADER_READ_SAMPLING_RATE (29)      ///< read failed during wave header parsing (sampling rate)
#define ERROR_WAVE_HEADER_READ_BYTES_PER_SECOND (30)   ///< read failed during wave header parsing (bytes per second)
#define ERROR_WAVE_HEADER_READ_BLOCK_ALIGN (31)        ///< read failed during wave header parsing (block align)
#define ERROR_WAVE_HEADER_READ_BITS_PER_SAMPLE (32)    ///< read failed during wave header parsing (bits per sample)
#define ERROR_WAVE_HEADER_READ_DATA_CHUNK (33)         ///< read failed during wave header parsing (data chunk)
#define ERROR_WAVE_HEADER_READ_BYTES_PER_SEC_INCONSISTENT \
    (34)                                       ///< bytes per second is inconsistent with other format fields
#define ERROR_WAVE_HEADER_READ_PAD_BYTES (35)  ///< read failed during wave header creation (pad bytes)
#define ERROR_WAVE_HEADER_READ_FSEEK_FAILED \
    (36)  ///< fseek failed during wave header parsing (rudimentary malformed header handling)
#define ERROR_PCM_FILE_LENGTH_READ_FSEEK_FAILED (37)  ///< fseek failed when reading PCM file length
#define ERROR_REWIND_FSEEK_FAILED (38)                ///< fseek failed during rewind operation
#define ERROR_WAVE_HEADER_READ_FTELL_FAILED \
    (40)                          ///< ftell failed during wave header parsing (rudimentary malformed header handling)
#define ERROR_FILE_TOO_LONG (41)  ///< file length calculation would have overflowed, indicating file is too long
#define ERROR_PCM_FILE_LENGTH_READ_FTELL_FAILED \
    (42)  ///< ftell failed during pcm file parsing (rudimentary malformed header handling)
#define ERROR_WAVE_HEADER_UPDATE_EOF_FSEEK_FAILED (44)  ///< fseek failed during wave header update (EOF)
#define ERROR_WAVE_HEADER_WRITE_METADATA (48)  ///< write failed during wave header creation (metadata chunk data)
#define ERROR_METADATA_READ_INSUFFICIENT_BUFFER_SIZE \
    (52)  ///< metadata read failed (metadata will not fit into supplied buffer)
#define ERROR_WAVE_HEADER_READ_METADATA_PAYLOAD (53)  ///< metadata read failed (chunk data could not be read)

//------ Private macros ------------------------------------------------------
//------ Private Function Declarations ---------------------------------------

static int setFlag(int flags, int mask);
static int getFlag(int flags, int mask);
static int validateFormat(AudioFile_format* format);
static int readChunkHeader(RiffChunk* buf, FILE* file);
static int readFromFile(void* buf, ssize_t size, ssize_t count, FILE* file);
static int writeToFile(void* buf, ssize_t size, ssize_t count, FILE* file);
static long getBytesPerSecond(AudioFile_format* format);
static short getBytesPerFrame(AudioFile_format* format);
static int updateWaveHeader(AudioFile_handle* file);
static int readWaveHeader(
    AudioFile_handle* file,
    long metadataChunkID,
    char metadata[],
    ssize_t sizeofBuffer,
    ssize_t* sizeRead);
static int readPcmFileLength(AudioFile_handle* file);
static int isWav(char* fileName, ssize_t length);
static int rewindFile(AudioFile_handle* file);
static void clearFileInstance(AudioFile_handle* file);
static ssize_t simpleStrnLen(const char* str, ssize_t count);

//------ Function Definitions ------------------------------------------------

/// @brief Get length of null-terminated string.
static ssize_t simpleStrnLen(const char* str, ssize_t count) {
    const char* strTerm;

    for (strTerm = str; count-- && *strTerm != '\0'; ++strTerm) /*nothing*/
        ;
    return strTerm - str;
}

/// @brief Clear structure representing AudioFile instance
static void clearFileInstance(AudioFile_handle* file) {
    // (avoid using memset)
    file->handle = NULL;
    file->format.sampleType = AUDIOFILE_WAVE_FORMAT_UNKNOWN;
    file->format.samplingRate = 0;
    file->format.samplesPerFrame = 0;
    file->format.bitsPerSample = 0;
    file->flags = 0;
    file->bytesPerFrame = 0;
    file->framesInFile = 0;
    file->framesRemaining = 0;

    return;
}

//////////////////////////////////////////////////////////////////////////////
/// @brief Check if file has a WAV extension (does not check for header).
///
/// @remark We assume the caller has checked that length >= 5.
///
/// @return One if WAV file extension detected; zero otherwise.
static int isWav(char* fileName, ssize_t length) {
    if ((fileName[length - 4] == '.') && ((fileName[length - 3] == 'W') || (fileName[length - 3] == 'w')) &&
        ((fileName[length - 2] == 'A') || (fileName[length - 2] == 'a')) &&
        ((fileName[length - 1] == 'V') || (fileName[length - 1] == 'v'))) {
        return 1;
    } else {
        return 0;
    }
}

int prlAudioFile_open(AudioFile_handle* file, char* fileName, AudioFile_format* format, ssize_t maxFileNameLength) {
    int error = 0;

    ssize_t fileName_length = simpleStrnLen(fileName, maxFileNameLength);

    clearFileInstance(file);

    if (fileName_length <= 4) {
        return ERROR_FILE_NAME_TOO_SHORT;
    }

    file->handle = fopen(fileName, "rb");

    if (file->handle == NULL) {
        return ERROR_FILE_OPEN_FAILED_RB;
    }

    if (isWav(fileName, fileName_length)) {  // Read format from the WAV header.
        file->flags = setFlag(file->flags, AUDIOFILE_IS_WAV);

        error = readWaveHeader(file, 0L, NULL, 0, NULL);

        if (error == 0) {  // Write format to supplied argument.
            *format = file->format;
        }
    } else if (format != NULL) {  // Read format from supplied argument.
        error = validateFormat(format);

        if (error == 0) {
            file->format = *format;

            error = readPcmFileLength(file);
        }
    }

    if (error) {
        fclose(file->handle);
    }

    return error;
}

ssize_t prlAudioFile_readFrames(AudioFile_handle* file, char buffer[], ssize_t numFrames) {
    if (getFlag(file->flags, AUDIOFILE_IS_WRITE) == 0) {
        ssize_t framesToRead;

        if (file->framesRemaining >= numFrames) {  // Attempt to read entire buffer from file.
            framesToRead = numFrames;
        } else {  // Attempt to read partial buffer from end of file.
            framesToRead = file->framesRemaining;
        }
        ssize_t charsPerFrame = file->bytesPerFrame / BYTES_PER_CHAR;
        ssize_t framesRead = fread(buffer, charsPerFrame, framesToRead, file->handle);

        buffer += charsPerFrame * framesRead;

        file->framesRemaining -= framesRead;

        // Deal with the remainder of the buffer.
        long framesLeft = numFrames - framesRead;

        if (framesLeft > 0) {
            if (getFlag(file->flags, AUDIOFILE_IS_LOOP) !=
                0) {  // Attempt to get rest of buffer from beginning of file.
                if (rewindFile(file) != 0) {
                    return framesRead;
                }

                // Danger of deep recursion and stack growth for short files and large buffer requests!
                framesRead += prlAudioFile_readFrames(file, buffer, framesLeft);

                framesLeft = numFrames - framesRead;
            }

            if (framesLeft > 0) {  // Clear the remaining frames in the buffer.
                ssize_t numCharLeft = charsPerFrame * framesLeft;

                // Avoid calling memset(); non-zero loop count guaranteed.
                do {
                    *buffer++ = 0;
                } while (--numCharLeft != 0);
            }
        }

        return framesRead;
    } else {
        return 0;
    }
}

int prlAudioFile_close(AudioFile_handle* file) {
    int error = 0;

    if ((getFlag(file->flags, AUDIOFILE_IS_WAV) != 0) && (getFlag(file->flags, AUDIOFILE_IS_WRITE) != 0)) {
        error = updateWaveHeader(file);
    }

    // Close file regardless of the update being successful or not.
    fclose(file->handle);

    return error;
}

//------ Private Functions ---------------------------------------------------

/// @brief Helper to set flag.
static int setFlag(int flags, int mask) {
    return flags | mask;
}

/// @brief Helper to get flag.
static int getFlag(int flags, int mask) {
    return flags & mask;
}

/// @brief Helper function to validate audio format
///
/// @return Zero if valid, non-zero if invalid.
static int validateFormat(AudioFile_format* format) {
    // We restrict the choices to PCM and floating-point.
    if ((format->sampleType != AUDIOFILE_WAVE_FORMAT_PCM) && (format->sampleType != AUDIOFILE_WAVE_FORMAT_IEEE_FLOAT)) {
        return ERROR_FORMAT_UNSUPPORTED_SAMPLE_TYPE;
    }

    // Any positive sampling rate is acceptable.
    if (format->samplingRate <= 0) {
        return ERROR_FORMAT_INVALID_SAMPLING_RATE;
    }

    // Any positive number of channels is accepted.
    if (format->samplesPerFrame <= 0) {
        return ERROR_FORMAT_INVALID_SAMPLES_PER_FRAME;
    }

    // We restrict the choices to 16 bits or 32 bits.
    // For 24-bit support, use 32-bit containers (please).
    if ((format->bitsPerSample != AUDIOFILE_16_BITS) && (format->bitsPerSample != AUDIOFILE_32_BITS)) {
        return ERROR_FORMAT_INVALID_BITS_PER_SAMPLE;
    }

    // All checks passed.
    return 0;
}

/// @brief Helper function to read a Riff chunk header from file.
/// @note This handles the fact that the size field is a uint32 in the
///       file but an long long in this program.
///
/// @return Zero for success, non-zero for failure.
static int readChunkHeader(RiffChunk* buf, FILE* file) {
    buf->id = 0;
    buf->size = 0;

    if (readFromFile(&buf->id, SIZE_4BYTES, 1, file)) {
        return 1;
    }
    if (readFromFile(&buf->size, SIZE_4BYTES, 1, file)) {
        return 2;
    }

    return 0;
}

/// @brief Helper function to verify we read all the bytes requested.
///
/// @return Zero for success, non-zero for failure.
static int readFromFile(void* buf, ssize_t size, ssize_t count, FILE* file) {
    ssize_t numOfElementsRead;

    count = (size * count) / BYTES_PER_CHAR;
    size = sizeof(char);
    numOfElementsRead = fread(buf, size, count, file);

    if (numOfElementsRead != count) {
        return 1;
    }

    return 0;
}

/// @brief Helper function to verify we wrote all the bytes requested.
///
/// @return Zero for success, non-zero for failure.
static int writeToFile(void* buf, ssize_t size, ssize_t count, FILE* file) {
    ssize_t numOfElementsWritten;

    count = (size * count) / BYTES_PER_CHAR;
    size = sizeof(char);
    numOfElementsWritten = fwrite(buf, size, count, file);

    if (numOfElementsWritten != count) {
        return 1;
    }

    return 0;
}

/// @brief Get bytes per second from format.
static long getBytesPerSecond(AudioFile_format* format) {
    return (format->samplingRate * format->bitsPerSample * format->samplesPerFrame) / 8;
}

/// @brief Helper function for computing bytes per frame.
static short getBytesPerFrame(AudioFile_format* format) {
    return (format->bitsPerSample * format->samplesPerFrame) / 8;
}

/// @brief Update RIFF WAVE file header when closing file.
///
/// @note This routine updates the RIFF chunk size and the data chunk size,
///     each of which are dependent on the number of bytes written to the
///     data chunk.
static int updateWaveHeader(AudioFile_handle* file) {
    AudioFile_handle tempFile = *file;

    // Read the WAV header to a fresh file handle to retrieve data chunk size.
    int error = readWaveHeader(&tempFile, 0L, NULL, 0, NULL);
    if (error != 0) {
        return error;
    }

    ssize_t dataChunkSize = tempFile.bytesPerFrame * tempFile.framesInFile;

    // Verify that file size is consistent with what we think was written.
    if (dataChunkSize != file->bytesPerFrame * file->framesInFile) {
        return ERROR_WAVE_HEADER_UPDATE_INCONSISTENT_FILE_SIZE;
    }

    FILE* handle = file->handle;

    // Rewind to data chunk header size field; some platforms do not implement SEEK_CUR.
    long dataChunkHeaderSizeLoc = ftell(handle) - ((long)SIZE_4BYTES) / BYTES_PER_CHAR;
    if (fseek(handle, dataChunkHeaderSizeLoc, SEEK_SET)) {
        return ERROR_WAVE_HEADER_UPDATE_DATA_CHUNK_SEEK_FAILED;
    }

    // Write the data chunk size
    if (writeToFile(&dataChunkSize, SIZE_4BYTES, 1, handle)) {
        return ERROR_WAVE_HEADER_UPDATE_DATA_CHUNK_WRITE_FAILED;
    }

    // Look for end of file.
    if (fseek(handle, 0L, SEEK_END)) {
        return ERROR_WAVE_HEADER_UPDATE_EOF_FSEEK_FAILED;
    }

    long endOfFile = BYTES_PER_CHAR * ftell(handle);

    long riffChunkSize = endOfFile - WAVFILE_RIFF_DATA_OFFSET;

    // Go to the RIFF chunk header size field
    if (fseek(handle, WAVFILE_RIFF_HDR_CHUNK_SIZE_OFFSET / BYTES_PER_CHAR, SEEK_SET)) {
        return ERROR_WAVE_HEADER_UPDATE_RIFF_CHUNK_SEEK_FAILED;
    }

    // Write the chunk size field in the header.
    if (writeToFile(&riffChunkSize, SIZE_4BYTES, 1, handle)) {
        return ERROR_WAVE_HEADER_UPDATE_RIFF_CHUNK_WRITE_FAILED;
    }

    return 0;
}

/// @brief Read a RIFF WAVE file header.
static int readWaveHeader(
    AudioFile_handle* file,
    long metadataChunkID,
    char metadata[],
    ssize_t sizeofBuffer,
    ssize_t* sizeRead) {
    // Clear out the format
    AudioFile_format* format = &file->format;
    memset(&file->format, 0, sizeof(file->format));

    FILE* handle = file->handle;

    // Go back to the start of the file
    if (fseek(handle, 0L, SEEK_SET)) {
        return ERROR_WAVE_HEADER_READ_FSEEK_FAILED;
    }

    RiffChunk riffChunk;

    // Read the RIFF chunk header.
    if ((readChunkHeader(&riffChunk, handle)) || (riffChunk.id != CHUNK_ID_RIFF)) {
        return ERROR_WAVE_HEADER_READ_RIFF_CHUNK;
    }

    long riffType = 0;  // Clear any high bytes not set by readFromFile()

    // Read the RIFF file type.
    if ((readFromFile(&riffType, SIZE_4BYTES, 1, handle)) || (riffType != FILE_TYPE_WAVE)) {
        return ERROR_WAVE_HEADER_READ_RIFF_FILE_TYPE;
    }

    RiffChunk fmtChunk;

    // Read the format chunk header.
    if ((readChunkHeader(&fmtChunk, handle)) || (fmtChunk.id != CHUNK_ID_FMT)) {
        return ERROR_WAVE_HEADER_READ_FORMAT_CHUNK;
    }

    short data16 = 0;  // Clear any high bytes not set by readFromFile()

    // Read the sample type.
    if (readFromFile(&data16, SIZE_2BYTES, 1, handle)) {
        return ERROR_WAVE_HEADER_READ_SAMPLE_TYPE;
    }
    format->sampleType = data16;

    data16 = 0;  // Clear any high bytes not set by readFromFile()

    // Read the samples per frame.
    if (readFromFile(&data16, SIZE_2BYTES, 1, handle)) {
        return ERROR_WAVE_HEADER_READ_SAMPLES_PER_FRAME;
    }
    format->samplesPerFrame = data16;

    long data32 = 0;  // Clear any high bytes not set by readFromFile()

    // Read the sampling rate.
    if (readFromFile(&data32, SIZE_4BYTES, 1, handle)) {
        return ERROR_WAVE_HEADER_READ_SAMPLING_RATE;
    }
    format->samplingRate = data32;

    // Read the bytes per second (cache this temporarily and verify later).
    long bytesPerSecond = 0;  // Clear any high bytes not set by readFromFile()

    if (readFromFile(&bytesPerSecond, SIZE_4BYTES, 1, handle)) {
        return ERROR_WAVE_HEADER_READ_BYTES_PER_SECOND;
    }

    data16 = 0;  // Clear any high bytes not set by readFromFile()

    // Read block align (and ignore it).
    if (readFromFile(&data16, SIZE_2BYTES, 1, handle)) {
        return ERROR_WAVE_HEADER_READ_BLOCK_ALIGN;
    }

    data16 = 0;  // Clear any high bytes not set by readFromFile()

    // Read bits per sample.
    if (readFromFile(&data16, SIZE_2BYTES, 1, handle)) {
        return ERROR_WAVE_HEADER_READ_BITS_PER_SAMPLE;
    }
    format->bitsPerSample = data16;

    // Verify bytes per second.
    if (bytesPerSecond != getBytesPerSecond(format)) {
        return ERROR_WAVE_HEADER_READ_BYTES_PER_SEC_INCONSISTENT;
    }

    // Skip any pad bytes.
    if (fmtChunk.size > WAVFILE_FMT_CHUNK_SIZE) {
        ssize_t padCount = fmtChunk.size - WAVFILE_FMT_CHUNK_SIZE;
        while (padCount > 0) {
            char padByte = 0;

            if (readFromFile(&padByte, SIZE_1BYTE, BYTES_PER_CHAR, handle)) {
                return ERROR_WAVE_HEADER_READ_PAD_BYTES;
            }
            padCount -= SIZE_1BYTE * BYTES_PER_CHAR;
        }
    }

    RiffChunk dataChunk;

    // WAVE supports other chunk IDs at this point in the file.  If we were passed in
    // a metadata buffer, we will look search for an ID matching the one supplied, and return the payload.
    do {
        if (readChunkHeader(&dataChunk, handle)) {
            return ERROR_WAVE_HEADER_READ_DATA_CHUNK;
        }
        if (dataChunk.id != CHUNK_ID_DATA) {
            if (metadata != NULL && dataChunk.id == metadataChunkID) {
                if ((uint32_t)sizeofBuffer < dataChunk.size) {
                    return ERROR_METADATA_READ_INSUFFICIENT_BUFFER_SIZE;
                }

                ssize_t bytesToRead = dataChunk.size;
                if (readFromFile(metadata, SIZE_1BYTE, bytesToRead, handle)) {
                    return ERROR_WAVE_HEADER_READ_METADATA_PAYLOAD;
                }
                *sizeRead = bytesToRead;
            } else {
                long dataChunkEndLoc = ftell(handle) + (dataChunk.size / BYTES_PER_CHAR);
                fseek(handle, dataChunkEndLoc, SEEK_SET);  //No SEEK_CUR in some platforms
            }
        }
    } while (dataChunk.id != CHUNK_ID_DATA);

    // Check for acceptable format.
    int error = validateFormat(&file->format);
    if (error) {
        return error;
    }

    // For repeated use during read/write operations.
    file->bytesPerFrame = getBytesPerFrame(format);

    // Get the data chunk size based on fseek/ftell operations.
    // Assumes there are are no chunks after the data chunk!
    long start = BYTES_PER_CHAR * ftell(handle);

    // Look for end of file.
    if (fseek(handle, 0L, SEEK_END)) {
        return ERROR_WAVE_HEADER_READ_FSEEK_FAILED;
    }

    long end = BYTES_PER_CHAR * ftell(handle);

    if ((start < 0) || (end < 0) || (end < start)) {
        return ERROR_WAVE_HEADER_READ_FTELL_FAILED;
    }

    if ((unsigned long)(end - start) > MAX_U4BYTES) {
        return ERROR_FILE_TOO_LONG;
    }

    long dataChunkSizeFromFile = end - start;

    // Go back to start of data chunk.
    if (fseek(handle, start / BYTES_PER_CHAR, SEEK_SET)) {
        return ERROR_WAVE_HEADER_READ_FSEEK_FAILED;
    }

    // We can do rudimentary checks for a malformed WAV file header
    if (dataChunk.size == 0) {
        // Typically happens if a WAV file we wrote was not closed properly.
        // Which could be during debugging.  When you really do not have
        // time for malformed WAV file headers.
        dataChunk.size = dataChunkSizeFromFile;
    } else {
        // Sanity check against the RIFF chunk file size, but assumes no chunks after data chunk.
        long long expectedDataChunkSize = riffChunk.size - (start - 2 * SIZE_4BYTES);

        // Unfortunately, we have behave nicely when opening audio that is known to have a mismatch
        // between the file size and data chunk size. Rather than return an error, we take the
        // min of the two values.
        // This is done with the knowledge that another library may parse this file differently
        // and thus will give different results.
        // See this Jira for more details: https://issues.labcollab.net/browse/BSFT-78069
        if (dataChunkSizeFromFile != expectedDataChunkSize) {
            if (dataChunkSizeFromFile < expectedDataChunkSize) {
                dataChunk.size = dataChunkSizeFromFile;
            } else {
                dataChunk.size = expectedDataChunkSize;
            }
        }
    }

    // For read, we decrement framesRemaining in the read routine.
    file->framesInFile = dataChunk.size / file->bytesPerFrame;

    // Reset down-counter.
    file->framesRemaining = file->framesInFile;

    return 0;
}

/// @brief Read number of frames in PCM file based on file format passed in.
static int readPcmFileLength(AudioFile_handle* file) {
    FILE* handle = file->handle;
    AudioFile_format* format = &file->format;

    if (fseek(handle, 0L, SEEK_END)) {
        return ERROR_PCM_FILE_LENGTH_READ_FSEEK_FAILED;
    }
    long dataChunkSize = BYTES_PER_CHAR * ftell(handle);

    if (fseek(handle, 0L, SEEK_SET)) {
        return ERROR_PCM_FILE_LENGTH_READ_FSEEK_FAILED;
    }

    if (dataChunkSize < 0) {
        return ERROR_PCM_FILE_LENGTH_READ_FTELL_FAILED;
    }

    // For repeated use during read/write operations.
    file->bytesPerFrame = getBytesPerFrame(format);

    // For read, we decrement framesRemaining in the read routine.
    file->framesInFile = dataChunkSize / file->bytesPerFrame;

    // Reset down-counter.
    file->framesRemaining = file->framesInFile;

    return 0;
}

/// @brief Rewind file
///
/// @remark Call this only for files being read.
static int rewindFile(AudioFile_handle* file) {
    // Track to end of file, in case we are not already there.
    if (fseek(file->handle, 0, SEEK_END)) {
        return ERROR_REWIND_FSEEK_FAILED;
    }

    // Compute offset back to start of data.
    long offset = BYTES_PER_CHAR * ftell(file->handle) - file->bytesPerFrame * file->framesInFile;

    // Rewind to start of data.
    //AECORE_ASSERT(offset % BYTES_PER_CHAR == 0);
    if (fseek(file->handle, offset / BYTES_PER_CHAR, SEEK_SET)) {
        return ERROR_REWIND_FSEEK_FAILED;
    }

    // Reset down-counter.
    file->framesRemaining = file->framesInFile;

    return 0;
}