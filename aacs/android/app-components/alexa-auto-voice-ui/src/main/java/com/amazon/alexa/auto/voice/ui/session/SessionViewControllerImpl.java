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

import android.view.ViewGroup;

import com.amazon.alexa.auto.apis.session.SessionViewController;

import java.util.Optional;

import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Implementation for {@link SessionViewController}.
 */
public class SessionViewControllerImpl implements SessionViewController {
    private final BehaviorSubject<Boolean> mTemplateDisplayed;
    private ViewGroup mVoiceView;

    public SessionViewControllerImpl() {
        mTemplateDisplayed = BehaviorSubject.createDefault(false);
    }

    @Override
    public boolean isSessionActive() {
        return mVoiceView != null;
    }

    @Override
    public void setSessionView(ViewGroup viewGroup) {
        mVoiceView = viewGroup;
    }

    @Override
    public Optional<ViewGroup> getTemplateRuntimeViewContainer() {
        return mVoiceView == null ? Optional.empty() : Optional.of(mVoiceView);
    }

    @Override
    public void clearTemplate() {
        if (mVoiceView != null) {
            mVoiceView.removeAllViews();
        }
        mTemplateDisplayed.onNext(false);
    }

    @Override
    public Observable<Boolean> getTemplateDisplayedObservable() {
        return mTemplateDisplayed;
    }

    @Override
    public void setTemplateDisplayed() {
        mTemplateDisplayed.onNext(true);
    }
}
