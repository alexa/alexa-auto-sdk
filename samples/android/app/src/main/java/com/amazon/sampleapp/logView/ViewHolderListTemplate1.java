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

import android.support.constraint.ConstraintLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.sampleapp.R;

public class ViewHolderListTemplate1 extends RecyclerView.ViewHolder {
    private final TextView mMainTitle;
    private final TextView mSubTitle;
    private final LinearLayout mIndexList;
    private final LinearLayout mContentList;
    private final LayoutInflater mInf;

    public ViewHolderListTemplate1(View v, LayoutInflater inf) {
        super(v);
        mMainTitle = v.findViewById(R.id.mainTitle);
        mSubTitle = v.findViewById(R.id.subTitle);
        ConstraintLayout listContainer = v.findViewById(R.id.listContainer);
        mIndexList = listContainer.findViewById(R.id.indexList);
        mContentList = listContainer.findViewById(R.id.contentList);
        mInf = inf;
    }

    public TextView getMainTitle() {
        return mMainTitle;
    }
    public TextView getSubTitle() {
        return mSubTitle;
    }

    public void insertListItem(String index, String content) {
        View indexItem = mInf.inflate(R.layout.card_list_template1_item_index, mIndexList, false);
        ((TextView) indexItem.findViewById(R.id.index)).setText(index);
        mIndexList.addView(indexItem);

        View contentItem = mInf.inflate(R.layout.card_list_template1_item_content, mContentList, false);
        ((TextView) contentItem.findViewById(R.id.content)).setText(content);
        mContentList.addView(contentItem);
    }

    public void clearLists() {
        mIndexList.removeAllViews();
        mContentList.removeAllViews();
    }
}
