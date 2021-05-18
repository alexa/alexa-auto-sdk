/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.apl.android.render.utils;

import com.amazon.apl.android.scaling.Scaling;

import java.util.Collection;

import lombok.NonNull;

public final class ViewportUtils {
    // [1280 x 800] 160 dpi -> [960 x 600] 213 dpi
    public static final int CROWN_REAL_WIDTH_DP = 960;
    public static final int CROWN_REAL_HEIGHT_DP = 600;

    private ViewportUtils() {}

    /**
     * Determines if a viewport's width and height are both within the ranges defined by the given list of
     * specifications.
     * @param viewportWidth the viewport width
     * @param viewportHeight the viewport height
     * @param supportedViewports a list of objects defining the valid ranges for a viewport's width and height
     */
    public static boolean isWithinSpecification(int viewportWidth, int viewportHeight,
            @NonNull final Collection<Scaling.ViewportSpecification> supportedViewports) {
        for (Scaling.ViewportSpecification viewportSpecification : supportedViewports) {
            if (isWithinSpecification(viewportWidth, viewportHeight, viewportSpecification)) {
                return true;
            }
        }
        return false;
    }

    /**
     * Determines if a viewport's width and height are both within the ranges defined by the given specification.
     * @param viewportWidth the viewport width
     * @param viewportHeight the viewport height
     * @param viewportSpecification an object defining the valid ranges for a viewport's width and height
     */
    public static boolean isWithinSpecification(
            int viewportWidth, int viewportHeight, @NonNull final Scaling.ViewportSpecification viewportSpecification) {
        int minWidth = viewportSpecification.getMinWidth();
        int maxWidth = viewportSpecification.getMaxWidth();
        int minHeight = viewportSpecification.getMinHeight();
        int maxHeight = viewportSpecification.getMaxHeight();

        return isInsideBounds(viewportWidth, minWidth, maxWidth)
                && isInsideBounds(viewportHeight, minHeight, maxHeight);
    }

    /**
     * Determines whether the given value is inside the specified boundaries.
     * @param value the value to verify
     * @param lowerBound the lower bound to be satisfied
     * @param upperBound the upper bound to be satisfied
     */
    public static boolean isInsideBounds(int value, int lowerBound, int upperBound) {
        return (lowerBound <= value) && (value <= upperBound);
    }
}
