/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ALEXA_FEATURE_DISCOVERY_H_
#define AACE_ALEXA_FEATURE_DISCOVERY_H_

#include <AACE/Core/PlatformInterface.h>

#include "AACE/Alexa/AlexaEngineInterfaces.h"

/** @file */

namespace aace {
namespace alexa {

class FeatureDiscovery : public aace::core::PlatformInterface {
protected:
    FeatureDiscovery() = default;

public:
    virtual ~FeatureDiscovery();

    /** 
     * Retrieves the features using the given discovery requests.
     * This is an asynchronous call. The result of the operation is provided by 
     * the featuresReceived() callback. 
     * 
     * @param [in] requestId The unique identifier of this request.
     * @param [in] discoveryRequests  An escaped JSON string of the feature discovery requests.
     * 
     * Sample discoveryRequests payload :
     * @code{.json}
     * [
     *     {
     *          "locale" : "<LOCALE>",
     *          "domain" : "<DOMAIN>",
     *          "eventType" : "<EVENT_TYPE>",
     *          "limit": <LIMIT>
     *     }
     * ]
     * @endcode
     * 
     * Refer to the Alexa module documentation for complete details of the 
     * discoveryRequests payload.
     */
    void getFeatures(const std::string& requestId, const std::string& discoveryRequests);

    /** 
     * Notifies the platform implementation of the discovered features.
     * The @c requestId corresponds to the ID from the original @c getFeatures() request.
     * 
     * @param [in] requestId The unique identifier of this request.
     * @param [in] discoveryResponses  An escaped JSON string of the feature discovery responses.
     * 
     * Sample discoveryResponses payload :
     * @code{.json}
     * [
     *     {
     *         "domain" : "<DOMAIN>",
     *         "eventType" : "<EVENT_TYPE>",
     *         "locale" : "<LOCALE>",
     *         "localizedContent": [
     *             {
     *                 "utteranceText": "Alexa, what time is it?",
     *                 "descriptionText": ""
     *             }
     *         ]
     *     }
     *  ]
     * @endcode
     * 
     * Refer to the Alexa module documentation for complete details of the 
     * discoveryResponses payload.
     */
    virtual void featuresReceived(const std::string& requestId, const std::string& discoveryResponses) = 0;

    /**
     * @internal
     * Sets the Engine interface delegate.
     *
     * Should *never* be called by the platform implementation.
     */
    void setEngineInterface(std::shared_ptr<FeatureDiscoveryEngineInterface> featureDiscoveryEngineInterface);

private:
    std::weak_ptr<FeatureDiscoveryEngineInterface> m_featureDiscoveryEngineInterface;
};

}  // namespace alexa
}  // namespace aace

#endif  // AACE_ALEXA_FEATURE_DISCOVERY_H_
