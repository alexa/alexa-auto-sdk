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
#ifndef AGL_AFB_EVENT_H_
#define AGL_AFB_EVENT_H_

#include <memory>
#include <string>

#include "interfaces/afb/IAFBApi.h"
#include "interfaces/utilities/logging/ILogger.h"

extern "C" {
#include "ctl-plugin.h"
#include <json-c/json.h>
}

using namespace std;

namespace agl {
namespace afb {
/*
 * This class encapsulates AFB Event.
 */
class AFBEventImpl : public agl::common::interfaces::IAFBApi::IAFBEvent {
public:
    static unique_ptr<AFBEventImpl> create(
        shared_ptr<agl::common::interfaces::ILogger> logger,
        AFB_ApiT api,
        const string& eventName);

    // Destructor
    ~AFBEventImpl();

    /// { @c IAFBEvent implementation
    string getName() const override;
    bool isValid() override;
    int publishEvent(struct json_object* payload) override;
    bool subscribe(agl::common::interfaces::IAFBRequest& request) override;
    bool unsubscribe(agl::common::interfaces::IAFBRequest& request) override;
    /// @c IAFBEvent implementation }

private:
    AFBEventImpl(shared_ptr<agl::common::interfaces::ILogger> logger, AFB_ApiT api, const string& eventName);

    // Make the event. This is a lazy make that happens
    // usually during the subscribe stage.
    void makeEventIfNeccessary();

    // Binding API reference
    AFB_ApiT mAfbApi;

    // AFB Event
    afb_event_t mAfbEvent;

    // Event Name
    string mEventName;

    // Logger
    shared_ptr<agl::common::interfaces::ILogger> mLogger;
};

}  // namespace afb
}  // namespace agl

#endif  // AGL_AFB_EVENT_H_
