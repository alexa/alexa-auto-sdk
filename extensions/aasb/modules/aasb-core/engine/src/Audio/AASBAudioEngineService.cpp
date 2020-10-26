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

#include <AASB/Engine/Audio/AASBAudioEngineService.h>
#include <AASB/Engine/Audio/AASBAudioInputProvider.h>
#include <AASB/Engine/Audio/AASBAudioOutputProvider.h>

#include <AACE/Engine/Core/EngineMacros.h>

namespace aasb {
namespace engine {
namespace audio {

// String to identify log entries originating from this file.
static const std::string TAG("aasb.audio.AASBAudioEngineService");

// Minimum version this module supports
static const aace::engine::core::Version minRequiredVersion = VERSION("3.0");

// register the service
REGISTER_SERVICE(AASBAudioEngineService);

AASBAudioEngineService::AASBAudioEngineService(const aace::engine::core::ServiceDescription& description) :
        aace::engine::aasb::AASBHandlerEngineService(
            description,
            minRequiredVersion,
            {"AudioInputProvider", "AudioOutputProvider"}) {
}

bool AASBAudioEngineService::postRegister() {
    try {
        auto aasbServiceInterface =
            getContext()->getServiceInterface<aace::engine::aasb::AASBServiceInterface>("aace.aasb");
        ThrowIfNull(aasbServiceInterface, "invalidAASBServiceInterface");

        // AudioInputProvider
        if (isInterfaceEnabled("AudioInputProvider")) {
            auto inputProvider = AASBAudioInputProvider::create(
                aasbServiceInterface->getMessageBroker(), aasbServiceInterface->getStreamManager());
            ThrowIfNull(inputProvider, "createAASBAudioInputProviderFailed");
            getContext()->registerPlatformInterface(inputProvider);
        }

        // AudioOutputProvider
        if (isInterfaceEnabled("AudioOutputProvider")) {
            auto outputProvider = AASBAudioOutputProvider::create(
                aasbServiceInterface->getMessageBroker(), aasbServiceInterface->getStreamManager());
            ThrowIfNull(outputProvider, "createAudioSocketOutputProviderFailed");
            getContext()->registerPlatformInterface(outputProvider);
        }

        return true;
    } catch (std::exception& ex) {
        AACE_WARN(LX(TAG).d("reason", ex.what()));
        return false;
    }
}

}  // namespace audio
}  // namespace engine
}  // namespace aasb
