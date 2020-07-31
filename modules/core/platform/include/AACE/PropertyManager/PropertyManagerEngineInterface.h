/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_INTERFACE_H
#define AACE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_INTERFACE_H

/** @file */

namespace aace {
namespace propertyManager {

class PropertyManagerEngineInterface {
public:
    /**
     * Describes the state of a property change.
     */
    enum class PropertyState {

        /**
         * The property change was successful.
         */
        SUCCEEDED,

        /**
         * The property change failed.
         */
        FAILED,

    };
    virtual bool onSetProperty(const std::string& name, const std::string& value) = 0;
    virtual std::string onGetProperty(const std::string& name) = 0;
};

}  // namespace propertyManager
}  // namespace aace

#endif  // AACE_PROPERTY_MANAGER_PROPERTY_MANAGER_ENGINE_INTERFACE_H
