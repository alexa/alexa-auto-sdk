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
#include "afb/include/AFBEventImpl.h"

static string TAG = "agl::afb::Event";

using Level = agl::common::interfaces::ILogger::Level;
using namespace agl::common::interfaces;

namespace agl {
namespace afb {

unique_ptr<AFBEventImpl> AFBEventImpl::create(
    shared_ptr<agl::common::interfaces::ILogger> logger,
    AFB_ApiT api,
    const string& eventName) {
    return unique_ptr<AFBEventImpl>(new AFBEventImpl(logger, api, eventName));
}

AFBEventImpl::AFBEventImpl(shared_ptr<agl::common::interfaces::ILogger> logger, AFB_ApiT api, const string& eventName) :
        mLogger(logger),
        mAfbApi(api),
        mEventName(eventName),
        mAfbEvent(nullptr) {
}

AFBEventImpl::~AFBEventImpl() {
}

string AFBEventImpl::getName() const {
    return mEventName;
}

bool AFBEventImpl::isValid() {
    makeEventIfNeccessary();
    return afb_event_is_valid(mAfbEvent) == 1 ? true : false;
}

bool AFBEventImpl::subscribe(IAFBRequest& requestInterface) {
    makeEventIfNeccessary();
    auto request = static_cast<AFB_ReqT>(requestInterface.getNativeRequest());
    if (isValid() && afb_req_subscribe(request, mAfbEvent) == 0) {
        return true;
    }

    return false;
}

bool AFBEventImpl::unsubscribe(IAFBRequest& requestInterface) {
    makeEventIfNeccessary();
    auto request = static_cast<AFB_ReqT>(requestInterface.getNativeRequest());
    if (isValid() && afb_req_unsubscribe(request, mAfbEvent) == 0) {
        return true;
    }

    return false;
}

int AFBEventImpl::publishEvent(struct json_object* payload) {
    makeEventIfNeccessary();
    return afb_event_push(mAfbEvent, payload);
}

void AFBEventImpl::makeEventIfNeccessary() {
    if (mAfbEvent) {
        return;
    }

    mLogger->log(Level::NOTICE, TAG, "Creating event: " + mEventName);
    mAfbEvent = afb_api_make_event(mAfbApi, mEventName.c_str());
}

}  // namespace afb
}  // namespace agl
