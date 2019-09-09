/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aace.navigation;

public class NavigationProperties {

    /**
     * This property is used with Engine.setProperty() to change the navigation provider name that is reported
     * to AVS. The value must be a valid string representation of the provider name. (e.g. "HERE")
     * @hideinitializer
     */
    public static final String NAVIGATION_PROVIDER_NAME = "aace.navigation.providerName";
}
