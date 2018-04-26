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

import android.support.constraint.ConstraintLayout;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.sampleapp.R;

public class ViewHolderListTemplate1 extends RecyclerView.ViewHolder
{
    private TextView mainTitle;
    private TextView subTitle;
    private ConstraintLayout listContainer;
    private LinearLayout indexList;
    private LinearLayout contentList;
    private LayoutInflater m_inf;

    public ViewHolderListTemplate1(View v, LayoutInflater inf )
    {
        super( v );
        mainTitle = v.findViewById( R.id.mainTitle );
        subTitle = v.findViewById( R.id.subTitle );
        listContainer = v.findViewById( R.id.listContainer );
        indexList = listContainer.findViewById( R.id.indexList );
        contentList = listContainer.findViewById( R.id.contentList );
        m_inf = inf;
    }

    public TextView getMainTitle(){ return mainTitle; }
    public TextView getSubTitle(){ return subTitle; }

    public void insertListItem( String index, String content )
    {
        View indexItem = m_inf.inflate( R.layout.card_list_template1_item_index, indexList, false );
        ( ( TextView ) indexItem.findViewById( R.id.index ) ).setText( index );
        indexList.addView( indexItem );

        View contentItem = m_inf.inflate( R.layout.card_list_template1_item_content, contentList, false );
        ( ( TextView ) contentItem.findViewById( R.id.content ) ).setText( content );
        contentList.addView( contentItem );
    }

    public void clearLists()
    {
        indexList.removeAllViews();
        contentList.removeAllViews();
    }
}
