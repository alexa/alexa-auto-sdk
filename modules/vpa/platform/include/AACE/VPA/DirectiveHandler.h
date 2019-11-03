/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_VPA_DIRECTIVEHANDLER_H
#define AACE_VPA_DIRECTIVEHANDLER_H

#include <string>

#include "AACE/Core/PlatformInterface.h"
#include "AACE/Engine/Storage/LocalStorageInterface.h"

/** @file */

namespace aace {
namespace vpa {

/**
 * Navigation should be extended to handle navigation directives from the Engine.
 */
class VpaDirective : public aace::core::PlatformInterface {
protected:
    VpaDirective() = default;

public:
    virtual ~VpaDirective();

    virtual bool sendDirective( const std::string& payload ) = 0;
    virtual void setLocalStorage (std::shared_ptr<aace::engine::storage::LocalStorageInterface> storage) = 0;
};

} // aace::vpa
} // aace

#endif // AACE_VPA_DIRECTIVEHANDLER_H
