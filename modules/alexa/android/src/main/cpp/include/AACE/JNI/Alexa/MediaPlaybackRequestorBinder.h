/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_ALEXA_MEDIAPLAYBACKREQUESTORBINDER_H_
#define AACE_JNI_ALEXA_MEDIAPLAYBACKREQUESTORBINDER_H_

#include <AACE/Alexa/MediaPlaybackRequestor.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class MediaPlaybackRequestorHandler : public aace::alexa::MediaPlaybackRequestor {
public:
    MediaPlaybackRequestorHandler(jobject obj);

    // aace::alexa::MediaPlaybackRequestor interface
    void mediaPlaybackResponse(MediaPlaybackRequestStatus mediaPlaybackRequestStatus) override;

private:
    JObject m_obj;
};

class MediaPlaybackRequestorBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    MediaPlaybackRequestorBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_mediaPlaybackRequestorHandler;
    }

    std::shared_ptr<MediaPlaybackRequestorHandler> getMediaPlaybackRequestor() {
        return m_mediaPlaybackRequestorHandler;
    }

private:
    std::shared_ptr<MediaPlaybackRequestorHandler> m_mediaPlaybackRequestorHandler;
};

//
// JInvocationReason
//
class JMediaPlaybackRequestorInvocationReasonConfig
        : public EnumConfiguration<MediaPlaybackRequestorHandler::InvocationReason> {
public:
    using T = MediaPlaybackRequestorHandler::InvocationReason;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/MediaPlaybackRequestor$InvocationReason";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::AUTOMOTIVE_STARTUP, "AUTOMOTIVE_STARTUP"}, {T::EXPLICIT_USER_ACTION, "EXPLICIT_USER_ACTION"}};
    }
};

using JInvocationReason =
    JEnum<MediaPlaybackRequestorHandler::InvocationReason, JMediaPlaybackRequestorInvocationReasonConfig>;

//
// JMediaPlaybackRequestStatus
//
class JMediaPlaybackRequestorMediaPlaybackRequestStatusConfig
        : public EnumConfiguration<MediaPlaybackRequestorHandler::MediaPlaybackRequestStatus> {
public:
    using T = MediaPlaybackRequestorHandler::MediaPlaybackRequestStatus;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/MediaPlaybackRequestor$MediaPlaybackRequestStatus";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {
            {T::SUCCESS, "SUCCESS"},
            {T::FAILED_CAN_RETRY, "FAILED_CAN_RETRY"},
            {T::FAILED_TIMEOUT, "FAILED_TIMEOUT"},
            {T::ERROR, "ERROR"},
        };
    }
};

using JMediaPlaybackRequestStatus = JEnum<
    MediaPlaybackRequestorHandler::MediaPlaybackRequestStatus,
    JMediaPlaybackRequestorMediaPlaybackRequestStatusConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_MEDIAPLAYBACKREQUESTORBINDER_H_
