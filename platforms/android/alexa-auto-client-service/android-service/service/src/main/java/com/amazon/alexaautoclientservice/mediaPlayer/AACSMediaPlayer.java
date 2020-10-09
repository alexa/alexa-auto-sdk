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

package com.amazon.alexaautoclientservice.mediaPlayer;

import com.amazon.aace.aasb.AASBStream;

/**
 * interface used for playing TTS
 */
public interface AACSMediaPlayer {
    void prepare(AASBStream mediaStream, boolean repeating, String token);

    void prepare(String url, boolean repeating, String token);

    void play();

    boolean pause();

    boolean resume();

    boolean stop();

    long getDuration(String replyToId);

    long getPosition(String replyToId);

    boolean setPosition(long position);

    void cleanUp();

    void volumeChanged(float volume);

    void mutedStateChanged(String state);

    long getNumBytesBuffered(String replyToId);
}
