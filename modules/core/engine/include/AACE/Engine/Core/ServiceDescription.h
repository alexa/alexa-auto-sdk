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

#ifndef AACE_ENGINE_CORE_SERVICE_DESCRIPTION_H
#define AACE_ENGINE_CORE_SERVICE_DESCRIPTION_H

#include <string>
#include <vector>

namespace aace {
namespace engine {
namespace core {

class Version {
public:
    Version(uint8_t major = 0, uint8_t minor = 0, uint8_t revision = 0, const std::string& tag = "");
    Version(const std::string& str);
    Version(const Version& version);

    std::string toString();

    bool operator==(const Version& other);
    bool operator<(const Version& other);

    friend std::ostream& operator<<(std::ostream& stream, const Version& version);

private:
    uint8_t m_major;
    uint8_t m_minor;
    uint8_t m_revision;
    std::string m_tag;
};

class ServiceDescription {
public:
    ServiceDescription() = default;
    ServiceDescription(const ServiceDescription& desc);
    ServiceDescription(
        const std::string& type,
        const Version& version,
        std::initializer_list<ServiceDescription> dependencies = {});

    const std::string& getType() const;
    const Version& getVersion() const;
    const std::vector<ServiceDescription>& getDependencies() const;

    ServiceDescription v(const Version& version) const;

private:
    std::string m_type;
    Version m_version;
    std::vector<ServiceDescription> m_dependencies;
};

}  // namespace core
}  // namespace engine
}  // namespace aace

// ServiceDescription Macros
#define SERVICE_REGISTERED_VAR_NAME s_engineService_registered

#define DESCRIBE(type, version, ...)                                                 \
public:                                                                              \
    static const aace::engine::core::ServiceDescription getServiceDescription() {    \
        return aace::engine::core::ServiceDescription(type, version, {__VA_ARGS__}); \
    }                                                                                \
                                                                                     \
private:                                                                             \
    static const bool SERVICE_REGISTERED_VAR_NAME;

#define DEPENDS(class) class ::getServiceDescription()
#define DEPENDS_V(class, ver) class ::getServiceDescription().v(ver)
#define VERSION(ver) aace::engine::core::Version(ver)

#endif  // AACE_ENGINE_CORE_SERVICE_DESCRIPTION_H
