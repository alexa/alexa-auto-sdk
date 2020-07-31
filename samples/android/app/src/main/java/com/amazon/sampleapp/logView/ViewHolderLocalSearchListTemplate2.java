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
import android.view.LayoutInflater;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.sampleapp.R;

class ViewHolderLocalSearchListTemplate2 extends RecyclerView.ViewHolder {
    private final TextView mTitle;

    private final LinearLayout mListContainer;
    private final LayoutInflater mInf;

    public ViewHolderLocalSearchListTemplate2(View v, LayoutInflater inf) {
        super(v);
        mTitle = v.findViewById(R.id.Title);

        mListContainer = v.findViewById(R.id.localSearchListlistContainer2);
        mInf = inf;
    }

    public void insertListItem(String index, String mainTitle, String subTitle, String address, String phoneNumber,
            String provider, String image, String travelDistance, String travelTime, String offRouteTime,
            String priceRange, String ratingValue, String ratingReviewCount, String ratingImage,
            String ratingProviderName, String ratingProviderImage, String currentStatus) {
        View listItem = mInf.inflate(R.layout.card_local_search_list_template2_item, mListContainer, false);
        ((TextView) listItem.findViewById(R.id.index)).setText(index);
        ((TextView) listItem.findViewById(R.id.mainTitle)).setText(mainTitle);
        ((TextView) listItem.findViewById(R.id.subTitle)).setText(subTitle);
        ((TextView) listItem.findViewById(R.id.address)).setText(address);
        // not used in example GUI
        //( ( TextView ) listItem.findViewById( R.id.phoneNumber ) ).setText( phoneNumber );
        ((TextView) listItem.findViewById(R.id.provider)).setText(provider);
        new DownloadImageTask((ImageView) listItem.findViewById(R.id.image)).execute(image);
        ((TextView) listItem.findViewById(R.id.travelDistance)).setText(travelDistance);
        ((TextView) listItem.findViewById(R.id.travelTime)).setText(travelTime);
        ((TextView) listItem.findViewById(R.id.offRouteTime)).setText(offRouteTime);
        // not used in example GUI
        //( ( TextView ) listItem.findViewById( R.id.priceRange ) ).setText( priceRange );
        ((TextView) listItem.findViewById(R.id.ratingValue)).setText(ratingValue);
        ((TextView) listItem.findViewById(R.id.ratingReviewCount)).setText(ratingReviewCount);
        new DownloadImageTask((ImageView) listItem.findViewById(R.id.ratingImage)).execute(ratingImage);
        //( ( TextView ) listItem.findViewById( R.id.ratingProviderName ) ).setText( ratingProviderName );
        new DownloadImageTask((ImageView) listItem.findViewById(R.id.ratingProviderImage)).execute(ratingProviderImage);
        ((TextView) listItem.findViewById(R.id.currentStatus)).setText(currentStatus);
        mListContainer.addView(listItem);
    }

    public TextView getTitle() {
        return mTitle;
    }

    public void clearList() {
        mListContainer.removeAllViews();
    }
}
