/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_LOGGER_SINK_FILE_SINK_H
#define AACE_ENGINE_LOGGER_SINK_FILE_SINK_H

#include "Sink.h"

namespace aace {
namespace engine {
namespace logger {
namespace sink {

class FileSink : public Sink {
private:
    FileSink(const std::string& id);

public:
    static std::shared_ptr<FileSink> create(
        const std::string& id,
        const std::string& path,
        const std::string& prefix = "aace",
        uint32_t maxSize = 5242880,
        uint32_t maxFiles = 3,
        bool append = true);

private:
    void log(Level level, std::chrono::system_clock::time_point time, const char* threadMoniker, const char* text)
        override;
    void flush() override;

    bool rotateLog();

    bool exists(const std::string& filename);

private:
    bool m_enabled = false;

    std::string m_path;
    std::string m_prefix;
    uint32_t m_maxSize;
    uint8_t m_maxFiles;
    bool m_append;

    std::string m_filename;
    std::shared_ptr<std::ofstream> m_stream;
};

}  // namespace sink
}  // namespace logger
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOGGER_SINK_FILE_SINK_H
