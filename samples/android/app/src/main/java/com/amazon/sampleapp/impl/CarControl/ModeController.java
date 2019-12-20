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
import java.util.ArrayList;

public class ModeController {
    private int index = 0;
    private ArrayList<String> modes;

    public ModeController() {
        modes = new ArrayList<>();
    }

    public void addMode(String mode) {
        modes.add(mode);
    }

    public String getMode() {
        return modes.get(index);
    }

    public void setMode(String mode) {
        int modeIndex = modes.indexOf(mode);
        if (modeIndex >= 0) {
            index = modeIndex;
        }
    }

    public void adjustMode(int delta) {
        index += delta;
        if (index < 0) {
            index = 0;
        } else
        if (index >= modes.size()) {
            index = modes.size() - 1;
        }
    }
}