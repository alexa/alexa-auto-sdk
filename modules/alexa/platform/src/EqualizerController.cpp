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

#include "AACE/Alexa/EqualizerController.h"

namespace aace {
namespace alexa {

EqualizerController::~EqualizerController() = default;

void EqualizerController::localSetBandLevels(const std::vector<EqualizerBandLevel>& bandLevels) {
    if (auto m_equalizerControllerEngineInterface_lock = m_equalizerControllerEngineInterface.lock()) {
        m_equalizerControllerEngineInterface_lock->onLocalSetBandLevels(bandLevels);
    }
}

void EqualizerController::localAdjustBandLevels(const std::vector<EqualizerBandLevel>& bandAdjustments) {
    if (auto m_equalizerControllerEngineInterface_lock = m_equalizerControllerEngineInterface.lock()) {
        m_equalizerControllerEngineInterface_lock->onLocalAdjustBandLevels(bandAdjustments);
    }
}

void EqualizerController::localResetBands(const std::vector<EqualizerBand>& bands) {
    if (auto m_equalizerControllerEngineInterface_lock = m_equalizerControllerEngineInterface.lock()) {
        m_equalizerControllerEngineInterface_lock->onLocalResetBands(bands);
    }
}

void EqualizerController::setEngineInterface(
    std::shared_ptr<aace::alexa::EqualizerControllerEngineInterface> equalizerControllerEngineInterface) {
    m_equalizerControllerEngineInterface = equalizerControllerEngineInterface;
}

}  // namespace alexa
}  // namespace aace
