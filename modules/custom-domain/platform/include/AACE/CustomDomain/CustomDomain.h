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

#ifndef AACE_CUSTOMDOMAIN_CUSTOMDOMAIN_H
#define AACE_CUSTOMDOMAIN_CUSTOMDOMAIN_H

#include <string>

#include "AACE/Core/PlatformInterface.h"
#include "CustomDomainEngineInterface.h"

namespace aace {
namespace customDomain {

class CustomDomain : public aace::core::PlatformInterface {
protected:
    CustomDomain() = default;

public:
    virtual ~CustomDomain();

    using ResultType = aace::customDomain::CustomDomainEngineInterface::ResultType;

    /**
     * Handle the action specified by the directive identified by @c directiveNamespace, @c name, and @c payload. 
     * If handling this directive succeeds, this @c CustomDomain should call @c reportDirectiveHandlingResult()
     * with matching @c directiveNamespace and @c messageId and @c ResultType::SUCCESS. Otherwise, if handling
     * this directive fails such that subsequent directives with
     * the same dialog request ID should be cancelled, this @c CustomDomain should instead call 
     * @c reportDirectiveHandlingResult() with a @c ResultType describing the failure.
     *
     * @note The implementation of this function MUST be thread-safe.
     * @note The implementation of this function MUST return quickly. Failure to do so blocks the processing of
     * subsequent directives
     * 
     * @param [in] directiveNamespace The namespace of the directive. It must be prefixed with "Custom" and followed by the encrypted vendorId from the manufacturer.
     * @param [in] name The name of the directive
     * @param [in] payload An opaque JSON object sent to the device
     * @param [in] correlationToken An opaque token that must be included in any events responding to this directive
     * @param [in] messageId The unique ID of the directive to be handled. Used to report directive handling result.
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
     * @note Do not call @c reportDirectiveHandlingResult for a cancelled directive.
     *
     * @param [in] directiveNamespace The namespace of the cancelled directive. 
     * @param [in] name The name of the cancelled directive
     * @param [in] correlationToken The correlationToken of the cancelled directive
     * @param [in] messageId The unique ID of the directive to cancel. 
     */
    virtual void cancelDirective(
        const std::string& directiveNamespace,
        const std::string& name,
        const std::string& correlationToken,
        const std::string& messageId) = 0;

    /**
    * Called to query current custom states under @c contextNamespace from device.
    * 
    * @note The implementation of this method must be THREAD-SAFE as potentially 
    * Capability Agent instances with multiple threads will invoke this.
    * 
    * @param [in] contextNamespace The namespace of the queried context
    * @return The queried context in string representation of JSON in the following structure:
    * 
    * @code{.json}
    * {
    *       "context": [                                               
    *       {
    *           "name": "{{String}}",
    *           "value": {{Object}} | "{{String}}" | {{Long}},
    *           "timeOfSample": "{{String}}",                                
    *           "uncertaintyInMilliseconds": {{Long}}                   
    *       },{
    *           "name": "{{String}}",
    *           "value": {{Object}} | "{{String}}" | {{Long}},
    *           "timeOfSample": "{{String}}",                                
    *           "uncertaintyInMilliseconds": {{Long}}                    
    *       },
    *       ...
    *       ]
    * }
    * @endcode
    * @li name (required): The name of the context property state.
    * @li value (required): The value of the context property state. Accepted format: JSON Object, string, or number.
    * @li timeOfSample (optional): The time at which the property value was recorded in ISO-8601 representation. If omitted, the default value is the current time. 
    * @li uncertaintyInMilliseconds (optional): The number of milliseconds that have elapsed since the property value was last confirmed. If omitted, the default value is 0.
    */
    virtual std::string getContext(const std::string& contextNamespace) = 0;

    /**
    * Notifies the engine about the result of a directive handling.
    * This should be used in response to @c handleDirective() when the handling is complete.
    * 
    * @param [in] directiveNamespace The namespace of the directive. 
    * @param [in] messageId The messageId that uniquely identifies which directive this report is for.
    * @param [in] result The result of the handling.
    */
    void reportDirectiveHandlingResult(
        const std::string& directiveNamespace,
        const std::string& messageId,
        ResultType result);

    /**
	* Notifes the engine to send a custom event.
    *
    * @param [in] eventNamespace The namespace of the event.
    * @param [in] name The name of the event.
    * @param [in] payload An opaque JSON object sent to the cloud with the event.
    * @param [in] requiresContext A boolean indicating if this event must be sent with context.
    * @param [in] correlationToken Optional. The token correlating this event to a directive. Empty string if this event is not a response to any directive.
    * @param [in] customContext Optional. The context corresponding to @a eventNamespace or an empty string if this event does not require context. 
    * 
    * customContext is a String representation of a JSON object in the following format:
    * @code{.json}
    * {
    *       "context": [                                               
    *       {
    *           "name": "{{String}}",
    *           "value": {{Object}} | "{{String}}" | {{Long}},
    *           "timeOfSample": "{{String}}",                                
    *           "uncertaintyInMilliseconds": {{Long}}                   
    *       },{
    *           "name": "{{String}}",
    *           "value": {{Object}} | "{{String}}" | {{Long}},
    *           "timeOfSample": "{{String}}",                                
    *           "uncertaintyInMilliseconds": {{Long}}                    
    *       },
    *       ...
    *       ]
    * }
    * @endcode
    * @li name (required): The name of the context property state.
    * @li value (required): The value of the context property state. Accepted format: JSON Object, string, or number.
    * @li timeOfSample (optional): The time at which the property value was recorded in ISO-8601 representation. If omitted, the default value is the current time.
    * @li uncertaintyInMilliseconds (optional): The number of milliseconds that have elapsed since the property value was last confirmed. If omitted, the default value is 0.
	*/
    void sendEvent(
        const std::string& eventNamespace,
        const std::string& name,
        const std::string& payload,
        bool requiresContext,
        const std::string& correlationToken = "",
        const std::string& customContext = "");

    /**
    * @internal
    * Sets the Engine interface delegate
    *
    * Should *never* be called by the platform implementation
    */
    void setEngineInterface(std::shared_ptr<CustomDomainEngineInterface> customDomainEngineInterface);

private:
    std::shared_ptr<CustomDomainEngineInterface> m_customDomainEngineInterface;
};

}  // namespace customDomain
}  // namespace aace

#endif  // AACE_CUSTOMDOMAIN_CUSTOMDOMAIN_H