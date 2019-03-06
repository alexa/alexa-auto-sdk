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
#ifndef AGL_AFB_AFBREQUESTIMPL_H_
#define AGL_AFB_AFBREQUESTIMPL_H_

#include <memory>

extern "C" {
#include "ctl-plugin.h"
}

#include "interfaces/afb/IAFBApi.h"

namespace agl {
namespace afb {

/**
 * AFB Request impl
 */
class AFBRequestImpl : public agl::common::interfaces::IAFBRequest {
public:
    static std::unique_ptr<AFBRequestImpl> create(AFB_ReqT afbRequest);

    // {@c IAFBRequest Implementation
    void* getNativeRequest() override;
    // @c IAFBRequest Implementation }

private:
    AFBRequestImpl(AFB_ReqT afbRequest);

    AFB_ReqT mAfbRequest;
};

}  // namespace afb
}  // namespace agl

#endif  // AGL_AFB_AFBREQUESTIMPL_H_
