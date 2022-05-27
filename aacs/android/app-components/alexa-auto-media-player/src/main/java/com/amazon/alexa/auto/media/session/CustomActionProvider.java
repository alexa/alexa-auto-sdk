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
package com.amazon.alexa.auto.media.session;

import android.os.Bundle;
import android.support.v4.media.session.PlaybackStateCompat;

import androidx.annotation.Nullable;

/**
 * Interface for facilitating integration of custom controls with Media Session.
 */
public interface CustomActionProvider {
    /**
     * Provide the {@link PlaybackStateCompat.CustomAction} object that encompass
     * the icon to be displayed for custom control as well as the action name that
     * is passed to {@link CustomActionProvider#onCustomAction} when user interacts
     * with the custom control.
     *
     * @return An {@link PlaybackStateCompat.CustomAction} object representing the
     * custom action.
     */
    PlaybackStateCompat.CustomAction getCustomAction();

    /**
     * Callback that is invoked when user interacts with the custom control.
     *
     * @param action Name of the action (that is passed from
     * {@link CustomActionProvider#getCustomAction}).
     * @param extras Additional parameters.
     */
    void onCustomAction(String action, @Nullable Bundle extras);
}
