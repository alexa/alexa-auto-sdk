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

#ifndef SAMPLEAPP_STATUS_H
#define SAMPLEAPP_STATUS_H

// C++ Standard Library
#include <cstdlib>   // EXIT_SUCCESS and EXIT_FAILURE macros
#include <iostream>  // std::ostream

namespace sampleApp {

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Status
//
////////////////////////////////////////////////////////////////////////////////////////////////////

// Conventional enums implicitly convert to an integer.
// https://isocpp.org/wiki/faq/cpp11-language-types#enum-class

enum Status { Success = EXIT_SUCCESS, Failure = EXIT_FAILURE, Restart, Unknown };

inline std::ostream& operator<<(std::ostream& stream, const Status& status) {
    switch (status) {
        case Status::Success:
            stream << "Success";
            break;
        case Status::Failure:
            stream << "Failure";
            break;
        case Status::Restart:
            stream << "Restart";
            break;
        case Status::Unknown:
            stream << "Unknown";
            break;
    }
    return stream;
}

}  // namespace sampleApp

#endif  // SAMPLEAPP_STATUS_H
