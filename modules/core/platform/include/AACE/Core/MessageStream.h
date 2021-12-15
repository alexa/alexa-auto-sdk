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

#ifndef AACE_CORE_MESSAGE_STREAM_H
#define AACE_CORE_MESSAGE_STREAM_H

#include <iostream>

/** @file */

namespace aace {
namespace core {

class MessageStream {
public:
    enum class Mode { READ, WRITE, READ_WRITE };

    virtual ~MessageStream() = default;

    /**
     * Reads data from the strean when available. Data will be
     * available while @c isClosed() returns false.
     *
     * @param [out] data The buffer where data should be copied
     * @param [in] size The size of the buffer
     * @return The number of bytes read, 0 if the end of stream is reached or data is not currently available,
     * or -1 if an error occurred
     */
    virtual ssize_t read(char* data, const size_t size) = 0;

    /**
     * Writes data to the stream.
     *
     * @param [in] data The data to be written to the stream
     * @param [in] size The number of bytes to be written
     * @return The number of bytes successfully written to the stream or a negative error code
     * if data could not be written
     */
    virtual ssize_t write(const char* data, const size_t size) = 0;

    /**
     * Checks if the stream has been closed.
     *
     * @return @c true if the stream is closed, @c false if more data
     * will be available
     */
    virtual bool isClosed() = 0;

    /**
     * Gets the stream mode.
     *
     * @return @c MessageStream::Mode of the stream.
     */
    virtual MessageStream::Mode getMode() = 0;
};

inline std::ostream& operator<<(std::ostream& stream, const MessageStream::Mode& mode) {
    switch (mode) {
        case MessageStream::Mode::READ:
            stream << "READ";
            break;
        case MessageStream::Mode::WRITE:
            stream << "WRITE";
            break;
        case MessageStream::Mode::READ_WRITE:
            stream << "READ_WRITE";
            break;
    }
    return stream;
}

}  // namespace core
}  // namespace aace

#endif  // AACE_CORE_MESSAGE_STREAM_H
