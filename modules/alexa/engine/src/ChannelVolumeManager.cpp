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
#include <memory>
#include "AACE/Engine/Alexa/ChannelVolumeManager.h"
#include "AACE/Engine/Core/EngineMacros.h"

namespace aace {
namespace engine {
namespace alexa {

using namespace alexaClientSDK::avsCommon::sdkInterfaces;
using namespace alexaClientSDK::avsCommon::avs::speakerConstants;

static const std::string TAG("ChannelVolumeManager");

/// The fraction of maximum volume used for the upper threshold in the default volume curve.
static const float UPPER_VOLUME_CURVE_FRACTION = 0.40;

/// The fraction of maximum volume used for the lower threshold in the default volume curve.
static const float LOWER_VOLUME_CURVE_FRACTION = 0.20;

/**
 * Checks whether a value is within the bounds.
 *
 * @tparam T The class type of the input parameters.
 * @param value The value to check.
 * @param min The minimum value.
 * @param max The maximum value.
 */
template <class T>
static bool withinBounds(T value, T min, T max) {
    if (value < min || value > max) {
        AACE_ERROR(LX("checkBoundsFailed").d("value", value).d("min", min).d("max", max));
        return false;
    }
    return true;
}

std::shared_ptr<ChannelVolumeManager> ChannelVolumeManager::create(
    std::shared_ptr<SpeakerInterface> speaker,
    ChannelVolumeInterface::Type type,
    VolumeCurveFunction volumeCurve,
    std::shared_ptr<DuckingInterface> duckingInterface) {
    if (!speaker) {
        AACE_ERROR(LX(__func__).d("reason", "Null SpeakerInterface").m("createFailed"));
        return nullptr;
    }

    auto channelVolumeManager =
        std::shared_ptr<ChannelVolumeManager>(new ChannelVolumeManager(speaker, type, volumeCurve, duckingInterface));

    /// Retrieve initial volume setting from underlying speakers
    SpeakerInterface::SpeakerSettings settings;
    if (!channelVolumeManager->getSpeakerSettings(&settings)) {
        AACE_ERROR(LX(__func__).m("createFailed").d("reason", "Unable To Retrieve Speaker Settings"));
        return nullptr;
    }
    channelVolumeManager->m_unduckedVolume = settings.volume;

    return channelVolumeManager;
}

ChannelVolumeManager::ChannelVolumeManager(
    std::shared_ptr<SpeakerInterface> speaker,
    ChannelVolumeInterface::Type type,
    VolumeCurveFunction volumeCurve,
    std::shared_ptr<DuckingInterface> duckingInterface) :
        ChannelVolumeInterface{},
        m_speaker{speaker},
        m_isDucked{false},
        m_unduckedVolume{AVS_SET_VOLUME_MIN},
        m_volumeCurveFunction{volumeCurve ? volumeCurve : defaultVolumeAttenuateFunction},
        m_DuckingInterface(duckingInterface),
        m_type{type} {
}

ChannelVolumeInterface::Type ChannelVolumeManager::getSpeakerType() const {
    std::lock_guard<std::mutex> locker{m_mutex};
    return m_type;
}

size_t ChannelVolumeManager::getId() const {
    return (size_t)m_speaker.get();
}

bool ChannelVolumeManager::startDucking() {
    std::lock_guard<std::mutex> locker{m_mutex};
    AACE_DEBUG(LX(__func__));
    if (m_isDucked) {
        AACE_WARN(LX(__func__).m("Channel is Already Attenuated"));
        return true;
    }

    if (!m_DuckingInterface) {
        AACE_WARN(LX(__func__).m("Ducking interface is misssing"));
        return false;
    } else if (!m_DuckingInterface->startDucking()) {
        AACE_WARN(LX(__func__).m("Failed to start ducking"));
        return false;
    }
    m_isDucked = true;
    return true;
}

bool ChannelVolumeManager::stopDucking() {
    std::lock_guard<std::mutex> locker{m_mutex};
    AACE_DEBUG(LX(__func__));

    // exit early if the channel is not attenuated
    if (!m_isDucked) {
        return true;
    }

    // Restore the speaker volume
    if (!m_DuckingInterface) {
        AACE_WARN(LX(__func__).m("Ducking interface is misssing"));
        return false;
    } else if (!m_DuckingInterface->stopDucking()) {
        return false;
    }

    AACE_DEBUG(LX(__func__));
    m_isDucked = false;
    return true;
}

bool ChannelVolumeManager::setUnduckedVolume(int8_t volume) {
    AACE_DEBUG(LX(__func__).d("volume", static_cast<int>(volume)));
    if (!withinBounds(volume, AVS_SET_VOLUME_MIN, AVS_SET_VOLUME_MAX)) {
        AACE_ERROR(LX(__func__).m("Invalid Volume"));
        return false;
    }

    std::lock_guard<std::mutex> locker{m_mutex};
    // store the volume
    m_unduckedVolume = volume;
    if (m_isDucked) {
        AACE_WARN(LX(__func__).m("Channel is Attenuated, Deferring Operation"));
        // New Volume shall be applied upon the next call to stopDucking()
        return true;
    }

    AACE_DEBUG(LX(__func__).d("Unducked Channel Volume", static_cast<int>(volume)));
    return m_speaker->setVolume(m_unduckedVolume);
}

bool ChannelVolumeManager::setMute(bool mute) {
    std::lock_guard<std::mutex> locker{m_mutex};
    AACE_DEBUG(LX(__func__).d("mute", static_cast<int>(mute)));
    return m_speaker->setMute(mute);
}

bool ChannelVolumeManager::getSpeakerSettings(
    alexaClientSDK::avsCommon::sdkInterfaces::SpeakerInterface::SpeakerSettings* settings) const {
    AACE_DEBUG(LX(__func__));
    if (!settings) {
        return false;
    }

    std::lock_guard<std::mutex> locker{m_mutex};
    if (!m_speaker->getSpeakerSettings(settings)) {
        AACE_ERROR(LX(__func__).m("Unable To Retrieve SpeakerSettings"));
        return false;
    }

    // if the channel is ducked : return the cached latest unducked volume
    if (m_isDucked) {
        AACE_DEBUG(LX(__func__).m("Channel is Already Attenuated"));
        settings->volume = m_unduckedVolume;
    }

    return true;
}

int8_t ChannelVolumeManager::defaultVolumeAttenuateFunction(int8_t currentVolume) {
    const int8_t lowerBreakPoint = static_cast<int8_t>(AVS_SET_VOLUME_MAX * LOWER_VOLUME_CURVE_FRACTION);
    const int8_t upperBreakPoint = static_cast<int8_t>(AVS_SET_VOLUME_MAX * UPPER_VOLUME_CURVE_FRACTION);

    if (currentVolume >= upperBreakPoint) {
        return lowerBreakPoint;
    } else if (currentVolume >= lowerBreakPoint && currentVolume <= upperBreakPoint) {
        return (currentVolume - lowerBreakPoint);
    } else {
        return alexaClientSDK::avsCommon::avs::speakerConstants::AVS_SET_VOLUME_MIN;
    }
}
}  // namespace alexa
}  // namespace engine
}  // namespace aace
