package com.amazon.alexa.auto.voice.ui.session;

import com.amazon.alexa.auto.apis.session.SessionActivityController;

import androidx.fragment.app.Fragment;
import io.reactivex.rxjava3.core.Observable;
import io.reactivex.rxjava3.subjects.BehaviorSubject;

/**
 * Implementation for {@link SessionActivityController}.
 */
public class SessionActivityControllerImpl implements SessionActivityController {
    private final BehaviorSubject<Boolean> mFragmentAdded;
    private Fragment mVoiceFragment;
    private boolean isFragmentAdded;

    public SessionActivityControllerImpl() {
        mFragmentAdded = BehaviorSubject.createDefault(false);
    }

    @Override
    public void addFragment(Fragment fragment) {
        mVoiceFragment = fragment;
        mFragmentAdded.onNext(true);
        isFragmentAdded = true;
    }

    @Override
    public void removeFragment() {
        mFragmentAdded.onNext(false);
        isFragmentAdded = false;
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
    public Observable<Boolean> getFragmentAddedObservable() {
        return mFragmentAdded;
    }
}
