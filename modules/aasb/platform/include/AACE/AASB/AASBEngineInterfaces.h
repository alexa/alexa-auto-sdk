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

#ifndef AACE_AASB_AASB_ENGINE_INTERFACE_H
#define AACE_AASB_AASB_ENGINE_INTERFACE_H

#include <memory>

#include "AASBStream.h"

namespace aace {
namespace aasb {

class AASBEngineInterface {
public:
    virtual void onPublish(const std::string& message) = 0;
    virtual std::shared_ptr<AASBStream> onOpenStream(const std::string& streamId, AASBStream::Mode mode) = 0;
};

}  // namespace aasb
}  // namespace aace

#endif
