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

#include "AACE/CBL/CBL.h"

namespace aace {
namespace cbl {

CBL::~CBL() = default;

void CBL::setEngineInterface(std::shared_ptr<CBLEngineInterface> cblEngineInterface) {
    m_cblEngineInterface = cblEngineInterface;
}

void CBL::start() {
    if (m_cblEngineInterface != nullptr) {
        m_cblEngineInterface->onStart();
    }
}

void CBL::cancel() {
    if (m_cblEngineInterface != nullptr) {
        m_cblEngineInterface->onCancel();
    }
}

void CBL::reset() {
    if (m_cblEngineInterface != nullptr) {
        m_cblEngineInterface->onReset();
    }
}

}  // namespace cbl
}  // namespace aace
