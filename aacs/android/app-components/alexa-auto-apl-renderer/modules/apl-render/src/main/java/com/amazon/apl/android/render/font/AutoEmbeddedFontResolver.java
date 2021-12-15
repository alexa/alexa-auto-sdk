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

package com.amazon.apl.android.render.font;

import android.content.Context;
import android.content.res.Resources;
import android.graphics.Typeface;
import android.util.Log;

import androidx.annotation.FontRes;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.core.content.res.ResourcesCompat;

import com.amazon.apl.android.font.EmbeddedFontResolver;
import com.amazon.apl.android.font.FontKey;
import com.amazon.apl.android.render.R;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * This class provides support for Bookerly Amazon fonts. The fonts must be downloaded from the
 * Amazon development portal.
 */
public class AutoEmbeddedFontResolver extends EmbeddedFontResolver {
    private static final String TAG = AutoEmbeddedFontResolver.class.getSimpleName();
    private static final String BOOKERLY_FONT_FAMILY = "bookerly";

    private final Context mAppContext;

    private static class ResFontKey {
        @FontRes
        final int fontRes;
        final int weight;

        ResFontKey(final int weight, @FontRes final int fontRes) {
            this.weight = weight;
            this.fontRes = fontRes;
        }
    }

    // Mapping of font weights to font
    private static final List<ResFontKey> sFontWeights = Collections.unmodifiableList(
            Arrays.asList(new ResFontKey(100, R.font.bookerly_lcd_lt), new ResFontKey(300, R.font.bookerly_lcd_rg),
                    new ResFontKey(600, R.font.bookerly_lcd_rg), new ResFontKey(700, R.font.bookerly_lcd_bd)));

    public AutoEmbeddedFontResolver(@NonNull final Context context) {
        super(context);
        mAppContext = context;
    }

    /**
     * If Bookerly font is requested then this method will handle
     * returning the correct typeface for that font family. Otherwise
     * it delegates to the parent class.
     *
     * @param key The requested font key.
     * @return The requested font, or null if not found.
     */
    @NonNull
    @Override
    public Typeface findFont(@NonNull FontKey key) {
        Typeface result = null;

        if (key.getFamily().equalsIgnoreCase(BOOKERLY_FONT_FAMILY)) {
            try {
                result = findAPLFont(key);
            } catch (final Resources.NotFoundException ex) {
                Log.e(TAG, "Exception finding embedded app font", ex);
            }
        }

        if (result == null) {
            Log.d(TAG, "Looking for non bookerly font");
            result = super.findFont(key);
        }

        return result;
    }

    @Nullable
    private Typeface findAPLFont(@NonNull final FontKey key) {
        // Get the closest APLFont font
        int minDiff = Integer.MAX_VALUE;
        AutoEmbeddedFontResolver.ResFontKey bestKey = null;
        List<AutoEmbeddedFontResolver.ResFontKey> fontWeights = sFontWeights;

        for (final AutoEmbeddedFontResolver.ResFontKey currentKey : fontWeights) {
            final int currentWeight = currentKey.weight;

            if (minDiff > Math.abs(currentWeight - key.getWeight())) {
                minDiff = Math.abs(currentWeight - key.getWeight());
                bestKey = currentKey;
            }
        }

        if (bestKey != null) {
            Log.i(TAG, "Best key: " + bestKey.weight + " requested: " + key.getWeight());
            final Typeface result = ResourcesCompat.getFont(mAppContext, bestKey.fontRes);
            if (key.isItalic()) {
                return Typeface.create(result, Typeface.ITALIC);
            }
            return result;
        }

        return null;
    }
}