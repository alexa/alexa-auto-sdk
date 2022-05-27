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
package com.amazon.alexa.auto.settings;

import android.content.res.TypedArray;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.cardview.widget.CardView;
import androidx.navigation.NavController;
import androidx.navigation.Navigation;
import androidx.recyclerview.widget.RecyclerView;

import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.auth.AuthMode;
import com.amazon.alexa.auto.apps.common.util.ModuleProvider;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * RecyclerView.Adapter for rendering the things-to-try category page.
 */
public class ThingsToTryAdapter extends RecyclerView.Adapter<ThingsToTryAdapter.ViewHolder> {
    private static final String TAG = ThingsToTryAdapter.class.getSimpleName();
    private static final int ITEM_PER_PAGE = 6;
    private final TypedArray mGridDrawables;
    private final TypedArray mGridStrings;
    private static final List<Integer> cardLayouts = Arrays.asList(
            R.id.ttt_item1, R.id.ttt_item2, R.id.ttt_item3, R.id.ttt_item4, R.id.ttt_item5, R.id.ttt_item6);

    public ThingsToTryAdapter(TypedArray gridDrawables, TypedArray gridStrings) {
        mGridDrawables = gridDrawables;
        mGridStrings = gridStrings;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                            .inflate(R.layout.settings_things_to_try_grid_laylout, parent, false);
        if (!ModuleProvider.isPreviewModeEnabled(parent.getContext())
                || !AuthMode.AUTH_PROVIDER_AUTHORIZATION.equals(
                        AlexaApp.from(parent.getContext()).getRootComponent().getAuthController().getAuthMode())) {
            TextView a = view.findViewById(R.id.ttt_preview_mode_text);
            a.setVisibility(View.GONE);
        }
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        int startIndex = position * ITEM_PER_PAGE;
        for (int i = 0; i < 6; i++) {
            if (i < holder.cardViews.size() && i + startIndex < mGridDrawables.length()
                    && i + startIndex < mGridStrings.length()) {
                CardView cardView = holder.cardViews.get(i);
                ImageView imageView = cardView.findViewById(R.id.ttt_imageView);
                TextView textView = cardView.findViewById(R.id.ttt_textView);

                final int index = i + startIndex;
                final int stringRes = mGridStrings.getResourceId(index, 0);
                imageView.setImageResource(mGridDrawables.getResourceId(index, 0));
                textView.setText(stringRes);
                cardView.setVisibility(View.VISIBLE);

                cardView.setOnClickListener(view -> {
                    NavController navController = Navigation.findNavController(view);
                    navController.getGraph()
                            .findNode(R.id.navigation_fragment_alexa_settings_things_to_try_details)
                            .setLabel(textView.getText());
                    Bundle bundle = new Bundle();
                    bundle.putInt(ThingsToTryDetailsFragment.BUNDLE_KEY_DOMAIN_INDEX, index);
                    bundle.putInt(ThingsToTryDetailsFragment.BUNDLE_KEY_STRING_RESOURCE_ID, stringRes);
                    navController.navigate(R.id.navigation_fragment_alexa_settings_things_to_try_details, bundle);
                });
            }
        }
    }

    @Override
    public int getItemCount() {
        return mGridDrawables.length() / ITEM_PER_PAGE + 1;
    }

    @Override
    public void onViewRecycled(@NonNull ViewHolder holder) {
        super.onViewRecycled(holder);
        mGridStrings.recycle();
        mGridDrawables.recycle();
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public List<CardView> cardViews;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            cardViews = new ArrayList<>();
            for (Integer cardLayoutInt : cardLayouts) {
                cardViews.add(itemView.findViewById(cardLayoutInt));
            }
        }
    }
}
