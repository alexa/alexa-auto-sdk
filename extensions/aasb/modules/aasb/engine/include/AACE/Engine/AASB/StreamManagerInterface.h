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

#ifndef AACE_ENGINE_AASB_STREAM_MANAGER_INTERFACE_H
#define AACE_ENGINE_AASB_STREAM_MANAGER_INTERFACE_H

#include <string>
#include <memory>

#include <AACE/AASB/AASBStream.h>

namespace aace {
namespace engine {
namespace aasb {

class StreamManagerInterface {
public:
    virtual bool registerStreamHandler(const std::string& streamId, std::shared_ptr<aace::aasb::AASBStream> stream) = 0;

    virtual std::shared_ptr<aace::aasb::AASBStream> requestStreamHandler(
        const std::string& streamId,
        aace::aasb::AASBStream::Mode mode) = 0;
};

}  // namespace aasb
}  // namespace engine
}  // namespace aace

#endif
