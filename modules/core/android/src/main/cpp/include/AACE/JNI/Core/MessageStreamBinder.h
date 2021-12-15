/*
 * Copyright 2020-21 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_JNI_CORE_MESSAGE_STREAM_BINDER_H
#define AACE_JNI_CORE_MESSAGE_STREAM_BINDER_H

#include <memory>

#include <AACE/JNI/Core/NativeLib.h>
#include <AACE/Core/MessageStream.h>

namespace aace {
namespace jni {
namespace core {

//
// MessageStreamBinder
//

class MessageStreamBinder {
public:
    MessageStreamBinder(std::shared_ptr<aace::core::MessageStream> stream);

    std::shared_ptr<aace::core::MessageStream> getMessageStream() {
        return m_stream;
    }

private:
    std::shared_ptr<aace::core::MessageStream> m_stream;
};

//
// JMode
//

class JModeConfig : public EnumConfiguration<aace::core::MessageStream::Mode> {
public:
    using T = aace::core::MessageStream::Mode;

    const char* getClassName() override {
        return "com/amazon/aace/core/MessageStream$Mode";
    }

    std::vector<std::pair<T, std::string>> getConfiguration() override {
        return {{T::READ, "READ"}, {T::WRITE, "WRITE"}, {T::READ_WRITE, "READ_WRITE"}};
    }
};

using JMode = JEnum<aace::core::MessageStream::Mode, JModeConfig>;

}  // namespace core
}  // namespace jni
}  // namespace aace

#endif  // AACE_JNI_CORE_MESSAGE_STREAM_BINDER_H
