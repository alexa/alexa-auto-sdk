/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
#ifndef AGL_COMMON_INTERFACES_AFBAPI_H_
#define AGL_COMMON_INTERFACES_AFBAPI_H_

#include <memory>
#include <functional>
#include <string>

#include <json-c/json_object.h>

using namespace std;

namespace agl {
namespace common {
namespace interfaces {

/**
 * Interface to represent AFB Request.
 */
class IAFBRequest {
public:
    /**
     * Gets the native request object.
     */
    virtual void* getNativeRequest() = 0;
};

/**
 * Interface to encapsulate all AFB (AGL Application Framework Binding)
 * functions.
 */
class IAFBApi {
public:
    /**
     * Interface to represent AFB Event
     */
    class IAFBEvent {
    public:
        /**
         * Gets human readable name of the event.
         */
        virtual std::string getName() const = 0;

        /**
         * Returns true if event is valid. False otherwise.
         */
        virtual bool isValid() = 0;

        /**
         * Publish event to all observers.
         *
         * @return The number of observers that received the event.
         */
        virtual int publishEvent(struct json_object* payload) = 0;

        /**
         * Subscribe to the event
         *
         * @c request Party interested in the event.
         */
        virtual bool subscribe(IAFBRequest& request) = 0;

        /**
         * Unsubscribe to the event
         *
         * @c request Party no longer interested in the event.
         */
        virtual bool unsubscribe(IAFBRequest& request) = 0;
    };

    /**
     * Creates a new AFB event.
     *
     * @param eventName name of the event.
     * @return Event object if successfully created, std::shared_ptr(nullptr) otherwise.
     */
    virtual std::shared_ptr<IAFBEvent> createEvent(const std::string& eventName) = 0;

    virtual int callSync(
        const std::string& api,
        const std::string& verb,
        struct json_object* request,
        struct json_object** result,
        std::string& error,
        std::string& info) = 0;

    typedef std::function<void(json_object* response, const std::string err, const std::string info)> CallbackFn;

    virtual void callAsync(
        const std::string& api,
        const std::string& verb,
        struct json_object* request,
        CallbackFn callbackFn) = 0;
};

}  // namespace interfaces
}  // namespace common
}  // namespace agl

#endif  // AGL_COMMON_INTERFACES_AFBAPI_H_
