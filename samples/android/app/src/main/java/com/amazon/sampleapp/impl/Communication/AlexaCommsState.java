/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.Communication;

import com.amazon.aace.communication.AlexaComms;

public class AlexaCommsState {
    AlexaComms.CallState m_currentCallState = AlexaComms.CallState.NONE;
    boolean m_selfMuted = false;
    boolean m_otherMuted = false;

    public AlexaComms.CallState getCurrentCallState() {
        return m_currentCallState;
    }

    public boolean isSelfMuted() {
        return m_selfMuted;
    }

    public boolean isOtherMuted() {
        return m_otherMuted;
    }
}