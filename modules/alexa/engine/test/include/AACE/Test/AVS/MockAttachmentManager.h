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

#ifndef AACE_TEST_AVS_MOCK_ATTACHMENT_MANAGER_H
#define AACE_TEST_AVS_MOCK_ATTACHMENT_MANAGER_H

#include <chrono>
#include <string>
#include <memory>

#include <AVSCommon/AVS/Attachment/AttachmentManagerInterface.h>
#include <gmock/gmock.h>

namespace aace {
namespace test {
namespace avs {

class MockAttachmentManager : public alexaClientSDK::avsCommon::avs::attachment::AttachmentManagerInterface {
public:
    MOCK_CONST_METHOD2(generateAttachmentId, std::string(const std::string& contextId, const std::string& contentId));
    MOCK_METHOD1(setAttachmentTimeoutMinutes, bool(std::chrono::minutes timeoutMinutes));
    MOCK_METHOD2(
        createWriter,
        std::unique_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentWriter>(
            const std::string& attachmentId,
            alexaClientSDK::avsCommon::utils::sds::WriterPolicy policy));
    MOCK_METHOD2(
        createReader,
        std::unique_ptr<alexaClientSDK::avsCommon::avs::attachment::AttachmentReader>(
            const std::string& attachmentId,
            alexaClientSDK::avsCommon::utils::sds::ReaderPolicy policy));
};

}  // namespace avs
}  // namespace test
}  // namespace aace

#endif  // AACE_TEST_AVS_MOCK_ATTACHMENT_MANAGER_H
