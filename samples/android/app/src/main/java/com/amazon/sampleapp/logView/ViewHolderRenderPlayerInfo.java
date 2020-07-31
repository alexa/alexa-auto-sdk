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
import android.widget.ImageView;
import android.widget.TextView;

import com.amazon.sampleapp.R;

class ViewHolderRenderPlayerInfo extends RecyclerView.ViewHolder {
    private final TextView mHeader;
    private final TextView mHeaderSubtext1;
    private final TextView mTitle;
    private final TextView mTitleSubtext1;
    private final TextView mTitleSubtext2;
    private final ImageView mPartnerLogo;
    private final ImageView mArt;

    public ViewHolderRenderPlayerInfo(View v) {
        super(v);
        mHeader = v.findViewById(R.id.header);
        mHeaderSubtext1 = v.findViewById(R.id.headerSubtext1);
        mTitle = v.findViewById(R.id.title);
        mTitleSubtext1 = v.findViewById(R.id.titleSubtext1);
        mTitleSubtext2 = v.findViewById(R.id.titleSubtext2);
        mPartnerLogo = v.findViewById(R.id.partnerLogo);
        mArt = v.findViewById(R.id.art);
    }

    public TextView getHeader() {
        return mHeader;
    }
    public TextView getHeaderSubtext1() {
        return mHeaderSubtext1;
    }
    public TextView getTitle() {
        return mTitle;
    }
    public TextView getTitleSubtext1() {
        return mTitleSubtext1;
    }
    public TextView getTitleSubtext2() {
        return mTitleSubtext2;
    }
    public ImageView getPartnerLogo() {
        return mPartnerLogo;
    }
    public ImageView getArt() {
        return mArt;
    }
}
