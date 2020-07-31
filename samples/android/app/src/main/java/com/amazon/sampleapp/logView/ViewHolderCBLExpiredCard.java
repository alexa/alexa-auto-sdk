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

import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import com.amazon.sampleapp.R;

public class ViewHolderCBLExpiredCard extends RecyclerView.ViewHolder {
    private final TextView mMessage;

    public ViewHolderCBLExpiredCard(View v) {
        super(v);
        mMessage = v.findViewById(R.id.message);
    }

    public void setMessage(String message) {
        mMessage.setText(message);
    }
}
