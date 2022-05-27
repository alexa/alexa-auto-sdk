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

#ifndef AACE_ENGINE_MESSAGE_BROKER_STREAM_MANAGER_IMPL_H
#define AACE_ENGINE_MESSAGE_BROKER_STREAM_MANAGER_IMPL_H

#include <unordered_map>
#include <mutex>

#include <AACE/Core/MessageStream.h>

#include "StreamManagerInterface.h"

namespace aace {
namespace engine {
namespace messageBroker {

class StreamManagerImpl
        : public StreamManagerInterface
        , public std::enable_shared_from_this<StreamManagerImpl> {
private:
    StreamManagerImpl() = default;

public:
    static std::shared_ptr<StreamManagerImpl> create();

    virtual ~StreamManagerImpl() = default;

    void shutdown();

    // aace::engine::messageBroker::StreamManagerInterface
    bool registerStreamHandler(const std::string& streamId, std::shared_ptr<aace::core::MessageStream> stream) override;
    std::shared_ptr<aace::core::MessageStream> requestStreamHandler(
        const std::string& streamId,
        aace::core::MessageStream::Mode mode) override;

private:
    std::unordered_map<std::string, std::shared_ptr<aace::core::MessageStream>> m_streamMap;
    std::mutex m_mutex;
};

}  // namespace messageBroker
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_MESSAGE_BROKER_STREAM_MANAGER_IMPL_H
