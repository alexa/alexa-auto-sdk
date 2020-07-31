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

#ifndef SAMPLEAPP_SUBJECT_H
#define SAMPLEAPP_SUBJECT_H

// C++ Standard Library
#include <map>     // std::map
#include <string>  // std::string
#include <vector>  //std::vector

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Subject
//
////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Event>
class Subject {
private:
    std::map<Event, std::vector<std::function<bool(const std::string&)>>> m_observers{};

public:
    template <typename Observer>
    void registerObserver(const Event& event, Observer&& observer) {
        m_observers[std::move(event)].push_back(std::forward<Observer>(observer));
    }
    template <typename Observer>
    void registerObserver(Event&& event, Observer&& observer) {
        m_observers[std::move(event)].push_back(std::forward<Observer>(observer));
    }
    auto clearObservers() -> void {
        m_observers.clear();
    }
    auto notify(const Event& event, const std::string& value = "") const -> bool {
        bool result = false;
        if (m_observers.count(event) > 0) {
            for (const auto& observer : m_observers.at(event)) {
                result |= observer(value);
            }
        }
        return result;
    }
};

}  // namespace sampleApp

#endif  // SAMPLEAPP_SUBJECT_H
