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
