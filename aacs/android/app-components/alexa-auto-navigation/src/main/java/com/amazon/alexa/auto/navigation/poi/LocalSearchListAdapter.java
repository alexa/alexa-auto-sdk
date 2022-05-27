/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.navigation.poi;

import android.annotation.SuppressLint;
import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.amazon.alexa.auto.aacs.common.navi.PointOfInterest;
import com.amazon.alexa.auto.navigation.R;
import com.amazon.alexa.auto.navigation.handlers.LocalSearchDirectiveHandler;
import com.amazon.alexa.auto.navigation.providers.NaviProvider;
import com.bumptech.glide.Glide;

import java.lang.ref.WeakReference;
import java.util.ArrayList;
import java.util.List;

/**
 * RecyclerView.Adapter for rendering the POI List Template Runtime template.
 */
public class LocalSearchListAdapter extends RecyclerView.Adapter<LocalSearchListAdapter.ViewHolder> {
    private static final String TAG = LocalSearchListAdapter.class.getSimpleName();

    public static final String POI_PROVIDER_YELP = "Yelp";

    private final NaviProvider mNaviProvider;
    private final List<PointOfInterest> mPOIs = new ArrayList<>();
    private final WeakReference<LocalSearchDirectiveHandler> mDirectiveHandlerWeakReference;
    private final Context mContext;

    public LocalSearchListAdapter(NaviProvider naviProvider,
            WeakReference<LocalSearchDirectiveHandler> directiveHandlerWeakReference, Context context) {
        mNaviProvider = naviProvider;
        mDirectiveHandlerWeakReference = directiveHandlerWeakReference;
        mContext = context;
    }

    /**
     * Add a POI to the list.
     * @param poi point of interest.
     */
    public void addPOI(PointOfInterest poi) {
        mPOIs.add(poi);
    }

    /**
     * Gets all the POIs in the list.
     * @return list of POIs.
     */
    public List<PointOfInterest> getPOIs() {
        return mPOIs;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        LayoutInflater inflater = LayoutInflater.from(parent.getContext());
        View poiItemView = inflater.inflate(R.layout.local_search_item, parent, false);
        return new ViewHolder(poiItemView);
    }

    @SuppressLint("SetTextI18n")
    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        final PointOfInterest poi = mPOIs.get(position);
        holder.mPOIOrdinal.setText(Integer.toString(position + 1));
        holder.mPOIName.setText(poi.getTitle().getMainTitle());
        holder.mPOIAddress.setText(poi.getAddress());
        holder.mEta.setText(poi.getTravelTime());
        holder.mPoiRoot.setOnClickListener(poiNameView -> {
            mNaviProvider.startNavigation(poi);
            if (mDirectiveHandlerWeakReference.get() != null) {
                mDirectiveHandlerWeakReference.get().clearTemplate();
            }
        });

        if (POI_PROVIDER_YELP.equals(poi.getProvider()) && poi.getRating().getImage().getSources().size() >= 1) {
            String yelpImageUrl = poi.getRating().getImage().getSources().get(0).getUrl();
            Glide.with(mContext).load(yelpImageUrl).into(holder.mYelpImage);
            holder.mYelpRatingCount.setText(String.format("(%s)", poi.getRating().getReviewCount()));
        } else {
            holder.mYelpRow.setVisibility(View.GONE);
        }

        if (position == (mPOIs.size() - 1)) {
            holder.mSplitter.setVisibility(View.GONE);
        }
    }

    @Override
    public int getItemCount() {
        return mPOIs.size();
    }

    static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView mYelpRatingCount;
        public ImageView mYelpImage;
        public View mYelpRow;
        public TextView mPOIOrdinal;
        public TextView mPOIAddress;
        public TextView mPOIName;
        public TextView mEta;
        public View mSplitter;
        public View mPoiRoot;
        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            mPOIOrdinal = itemView.findViewById(R.id.poi_ordinal);
            mPOIName = itemView.findViewById(R.id.poi_name);
            mPOIAddress = itemView.findViewById(R.id.poi_address);
            mEta = itemView.findViewById(R.id.poi_list_eta);
            mSplitter = itemView.findViewById(R.id.poi_splitter);
            mPoiRoot = itemView.findViewById(R.id.poi_item_root);
            mYelpRow = itemView.findViewById(R.id.yelp_rating_row);
            mYelpImage = itemView.findViewById(R.id.yelp_rating_image);
            mYelpRatingCount = itemView.findViewById(R.id.yelp_rating_count);
        }
    }
}
