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
package com.amazon.maccandroid.model.state;

import com.amazon.maccandroid.MediaApp;
import com.amazon.maccandroid.model.SupportedOperations;

public class ExternalMediaPlayerState {
    private final MediaAppPlaybackState mMediaAppPlaybackState;
    private final MediaAppSessionState mMediaAppSessionState;

    public ExternalMediaPlayerState(MediaApp app, SupportedOperations supportedOperations) {
        mMediaAppPlaybackState = new MediaAppPlaybackState(app, supportedOperations);
        mMediaAppSessionState = new MediaAppSessionState(app);
    }

    public MediaAppPlaybackState getMediaAppPlaybackState() {
        return mMediaAppPlaybackState;
    }

    public MediaAppSessionState getMediaAppSessionState() {
        return mMediaAppSessionState;
    }
}
