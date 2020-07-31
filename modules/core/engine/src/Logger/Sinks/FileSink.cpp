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

#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

#include <sys/types.h>
#include <sys/stat.h>

#include "AACE/Engine/Logger/Sinks/FileSink.h"
#include "AACE/Engine/Logger/LogFormatter.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace logger {
namespace sink {

// String to identify log entries originating from this file.
static const std::string TAG("aace.logger.sink.FileSink");

FileSink::FileSink(const std::string& id) : Sink(id) {
}

std::shared_ptr<FileSink> FileSink::create(
    const std::string& id,
    const std::string& path,
    const std::string& prefix,
    uint32_t maxSize,
    uint32_t maxFiles,
    bool append) {
    try {
        struct stat info;

        // check to make sure the path is valid
        ThrowIf(stat(path.c_str(), &info) != 0, "invalidPath");
        ThrowIf((info.st_mode & S_IFDIR) == 0, "invalidPath");

        // create the file sink
        auto sink = std::shared_ptr<FileSink>(new FileSink(id));

        sink->m_path = path;
        sink->m_prefix = prefix;
        sink->m_maxSize = maxSize;
        sink->m_maxFiles = maxFiles;
        sink->m_append = append;

        // append path separator if necessary
        if (sink->m_path[sink->m_path.length() - 1] != '/') {
            sink->m_path += '/';
        }

        // create the main log filename
        sink->m_filename = sink->m_path + sink->m_prefix + ".log";

        // create the log file stream
        sink->m_stream =
            std::make_shared<std::ofstream>(sink->m_filename, append ? std::ios::out : std::ios::out | std::ios::trunc);
        ThrowIfNot(sink->m_stream->is_open(), "openStreamFailed");

        // enbale the sink
        sink->m_enabled = true;

        return sink;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG, "create").d("reason", ex.what()));
        return nullptr;
    }
}

void FileSink::log(
    Level level,
    std::chrono::system_clock::time_point time,
    const char* threadMoniker,
    const char* text) {
    if (m_enabled) {
        try {
            std::string log = aace::engine::logger::LogFormatter::format(level, time, threadMoniker, text);

            // check if the log file needs to be rotated
            if ((long)m_stream->tellp() + log.length() + 1 > m_maxSize) {
                ThrowIfNot(rotateLog(), "rotateLogFailed");
            }

            // log the event to file stream
            *m_stream << log << std::endl;
        } catch (std::exception& ex) {
            // disable the sink so that the error message doesn't cause the logger to
            // get caught in an infinite loop.. ok if another sink handles the event!
            m_enabled = false;

            // log the error
            AACE_ERROR(LX(TAG, "log").d("reason", ex.what()));
        }
    }
}

void FileSink::flush() {
    m_stream->flush();
}

bool FileSink::rotateLog() {
    try {
        // close the current log stream
        m_stream->close();

        for (int j = m_maxFiles; j > 0; j--) {
            std::string src = j > 1 ? m_filename + '.' + std::to_string(j - 1) : m_filename;
            std::string target = m_filename + '.' + std::to_string(j);

            // remove the target file if it exists
            if (exists(target)) {
                ThrowIf(std::remove(target.c_str()) != 0, "rotateLogFailed");
            }

            if (exists(src)) {
                ThrowIf(std::rename(src.c_str(), target.c_str()) != 0, "rotateLogFailed");
            }
        }

        m_stream = std::make_shared<std::ofstream>(m_filename, std::ios::out | std::ios::trunc);
        ThrowIfNot(m_stream->is_open(), "openStreamFailed");

        return true;
    } catch (std::exception& ex) {
        // disable the sink so that the error message doesn't cause the logger to
        // get caught in an infinite loop.. ok if another sink handles the event!
        m_enabled = false;

        // log the error
        AACE_ERROR(LX(TAG, "rotateLog").d("reason", ex.what()));

        return false;
    }
}

bool FileSink::exists(const std::string& filename) {
    struct stat info;

    return stat(filename.c_str(), &info) == 0 && (info.st_mode & S_IFDIR) == 0;
}

}  // namespace sink
}  // namespace logger
}  // namespace engine
}  // namespace aace
