/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "SampleApp/AudioFileReader.h"

// C++ Standard Library
#include <thread> // std::this_thread::sleep_for

namespace sampleApp {

static const int RIFF_HEADER_SIZE = 44;
static const int SAMPLING_INTERVAL = 10; // in milliseconds

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioFileReader
//
////////////////////////////////////////////////////////////////////////////////////////////////////

AudioFileReader::AudioFileReader(const std::string &filePath) {
    m_inputStream = std::make_shared<std::ifstream>(filePath, std::ios::binary);
    m_lastSampleTime = std::chrono::high_resolution_clock::now();
    m_sampleCount = 0;
}

AudioFileReader::~AudioFileReader() { m_inputStream->close(); }

bool AudioFileReader::close() {
    m_inputStream->close();
    return true;
}

int AudioFileReader::getSampleCount() { return m_sampleCount; }

bool AudioFileReader::open() {
    if (!m_inputStream->is_open()) {
        return false;
    }
    m_inputStream->seekg(0, std::ios_base::end);
    int length = m_inputStream->tellg();
    if (length <= RIFF_HEADER_SIZE) {
        return false;
    }
    char buffer[4];
    m_inputStream->seekg(0, std::ios_base::beg);
    m_inputStream->read(buffer, 4);
    if (buffer[0] != 'R' || buffer[1] != 'I' || buffer[2] != 'F' || buffer[3] != 'F') {
        return false;
    }
    m_sampleCount = (length - RIFF_HEADER_SIZE) / 2;
    reset();
    return true;
}

ssize_t AudioFileReader::read(const int16_t *data, const size_t size) { return readSamplesInRealTime(data, size); }

int AudioFileReader::readSamples(const int16_t *buffer, const int size) {
    int count = 0;
    if (!m_inputStream->eof()) {
        m_inputStream->read((char *)buffer, size * 2);
        count = m_inputStream->gcount();
    }
    return count / 2;
}

int AudioFileReader::readSamplesInRealTime(const int16_t *buffer, const int size) {
    int count = readSamples(buffer, size);
    auto now = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastSampleTime);
    if (duration < std::chrono::milliseconds(SAMPLING_INTERVAL)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(SAMPLING_INTERVAL) - duration);
    }
    m_lastSampleTime = std::chrono::high_resolution_clock::now();
    return count;
}

// private

void AudioFileReader::reset() { m_inputStream->seekg(RIFF_HEADER_SIZE, std::ios_base::beg); }

} // namespace sampleApp
