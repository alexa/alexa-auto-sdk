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

#include "aasb/RequestHandler.h"

#include "RequestProcessor.h"
#include "PlatformSpecificLoggingMacros.h"

namespace aasb {
namespace bridge {

RequestHandler::RequestHandler() {
    m_request_processor = aasb::bridge::RequestProcessor::create();
    if (m_request_processor != nullptr)
        AASB_INFO("Alexa Auto Services Bridge created.");
    else
        AASB_ERROR("Failed to create Alexa Auto Services Bridge.");
}

bool RequestHandler::init(std::shared_ptr<IConfigurationProvider> configProvider) {
    if (m_request_processor == nullptr) return false;

    return m_request_processor->init(configProvider);
}

bool RequestHandler::start() {
    if (m_request_processor == nullptr) return false;

    return m_request_processor->start();
}

bool RequestHandler::registerCapabilityDirectiveListener(std::shared_ptr<IAlexaCapabilityDirectiveListener> listener) {
    if (m_request_processor == nullptr) return false;

    return m_request_processor->registerCapabilityDirectiveListener(listener);
}

void RequestHandler::onReceivedEvent(const std::string& topic, const std::string& action, const std::string& payload) {
    m_request_processor->onReceivedEvent(topic, action, payload);
}

bool RequestHandler::startListening(std::string mode) {
    return m_request_processor->startListening(mode);
}

ssize_t RequestHandler::writeAudioSamples(const int16_t* data, const size_t size) {
    return m_request_processor->writeAudioSamples(data, size);
}

std::string RequestHandler::getAuthState() {
    return m_request_processor->getAuthState();
}

std::string RequestHandler::getDialogState() {
    return m_request_processor->getDialogState();
}

std::string RequestHandler::getConnectionStatus() {
    return m_request_processor->getConnectionStatus();
}
}  // namespace bridge
}  // namespace aasb
