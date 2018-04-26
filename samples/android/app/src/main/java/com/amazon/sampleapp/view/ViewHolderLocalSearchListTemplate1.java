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
import android.view.LayoutInflater;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.sampleapp.R;

public class ViewHolderLocalSearchListTemplate1 extends RecyclerView.ViewHolder
{
    private LinearLayout listContainer;
    private LayoutInflater m_inf;

    public ViewHolderLocalSearchListTemplate1(View v, LayoutInflater inf )
    {
        super( v );
        listContainer = v.findViewById( R.id.listContainer );
        m_inf = inf;
    }

    public void insertListItem( String index, String dist, String name, String address )
    {
        View listItem = m_inf.inflate( R.layout.card_local_search_list_template1_item, listContainer, false );
        ( ( TextView ) listItem.findViewById( R.id.index ) ).setText( index );
        ( ( TextView ) listItem.findViewById( R.id.dist ) ).setText( dist );
        ( ( TextView ) listItem.findViewById( R.id.name ) ).setText( name );
        ( ( TextView ) listItem.findViewById( R.id.address ) ).setText( address );
        listContainer.addView( listItem );
    }

    public void clearList() { listContainer.removeAllViews(); }
}