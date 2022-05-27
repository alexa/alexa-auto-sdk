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
package com.amazon.alexa.auto.app.common.util;

import static android.view.View.GONE;
import static android.view.View.INVISIBLE;
import static android.view.View.VISIBLE;

import android.view.View;

/**
 * Contains reusable view utilities
 */
public class ViewUtils {
    public static void toggleViewVisibility(View view, int visible) {
        switch (visible) {
            case VISIBLE:
                view.setVisibility(INVISIBLE);
                break;
            case GONE:
            case INVISIBLE:
                view.setVisibility(VISIBLE);
                break;
        }
    }
}
