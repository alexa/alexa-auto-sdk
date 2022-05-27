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
package com.amazon.alexa.auto.media.aacs.handlers;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessage;
import com.amazon.alexa.auto.aacs.common.TemplateRuntimeMessages;
import com.amazon.alexa.auto.media.session.MediaSessionManager;

/**
 * Handler for TemplateRuntime commands coming from AACS.
 */
public class TemplateRuntimeHandler {
    private final static String TAG = TemplateRuntimeHandler.class.getSimpleName();

    @NonNull
    private final MediaSessionManager mMediaSessionManager;

    /**
     * Constructs an instance of @c TemplateRuntimeHandler
     *
     * @param mediaSessionManager Media session manager which will consume the
     *                            media templates.
     */
    public TemplateRuntimeHandler(@NonNull MediaSessionManager mediaSessionManager) {
        this.mMediaSessionManager = mediaSessionManager;
    }

    /**
     * Handle commands coming from AACS.
     *
     * @param message AACS Message.
     */
    public void handleAACSCommand(@NonNull AACSMessage message) {
        if (Action.TemplateRuntime.RENDER_PLAYER_INFO.equals(message.action)) {
            handleRenderPlayer(message);
        }
    }

    /**
     * Handle Render Player Info message from AACS.
     *
     * @param message Render player info message.
     */
    private void handleRenderPlayer(@NonNull AACSMessage message) {
        TemplateRuntimeMessages.parseRenderPlayerInfo(message.payload)
                .ifPresent(mMediaSessionManager::updateMediaSession);
    }
}
