/*
 * Copyright 2017-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <sstream>

// testing includes
#include <AACE/Test/Unit/Core/CoreTestHelper.h>
// engine includes
#include <AACE/Engine/Core/ServiceDescription.h>

using namespace aace::test::unit::core;
using Version = aace::engine::core::Version;

/// Test harness for @c ServiceDescription class
class ServiceDescriptionTest : public ::testing::Test {
public:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(ServiceDescriptionTest, versionAssignment) {
    Version v1("1.2.3-tag");
    ASSERT_TRUE(v1.major_version() == 1) << "Invalid major version!";
    ASSERT_TRUE(v1.minor_version() == 2) << "Invalid minor version!";
    ASSERT_TRUE(v1.revision_version() == 3) << "Invalid revision version!";
    ASSERT_TRUE(v1.tag_version() == "tag") << "Invalid tag version: " << v1.tag_version();
    Version v2 = v1;
    ASSERT_TRUE(v2 == v1) << "Assignment failed!";
}

TEST_F(ServiceDescriptionTest, versionComparator) {
    // test major version comparison
    ASSERT_TRUE(Version("1.0.0") == Version("1.0.0")) << "Major version equal comparator failed!";
    ASSERT_FALSE(Version("1.0.0") == Version("1.1.0")) << "Major version not-equal comparator failed!";
    ASSERT_TRUE(Version("1.0.0") < Version("2.0.0")) << "Major version less-than comparator failed!";
    ASSERT_FALSE(Version("2.0.0") < Version("1.1.1")) << "Major version not less-than comparator failed!";
    ASSERT_TRUE(Version("1.0.0") > Version("0.1.0")) << "Major version greater-than comparator failed!";
    ASSERT_FALSE(Version("0.2.0") > Version("1.1.1")) << "Major version not greater-than comparator failed!";

    // test minor version comparison
    ASSERT_TRUE(Version("1.1.0") == Version("1.1.0")) << "Minor version equal comparator failed!";
    ASSERT_TRUE(Version("1.1.0") < Version("1.2.0")) << "Minor version less-than comparator failed!";
    ASSERT_TRUE(Version("1.1.0") > Version("1.0.0")) << "Minor version greater-than comparator failed!";

    // test revesion version comparison
    ASSERT_TRUE(Version("1.0.1") == Version("1.0.1")) << "Revision version equal comparator failed!";
    ASSERT_TRUE(Version("1.0.0") < Version("1.0.1")) << "Revision version less-than comparator failed!";
    ASSERT_TRUE(Version("1.0.1") > Version("1.0.0")) << "Revision version greater-than comparator failed!";

    // test accessors
    auto v = Version();
    ASSERT_TRUE(v.major_version() == 0 && v.minor_version() == 0 && v.revision_version() == 0 && v.tag_version() == "")
        << "Default version invalid!";
    v = Version("1.2.3-tag");
    ASSERT_TRUE(v.major_version() == 1) << "Major version accessor failed!";
    ASSERT_TRUE(v.minor_version() == 2) << "Minor version accessor failed!";
    ASSERT_TRUE(v.revision_version() == 3) << "Revision version accessor failed!";
    ASSERT_TRUE(v.tag_version() == "tag") << "Tag version accessor failed!";
    v = Version("1");
    ASSERT_TRUE(v.major_version() == 1 && v.minor_version() == 0 && v.revision_version() == 0 && v.tag_version() == "")
        << "Major version with defaults invalid!";
    v = Version("0.1");
    ASSERT_TRUE(v.major_version() == 0 && v.minor_version() == 1 && v.revision_version() == 0 && v.tag_version() == "")
        << "Minor version with defaults invalid!";
    v = Version("0.0.1");
    ASSERT_TRUE(v.major_version() == 0 && v.minor_version() == 0 && v.revision_version() == 1 && v.tag_version() == "")
        << "Revision version with defaults invalid!";

    // test assignment
    auto v1 = Version("1.0.0");
    ASSERT_TRUE(v1 == Version("1.0.0")) << "Assignment failed!";
    auto v2 = v1;
    ASSERT_TRUE(v2 == v1) << "Assignment failed!";

    // test string
    ASSERT_TRUE(VERSION("1.2.3-tag").toString() == "1.2.3-tag") << "toString failed!";
}
