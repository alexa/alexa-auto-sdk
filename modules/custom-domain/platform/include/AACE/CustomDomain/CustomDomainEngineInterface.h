/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_CUSTOMDOMAIN_CUSTOMDOMAINENGINEINTERFACE_H
#define AACE_CUSTOMDOMAIN_CUSTOMDOMAINENGINEINTERFACE_H

namespace aace {
namespace customDomain {

class CustomDomainEngineInterface {
public:
    /**
     * Specifies the type of directive handling result.
     */
    enum class ResultType {
        /**
         * The directive sent to your client was malformed or the payload does not conform to the directive specification.
         */
        UNEXPECTED_INFORMATION_RECEIVED,

        /**
         * The operation specified by the namespace/name in the directive's header are not supported by the client.
         */
        UNSUPPORTED_OPERATION,

        /**
         * An error occurred while the device was handling the directive and the error does not fall into the specified categories.
         */
        INTERNAL_ERROR,

        /**
         * The directive handling is successful
         */
        SUCCESS
    };

    virtual void onSendEvent(
        const std::string& eventNamespace,
        const std::string& name,
        const std::string& payload,
        bool requiresContext,
        const std::string& correlationToken,
        const std::string& customContext) = 0;

    virtual void onReportDirectiveHandlingResult(
        const std::string& directiveNamespace,
        const std::string& messageId,
        ResultType result) = 0;
};

}  // namespace customDomain
}  // namespace aace

#endif  // AACE_CUSTOMDOMAIN_CUSTOMDOMAINENGINEINTERFACE_H
