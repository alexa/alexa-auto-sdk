/*
 * Copyright 2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.CarControl;

/**
 * Application needs to maintain power or toggle state for
 * "Alexa.PowerController" and "Alexa.ToggleController". This class
 * provides a way to save and retrieve that state.
 */
public class BoolController {

    private boolean m_value = false;

    public BoolController() {
    }

    public void setValue(boolean value) {
        m_value = value;
    }

    boolean getValue() {
        return m_value;
    }
}