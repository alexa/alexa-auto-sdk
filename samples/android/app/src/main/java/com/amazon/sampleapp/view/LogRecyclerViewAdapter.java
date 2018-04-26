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

import android.content.Context;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.amazon.sampleapp.R;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class LogRecyclerViewAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder>
{
    private Context mContext;

    // The items to display in your RecyclerView
    private List<LogEntry> mItems;
    private List<LogEntry> mFilteredItems;

    // The items to filter out of the log
    private Set<String> mFilteredLevels;
    private Set<String> mFilteredSources;

    private final int LOG = 0, BODY_TEMPLATE1 = 1, BODY_TEMPLATE2 = 2, LIST_TEMPLATE1 = 3,
            WEATHER_TEMPLATE = 4, SET_DESTINATION_TEMPLATE = 5, LOCAL_SEARCH_LIST_TEMPLATE1 = 6,
            RENDER_PLAYER_INFO = 7, DEFAULT = -1;

    // Provide a suitable constructor (depends on the kind of dataset)
    public LogRecyclerViewAdapter(List<LogEntry> items, Context context )
    {
        mContext = context;
        mItems = items;
        mFilteredItems = new ArrayList<>();
        mFilteredItems.addAll( items );
        mFilteredLevels = new HashSet<>();
        mFilteredSources = new HashSet<>();
    }

    // Return the size of your dataset (invoked by the layout manager)
    @Override
    public int getItemCount() { return mFilteredItems != null ? mFilteredItems.size() : mItems.size(); }

    //Returns the view type of the item at position for the purposes of view recycling.
    @Override
    public int getItemViewType( int position )
    {
        if( mFilteredItems.get( position ) != null )
        {
            switch ( mFilteredItems.get( position ).getType() )
            {
                case "logText":
                    return LOG;
                case "bodyTemplate1":
                    return BODY_TEMPLATE1;
                case "bodyTemplate2":
                    return BODY_TEMPLATE2;
                case "listTemplate1":
                    return LIST_TEMPLATE1;
                case "weatherTemplate":
                    return WEATHER_TEMPLATE;
                case "setDestinationTemplate":
                    return SET_DESTINATION_TEMPLATE;
                case "localSearchListTemplate1":
                    return LOCAL_SEARCH_LIST_TEMPLATE1;
                case "renderPlayerInfo":
                    return RENDER_PLAYER_INFO;
                default:
                    return DEFAULT;
            }
        }
        return -1;
    }

    /**
     * This method creates different RecyclerView.ViewHolder objects based on the item view type.\
     *
     * @param viewGroup ViewGroup container for the item
     * @param viewType type of view to be inflated
     * @return viewHolder to be inflated
     */
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder( ViewGroup viewGroup, int viewType )
    {
        RecyclerView.ViewHolder viewHolder;
        View view;
        ViewGroup cardContainer;
        LayoutInflater inflater = LayoutInflater.from( viewGroup.getContext() );

        switch ( viewType )
        {
            case LOG:
                view = inflater.inflate( R.layout.log_item, viewGroup, false );
                viewHolder = new ViewHolderTextLog( view );
                break;
            case BODY_TEMPLATE1:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = ( ViewGroup ) view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_body_template1, cardContainer, true );
                viewHolder = new ViewHolderBodyTemplate1( view );
                break;
            case BODY_TEMPLATE2:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = ( ViewGroup ) view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_body_template2, cardContainer, true );
                viewHolder = new ViewHolderBodyTemplate2( view );
                break;
            case LIST_TEMPLATE1:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = ( ViewGroup ) view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_list_template1, cardContainer, true );
                viewHolder = new ViewHolderListTemplate1( view, inflater );
                break;
            case WEATHER_TEMPLATE:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = ( ViewGroup ) view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_weather_template, cardContainer, true );
                viewHolder = new ViewHolderWeatherTemplate( view );
                break;
            case SET_DESTINATION_TEMPLATE:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = ( ViewGroup ) view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_set_destination_template, cardContainer, true );
                viewHolder = new ViewHolderSetDestinationTemplate( view );
                break;
            case LOCAL_SEARCH_LIST_TEMPLATE1:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = ( ViewGroup ) view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_local_search_list_template1, cardContainer, true );
                viewHolder = new ViewHolderLocalSearchListTemplate1( view, inflater );
                break;
            case RENDER_PLAYER_INFO:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = ( ViewGroup ) view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_render_player_info, cardContainer, true );
                viewHolder = new ViewHolderRenderPlayerInfo( view );
                break;
            default:
                view = inflater.inflate( R.layout.log_item, viewGroup, false );
                viewHolder = new ViewHolderTextLog( view );
                break;
        }
        return viewHolder;
    }

    /**
     * This method internally calls onBindViewHolder(ViewHolder, int) to update the
     * RecyclerView.ViewHolder contents with the item at the given position
     * and also sets up some private fields to be used by RecyclerView.
     *
     * @param viewHolder The type of RecyclerView.ViewHolder to populate
     * @param position Item position in the viewgroup.
     */
    @Override
    public void onBindViewHolder( RecyclerView.ViewHolder viewHolder, int position )
    {
        // Alternate log item color
        int color = position % 2 == 0 ? mContext.getResources().getColor( R.color.logEntry1 )
                : mContext.getResources().getColor( R.color.logEntry2 );
        viewHolder.itemView.setBackgroundColor( color );

        LogEntry log = mFilteredItems.get( position );
        JSONObject json = log.getJSON();

        switch ( viewHolder.getItemViewType() )
        {
            case LOG:
                ConfigureViewHolder.configureTextLog( (ViewHolderTextLog) viewHolder, json );
                break;
            case BODY_TEMPLATE1:
                ConfigureViewHolder.configureBodyTemplate1( (ViewHolderBodyTemplate1) viewHolder, json );
                break;
            case BODY_TEMPLATE2:
                ConfigureViewHolder.configureBodyTemplate2( (ViewHolderBodyTemplate2) viewHolder, json );
                break;
            case LIST_TEMPLATE1:
                ConfigureViewHolder.configureListTemplate1( (ViewHolderListTemplate1) viewHolder, json );
                break;
            case WEATHER_TEMPLATE:
                ConfigureViewHolder.configureWeatherTemplate( (ViewHolderWeatherTemplate) viewHolder, json );
                break;
            case SET_DESTINATION_TEMPLATE:
                ConfigureViewHolder.configureSetDestinationTemplate( (ViewHolderSetDestinationTemplate) viewHolder, json );
                break;
            case LOCAL_SEARCH_LIST_TEMPLATE1:
                ConfigureViewHolder.configureLocalSearchListTemplate1( (ViewHolderLocalSearchListTemplate1) viewHolder, json );
                break;
            case RENDER_PLAYER_INFO:
                ConfigureViewHolder.configureRenderPlayerInfo( (ViewHolderRenderPlayerInfo) viewHolder, json );
                break;
            default:
                ConfigureViewHolder.configureDefaultLog( (ViewHolderTextLog) viewHolder );
                break;
        }
    }

    //
    // For filtering logs by level type
    //

    // filter item out of log
    public void addFilterItem( String type, String level )
    {
        if( type.equals( "level") )
        {
            mFilteredLevels.add( level );
        }
        else if( type.equals( "source" ) )
        {
            mFilteredSources.add( level );
        }
        filter();

    }

    // put item back in log
    public void removeFilterItem( String type, String level )
    {
        if( type.equals( "level") )
        {
            mFilteredLevels.remove( level );
        }
        else if( type.equals( "source" ) )
        {
            mFilteredSources.remove( level );
        }
        filter();

    }

    public void filter()
    {
        mFilteredItems.clear();

        if ( mFilteredLevels.isEmpty() && mFilteredSources.isEmpty() )
        {
            mFilteredItems.addAll( mItems );
        } else
        {
            for ( LogEntry item : mItems )
            {
                JSONObject json = item.getJSON();
                try
                {
                    String level = json.getString( "level" );
                    String source = json.getString( "source" );
                    if ( !mFilteredLevels.contains( level ) && !mFilteredSources.contains( source ) )
                    {
                        mFilteredItems.add( item );
                    }
                } catch ( JSONException e )
                {
                    e.printStackTrace();
                    mFilteredItems.add( item );
                }
            }
        }
        notifyDataSetChanged();
    }
}