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

#ifndef AACE_AASB_AASB_H
#define AACE_AASB_AASB_H

#include <AACE/Core/PlatformInterface.h>
#include "AASBEngineInterfaces.h"

#include <string>

namespace aace {
namespace aasb {

/**
 * AASB should be extended to handle sending and receiving AASB messages
 * to and from the Engine.
 */
class AASB : public aace::core::PlatformInterface {
protected:
    AASB() = default;

public:
    virtual ~AASB();

    /**
     * Notifies the platform implementation that an AASB message has been received from the Engine.
     *
     * @param [in] topic The AASB message topic.
     * @param [in] topic The AASB message action.
     * @param [in] messageId The unique id for this AASB message.     
     * @param [in] payload The AASB message payload.
     */
    virtual void messageReceived(const std::string& message) = 0;

    /**
     * Publishes an AASB message to the Engine.
     *
     * @param [in] message The AASB message.
     */
    void publish(const std::string& message);

    /**
     * Opens an AASB stream that has been registered by the Engine.
     *
     * @param [in] streamId The @c id of the stream being opened.
     * @param [in] mode The stream operation @ mode being requested.
     */
    std::shared_ptr<AASBStream> openStream(const std::string& streamId, AASBStream::Mode mode);

    /**
     * @internal
     * Sets the Engine interface delagate
     * 
     * Should *never* be called by the platform implementation
     */
    void setEngineInterface(std::shared_ptr<AASBEngineInterface> aasbEngineInterface);

private:
    std::shared_ptr<AASBEngineInterface> m_aasbEngineInterface;
};

}  // namespace aasb
}  // namespace aace

#endif
