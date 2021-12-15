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

#include <AACE/AASB/AASB.h>

namespace aace {
namespace aasb {

AASB::~AASB() = default;

void AASB::setEngineInterface(std::shared_ptr<AASBEngineInterface> aasbEngineInterface) {
    m_aasbEngineInterface = aasbEngineInterface;
}

//
// AASBEngineInterface
//

void AASB::publish(const std::string& message) {
    if (m_aasbEngineInterface != nullptr) {
        m_aasbEngineInterface->onPublish(message);
    }
}

std::shared_ptr<AASBStream> AASB::openStream(const std::string& streamId, AASBStream::Mode mode) {
    return m_aasbEngineInterface != nullptr ? m_aasbEngineInterface->onOpenStream(streamId, mode) : nullptr;
}

}  // namespace aasb
}  // namespace aace
