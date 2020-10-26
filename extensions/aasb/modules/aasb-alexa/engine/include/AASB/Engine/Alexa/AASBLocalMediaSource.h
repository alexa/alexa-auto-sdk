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
#ifndef AASB_LOCAL_MEDIA_SOURCE_H
#define AASB_LOCAL_MEDIA_SOURCE_H

#include <AACE/Alexa/LocalMediaSource.h>
#include <AACE/Engine/AASB/MessageBrokerInterface.h>

namespace aasb {
namespace engine {
namespace alexa {

class AASBLocalMediaSource
        : public aace::alexa::LocalMediaSource
        , public std::enable_shared_from_this<AASBLocalMediaSource> {
private:
    AASBLocalMediaSource(LocalMediaSource::Source source);

    bool initialize(std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

public:
    static std::shared_ptr<AASBLocalMediaSource> create(
        aace::alexa::LocalMediaSource::Source source,
        std::shared_ptr<aace::engine::aasb::MessageBrokerInterface> messageBroker);

    // aace::alexa::LocalMediaSource
    bool play(ContentSelector contentSelectorType, const std::string& payload) override;
    bool playControl(aace::alexa::LocalMediaSource::PlayControlType controlType) override;
    bool seek(std::chrono::milliseconds offset) override;
    bool adjustSeek(std::chrono::milliseconds deltaOffset) override;
    LocalMediaSourceState getState() override;
    bool volumeChanged(float volume) override;
    bool mutedStateChanged(aace::alexa::LocalMediaSource::MutedState state) override;

private:
    std::weak_ptr<aace::engine::aasb::MessageBrokerInterface> m_messageBroker;
};

}  // namespace alexa
}  // namespace engine
}  // namespace aasb

#endif  //AASB_LOCAL_MEDIA_SOURCE_H
