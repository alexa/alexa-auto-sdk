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

#ifndef AACE_ENGINE_STORAGE_STORAGE_ENGINE_SERVICE_H
#define AACE_ENGINE_STORAGE_STORAGE_ENGINE_SERVICE_H

#include "AACE/Engine/Core/EngineService.h"
#include "LocalStorageInterface.h"

namespace aace {
namespace engine {
namespace storage {

class StorageEngineService : public aace::engine::core::EngineService {
public:
    DESCRIBE("aace.storage", VERSION("1.0"))

private:
    StorageEngineService(const aace::engine::core::ServiceDescription& description);

public:
    virtual ~StorageEngineService() = default;

protected:
    bool configure(std::shared_ptr<std::istream> configuration) override;

private:
    std::shared_ptr<LocalStorageInterface> m_localStorage;
};

}  // namespace storage
}  // namespace engine
}  // namespace aace

#endif  // AACE_ENGINE_STORAGE_STORAGE_ENGINE_SERVICE_H
