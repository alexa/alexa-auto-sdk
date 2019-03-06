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

#include "afb/AFBRequestImpl.h"

extern "C" {
#include "afb-definitions.h"
}

namespace agl {
namespace afb {

std::unique_ptr<AFBRequestImpl> AFBRequestImpl::create(AFB_ReqT afbRequest) {
    return std::unique_ptr<AFBRequestImpl>(new AFBRequestImpl(afbRequest));
}

AFBRequestImpl::AFBRequestImpl(AFB_ReqT afbRequest) : mAfbRequest(afbRequest) {
}

void* AFBRequestImpl::getNativeRequest() {
    return mAfbRequest;
}

}  // namespace afb
}  // namespace agl
