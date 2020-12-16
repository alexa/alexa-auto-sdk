/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.sampleapp.core;

import java.util.List;

public interface AuthorizationHandlerFactoryInterface {
    /**
     * Retrieves the list of layout number to be added to the default layout.
     *
     * @return  Returns a list of resource enums used for UX.
     */
    List<Integer> getLayoutResourceNums();

    /**
     * Retrieves the list of @c AuthorizationHandlerObserverInterface
     *
     * @param sampleAppContext Provides the access to the application context.
     * @return  Returns a list of @c AuthorizationHandlerObserverInterface.
     */
    List<AuthorizationHandlerObserverInterface> getModuleAuthorizationHandlerInterfaces(
            SampleAppContext sampleAppContext);
}
