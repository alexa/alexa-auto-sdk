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

package com.amazon.aacsconstants;

public class MediaConstants {
    public static class MediaState {
        public static final String STOPPED = "STOPPED";
        public static final String PLAYING = "PLAYING";
        public static final String BUFFERING = "BUFFERING";
    }

    public static class MediaError {
        public static final String MEDIA_ERROR_UNKNOWN = "MEDIA_ERROR_UNKNOWN";
        public static final String MEDIA_ERROR_INVALID_REQUEST = "MEDIA_ERROR_INVALID_REQUEST";
        public static final String MEDIA_ERROR_SERVICE_UNAVAILABLE = "MEDIA_ERROR_SERVICE_UNAVAILABLE";
        public static final String MEDIA_ERROR_INTERNAL_SERVER_ERROR = "MEDIA_ERROR_INTERNAL_SERVER_ERROR";
        public static final String MEDIA_ERROR_INTERNAL_DEVICE_ERROR = "MEDIA_ERROR_INTERNAL_DEVICE_ERROR";
    }

    public static final String TOKEN = "token";
    public static final String CHANNEL = "channel";
    public static final String STATE = "state";
    public static final String DURATION = "duration";
    public static final String POSITION = "position";
    public static final String MEDIA_ERROR = "error";
    public static final String ERROR_DESCRIPTION = "description";
}
