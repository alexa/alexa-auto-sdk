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

package com.amazon.sampleapp.logView;

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.amazon.aace.logger.Logger;
import com.amazon.maccandroid.Log;
import com.amazon.sampleapp.R;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class LogRecyclerViewAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    private final Context mContext;

    // The items to display in your RecyclerView
    private List<LogEntry> mItems;
    private List<LogEntry> mFilteredItems;

    // The items to filter out of the log
    private Set<String> mFilteredLevels;
    private Set<String> mFilteredSources;
    private boolean mHideCards;
    private boolean mHideJsonTemplates;
    private boolean mHideMapView;
    private static final String GoogleMapsAPIkey = "com.google.android.geo.API_KEY";

    private static final String sTag = "LogRecyclerViewAdapter";

    public static final int TEXT_LOG = 0, BODY_TEMPLATE1 = 1, BODY_TEMPLATE2 = 2, LIST_TEMPLATE1 = 3,
            WEATHER_TEMPLATE = 4, SET_DESTINATION_TEMPLATE = 5, LOCAL_SEARCH_LIST_TEMPLATE1 = 6,
            RENDER_PLAYER_INFO = 7, CBL_CODE = 8, CBL_CODE_EXPIRED = 9, JSON_TEXT = 10;
    public enum FILTER_TYPE { SOURCE, LEVEL, JSON, CARD }
    public LogRecyclerViewAdapter( List<LogEntry> items, Context context ) {
        mContext = context;
        mItems = items;
        mFilteredItems = new ArrayList<>();
        mFilteredItems.addAll( items );
        mFilteredLevels = new HashSet<>();
        mFilteredSources = new HashSet<>();
        mHideCards = false;
        mHideJsonTemplates = false;
        mHideMapView = false;

        final PackageManager pm = mContext.getPackageManager();
        // check for API key before trying to load map
        try {
            final ApplicationInfo info = pm.getApplicationInfo(mContext.getPackageName(),
                    PackageManager.GET_META_DATA);
            if (info.metaData != null) {
                final String value = String.valueOf(info.metaData.get(GoogleMapsAPIkey));
                if ( value.isEmpty() ) {
                    mHideMapView = true;
                    Log.i(sTag, "No Google maps API key in AndroidManifest.xml");
                }
            }
        } catch (final PackageManager.NameNotFoundException e) {
            Log.i(sTag, e.toString());
        }
    }

    // Return the size of your dataset (invoked by the layout manager)
    @Override
    public int getItemCount() { return mFilteredItems != null ? mFilteredItems.size() : mItems.size(); }

    //Returns the view type of the item at position for the purposes of view recycling.
    @Override
    public int getItemViewType( int position ) {
        if ( mFilteredItems.get( position ) != null ) {
            return mFilteredItems.get( position ).getType();
        }
        return -1;
    }

    /**
     * This method creates different RecyclerView.ViewHolder objects based on the item view type.
     *
     * @param viewGroup ViewGroup container for the item
     * @param viewType type of view to be inflated
     * @return viewHolder to be inflated
     */
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder( ViewGroup viewGroup, int viewType ) {
        RecyclerView.ViewHolder viewHolder;
        View view;
        ViewGroup cardContainer;
        LayoutInflater inflater = LayoutInflater.from( viewGroup.getContext() );

        switch ( viewType ) {
            case TEXT_LOG:
                view = inflater.inflate( R.layout.log_item, viewGroup, false );
                viewHolder = new ViewHolderTextLog( view );
                break;
            case BODY_TEMPLATE1:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_body_template1, cardContainer, true );
                viewHolder = new ViewHolderBodyTemplate1( view );
                break;
            case BODY_TEMPLATE2:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_body_template2, cardContainer, true );
                viewHolder = new ViewHolderBodyTemplate2( view );
                break;
            case LIST_TEMPLATE1:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_list_template1, cardContainer, true );
                viewHolder = new ViewHolderListTemplate1( view, inflater );
                break;
            case WEATHER_TEMPLATE:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_weather_template, cardContainer, true );
                viewHolder = new ViewHolderWeatherTemplate( view );
                break;
            case SET_DESTINATION_TEMPLATE:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_set_destination_template, cardContainer, true );
                viewHolder = new ViewHolderSetDestinationTemplate( view, mHideMapView );
                break;
            case LOCAL_SEARCH_LIST_TEMPLATE1:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_local_search_list_template1, cardContainer, true );
                viewHolder = new ViewHolderLocalSearchListTemplate1( view, inflater );
                break;
            case RENDER_PLAYER_INFO:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_render_player_info, cardContainer, true );
                viewHolder = new ViewHolderRenderPlayerInfo( view );
                break;
            case CBL_CODE:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_lwa_cbl, cardContainer, true );
                viewHolder = new ViewHolderCBLCard( view, mContext );
                break;
            case CBL_CODE_EXPIRED:
                view = inflater.inflate( R.layout.log_card_container, viewGroup, false );
                cardContainer = view.findViewById( R.id.container );
                inflater.inflate( R.layout.card_lwa_cbl_expired, cardContainer, true );
                viewHolder = new ViewHolderCBLExpiredCard( view );
                break;
            case JSON_TEXT:
                view = inflater.inflate( R.layout.log_item, viewGroup, false );
                viewHolder = new ViewHolderTextLog( view );
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
    public void onBindViewHolder( RecyclerView.ViewHolder viewHolder, int position ) {
        // Alternate log item color
        int color = position % 2 == 0 ? ContextCompat.getColor( mContext, R.color.logEntry1 )
                : ContextCompat.getColor( mContext, R.color.logEntry2 );
        viewHolder.itemView.setBackgroundColor( color );

        LogEntry log = mFilteredItems.get( position );
        JSONObject json = log.getJSON();

        switch ( viewHolder.getItemViewType() ) {
            case TEXT_LOG:
                ConfigureViewHolder.configureTextLog( ( ViewHolderTextLog ) viewHolder, json );
                break;
            case BODY_TEMPLATE1:
                ConfigureViewHolder.configureBodyTemplate1( ( ViewHolderBodyTemplate1 ) viewHolder,
                        json );
                break;
            case BODY_TEMPLATE2:
                ConfigureViewHolder.configureBodyTemplate2( ( ViewHolderBodyTemplate2 ) viewHolder,
                        json );
                break;
            case LIST_TEMPLATE1:
                ConfigureViewHolder.configureListTemplate1( ( ViewHolderListTemplate1 ) viewHolder,
                        json );
                break;
            case WEATHER_TEMPLATE:
                ConfigureViewHolder.configureWeatherTemplate(
                        ( ViewHolderWeatherTemplate ) viewHolder, json );
                break;
            case SET_DESTINATION_TEMPLATE:
                ConfigureViewHolder.configureSetDestinationTemplate(
                        ( ViewHolderSetDestinationTemplate ) viewHolder, json );
                break;
            case LOCAL_SEARCH_LIST_TEMPLATE1:
                ConfigureViewHolder.configureLocalSearchListTemplate1(
                        ( ViewHolderLocalSearchListTemplate1 ) viewHolder, json );
                break;
            case RENDER_PLAYER_INFO:
                ConfigureViewHolder.configureRenderPlayerInfo(
                        ( ViewHolderRenderPlayerInfo ) viewHolder, json );
                break;
            case CBL_CODE:
                ConfigureViewHolder.configureCBLCard( ( ViewHolderCBLCard ) viewHolder, json );
                break;
            case CBL_CODE_EXPIRED:
                ConfigureViewHolder.configureCBLExpiredCard(
                        ( ViewHolderCBLExpiredCard ) viewHolder, json );
                break;
            case JSON_TEXT:
                ConfigureViewHolder.configureTextLog( ( ViewHolderTextLog ) viewHolder, json );
                break;
            default:
                ConfigureViewHolder.configureDefaultLog( ( ViewHolderTextLog ) viewHolder );
                break;
        }
    }

    public void clear() {
        final int size = mFilteredItems.size();
        mItems.clear();
        mFilteredItems.clear();
        notifyItemRangeRemoved( 0, size );
    }

    //
    // For filtering displayed logs by type (e.g. display cards, AVS source logs, etc.)
    //

    public void setCardDisplayMode( boolean display ) {
        toggleFilterSwitch( display, "", FILTER_TYPE.CARD );
    }

    public void setJsonDisplayMode( boolean display ) {
        toggleFilterSwitch( display, "", FILTER_TYPE.JSON );
    }

    public void setSourceDisplayMode( String source, boolean display ) {
        toggleFilterSwitch( display, source, FILTER_TYPE.SOURCE );
    }

    // Filter out levels below selected level
    public void filterLevelDisplay( Logger.Level selectedLevel ) {
        for ( Logger.Level level : Logger.Level.values() ) {
            if ( level.ordinal() >= selectedLevel.ordinal() ) {
                includeInLog( FILTER_TYPE.LEVEL, level.toString() );
            } else {
                hideFromLog( FILTER_TYPE.LEVEL, level.toString() );
            }
        }
    }

    // Toggle filter mode for a type of log
    private void toggleFilterSwitch( boolean isChecked,
                                    String filter,
                                    FILTER_TYPE type ) {
        if ( !isChecked ) {
            // Filter item type from log
            hideFromLog( type, filter );
        } else {
            // Include item type in log
            includeInLog( type, filter );
        }
    }

    // Filter item out of log
    private void hideFromLog( FILTER_TYPE type, String value ) {
        switch ( type ) {
            case LEVEL:
                mFilteredLevels.add( value );
                break;
            case SOURCE:
                mFilteredSources.add( value );
                break;
            case CARD:
                mHideCards = true;
                break;
            case JSON:
                mHideJsonTemplates = true;
                break;
        }
        filter();
    }

    // Show item in log
    private void includeInLog( FILTER_TYPE type, String value ) {
        switch ( type ) {
            case LEVEL:
                mFilteredLevels.remove( value );
                break;
            case SOURCE:
                mFilteredSources.remove( value );
                break;
            case CARD:
                mHideCards = false;
                break;
            case JSON:
                mHideJsonTemplates = false;
                break;
        }
        filter();
    }

    // Update filters
    public void filter() {
        mFilteredItems.clear();

        if ( mFilteredLevels.isEmpty() && mFilteredSources.isEmpty() && !mHideCards && !mHideJsonTemplates) {
            mFilteredItems.addAll( mItems );
        } else {
            for ( LogEntry item : mItems ) {
                JSONObject json = item.getJSON();
                try {
                    String level = json.getString( "level" );
                    String source = json.getString( "source" );
                    boolean isHiddenCard = mHideCards && item.getType() != TEXT_LOG
                            && item.getType() != JSON_TEXT;
                    boolean isHiddenJsonTemplate = mHideJsonTemplates && item.getType() == JSON_TEXT;

                    if ( !mFilteredLevels.contains( level ) && !mFilteredSources.contains( source )
                            && !isHiddenCard && !isHiddenJsonTemplate ) {
                        mFilteredItems.add( item );
                    }
                } catch ( JSONException e ) {
                    e.printStackTrace();
                    mFilteredItems.add( item );
                }
            }
        }
        notifyDataSetChanged();
    }
}
