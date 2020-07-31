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

#ifndef AACE_ENGINE_LOGGER_LOGENTRY_H
#define AACE_ENGINE_LOGGER_LOGENTRY_H

#include <string>

#include "LogEntryStream.h"

namespace aace {
namespace engine {
namespace logger {

class LogEntry {
public:
    LogEntry(const std::string& tag, const char* event);
    LogEntry(const std::string& tag, const std::string& event);

    LogEntry& d(const char* key, const char* value);
    LogEntry& d(const char* key, const std::string& value);
    LogEntry& d(const char* key, bool value);

    template <typename ValueType>
    inline LogEntry& d(const char* key, const ValueType& value);

    template <typename ValueType>
    inline LogEntry& sensitive(const char* key, const ValueType& value);

    LogEntry& m(const char* message);
    LogEntry& m(const std::string& message);

    const std::string& tag() const;
    const char* c_str() const;

private:
    void prefixKeyValuePair();
    void prefixMessage();
    void appendEscapedString(const char* in);

    // Character used to separate @c key from @c value text in metadata.
    static const char KEY_VALUE_SEPARATOR = '=';

    // tag
    std::string m_tag;

    // Flag indicating (if true) that some metadata has already been appended to this LogEntry.
    bool m_hasMetadata;

    // A stream with which to accumulate the text for this LogEntry.
    LogEntryStream m_stream;
};

template <typename ValueType>
LogEntry& LogEntry::d(const char* key, const ValueType& value) {
    prefixKeyValuePair();
    m_stream << key << KEY_VALUE_SEPARATOR << value;
    return *this;
}

#ifdef AAC_EMIT_SENSITIVE_LOGS
template <typename ValueType>
LogEntry& LogEntry::sensitive(const char* key, const ValueType& value) {
    return d(key, value);
}
#else
template <typename ValueType>
LogEntry& LogEntry::sensitive(const char* key, const ValueType& value) {
    return *this;
}
#endif

}  // namespace logger
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOGGER_LOGENTRY_H
