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

#include <AACE/Engine/Utils/Threading/Executor.h>

namespace aace {
namespace engine {
namespace utils {
namespace threading {

Executor::Executor() :
        m_taskQueue{std::make_shared<TaskQueue>()},
        m_taskThread{std::unique_ptr<TaskThread>(new TaskThread(m_taskQueue))} {
    m_taskThread->start();
}

Executor::~Executor() {
    shutdown();
}

void Executor::waitForSubmittedTasks() {
    std::promise<void> flushedPromise;
    auto flushedFuture = flushedPromise.get_future();
    auto task = [&flushedPromise]() { flushedPromise.set_value(); };
    submit(task);
    flushedFuture.get();
}

void Executor::shutdown() {
    m_taskQueue->shutdown();
    m_taskThread.reset();
}

bool Executor::isShutdown() {
    return m_taskQueue->isShutdown();
}

}  // namespace threading
}  // namespace utils
}  // namespace engine
}  // namespace aace
