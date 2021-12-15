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

#ifndef AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_HANDLER_INTERFACE_H
#define AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_HANDLER_INTERFACE_H

#include <string>

namespace aace {
namespace engine {
namespace customDomain {

class CustomDomainHandlerInterface {
public:
    /**
     * Destructor
     */
    virtual ~CustomDomainHandlerInterface() = default;

    /**
     * Handle the action specified by the directive identified by @c directiveNamespace, @c name, and @c payload.
     * If handling this directive succeeds, this @c CustomDomainHandlerInterface should call 
     * @c CustomDomainCapabilityAgent::reportDirectiveHandlingResult() with matching @c messageId and
     * @c succeeded @c true. Otherwise, if handling of this directive fails such that subsequent directives with
     * the same dialog request ID should be cancelled, this @c CustomDomainHandlerInterface should instead call 
     * @c CustomDomainCapabilityAgent::reportDirectiveHandlingResult() with @c succeeded @c false and 
     * @c errorType describing the failure.
     *
     * @note The implementation of this function must be thread-safe.
     * @note The implementation of this function must return quickly. Failure to do so blocks the processing of
     * subsequent directives
     * @param directiveNamespace The namespace of the directive. It must be prefixed with "Custom" and followed by the encrypted vendorId from the manufacturer.
     * @param name The name of the directive
     * @param payload An opaque JSON directive sent to the device
     * @param correlationToken An opaque token that must be included in any events responding to this directive
     * @param messageId A unique ID used to identify a specific directive. Used to report directive handling result.
     */
    virtual void handleDirective(
        const std::string& directiveNamespace,
        const std::string& name,
        const std::string& payload,
        const std::string& correlationToken,
        const std::string& messageId) = 0;

    /**
     * Cancel an ongoing @c handleDirective() operation with matching @c directiveNamespace, @c name, @c correlationToken, and @c messageId.
     *
     * @note The implementation of this method MUST be thread-safe.
     * @note The implementation of this method MUST return quickly. Failure to do so blocks the processing
     * of subsequent directives.
     *
     * @param directiveNamespace The namespace of the cancelled directive. 
     * @param name The name of the cancelled directive
     * @param correlationToken The correlationToken of the cancelled directive
     * @param messageId A unique ID used to identify a specific directive. 
     */
    virtual void cancelDirective(
        const std::string& directiveNamespace,
        const std::string& name,
        const std::string& correlationToken,
        const std::string& messageId) = 0;

    /**
    * Called to query current custom states under given namespace from the device.
    * @param contextNamespace The namespace of the context.
    * @return The JSON string of the context value 
    * @code{.json}
    * {
    *       "context": [                                               
    *       {
    *           "name": {{String}},
    *           "value": {{Object}},
    *           "timeOfSample": {{String}},                                
    *           "uncertaintyInMilliseconds": {{Integer}}                   
    *       },{
    *           "name": {{String}},
    *           "value": {{Object}},
    *           "timeOfSample": {{String}},                                
    *           "uncertaintyInMilliseconds": {{Integer}}                   
    *       },
    *       ...
    *       ]
    * }
    */
    virtual std::string getContext(const std::string& contextNamespace) = 0;
};
}  // namespace customDomain
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CUSTOMDOMAIN_CUSTOMDOMAIN_HANDLER_INTERFACE_H