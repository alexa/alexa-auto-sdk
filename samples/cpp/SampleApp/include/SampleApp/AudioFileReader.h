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

#ifndef SAMPLEAPP_AUDIOFILEREADER_H
#define SAMPLEAPP_AUDIOFILEREADER_H

// C++ Standard Library
#include <chrono>
#include <fstream>
#include <memory>

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  AudioFileReader
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class AudioFileReader {
  private:
    int m_sampleCount{};
    std::chrono::high_resolution_clock::time_point m_lastSampleTime{};
    std::shared_ptr<std::ifstream> m_inputStream{};

  public:
    AudioFileReader(const std::string &filePath);
    virtual ~AudioFileReader();
    auto close() -> bool;
    auto getSampleCount() -> int;
    auto open() -> bool;
    auto read(const int16_t *data, const size_t size) -> ssize_t;
    auto readSamples(const int16_t *buffer, const int size) -> int;
    auto readSamplesInRealTime(const int16_t *buffer, const int size) -> int;

  private:
    auto reset() -> void;
};

} // namespace sampleApp

#endif // SAMPLEAPP_AUDIOFILEREADER_H
