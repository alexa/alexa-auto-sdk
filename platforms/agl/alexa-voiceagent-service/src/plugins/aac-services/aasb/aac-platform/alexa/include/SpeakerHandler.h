/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AASB_ALEXA_SPEAKERHANDLER_H
#define AASB_ALEXA_SPEAKERHANDLER_H

#include <memory>

#include <AACE/Alexa/Speaker.h>

#include "LoggerHandler.h"

namespace aasb {
namespace alexa {

class SpeakerHandler : public aace::alexa::Speaker {
public:
    static std::shared_ptr<SpeakerHandler> create(std::shared_ptr<aasb::core::logger::LoggerHandler> logger);

    /// @name aace::alexa::Speaker Functions
    /// @{
    bool setVolume(int8_t volume) override;
    bool adjustVolume(int8_t delta) override;
    bool setMute(bool mute) override;
    int8_t getVolume() override;
    bool isMuted() override;
    /// @}

private:
    SpeakerHandler() = default;

    // aasb::core::logger::LoggerHandler
    std::shared_ptr<aasb::core::logger::LoggerHandler> m_logger;
};

}  // namespace alexa
}  // namespace aasb

#endif  // AASB_ALEXA_SPEAKERHANDLER_H