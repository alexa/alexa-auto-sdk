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

#ifndef AACE_JNI_PHONECONTROL_PHONECALLCONTROLLER_BINDER_H
#define AACE_JNI_PHONECONTROL_PHONECALLCONTROLLER_BINDER_H

#include <AACE/PhoneCallController/PhoneCallController.h>
#include <AACE/JNI/Core/PlatformInterfaceBinder.h>

namespace aace {
namespace jni {
namespace phonecontrol {

//
// PhoneCallControllerHandler
//

class PhoneCallControllerHandler : public aace::phoneCallController::PhoneCallController {
public:
    PhoneCallControllerHandler(jobject obj);

    // aace::phoneCallController::PhoneCallController
    bool dial(const std::string& payload) override;
    bool redial(const std::string& payload) override;
    void answer(const std::string& payload) override;
    void stop(const std::string& payload) override;
    void sendDTMF(const std::string& payload) override;

private:
    JObject m_obj;
};

//
// PhoneCallControllerBinder
//

class PhoneCallControllerBinder : public aace::jni::core::PlatformInterfaceBinder {
public:
    PhoneCallControllerBinder(jobject obj);

    std::shared_ptr<aace::core::PlatformInterface> getPlatformInterface() override {
        return m_phoneCallControllerHandler;
    }

    std::shared_ptr<PhoneCallControllerHandler> getPhoneCallController() {
        return m_phoneCallControllerHandler;
    }

private:
    std::shared_ptr<PhoneCallControllerHandler> m_phoneCallControllerHandler;
};

//
// JConnectionState
//

class JConnectionStateConfig : public EnumConfiguration<PhoneCallControllerHandler::ConnectionState> {
public:
    using T = PhoneCallControllerHandler::ConnectionState;

    const char* getClassName() override {
        return "com/amazon/aace/phonecontrol/PhoneCallController$ConnectionState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::CONNECTED, "CONNECTED"}, {T::DISCONNECTED, "DISCONNECTED"}};
    }
};

using JConnectionState = JEnum<PhoneCallControllerHandler::ConnectionState, JConnectionStateConfig>;

//
// JCallState
//

class JCallStateConfig : public EnumConfiguration<PhoneCallControllerHandler::CallState> {
public:
    using T = PhoneCallControllerHandler::CallState;

    const char* getClassName() override {
        return "com/amazon/aace/phonecontrol/PhoneCallController$CallState";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::IDLE, "IDLE"},
                {T::DIALING, "DIALING"},
                {T::OUTBOUND_RINGING, "OUTBOUND_RINGING"},
                {T::ACTIVE, "ACTIVE"},
                {T::CALL_RECEIVED, "CALL_RECEIVED"},
                {T::INBOUND_RINGING, "INBOUND_RINGING"}};
    }
};

using JCallState = JEnum<PhoneCallControllerHandler::CallState, JCallStateConfig>;

//
// JCallingDeviceConigurationProperty
//

class JCallingDeviceConfigurationPropertyConfig
        : public EnumConfiguration<PhoneCallControllerHandler::CallingDeviceConfigurationProperty> {
public:
    using T = PhoneCallControllerHandler::CallingDeviceConfigurationProperty;

    const char* getClassName() override {
        return "com/amazon/aace/phonecontrol/PhoneCallController$CallingDeviceConfigurationProperty";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::DTMF_SUPPORTED, "DTMF_SUPPORTED"}};
    }
};

using JCallingDeviceConfigurationProperty =
    JEnum<PhoneCallControllerHandler::CallingDeviceConfigurationProperty, JCallingDeviceConfigurationPropertyConfig>;

//
// JCallError
//

class JCallErrorConfig : public EnumConfiguration<PhoneCallControllerHandler::CallError> {
public:
    using T = PhoneCallControllerHandler::CallError;

    const char* getClassName() override {
        return "com/amazon/aace/phonecontrol/PhoneCallController$CallError";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::NO_CARRIER, "NO_CARRIER"},
                {T::BUSY, "BUSY"},
                {T::NO_ANSWER, "NO_ANSWER"},
                {T::NO_NUMBER_FOR_REDIAL, "NO_NUMBER_FOR_REDIAL"},
                {T::OTHER, "OTHER"}};
    }
};

using JCallError = JEnum<PhoneCallControllerHandler::CallError, JCallErrorConfig>;

//
// JDTMFError
//

class JDTMFErrorConfig : public EnumConfiguration<PhoneCallControllerHandler::DTMFError> {
public:
    using T = PhoneCallControllerHandler::DTMFError;

    const char* getClassName() override {
        return "com/amazon/aace/phonecontrol/PhoneCallController$DTMFError";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::CALL_NOT_IN_PROGRESS, "CALL_NOT_IN_PROGRES"}, {T::DTMF_FAILED, "DTMF_FAILED"}};
    }
};

using JDTMFError = JEnum<PhoneCallControllerHandler::DTMFError, JDTMFErrorConfig>;

}  // namespace phonecontrol
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_PHONECONTROL_PHONECALLCONTROLLER_BINDER_H