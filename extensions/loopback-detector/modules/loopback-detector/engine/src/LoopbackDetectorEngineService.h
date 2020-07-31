/*
 * Copyright 2019-2020 Amazon.com, Inc. and its affiliates. All Rights Reserved.
 *
 * SPDX-License-Identifier: LicenseRef-.amazon.com.-ASL-1.0
 *
 * Licensed under the Amazon Software License (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#ifndef AACE_ENGINE_LOOPBACKDETECTOR_LOOPBACK_DETECTOR_ENGINE_SERVICE_H
#define AACE_ENGINE_LOOPBACKDETECTOR_LOOPBACK_DETECTOR_ENGINE_SERVICE_H

#include <memory>
#include <AACE/Engine/Core/EngineService.h>
#include <AACE/Engine/Alexa/AlexaEngineService.h>
#include <AACE/Engine/Alexa/WakewordVerifier.h>

namespace aace {
namespace engine {
namespace loopbackDetector {

class LoopbackDetectorEngineService : public core::EngineService {
    // AACE Service Definition
    DESCRIBE("aace.loopbackDetector", VERSION("1.0"), DEPENDS(alexa::AlexaEngineService))

public:
    virtual ~LoopbackDetectorEngineService() = default;

protected:
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool preRegister() override;

private:
    LoopbackDetectorEngineService(const core::ServiceDescription& description);
    bool prepareVerifier();

    std::string m_wakewordEngineName;
    std::shared_ptr<alexa::WakewordVerifier> m_wakewordVerifier;
};

}  // namespace loopbackDetector
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_LOOPBACKDETECTOR_LOOPBACK_DETECTOR_ENGINE_SERVICE_H
