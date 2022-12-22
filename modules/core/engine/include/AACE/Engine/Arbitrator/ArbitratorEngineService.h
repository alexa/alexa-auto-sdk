/*
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     https://aws.amazon.com/asl/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */


#ifndef AACE_ENGINE_ARBITRATOR_ARBITRATOR_ENGINE_SERVICE_H
#define AACE_ENGINE_ARBITRATOR_ARBITRATOR_ENGINE_SERVICE_H

#include <unordered_map>

#include <AACE/Arbitrator/ArbitratorEngineInterface.h>
#include <AACE/Engine/Core/EngineService.h>

#include "ArbitratorEngineImpl.h"
#include "ArbitratorObserverInterface.h"
#include "ArbitratorServiceInterface.h"

namespace aace {
namespace engine {
namespace arbitrator {

class ArbitratorEngineService
        : public aace::engine::core::EngineService
        , public ArbitratorServiceInterface
        , public std::enable_shared_from_this<ArbitratorEngineService> {
public:
    DESCRIBE("aace.arbitrator", VERSION("1.0"))

private:
    ArbitratorEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~ArbitratorEngineService() = default;

    /// @name ArbitratorServiceInterface
    /// @{
    bool registerAgent(
        const std::string& assistantId,
        const std::string& name,
        std::map<std::string, bool> dialogStateRules) override;
    bool deregisterAgent(const std::string& assistantId) override;
    bool startDialog(
        const std::string& assistantId,
        const std::string& mode,
        std::string& dialogId,
        std::string& denyReason) override;
    void stopDialog(const std::string& assistantId, const std::string& dialogId) override;
    void setDialogState(const std::string& assistantId, const std::string& dialogId, const std::string& state) override;
    void addObserver(
        std::shared_ptr<aace::engine::arbitrator::ArbitratorObserverInterface> arbitratorObserver) override;
    void removeObserver(
        std::shared_ptr<aace::engine::arbitrator::ArbitratorObserverInterface> arbitratorObserver) override;
    /// @}

protected:
    bool configure(std::shared_ptr<std::istream> configuration) override;
    bool shutdown() override;
    bool registerPlatformInterface(std::shared_ptr<aace::core::PlatformInterface> platformInterface) override;

private:
    // platform interface registration
    template <class T>
    bool registerPlatformInterfaceType(std::shared_ptr<aace::core::PlatformInterface> platformInterface) {
        std::shared_ptr<T> typedPlatformInterface = std::dynamic_pointer_cast<T>(platformInterface);
        return typedPlatformInterface != nullptr ? registerPlatformInterfaceType(typedPlatformInterface) : false;
    }

    bool registerPlatformInterfaceType(std::shared_ptr<aace::arbitrator::Arbitrator> arbitrator);

private:
    /// The reference to the arbitrator engine implementation
    std::shared_ptr<ArbitratorEngineImpl> m_arbitratorEngineImpl;

    /// Arbitrator system level interruption config
    std::string m_arbitratorConfig;
};

}  // namespace arbitrator
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_ARBITRATOR_ARBITRATOR_ENGINE_SERVICE_H
