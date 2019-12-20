/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_ENGINE_SERVICE_H
#define AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_ENGINE_SERVICE_H

#include <memory>
#include <unordered_map>

#include "AACE/CarControl/CarControl.h"
#include "AACE/Engine/CarControl/CarControlEngineImpl.h"
#include "AACE/Engine/CarControl/Endpoint.h"
#include "AACE/Engine/Storage/StorageEngineService.h"

namespace aace {
namespace engine {
namespace carControl {

class CarControlEngineService :
    public aace::engine::core::EngineService,
    public std::enable_shared_from_this<CarControlEngineService> {
public:
    DESCRIBE("aace.carControl", VERSION("1.0"), DEPENDS(aace::engine::storage::StorageEngineService))
public:
    virtual ~CarControlEngineService();

    /// @name @c EngineService methods
    /// @{
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

    /**
     * Set whether the @c CarControlLocalService is available
     *
     * @param [in] isAvailable @c true if the local service is available
     */
    virtual void setLocalServiceAvailability(bool isAvailable);
    /**
     * Check whether the @c CarControlLocalService is available
     *
     * @return @c true if the local service is available
     */
    virtual bool isLocalServiceAvailable();;

protected:
    bool initialize() override;
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool setup() override;
    bool shutdown() override;
    bool setProperty(const std::string& key, const std::string& value) override;
    /// @}

    CarControlEngineService(const aace::engine::core::ServiceDescription& description);

private:
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }
    bool registerPlatformInterfaceType(std::shared_ptr<aace::carControl::CarControl> platformInterface);

    /// The engine implementation instance
    std::shared_ptr<aace::engine::carControl::CarControlEngineImpl> m_carControlEngineImpl;
    /// Whether the @c CarControlEngineService has been configured
    bool m_configured = false;
    /// A map of endpoint ID to the corresponding @c Endpoint object
    std::unordered_map<std::string, std::shared_ptr<aace::engine::carControl::Endpoint>> m_endpoints;
    /// Whether the @c CarControlLocalService is available
    bool m_isLocalServiceAvailable = false;
};

}  // namespace carControl
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_CAR_CONTROL_CAR_CONTROL_ENGINE_SERVICE_H
