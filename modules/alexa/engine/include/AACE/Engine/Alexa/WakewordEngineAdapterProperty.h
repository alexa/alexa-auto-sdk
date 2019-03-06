/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_ENGINE_WAKEWORD_ENGINE_ADAPTER_PROPERTY_H
#define AACE_ENGINE_WAKEWORD_ENGINE_ADAPTER_PROPERTY_H

#include <string>

/** @file */

namespace aace {
namespace engine {
namespace wakeword {
namespace property {

/**
 * This property is used with Engine::setProperty() to set AmazonLite locale.
 * This is set-only and internal engine property.
 */
static const std::string WAKEWORD_ENGINE_LOCALE = "aace.wakeword.locale";

} // aace::engine::wakeword::property
} // aace::engine::wakeword
} // aace::engine
} // aace

#endif // AACE_ENGINE_WAKEWORD_ENGINE_ADAPTER_PROPERTY_H
