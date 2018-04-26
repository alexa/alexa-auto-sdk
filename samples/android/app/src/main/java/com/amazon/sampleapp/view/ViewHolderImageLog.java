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

import com.amazon.sampleapp.R;


public class ViewHolderImageLog extends RecyclerView.ViewHolder
{

    private ImageView image;

    public ViewHolderImageLog( View v )
    {
        super( v );
        image = ( ImageView ) v.findViewById( R.id.item_image );
    }

    public ImageView getImageView() {
        return image;
    }

    public void setLog( ImageView img ) {
        this.image = img;
    }

}
