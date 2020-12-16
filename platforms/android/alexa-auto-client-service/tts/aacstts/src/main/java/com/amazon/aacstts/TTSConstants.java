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

package com.amazon.aacstts;

public class TTSConstants {
    public static final String AACS_TTS_LOG_PREFIX = "AACS-TTS:";
    public static final String TOPIC = "TextToSpeech";
    public static final String DEFAULT_PROVIDER = "text-to-speech-provider";
    public static class Action {
        public static final String GET_CAPABILITIES = "GetCapabilities";
        public static final String PREPARE_SPEECH_COMPLETED = "PrepareSpeechCompleted";
        public static final String PREPARE_SPEECH_FAILED = "PrepareSpeechFailed";
        public static final String PREPARE_SPEECH = "PrepareSpeech";
    }
}
