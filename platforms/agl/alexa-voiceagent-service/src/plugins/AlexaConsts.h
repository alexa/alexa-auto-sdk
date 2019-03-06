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
#ifndef JSON_ATTRIBUTE_CONSTS_H_
#define JSON_ATTRIBUTE_CONSTS_H_

#include <string>

namespace agl {
namespace alexa {

/// vshl-capabilities API name.
extern const std::string VSHL_CAPABILITIES_API_NAME;

extern const std::string JSON_ATTR_AUTH_TOKEN;
extern const std::string JSON_ATTR_REFRESH_TOKEN;
extern const std::string JSON_ATTR_VOICEAGENT_ID;
extern const std::string JSON_ATTR_STATE;
extern const std::string JSON_ATTR_ACTION;
extern const std::string JSON_ATTR_ACTIONS;
extern const std::string JSON_ATTR_PAYLOAD;

}  // namespace alexa
}  // namespace agl

#endif  // JSON_ATTRIBUTE_CONSTS_H_
