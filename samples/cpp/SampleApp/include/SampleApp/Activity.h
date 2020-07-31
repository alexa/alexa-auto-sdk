/*
 * Copyright 2018-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef SAMPLEAPP_ACTIVITY_H
#define SAMPLEAPP_ACTIVITY_H

#include "SampleApp/Event.h"
#include "SampleApp/Executor.h"
#include "SampleApp/Subject.h"
#include "SampleApp/Views.h"

// C++ Standard Library
#include <memory>
#include <string>
#include <vector>

namespace sampleApp {

class ApplicationContext;  // forward declare

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Activity
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Activity : public Subject<Event> {
private:
    bool m_singleThreadedUI{false};
    std::shared_ptr<ApplicationContext> m_applicationContext{};
    std::shared_ptr<Executor> m_executor{};
    std::vector<std::shared_ptr<View>> m_views{};

protected:
    Activity(std::shared_ptr<ApplicationContext> applicationContext, std::vector<std::shared_ptr<View>> views);

public:
    template <typename... Args>
    static std::shared_ptr<Activity> create(Args&&... args) {
        return std::shared_ptr<Activity>(new Activity(args...));
    }
    template <typename... Args>
    void runOnUIThread(Args&&... args) {
        if (m_singleThreadedUI) {
            run(args...);  // UI runs on the main thread
        } else {
            m_executor->submit(args...);
        }
    };
    template <typename Func, typename... Args>
    auto run(Func&& func, Args&&... args) -> void {
        func(std::forward<Args>(args)...);
    }
    auto findViewById(const std::string& id) -> std::weak_ptr<View>;
    auto getApplicationContext() -> std::shared_ptr<ApplicationContext>;
    auto getExecutor() -> std::shared_ptr<Executor>;
};

}  // namespace sampleApp

#endif  // SAMPLEAPP_ACTIVITY_H
