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

#ifndef AACE_ENGINE_MOBILE_BRIDGE_DATA_STREAM_PIPE_H
#define AACE_ENGINE_MOBILE_BRIDGE_DATA_STREAM_PIPE_H

#include <memory>

#include "AACE/Engine/MobileBridge/DataStream.h"

namespace aace {
namespace engine {
namespace mobileBridge {

/**
 * A pipe with two ends: one for input and the other for output. Reading on the output end
 * will block if the pipe is empty. Writing on the input end will block if the pipe is full.
 *
 * Note that this class is designed for single-producer single-consumer (SPSC). It's safe to
 * read with one thread and write with the other thread. However it is not thread-safe to read
 * or wrte with multiple threads.
 */
class DataStreamPipe : public DataStream {
public:
    DataStreamPipe(size_t bufferSize);
    ~DataStreamPipe() override;

    void close() override;

    std::shared_ptr<DataInputStream> getInput();
    std::shared_ptr<DataOutputStream> getOutput();

    size_t size();
    size_t waitForAvailableBytes(size_t minAvailable);

private:
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

}  // namespace mobileBridge
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MOBILE_BRIDGE_DATA_STREAM_PIPE_H
