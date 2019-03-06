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

#include "afb/AFBApiImpl.h"

#include "afb/include/AFBEventImpl.h"
#include "utilities/logging/Logger.h"

extern "C" {
#define AFB_BINDING_VERSION 3
#define FREEIF(x) \
    if (!x) {     \
        free(x);  \
    }
#define BREAKIF(x)      \
    if (x) {            \
        result = false; \
        break;          \
    }

#include "afb-definitions.h"
}

static std::string TAG = "agl::afb::AFBApiImpl";

/**
 * Specifies the severity level of a log message
 */
using Level = agl::common::interfaces::ILogger::Level;
using namespace agl::common::interfaces;
using namespace agl::utilities::logging;

namespace agl {
namespace afb {

std::unique_ptr<AFBApiImpl> AFBApiImpl::create(AFB_ApiT api) {
    return std::unique_ptr<AFBApiImpl>(new AFBApiImpl(api));
}

AFBApiImpl::AFBApiImpl(AFB_ApiT api) : mApi(api), mLogger(Logger::create(api)) {
}

AFBApiImpl::~AFBApiImpl() {
}

std::shared_ptr<IAFBApi::IAFBEvent> AFBApiImpl::createEvent(const std::string& eventName) {
    return AFBEventImpl::create(mLogger, mApi, eventName);
}

int AFBApiImpl::callSync(
    const std::string& api,
    const std::string& verb,
    struct json_object* request,
    struct json_object** result,
    std::string& error,
    std::string& info) {
    char* errorStr = NULL;
    char* infoStr = NULL;
    int rc = AFB_ApiSync(mApi, api.c_str(), verb.c_str(), request, result, &errorStr, &infoStr);

    if (errorStr) {
        error = errorStr;
        free(errorStr);
    }

    if (infoStr) {
        info = infoStr;
        free(infoStr);
    }

    return rc;
}

/// Shim to transfer C++ function to C callback using void*
typedef struct {
    IAFBApi::CallbackFn callbackFn;
} CallbackShim;

static void asyncCallback(
    void* closure,
    struct json_object* object,
    const char* error,
    const char* info,
    AFB_ApiT api) {
    CallbackShim* callbackShim = (CallbackShim*)closure;
    auto callbackFn = callbackShim->callbackFn;
    delete callbackShim;

    if (callbackFn) {
        callbackFn(object, error ? std::string(error) : std::string(""), info ? std::string(info) : std::string(""));
    }
}

void AFBApiImpl::callAsync(
    const std::string& api,
    const std::string& verb,
    struct json_object* request,
    CallbackFn callbackFn) {
    CallbackShim* callbackShim = new CallbackShim();
    callbackShim->callbackFn = callbackFn;

    AFB_ApiCall(mApi, api.c_str(), verb.c_str(), request, asyncCallback, callbackShim);
}

}  // namespace afb
}  // namespace agl
