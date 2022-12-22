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
package com.amazon.alexa.auto.settings

import android.content.res.TypedArray
import android.os.Bundle
import android.util.Log
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.cardview.widget.CardView
import androidx.navigation.Navigation.findNavController
import androidx.recyclerview.widget.RecyclerView
import com.amazon.alexa.auto.apis.app.AlexaApp
import com.amazon.alexa.auto.apis.auth.AuthMode
import com.amazon.alexa.auto.apps.common.util.ModuleProvider
import java.util.*

/**
 * RecyclerView.Adapter for rendering the things-to-try category page.
 */
class ThingsToTryAdapter(
    private val mGridDrawables: TypedArray,
    private val mGridStrings: TypedArray
) : RecyclerView.Adapter<ThingsToTryAdapter.ViewHolder>() {

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.settings_things_to_try_grid_layout, parent, false)
        if (!ModuleProvider.isPreviewModeEnabled(parent.context)
            || AuthMode.AUTH_PROVIDER_AUTHORIZATION != AlexaApp.from(parent.context).rootComponent.authController.authMode
        ) {
            val a = view.findViewById<TextView>(R.id.ttt_preview_mode_text)
            a.visibility = View.GONE
        }
        return ViewHolder(view)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val startIndex = position * ITEM_PER_PAGE
        for (i in 0 until ITEM_PER_PAGE) {
            if (i < holder.cardViews.size && i + startIndex < mGridDrawables.length() && i + startIndex < mGridStrings.length()) {
                val cardView = holder.cardViews[i]
                val imageView = cardView.findViewById<ImageView>(R.id.ttt_imageView)
                val textView = cardView.findViewById<TextView>(R.id.ttt_textView)
                val index = i + startIndex
                val stringRes = mGridStrings.getResourceId(index, 0)
                imageView.setImageResource(mGridDrawables.getResourceId(index, 0))
                textView.setText(stringRes)
                cardView.visibility = View.VISIBLE
                cardView.setOnClickListener { view: View? ->
                    val navController = findNavController(
                        view!!
                    )
                    navController.graph
                        .findNode(R.id.navigation_fragment_alexa_settings_things_to_try_details)
                        ?.label = textView.text
                    val bundle = Bundle()
                    bundle.putInt(ThingsToTryDetailsFragment.BUNDLE_KEY_DOMAIN_INDEX, index)
                    bundle.putInt(
                        ThingsToTryDetailsFragment.BUNDLE_KEY_STRING_RESOURCE_ID,
                        stringRes
                    )
                    bundle.putString(ThingsToTryDetailsFragment.BUNDLE_KEY_STRING_TITLE,
                        textView.text as String?
                    )
                    navController.navigate(
                        R.id.navigation_fragment_alexa_settings_things_to_try_details,
                        bundle
                    )
                }
            }
        }
    }

    override fun getItemCount(): Int {
        return mGridDrawables.length() / ITEM_PER_PAGE + 1
    }

    override fun onViewRecycled(holder: ViewHolder) {
        super.onViewRecycled(holder)
        mGridStrings.recycle()
        mGridDrawables.recycle()
    }

    class ViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
        var cardViews: MutableList<CardView> = ArrayList()

        init {
            for (cardLayoutInt in cardLayouts) {
                cardViews.add(itemView.findViewById(cardLayoutInt))
            }
        }
    }

    companion object {
        private val TAG = ThingsToTryAdapter::class.java.simpleName
        private const val ITEM_PER_PAGE = 6
        private val cardLayouts = listOf(
            R.id.ttt_item1,
            R.id.ttt_item2,
            R.id.ttt_item3,
            R.id.ttt_item4,
            R.id.ttt_item5,
            R.id.ttt_item6
        )
    }
}