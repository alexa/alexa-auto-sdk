/*
 * Copyright 2018-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_EXECUTORSERVICE_H
#define SAMPLEAPP_EXECUTORSERVICE_H

// C++ Standard Library
#include <future> // std::future
#include <vector> // std::vector

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  ExecutorService
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class ExecutorService {
  private:
    std::vector<std::future<void>> m_futures{};

  public:
    ExecutorService();
    virtual ~ExecutorService();
    template <typename Fn, typename... Args> auto call(Fn &&fn, Args &&... args) -> std::future<decltype(fn(args...))> {
        return std::async(std::launch::async, [=]() -> decltype(fn(args...)) { return fn(std::forward<Args>(args)...); });
    }
    template <typename Fn, typename... Args> auto run(Fn &&fn, Args &&... args) -> void {
        auto future = std::async(std::launch::async, [=]() { fn(std::forward<Args>(args)...); });
        m_futures.push_back(std::move(future));
    }
    ExecutorService(const ExecutorService &) = default;            // NOLINT(readability-named-parameter)
    ExecutorService &operator=(const ExecutorService &) = default; // NOLINT(readability-named-parameter)
    ExecutorService(ExecutorService &&) = default;                 // NOLINT(readability-named-parameter)
    ExecutorService &operator=(ExecutorService &&) = default;      // NOLINT(readability-named-parameter)
};

} // namespace sampleApp

#endif // SAMPLEAPP_EXECUTORSERVICE_H
