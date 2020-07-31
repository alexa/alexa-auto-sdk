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

#include <iostream>
#include <sstream>
#include <inttypes.h>

#include "AACE/Engine/Core/ServiceDescription.h"

namespace aace {
namespace engine {
namespace core {

ServiceDescription::ServiceDescription(const ServiceDescription& desc) {
    m_type = desc.m_type;
    m_version = desc.m_version;
    m_dependencies = desc.m_dependencies;
}

ServiceDescription::ServiceDescription(
    const std::string& type,
    const Version& version,
    std::initializer_list<ServiceDescription> dependencies) :
        m_type(type), m_version(version), m_dependencies(dependencies) {
}

const std::string& ServiceDescription::getType() const {
    return m_type;
}

const Version& ServiceDescription::getVersion() const {
    return m_version;
}

const std::vector<ServiceDescription>& ServiceDescription::getDependencies() const {
    return m_dependencies;
}

ServiceDescription ServiceDescription::v(const Version& version) const {
    ServiceDescription desc = *this;

    desc.m_version = version;

    return desc;
}

//
// Version
//

Version::Version(uint8_t major, uint8_t minor, uint8_t revision, const std::string& tag) :
        m_major(major), m_minor(minor), m_revision(revision), m_tag(tag) {
}

Version::Version(const std::string& version) : m_major(0), m_minor(0), m_revision(0) {
    char tag[65] = {0};

    std::sscanf(version.c_str(), "%2" SCNu8 ".%2" SCNu8 ".%2" SCNu8 "-%64s", &m_major, &m_minor, &m_revision, tag);

    m_tag = tag;
}

Version::Version(const Version& version) {
    m_major = version.m_major;
    m_minor = version.m_minor;
    m_revision = version.m_revision;
    m_tag = version.m_tag;
}

std::string Version::toString() {
    std::stringstream ss;

    ss << *this;

    return ss.str();
}

bool Version::operator==(const Version& other) {
    return m_major == other.m_major && m_minor == other.m_minor && m_revision == other.m_revision;
}

bool Version::operator<(const Version& other) {
    return m_major < other.m_major || m_minor < other.m_minor || m_revision < other.m_revision;
}

std::ostream& operator<<(std::ostream& stream, const Version& version) {
    stream << std::to_string(version.m_major) << "." << std::to_string(version.m_minor) << "."
           << std::to_string(version.m_revision);

    if (version.m_tag.empty() == false) {
        stream << "-" << version.m_tag;
    }

    return stream;
}

}  // namespace core
}  // namespace engine
}  // namespace aace
