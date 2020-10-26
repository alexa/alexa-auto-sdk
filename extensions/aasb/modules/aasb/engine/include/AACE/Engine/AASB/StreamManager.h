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

#ifndef AACE_ENGINE_AASB_STREAM_MANAGER_H
#define AACE_ENGINE_AASB_STREAM_MANAGER_H

#include <unordered_map>
#include <mutex>

#include <AACE/AASB/AASBStream.h>

#include "StreamManagerInterface.h"

namespace aace {
namespace engine {
namespace aasb {

class StreamManager
        : public StreamManagerInterface
        , public std::enable_shared_from_this<StreamManager> {
private:
    StreamManager() = default;

public:
    static std::shared_ptr<StreamManager> create();

    virtual ~StreamManager() = default;

    void shutdown();

    // aace::engine::aasb::StreamManagerInterface
    bool registerStreamHandler(const std::string& streamId, std::shared_ptr<aace::aasb::AASBStream> stream) override;
    std::shared_ptr<aace::aasb::AASBStream> requestStreamHandler(
        const std::string& streamId,
        aace::aasb::AASBStream::Mode mode) override;

private:
    std::unordered_map<std::string, std::shared_ptr<aace::aasb::AASBStream>> m_streamMap;
    std::mutex m_mutex;
};

}  // namespace aasb
}  // namespace engine
}  // namespace aace

#endif
