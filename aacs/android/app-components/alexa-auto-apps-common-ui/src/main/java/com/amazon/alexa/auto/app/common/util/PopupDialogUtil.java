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

import android.app.Dialog;
import android.content.Context;
import android.text.SpannableString;
import android.text.Spanned;
import android.text.TextPaint;
import android.text.method.LinkMovementMethod;
import android.text.style.ClickableSpan;
import android.view.View;
import android.webkit.WebView;
import android.widget.ImageButton;
import android.widget.TextView;

import com.amazon.alexa.auto.apps.common.ui.R;

import org.jetbrains.annotations.NotNull;

import kotlin.jvm.internal.Intrinsics;

/**
 * Contains Popup UI utility methods
 */
public class PopupDialogUtil {
    public static final String USER_AGENT_ANDROID = "Android Mobile";

    /***
     * Converts part of textView to a clickable popup with a close button
     * and embeds the given url onto it
     *
     * @param context View context
     * @param textView TextView to apply change on
     * @param startIndex Start index of text to be converted into a clickable link
     * @param endIndex End index of text to be converted into a clickable link
     * @param url URL to emned in the popup
     * @param color Link Color (defaults to current textView colour if null)
     *
     */
    public static void embedUrlInPopupDialog(
            Context context, TextView textView, int startIndex, int endIndex, String url, Integer color) {
        SpannableString spannableString = new SpannableString(textView.getText());
        final int textColor = color != null ? color : textView.getCurrentTextColor();
        ClickableSpan clickableSpan = new ClickableSpan() {
            @Override
            public void onClick(@NotNull View textView) {
                final Dialog dialog = new Dialog(context);
                dialog.setContentView(R.layout.simple_dialog_layout);

                WebView webView = dialog.findViewById(R.id.webView);
                webView.getSettings().setUserAgentString(USER_AGENT_ANDROID);
                webView.loadUrl(url);

                ImageButton closeButton = dialog.findViewById(R.id.dialogButtonClose);
                closeButton.setOnClickListener(v -> dialog.dismiss());
                dialog.show();
            }

            @Override
            public void updateDrawState(@NotNull TextPaint drawState) {
                Intrinsics.checkNotNullParameter(drawState, "drawState");
                super.updateDrawState(drawState);
                drawState.setUnderlineText(true);
                drawState.setColor(textColor);
            }
        };

        spannableString.setSpan(clickableSpan, startIndex, endIndex, Spanned.SPAN_EXCLUSIVE_EXCLUSIVE);
        textView.setText(spannableString);
        textView.setMovementMethod(LinkMovementMethod.getInstance());
        textView.setHighlightColor(0);
    }
}
