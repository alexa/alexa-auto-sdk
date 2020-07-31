/*
 * Copyright 2017-2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.logView;

import android.content.Context;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.RecyclerView;
import android.text.Spannable;
import android.text.SpannableStringBuilder;
import android.text.style.ForegroundColorSpan;
import android.view.View;
import android.widget.TextView;

import com.amazon.sampleapp.R;

public class ViewHolderCBLCard extends RecyclerView.ViewHolder {
    private final TextView mMessage;
    private final TextView mCode;
    private final Context mContext;

    public ViewHolderCBLCard(View v, Context context) {
        super(v);
        mContext = context;
        mMessage = v.findViewById(R.id.message);
        mCode = v.findViewById(R.id.code);
    }

    public void setMessage(String url) {
        String pre = "Go to ";
        String post = " on your smartphone, computer, or tablet and enter this code:";

        final SpannableStringBuilder sb = new SpannableStringBuilder(pre + url + post);
        final ForegroundColorSpan fcs = new ForegroundColorSpan(ContextCompat.getColor(mContext, R.color.cblCode));
        sb.setSpan(fcs, pre.length(), pre.length() + url.length(), Spannable.SPAN_INCLUSIVE_INCLUSIVE);

        mMessage.setText(sb);
    }

    public void setCode(String newCode) {
        mCode.setText(newCode);
    }
}
