package com.amazon.alexa.auto.app.audio;

import android.app.Notification;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;

import androidx.annotation.Nullable;

import com.amazon.aacsipc.AACSReceiver;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.app.AutoApplication;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import javax.inject.Inject;

import io.reactivex.rxjava3.disposables.CompositeDisposable;

/**
 * An android service to Feed audio input to AACS and to playback audio output
 * coming from AACS.
 */
public class AudioIOService extends Service {
    @Inject
    AudioInputHandler mAudioInputHandler;
    @Inject
    AACSReceiver.Builder mAACSReceiverBuilder;

    private AACSReceiver mAACSReceiver;
    private CompositeDisposable mWorkersStateDisposable = new CompositeDisposable();

    // State
    boolean mServiceStarted = false;

    @Nullable
    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onCreate() {
        super.onCreate();

        AutoApplication app = (AutoApplication) AlexaApp.from(this);
        app.getAppComponent().createAudioIOComponent().inject(this);

        mAACSReceiver = mAACSReceiverBuilder.withFetchCallback(mAudioInputHandler).build();

        subscribeToWorkersState();
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        mWorkersStateDisposable.dispose();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        if (intent == null) {
            return super.onStartCommand(intent, flags, startId);
        }

        ensureServiceStarted();
        mAACSReceiver.receive(intent, null);

        return super.onStartCommand(intent, flags, startId);
    }

    /**
     * Subscribe to workers and stop the service if worker state goes
     * to idle.
     */
    private void subscribeToWorkersState() {
        mWorkersStateDisposable.add(mAudioInputHandler.getWorkerState().subscribe(state -> {
            @AudioIOServiceWorker.WorkerState
            int workerState = state;
            switch (workerState) {
                case AudioIOServiceWorker.IDLE:
                    stopSelf();
                    break;
                case AudioIOServiceWorker.WORKING:
                    ensureServiceStarted();
                    break;
            }
        }));
    }

    /**
     * Transition the service to start state if not already started.
     */
    private void ensureServiceStarted() {
        if (!mServiceStarted) {
            startSelf();
            mServiceStarted = true;
        }
    }

    /**
     * Transition the service to start state.
     */
    private void startSelf() {
        NotificationManager notificationManager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        Preconditions.checkNotNull(notificationManager);

        Notification notification = AudioServiceNotificationBuilder.buildNotification(this, notificationManager);
        startForeground(AudioServiceNotificationBuilder.AUDIO_NOTIFICATION_ID, notification);
    }
}
