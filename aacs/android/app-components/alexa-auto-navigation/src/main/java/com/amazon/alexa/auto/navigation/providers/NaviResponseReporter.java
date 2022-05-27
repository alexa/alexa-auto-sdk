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
package com.amazon.alexa.auto.navigation.providers;

import com.amazon.aacsconstants.NaviProviderConstants;
import com.amazon.alexa.auto.aacs.common.navi.NavigationEntry;
import com.amazon.alexa.auto.aacs.common.navi.ShowAlternativeRoutes;

import org.jetbrains.annotations.NotNull;

import java.util.List;

/**
 * This interface would allow implementation of {@link NaviProvider} interface to report the
 * asynchronous responses, events and errors
 */
public interface NaviResponseReporter {
    /**
     * Asynchronous response to {@link NaviProvider#getNavigationState(String messageId)} method
     * @param messageId message Id provided by the getNavigationState method
     * @param payload Navigation state stringified json (Refer the documentation)
     */
    void provideNavigationState(@NotNull String messageId, @NotNull String payload);

    /**
     * Reporting the navigation error
     * @param errorType errorType (Refer the documentation)
     * @param errorCode errorCode (Refer the documentation)
     * @param description error description
     */
    void reportNavigationError(@NotNull String errorType, @NotNull String errorCode, @NotNull String description);

    /**
     * Report the navigation event
     * @param event event (Refer the documentation)
     */
    void reportNaviEvent(@NotNull String event);

    /**
     * Asynchronous response to {@link NaviProvider#showAlternativeRoutes(ShowAlternativeRoutes)} method
     * @param alternateRoutes alternateRoutes stringified json (Refer the documentation)
     */
    void reportAlternativeRoutesShown(@NotNull String alternateRoutes);

    /**
     * Asynchronous response to {@link NaviProvider#fetchNaviFavorites()} method
     * @param navigationEntry navigationEntry stringified json (Refer {@link
     *         NaviProviderConstants#PAYLOAD_NAVIGATION_FAVORITES})
     */
    void reportNaviFavorites(@NotNull List<NavigationEntry> navigationEntry);

    /**
     * Clear the uploaded favorites
     */
    void clearNavigationFavorites();
}
