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
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import com.amazon.aacsconstants.AACSPropertyConstants;
import com.amazon.aacsconstants.FeatureDiscoveryConstants;
import com.amazon.alexa.auto.apps.common.util.FeatureDiscoveryUtil;
import com.amazon.alexa.auto.apps.common.util.config.AlexaPropertyManager;
import com.amazon.alexa.auto.settings.dependencies.AndroidModule;
import com.amazon.alexa.auto.settings.dependencies.DaggerSettingsComponent;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;
import java.util.Optional;
import java.util.Set;

import javax.inject.Inject;

public class ThingsToTryDetailsFragment extends Fragment {
    private static final String TAG = ThingsToTryDetailsFragment.class.getSimpleName();
    public static final String BUNDLE_KEY_DOMAIN_INDEX = "domainIndex";
    public static final String BUNDLE_KEY_STRING_RESOURCE_ID = "stringResId";

    @Inject
    AlexaPropertyManager mAlexaPropertyManager;

    private static final Map<Integer, String> mDomainMap = new HashMap<>();
    static {
        mDomainMap.put(R.string.ttt_getting_started, FeatureDiscoveryConstants.Domain.GETTING_STARTED);
        mDomainMap.put(R.string.ttt_alexa_s_talents, FeatureDiscoveryConstants.Domain.TALENTS);
        mDomainMap.put(R.string.ttt_entertainment, FeatureDiscoveryConstants.Domain.ENTERTAINMENT);
        mDomainMap.put(R.string.ttt_communication, FeatureDiscoveryConstants.Domain.COMMS);
        mDomainMap.put(R.string.ttt_weather, FeatureDiscoveryConstants.Domain.WEATHER);
        mDomainMap.put(R.string.ttt_smart_home, FeatureDiscoveryConstants.Domain.SMART_HOME);
        mDomainMap.put(R.string.ttt_news_and_information, FeatureDiscoveryConstants.Domain.NEWS);
        mDomainMap.put(R.string.ttt_navigation, FeatureDiscoveryConstants.Domain.NAVIGATION);
        mDomainMap.put(R.string.ttt_traffic_information, FeatureDiscoveryConstants.Domain.TRAFFIC);
        mDomainMap.put(R.string.ttt_skills, FeatureDiscoveryConstants.Domain.SKILLS);
        mDomainMap.put(R.string.ttt_lists, FeatureDiscoveryConstants.Domain.LISTS);
        mDomainMap.put(R.string.ttt_shopping, FeatureDiscoveryConstants.Domain.SHOPPING);
        mDomainMap.put(R.string.ttt_questions_and_answers, FeatureDiscoveryConstants.Domain.QUESTIONS_ANSWERS);
        mDomainMap.put(R.string.ttt_sports, FeatureDiscoveryConstants.Domain.SPORTS);
        mDomainMap.put(R.string.ttt_calendar, FeatureDiscoveryConstants.Domain.CALENDAR);
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        DaggerSettingsComponent.builder().androidModule(new AndroidModule(getContext())).build().inject(this);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        super.onCreateView(inflater, container, savedInstanceState);
        View view = inflater.inflate(R.layout.settings_things_to_try_layout, container, false);
        return view;
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        int domainIndex = getArguments().getInt(BUNDLE_KEY_DOMAIN_INDEX);
        int stringResId = getArguments().getInt(BUNDLE_KEY_STRING_RESOURCE_ID);
        mAlexaPropertyManager.getAlexaProperty(AACSPropertyConstants.LOCALE)
                .filter(Optional::isPresent)
                .map(Optional::get)
                .subscribe(alexaLocale -> {
                    if (!mDomainMap.containsKey(stringResId)) {
                        Log.e(TAG, "could not find matching domain for string resource id: " + stringResId);
                        return;
                    }
                    String domainString = mDomainMap.getOrDefault(stringResId, "");
                    String utteranceTag = FeatureDiscoveryUtil.createTag(
                            alexaLocale, domainString, FeatureDiscoveryConstants.EventType.THINGS_TO_TRY);
                    Set<String> utteranceSet = FeatureDiscoveryUtil.getFeaturesByTag(view.getContext(), utteranceTag);
                    CharSequence[] utteranceArray;
                    if (utteranceSet.size() > 0) {
                        Log.i(TAG, "Utterances were found for tag: " + utteranceTag);
                        utteranceArray =
                                Arrays.copyOf(utteranceSet.toArray(), utteranceSet.size(), CharSequence[].class);
                    } else {
                        Log.w(TAG, "Utterances were not found for tag: " + utteranceTag);
                        TypedArray typedArray = getResources().obtainTypedArray(R.array.things_to_try_utterances);
                        utteranceArray = typedArray.getTextArray(domainIndex);
                        typedArray.recycle();
                    }

                    RecyclerView recyclerView = view.findViewById(R.id.ttt_recyclerView);
                    LinearLayoutManager layoutManager =
                            new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false);
                    recyclerView.setLayoutManager(layoutManager);
                    recyclerView.setAdapter(new ThingsToTryDetailsAdapter(utteranceArray));

                    FeatureDiscoveryUtil.checkNetworkAndPublishGetFeaturesMessage(
                            getContext(), domainString, FeatureDiscoveryConstants.EventType.THINGS_TO_TRY);
                });
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
    }
}
