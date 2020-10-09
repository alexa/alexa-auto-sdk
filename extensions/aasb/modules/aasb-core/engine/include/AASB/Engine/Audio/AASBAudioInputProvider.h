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

#ifndef AASB_ENGINE_AUDIO_AASB_AUDIO_INPUT_PROVIDER_H
#define AASB_ENGINE_AUDIO_AASB_AUDIO_INPUT_PROVIDER_H

#include <AACE/Audio/AudioInputProvider.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>
#include <AACE/Engine/AASB/StreamManagerInterface.h>

#include "AASBAudioInput.h"

#include <memory>
#include <string>

namespace aasb {
namespace engine {
namespace audio {

class AASBAudioInputProvider : public aace::audio::AudioInputProvider {
private:
    AASBAudioInputProvider() = default;

    bool initialize(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager);

public:
    virtual ~AASBAudioInputProvider() = default;

    static std::shared_ptr<AASBAudioInputProvider> create(
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker,
        std::shared_ptr<aace::engine::aasb::StreamManagerInterface> streamManager);

    // aace::audio::AudioInputProvider
    std::shared_ptr<aace::audio::AudioInput> openChannel(const std::string& name, AudioInputType type) override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
    std::weak_ptr<aace::engine::aasb::StreamManagerInterface> m_streamManager;
};

}  // namespace audio
}  // namespace engine
}  // namespace aasb

#endif  // AASB_ENGINE_AUDIO_AASB_AUDIO_INPUT_PROVIDER_H
