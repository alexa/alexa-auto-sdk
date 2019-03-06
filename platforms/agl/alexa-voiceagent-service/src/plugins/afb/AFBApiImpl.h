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
#ifndef AGL_AFB_AFBAPIIMPL_H_
#define AGL_AFB_AFBAPIIMPL_H_

#include <memory>

extern "C" {
#include "ctl-plugin.h"
}

#include "interfaces/afb/IAFBApi.h"
#include "interfaces/utilities/logging/ILogger.h"

using namespace std;

namespace agl {
namespace afb {

class AFBApiImpl : public agl::common::interfaces::IAFBApi {
public:
    static std::unique_ptr<AFBApiImpl> create(AFB_ApiT api);

    ~AFBApiImpl();

    std::shared_ptr<IAFBEvent> createEvent(const std::string& eventName) override;

    int callSync(
        const std::string& api,
        const std::string& verb,
        struct json_object* request,
        struct json_object** result,
        std::string& error,
        std::string& info) override;

    void callAsync(const std::string& api, const std::string& verb, struct json_object* request, CallbackFn callbackFn)
        override;

private:
    AFBApiImpl(AFB_ApiT api);

    // AFB API Binding
    AFB_ApiT mApi;

    // Logger
    std::shared_ptr<agl::common::interfaces::ILogger> mLogger;
};

}  // namespace afb
}  // namespace agl

#endif  // AGL_AFB_AFBAPIIMPL_H_
