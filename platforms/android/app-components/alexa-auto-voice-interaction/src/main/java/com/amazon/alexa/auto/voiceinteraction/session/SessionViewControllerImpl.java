package com.amazon.alexa.auto.voiceinteraction.session;

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
        mTemplateDisplayed.onNext(true);
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
}
