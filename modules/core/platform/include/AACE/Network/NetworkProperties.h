/*
 * Copyright 2019-2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#ifndef AACE_NETWORK_PROPERTY_NETWORK_PROPERTIES_H
#define AACE_NETWORK_PROPERTY_NETWORK_PROPERTIES_H

#include <string>

/** @file */

namespace aace {
namespace network {
namespace property {

/**
 * This property is used with
 * aace::propertyManager::PropertyManager::setProperty() to set the network
 * interface to be used for the network connection. The value must be an IP
 * address or network interface name.
*/
static const std::string NETWORK_INTERFACE = "aace.network.networkInterface";

/**
 * This property is used with aace::propertyManager::PropertyManager::setProperty()
 * to set the custom HTTP header to pass in the HTTP request sent to a proxy. The headers
 * should be `\n` separated.
 *
 * For example:
 * `"Proxy-Authorization: Bearer 1234"` (should not be CRLF-terminated)
 *
 * @note To apply the custom headers you are required to specify the CURLOPT_PROXY` in the config.
 * The specified headers will be applied to all subsequent requests sent to a proxy.
 */
static const std::string NETWORK_HTTP_PROXY_HEADERS = "aace.network.httpProxyHeaders";

}  // namespace property
}  // namespace network
}  // namespace aace

#endif  // AACE_NETWORK_PROPERTY_NETWORK_PROPERTIES_H
