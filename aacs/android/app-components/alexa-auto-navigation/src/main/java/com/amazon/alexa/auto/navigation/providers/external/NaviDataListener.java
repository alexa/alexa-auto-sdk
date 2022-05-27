/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.navigation.providers.external;

import com.amazon.alexa.auto.navigation.providers.NaviProvider;

/**
 * This interface needs to be implemented to receive the data from the 3rd party client application
 */
public interface NaviDataListener {
    /**
     * Response for the {@link NaviProvider}'s showAlternativeRoutes
     * method
     * @param alternateRoutes representing a json playload
     */
    void showAlternativeRoutesSucceeded(String alternateRoutes);

    /**
     * Response for the {@link NaviProvider}'s getNavigationState
     * method
     * @param navState representing a json playload for the navigation state
     */
    void setNavState(String navState);

    /**
     * Client navigation app reports the error
     * @param errorType
     * @param errorCode
     * @param description
     */
    void setNavError(String errorType, String errorCode, String description);

    /**
     * Client navigation app reports the navigation event
     * @param event
     */
    void setNavEvent(String event);

    /**
     * Client navigation app reports the navigation favorites results
     * @param navFavorites
     */
    void setNavFavorites(String navFavorites);
}
