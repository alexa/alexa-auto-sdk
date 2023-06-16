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

#ifndef PRL_AUDIO_FILE_H
#define PRL_AUDIO_FILE_H


#include <stdio.h>  // For FILE

#ifdef __cplusplus
extern "C" {
#endif

    //////////////////////////////////////////////////////////////////////////
    /// PCM/WAV file reader/writer.  Should be able to handle PCM and floating
    /// point in various formats.  Current read/write functions do not
    /// deinterleave multi-channel audio buffers, but additional functions
    /// could be added to support that, if desired.
    ///
    /// In this context, an audio frame is a set of samples that were captured
    /// at the same point in time.  So you could have a mono frame, stereo
    /// frame, etc.  The frame rate is thus equal to the sampling rate.  A
    /// collection of frames is referred to as an audio buffer.  To further
    /// clarify, a stereo frame consists of a Left channel sample followed by
    /// a Right channel sample.
    ///
    /// This module is meant to be relatively lightweight.  So any extension
    /// should try to keep the basic functionality running efficiently and
    /// unbloated.
    ///

    /// Choices for sampleType in AudioFile_format
#define AUDIOFILE_WAVE_FORMAT_PCM           1 ///< Defined by Microsoft spec
#define AUDIOFILE_WAVE_FORMAT_IEEE_FLOAT    3 ///< Defined by Microsoft spec

    ///
    /// @brief Audio file format.
    ///
    typedef struct prlAudioFile_format_s
    {
        short sampleType;         ///< PCM or Float
        long samplingRate;        ///< In Hz
        short samplesPerFrame;    ///< numChannels (could be greater than 2, but non-standard)
        short bitsPerSample;      ///< Width of each sample in bits (16 or 32).
    } AudioFile_format, prlAudioFile_format;


    ///
    /// @brief Audio file handle.
    ///
    typedef struct prlAudioFile_handle_s
    {
        FILE *handle;               ///< File handle
        AudioFile_format format;    ///< Audio format
        int flags;                  ///< Internal flags
        short bytesPerFrame;     ///< Bytes per audio frame
        long framesInFile;       ///< READ:  total frames in file, WRITE:  total frames written (could also use fseek SEEK_CUR)
        long framesRemaining;    ///< READ:  frames remaining to read, WRITE:  zero
    } AudioFile_handle, prlAudioFile_handle;


    //////////////////////////////////////////////////////////////////////////////
    /// @brief Open existing audio file for reading.
    ///
    /// @param file [out]       Handle to initialize.
    /// @param fileName [in]    Name of file to read.
    /// @param format [i/o]     If file name contains WAV extension, the format
    ///     is read from the WAV header and returned to the caller.  Otherwise, the
    ///     user must specify the format for the raw audio file being read.
    /// @param maxFileNameLength [in]  Maximum number of characters in fileName.
    ///
    /// @return Zero for success, non-zero error code otherwise.
    int prlAudioFile_open(AudioFile_handle* file, char *fileName, AudioFile_format* format, ssize_t maxFileNameLength);


    //////////////////////////////////////////////////////////////////////////////
    /// @brief Read audio frames from a file.
    ///
    /// @param file [i/o]       Handle to file for reading.
    /// @param buffer [out]     Buffer for data read from file.
    ///     Multi-channel audio buffers are assumed to be interleaved.
    /// @param numFrames [in]   Number of frames to read from file.
    ///
    /// @note User must check if return values matches numFrames.
    ///
    /// @return Number of frames successfully read.
    ssize_t prlAudioFile_readFrames(AudioFile_handle *file, char buffer[], ssize_t numFrames);


    //////////////////////////////////////////////////////////////////////////////
    /// @brief Close an audio file.
    ///
    /// @param file [in]        Handle to file to be closed.
    ///
    /// @note For WAV files, this needs to be called to update the file header;
    ///     otherwise the file header will be malformed.
    ///
    /// @return Zero for success, non-zero error code otherwise.
    int prlAudioFile_close(AudioFile_handle *file);


   

#ifdef __cplusplus
} // extern "C"
#endif

#endif // PRL_AUDIO_FILE_H