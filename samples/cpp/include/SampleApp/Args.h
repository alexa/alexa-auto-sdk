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

#ifndef SAMPLEAPP_ARGS_H
#define SAMPLEAPP_ARGS_H

// C++ Standard Library
#include <string>  // std::string
#include <vector>  // std::vector

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Args
//
////////////////////////////////////////////////////////////////////////////////////////////////////

class Args {
private:
    std::string m_name{};
    std::vector<std::string> m_list{};

public:
    Args(int argc, const char* argv[])
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            :
            m_name{argv[0]}, m_list{argv + 1, argv + argc} {
    }
    Args(std::initializer_list<std::string> args)
            // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
            :
            m_name{*args.begin()}, m_list{args.begin() + 1, args.end()} {
    }
    auto name() const -> std::string {
        return m_name;
    }
    auto list() const -> std::vector<std::string> {
        return m_list;
    }
};

}  // namespace sampleApp

#endif  // SAMPLEAPP_ARGS_H
