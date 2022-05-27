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

import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

/**
 * RecyclerView.Adapter for rendering the things-to-try detail page.
 */
public class ThingsToTryDetailsAdapter extends RecyclerView.Adapter<ThingsToTryDetailsAdapter.ViewHolder> {
    private static final String TAG = ThingsToTryDetailsAdapter.class.getSimpleName();
    private final CharSequence[] utteranceList;

    public ThingsToTryDetailsAdapter(CharSequence[] utteranceList) {
        this.utteranceList = utteranceList;
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext())
                            .inflate(R.layout.settings_things_to_try_utterance_layout, parent, false);
        return new ViewHolder(view);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        if (position < utteranceList.length) {
            holder.textView.setText(utteranceList[position]);
            holder.textView.setVisibility(View.VISIBLE);
        } else {
            Log.e(TAG, "invalid position received");
        }
    }

    @Override
    public int getItemCount() {
        return utteranceList.length;
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView textView;

        public ViewHolder(@NonNull View itemView) {
            super(itemView);
            textView = itemView.findViewById(R.id.ttt_utterance_text);
        }
    }
}
