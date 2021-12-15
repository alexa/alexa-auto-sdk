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

#ifndef AACE_JNI_ALEXA_NOTIFICATIONS_BINDER_H
#define AACE_JNI_ALEXA_NOTIFICATIONS_BINDER_H

#include <AACE/Alexa/Notifications.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class NotificationsHandler : public aace::alexa::Notifications {
public:
    NotificationsHandler(jobject obj);

    // aace::alexa::Notifications
    void setIndicator(IndicatorState state) override;

    void onNotificationReceived() override;

private:
    JObject m_obj;
};

class NotificationsBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    NotificationsBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_notificationsHandler;
    }

    std::shared_ptr<NotificationsHandler> getNotifications() {
        return m_notificationsHandler;
    }

private:
    std::shared_ptr<NotificationsHandler> m_notificationsHandler;
};

//
// JIndicatorStateConfig
//

class JIndicatorStateConfig : public EnumConfiguration<NotificationsHandler::IndicatorState> {
public:
    using T = NotificationsHandler::IndicatorState;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/Notifications$IndicatorState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::OFF, "OFF"}, {T::ON, "ON"}, {T::UNKNOWN, "UNKNOWN"}};
    }
};

using JIndicatorState = JEnum<NotificationsHandler::IndicatorState, JIndicatorStateConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_NOTIFICATIONS_BINDER_H
