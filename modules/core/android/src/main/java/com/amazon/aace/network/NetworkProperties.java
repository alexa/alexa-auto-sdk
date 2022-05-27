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

package com.amazon.aace.network;

public class NetworkProperties {
    /**
     * This property is used with
     * com.amazon.aace.propertyManager.PropertyManager.setProperty() to set the
     * network interface to be used for the network connection. The value must
     * be an IP address or network interface name.
     * @hideinitializer
     */
    public static final String NETWORK_INTERFACE = "aace.network.networkInterface";

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
    public static final String NETWORK_HTTP_PROXY_HEADERS = "aace.network.httpProxyHeaders";
}
