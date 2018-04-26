/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.view;

import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.amazon.sampleapp.R;

public class ViewHolderRenderPlayerInfo extends RecyclerView.ViewHolder
{
    private TextView header;
    private TextView headerSubtext1;
    private TextView title;
    private TextView titleSubtext1;
    private TextView titleSubtext2;
    private ImageView partnerLogo;
    private ImageView art;

    public ViewHolderRenderPlayerInfo(View v )
    {
        super( v );
        header = v.findViewById( R.id.header );
        headerSubtext1 = v.findViewById( R.id.headerSubtext1 );
        title = v.findViewById(R.id.title);
        titleSubtext1 = v.findViewById( R.id.titleSubtext1 );
        titleSubtext2 = v.findViewById( R.id.titleSubtext2 );
        partnerLogo = v.findViewById( R.id.partnerLogo );
        art = v.findViewById( R.id.art );
    }

    public TextView getHeader() { return header; }

    public TextView getHeaderSubtext1() { return headerSubtext1; }

    public TextView getTitle() { return title; }

    public TextView getTitleSubtext1() { return titleSubtext1; }

    public TextView getTitleSubtext2() { return titleSubtext2; }

    public ImageView getPartnerLogo() { return partnerLogo; }

    public ImageView getArt() { return art; }
}

