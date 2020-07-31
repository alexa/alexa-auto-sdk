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

#ifndef AACE_ENGINE_LOGGER_LOGENTRYBUFFER_H
#define AACE_ENGINE_LOGGER_LOGENTRYBUFFER_H

#include <memory>
#include <streambuf>
#include <vector>

#ifndef AACE_LOG_ENTRY_BUFFER_SMALL_BUFFER_SIZE
#define AACE_LOG_ENTRY_BUFFER_SMALL_BUFFER_SIZE 128
#endif

namespace aace {
namespace engine {
namespace logger {

class LogEntryBuffer : public std::streambuf {
public:
    LogEntryBuffer();

    int_type overflow(int_type ch) override;
    const char* c_str() const;

private:
    char m_smallBuffer[AACE_LOG_ENTRY_BUFFER_SMALL_BUFFER_SIZE];
    char* m_base;
    std::unique_ptr<std::vector<char>> m_largeBuffer;
};

}  // namespace logger
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOGGER_LOGENTRYBUFFER_H
