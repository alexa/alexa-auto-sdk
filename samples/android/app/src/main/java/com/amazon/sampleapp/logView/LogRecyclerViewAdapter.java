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

import android.content.Context;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.support.v4.content.ContextCompat;
import android.support.v7.widget.RecyclerView;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import com.amazon.aace.logger.Logger;
import com.amazon.sampleapp.LimitedSizeArrayList;
import com.amazon.sampleapp.R;

import org.json.JSONException;
import org.json.JSONObject;

import java.util.HashSet;
import java.util.List;
import java.util.Set;

public class LogRecyclerViewAdapter extends RecyclerView.Adapter<RecyclerView.ViewHolder> {
    /// A string to identify log entries originating from this file
    private static final String TAG = "LogRecyclerViewAdapter";

    // Integers to identify LogEntry types for view recycling
    public static final int TEXT_LOG = 0, BODY_TEMPLATE1 = 1, BODY_TEMPLATE2 = 2, LIST_TEMPLATE1 = 3,
                            WEATHER_TEMPLATE = 4, START_NAVIGATION_TEMPLATE = 5, LOCAL_SEARCH_LIST_TEMPLATE1 = 6,
                            RENDER_PLAYER_INFO = 7, CBL_CODE = 8, CBL_CODE_EXPIRED = 9, JSON_TEXT = 10,
                            PREVIOUS_WAYPOINTS_TEMPLATE = 11, LOCAL_SEARCH_LIST_TEMPLATE2 = 12,
                            LOCAL_SEARCH_DETAIL_TEMPLATE1 = 13, TRAFFIC_DETAILS_TEMPLATE = 14;

    /// Max number of logs to be kept in memory
    private static final int MAX_NUM_LOGS = 2000;
    /// Max number of logs to be displayed in the adapter
    private static final int MAX_NUM_FILTERED_LOGS = 500;

    /// The full list of log items
    private List<LogEntry> mItems;
    /// The log items to display in the RecyclerView
    private List<LogEntry> mFilteredItems;

    /// The log levels to hide from the log view
    private Set<String> mHiddenLevels;
    /// The log sources to hide from the log view
    private Set<String> mHiddenSources;
    /// Whether display card entries should be hidden from the log view
    private boolean mHideCards;
    /// Whether pretty-printed JSON text entries should be hidden from the log view
    private boolean mHideJsonTemplates;

    private boolean mHideMapView;
    private static final String GoogleMapsAPIkey = "com.google.android.geo.API_KEY";

    private final Context mContext;

    public LogRecyclerViewAdapter(Context context) {
        mContext = context;

        mItems = new LimitedSizeArrayList<>(MAX_NUM_LOGS);
        mFilteredItems = new LimitedSizeArrayList<>(MAX_NUM_FILTERED_LOGS);
        mHiddenLevels = new HashSet<>();
        mHiddenSources = new HashSet<>();
        mHideCards = false;
        mHideJsonTemplates = false;

        mHideMapView = false;
        final PackageManager pm = mContext.getPackageManager();
        // check for API key before trying to load map
        try {
            final ApplicationInfo info = pm.getApplicationInfo(mContext.getPackageName(), PackageManager.GET_META_DATA);
            if (info.metaData != null) {
                final String value = String.valueOf(info.metaData.get(GoogleMapsAPIkey));
                if (value.isEmpty()) {
                    mHideMapView = true;
                    Log.i(TAG, "No Google maps API key in AndroidManifest.xml");
                }
            }
        } catch (final PackageManager.NameNotFoundException e) {
            Log.i(TAG, e.toString());
        }
    }

    /// Returns the size of the data set (invoked by the layout manager).
    @Override
    public int getItemCount() {
        return mFilteredItems.size();
    }

    /// Returns the view type of the item at @a position for view recycling.
    @Override
    public int getItemViewType(int position) {
        if (mFilteredItems.get(position) != null) {
            return mFilteredItems.get(position).getType();
        }
        return -1;
    }

    /**
     * Called when RecyclerView needs a new RecyclerView.ViewHolder of the given type to represent
     * an item. This new ViewHolder should be constructed with a new View that can represent the
     * items of the given type.
     *
     * @param viewGroup The ViewGroup into which the new View will be added after it is bound to an
     *                  adapter position.
     * @param viewType The view type of the new View.
     * @return A new ViewHolder that holds a View of the given view type.
     */
    @Override
    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup viewGroup, int viewType) {
        RecyclerView.ViewHolder viewHolder;
        View view;
        ViewGroup cardContainer;
        LayoutInflater inflater = LayoutInflater.from(viewGroup.getContext());

        switch (viewType) {
            case TEXT_LOG:
                view = inflater.inflate(R.layout.log_item, viewGroup, false);
                viewHolder = new ViewHolderTextLog(view);
                break;
            case BODY_TEMPLATE1:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_body_template1, cardContainer, true);
                viewHolder = new ViewHolderBodyTemplate1(view);
                break;
            case BODY_TEMPLATE2:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_body_template2, cardContainer, true);
                viewHolder = new ViewHolderBodyTemplate2(view);
                break;
            case LIST_TEMPLATE1:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_list_template1, cardContainer, true);
                viewHolder = new ViewHolderListTemplate1(view, inflater);
                break;
            case WEATHER_TEMPLATE:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_weather_template, cardContainer, true);
                viewHolder = new ViewHolderWeatherTemplate(view);
                break;
            case START_NAVIGATION_TEMPLATE:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_start_navigation_template, cardContainer, true);
                viewHolder = new ViewHolderStartNavigationTemplate(view, mHideMapView);
                break;
            case PREVIOUS_WAYPOINTS_TEMPLATE:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_previous_waypoints_template, cardContainer, true);
                viewHolder = new ViewHolderPreviousWaypointsTemplate(view);
                break;
            case LOCAL_SEARCH_LIST_TEMPLATE1:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_local_search_list_template1, cardContainer, true);
                viewHolder = new ViewHolderLocalSearchListTemplate1(view, inflater);
                break;
            case RENDER_PLAYER_INFO:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_render_player_info, cardContainer, true);
                viewHolder = new ViewHolderRenderPlayerInfo(view);
                break;
            case CBL_CODE:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_lwa_cbl, cardContainer, true);
                viewHolder = new ViewHolderCBLCard(view, mContext);
                break;
            case CBL_CODE_EXPIRED:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_lwa_cbl_expired, cardContainer, true);
                viewHolder = new ViewHolderCBLExpiredCard(view);
                break;
            case JSON_TEXT:
                view = inflater.inflate(R.layout.log_item, viewGroup, false);
                viewHolder = new ViewHolderTextLog(view);
                break;
            case LOCAL_SEARCH_LIST_TEMPLATE2:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_local_search_list_template2, cardContainer, true);
                viewHolder = new ViewHolderLocalSearchListTemplate2(view, inflater);
                break;
            case LOCAL_SEARCH_DETAIL_TEMPLATE1:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_local_search_detail_template1, cardContainer, true);
                viewHolder = new ViewHolderLocalSearchDetailTemplate1(view);
                break;
            case TRAFFIC_DETAILS_TEMPLATE:
                view = inflater.inflate(R.layout.log_card_container, viewGroup, false);
                cardContainer = view.findViewById(R.id.container);
                inflater.inflate(R.layout.card_traffic_details_template, cardContainer, true);
                viewHolder = new ViewHolderTrafficDetailsTemplate(view);
                break;
            default:
                view = inflater.inflate(R.layout.log_item, viewGroup, false);
                viewHolder = new ViewHolderTextLog(view);
                break;
        }
        return viewHolder;
    }

    /**
     * Called by RecyclerView to display the data at the specified position.
     * This method should update the contents of the itemView to reflect the item at the given
     * position
     *
     * @param viewHolder The ViewHolder which should be updated to represent the contents of the
     *                   item at the given position in the data set.
     * @param position The position of the item within the adapter's data set.
     */
    @Override
    public void onBindViewHolder(RecyclerView.ViewHolder viewHolder, int position) {
        // Alternate log item color
        int color = position % 2 == 0 ? ContextCompat.getColor(mContext, R.color.logEntry1)
                                      : ContextCompat.getColor(mContext, R.color.logEntry2);
        viewHolder.itemView.setBackgroundColor(color);

        LogEntry log = mFilteredItems.get(position);
        JSONObject json = log.getJSON();
        switch (viewHolder.getItemViewType()) {
            case TEXT_LOG:
                ConfigureViewHolder.configureTextLog((ViewHolderTextLog) viewHolder, json);
                break;
            case BODY_TEMPLATE1:
                ConfigureViewHolder.configureBodyTemplate1((ViewHolderBodyTemplate1) viewHolder, json);
                break;
            case BODY_TEMPLATE2:
                ConfigureViewHolder.configureBodyTemplate2((ViewHolderBodyTemplate2) viewHolder, json);
                break;
            case LIST_TEMPLATE1:
                ConfigureViewHolder.configureListTemplate1((ViewHolderListTemplate1) viewHolder, json);
                break;
            case WEATHER_TEMPLATE:
                ConfigureViewHolder.configureWeatherTemplate((ViewHolderWeatherTemplate) viewHolder, json);
                break;
            case START_NAVIGATION_TEMPLATE:
                ConfigureViewHolder.configureStartNavigationTemplate(
                        (ViewHolderStartNavigationTemplate) viewHolder, json);
                break;
            case PREVIOUS_WAYPOINTS_TEMPLATE:
                ConfigureViewHolder.configurePreviousWaypointsTemplate(
                        (ViewHolderPreviousWaypointsTemplate) viewHolder, json);
                break;
            case LOCAL_SEARCH_LIST_TEMPLATE1:
                ConfigureViewHolder.configureLocalSearchListTemplate1(
                        (ViewHolderLocalSearchListTemplate1) viewHolder, json);
                break;
            case RENDER_PLAYER_INFO:
                ConfigureViewHolder.configureRenderPlayerInfo((ViewHolderRenderPlayerInfo) viewHolder, json);
                break;
            case CBL_CODE:
                ConfigureViewHolder.configureCBLCard((ViewHolderCBLCard) viewHolder, json);
                break;
            case CBL_CODE_EXPIRED:
                ConfigureViewHolder.configureCBLExpiredCard((ViewHolderCBLExpiredCard) viewHolder, json);
                break;
            case JSON_TEXT:
                ConfigureViewHolder.configureTextLog((ViewHolderTextLog) viewHolder, json);
                break;
            case LOCAL_SEARCH_LIST_TEMPLATE2:
                ConfigureViewHolder.configureLocalSearchListTemplate2(
                        (ViewHolderLocalSearchListTemplate2) viewHolder, json);
                break;
            case LOCAL_SEARCH_DETAIL_TEMPLATE1:
                ConfigureViewHolder.configureLocalSearchDetailTemplate1(
                        (ViewHolderLocalSearchDetailTemplate1) viewHolder, json);
                break;
            case TRAFFIC_DETAILS_TEMPLATE:
                ConfigureViewHolder.configureTrafficDetailsTemplate(
                        (ViewHolderTrafficDetailsTemplate) viewHolder, json);
                break;
            default:
                ConfigureViewHolder.configureDefaultLog((ViewHolderTextLog) viewHolder);
                break;
        }
    }

    /// Set whether display cards should be displayed in the log view.
    /// Note: Modifies adapter contents. Call from the main thread *only*
    public void setCardDisplayMode(boolean shouldDisplay) {
        mHideCards = !shouldDisplay;
        updateFilteredItemList();
    }

    /// Sets whether pretty-printed JSON text should be displayed in the log view.
    /// Note: Modifies adapter contents. Call from the main thread *only*
    public void setJsonDisplayMode(boolean shouldDisplay) {
        mHideJsonTemplates = !shouldDisplay;
        updateFilteredItemList();
    }

    /// Sets whether a log source type should be displayed in the log view.
    /// Note: Modifies adapter contents. Call from the main thread *only*
    public void setSourceDisplayMode(String source, boolean display) {
        if (!display)
            mHiddenSources.add(source);
        else
            mHiddenSources.remove(source);
        updateFilteredItemList();
    }

    /// Sets the log level filter. Hides levels below @a selectedLevel from the log view.
    /// Note: Modifies adapter contents. Call from the main thread *only*
    public void setDisplayLevel(Logger.Level selectedLevel) {
        for (Logger.Level level : Logger.Level.values()) {
            if (level.ordinal() >= selectedLevel.ordinal()) {
                mHiddenLevels.remove(level.toString());
            } else {
                mHiddenLevels.add(level.toString());
            }
        }
        updateFilteredItemList();
    }

    /// Inserts @c item to the log view backing list. @c item will be filtered from the view
    /// as necessary.
    /// Note: Modifies adapter contents. Call from the main thread *only*
    public void insertItem(LogEntry item) {
        mItems.add(item);
        if (shouldDisplayItem(item))
            mFilteredItems.add(item);
        else
            return;

        if (mFilteredItems.size() < MAX_NUM_FILTERED_LOGS) {
            notifyDataSetChanged();
        } else {
            // Note: mFiltered items is a limited size array list that removes the first
            // item and inserts a new item at the end when the max size is reached.
            // A more efficient way to notify of this structural/item change than
            // notifyDataSetChanged is TODO
            notifyDataSetChanged();
        }
    }

    /// Clears the log view and resets the data in the backing list.
    /// Note: Modifies adapter contents. Call from the main thread *only*
    public void clear() {
        int count = getItemCount();
        mItems.clear();
        mFilteredItems.clear();
        notifyItemRangeRemoved(0, count);
    }

    /// Reconstructs the filtered list of log entries to display.
    /// Note: Will force the LayoutManager to rebind and relayout all visible views.
    /// Note: Modifies adapter contents. Call from the main thread *only*
    private void updateFilteredItemList() {
        mFilteredItems.clear();
        for (LogEntry item : mItems) {
            if (shouldDisplayItem(item))
                mFilteredItems.add(item);
        }
        notifyDataSetChanged();
    }

    /// Checks whether @a item should be included in the filtered log view.
    private boolean shouldDisplayItem(LogEntry item) {
        JSONObject json = item.getJSON();
        try {
            String level = json.getString("level");
            if (mHiddenLevels.contains(level))
                return false;

            String source = json.getString("source");
            if (mHiddenSources.contains(source))
                return false;

            int type = item.getType();
            boolean isHiddenCard = mHideCards && type != TEXT_LOG && type != JSON_TEXT;
            if (isHiddenCard)
                return false;

            boolean isHiddenJsonTemplate = mHideJsonTemplates && type == JSON_TEXT;
            if (isHiddenJsonTemplate)
                return false;

            return true;
        } catch (JSONException e) {
            return true;
        }
    }
}
