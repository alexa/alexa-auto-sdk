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

#include "SampleApp/Activity.h"

#include "SampleApp/ApplicationContext.h"

// C++ Standard Library
#include <algorithm>

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Activity
//
////////////////////////////////////////////////////////////////////////////////////////////////////

Activity::Activity(std::shared_ptr<ApplicationContext> applicationContext, std::vector<std::shared_ptr<View>> views) :
        m_applicationContext{std::move(applicationContext)}, m_views{std::move(views)} {
    // Expects(m_applicationContext != nullptr);
    m_executor = std::make_shared<Executor>();
    m_singleThreadedUI = m_applicationContext->isSingleThreadedUI();
}

std::weak_ptr<View> Activity::findViewById(const std::string& id) {
    auto it = std::find_if(
        m_views.begin(), m_views.end(), [&id](const std::shared_ptr<View> view) { return view->getId() == id; });
    if (it != m_views.end()) {
        return *it;
    }
    return {};
}

std::shared_ptr<ApplicationContext> Activity::getApplicationContext() {
    return m_applicationContext;
}

std::shared_ptr<Executor> Activity::getExecutor() {
    return m_executor;
}

}  // namespace sampleApp
