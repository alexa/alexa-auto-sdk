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
package com.amazon.alexa.auto.voice.ui.session;

import android.util.Log;

import androidx.fragment.app.Fragment;

import com.amazon.alexa.auto.apis.session.SessionActivityController;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Implementation for {@link SessionActivityController}.
 */
public class SessionActivityControllerImpl implements SessionActivityController {
    private static final String TAG = SessionActivityControllerImpl.class.getSimpleName();

    private final BehaviorSubject<Boolean> mFragmentAdded;
    private Fragment mVoiceFragment;
    private boolean isFragmentAdded;
    private boolean isFragmentConsumed;

    public SessionActivityControllerImpl() {
        Log.d(TAG, "constructor");
        isFragmentAdded = false;
        isFragmentConsumed = false;
        mFragmentAdded = BehaviorSubject.createDefault(isFragmentAdded);
    }

    @Override
    public void addFragment(Fragment fragment) {
        Log.d(TAG, "addFragment");
        mVoiceFragment = fragment;
        isFragmentConsumed = false;
        isFragmentAdded = true;
        mFragmentAdded.onNext(isFragmentAdded);
    }

    @Override
    public void removeFragment() {
        Log.d(TAG, "removeFragment");
        isFragmentConsumed = true;
        isFragmentAdded = false;
        mFragmentAdded.onNext(isFragmentAdded);
    }

    @Override
    public void consumeFragment() {
        isFragmentConsumed = true;
    }

    @Override
    public Fragment getFragment() {
        return mVoiceFragment;
    }

    @Override
    public boolean isFragmentAdded() {
        return isFragmentAdded;
    }

    @Override
    public boolean isFragmentConsumed() {
        return isFragmentConsumed;
    }

    @Override
    public Observable<Boolean> getFragmentAddedObservable() {
        return mFragmentAdded;
    }
}
