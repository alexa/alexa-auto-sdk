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

#ifndef AACE_JNI_ALEXA_ALERTS_BINDER_H
#define AACE_JNI_ALEXA_ALERTS_BINDER_H

#include <AACE/Alexa/Alerts.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace alexa {

class AlertsHandler : public aace::alexa::Alerts {
public:
    AlertsHandler(jobject obj);

    // aace::alexa::Alerts
    void alertStateChanged(
        const std::string& alertToken,
        aace::alexa::Alerts::AlertState state,
        const std::string& reason) override;
    void alertCreated(const std::string& alertToken, const std::string& detailedInfo) override;
    void alertDeleted(const std::string& alertToken) override;

private:
    JObject m_obj;
};

class AlertsBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    AlertsBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_alertsHandler;
    }

    std::shared_ptr<AlertsHandler> getAlerts() {
        return m_alertsHandler;
    }

private:
    std::shared_ptr<AlertsHandler> m_alertsHandler;
};

//
// JAlertState
//
class JAlertStateConfig : public EnumConfiguration<AlertsHandler::AlertState> {
public:
    using T = AlertsHandler::AlertState;

    const char* getClassName() override {
        return "com/amazon/aace/alexa/Alerts$AlertState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::READY, "READY"},
                {T::STARTED, "STARTED"},
                {T::STOPPED, "STOPPED"},
                {T::SNOOZED, "SNOOZED"},
                {T::COMPLETED, "COMPLETED"},
                {T::PAST_DUE, "PAST_DUE"},
                {T::FOCUS_ENTERED_FOREGROUND, "FOCUS_ENTERED_FOREGROUND"},
                {T::FOCUS_ENTERED_BACKGROUND, "FOCUS_ENTERED_BACKGROUND"},
                {T::ERROR, "ERROR"},
                {T::DELETED, "DELETED"},
                {T::SCHEDULED_FOR_LATER, "SCHEDULED_FOR_LATER"}};
    }
};

using JAlertState = JEnum<JAlertStateConfig::T, JAlertStateConfig>;

}  // namespace alexa
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_ALEXA_ALERTS_BINDER_H
