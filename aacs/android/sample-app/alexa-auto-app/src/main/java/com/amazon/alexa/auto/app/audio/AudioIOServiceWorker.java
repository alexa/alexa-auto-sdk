package com.amazon.alexa.auto.app.audio;

import androidx.annotation.IntDef;
import androidx.annotation.NonNull;

import com.amazon.alexa.auto.aacs.common.AACSMessage;

import java.lang.annotation.Documented;
import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

import io.reactivex.rxjava3.core.Observable;

/**
 * An interface to represent a work performed by Audio IO service.
 * The service determines its own lifecycle by combining the state of all the
 * work items.
 */
public interface AudioIOServiceWorker {
    @Documented
    @Retention(RetentionPolicy.SOURCE)
    @IntDef({IDLE, WORKING})
    @interface WorkerState {}

    int IDLE = 0;
    int WORKING = 1;

    /**
     * Obtain a stream to listen to worker state.
     *
     * @return Stream of worker state.
     */
    @NonNull
    Observable<Integer> getWorkerState();
}
