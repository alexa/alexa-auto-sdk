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

#include <AACE/Engine/AASB/StreamManager.h>
#include <AACE/Engine/Core/EngineMacros.h>

#include <sstream>

namespace aace {
namespace engine {
namespace aasb {

// String to identify log entries originating from this file.
static const std::string TAG("aace.aasb.StreamManager");

std::shared_ptr<StreamManager> StreamManager::create() {
    return std::shared_ptr<StreamManager>(new StreamManager());
}

void StreamManager::shutdown() {
}

//
// aace::engine::aasb::StreamManagerInterface
//

bool StreamManager::registerStreamHandler(const std::string& streamId, std::shared_ptr<aace::aasb::AASBStream> stream) {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        ThrowIf(m_streamMap.find(streamId) != m_streamMap.end(), "streamAlreadyRegistered");

        m_streamMap[streamId] = stream;

        return true;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

std::shared_ptr<aace::aasb::AASBStream> StreamManager::requestStreamHandler(
    const std::string& streamId,
    aace::aasb::AASBStream::Mode mode) {
    try {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_streamMap.find(streamId);
        ThrowIf(it == m_streamMap.end(), "invalidStream");

        auto stream = it->second;

        // check that the stream mode is valid
        ThrowIfNot(
            stream->getMode() == aace::aasb::AASBStream::Mode::READ_WRITE || stream->getMode() == mode,
            "invalidStreamMode");

        // remove the stream from the map
        m_streamMap.erase(it);

        return stream;
    } catch (std::exception& ex) {
        AACE_ERROR(LX(TAG).d("reason", ex.what()));
        return nullptr;
    }
}

}  // namespace aasb
}  // namespace engine
}  // namespace aace
