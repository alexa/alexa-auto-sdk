/*
 * Copyright 2021 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.apl.android.render.interfaces;

import android.view.KeyEvent;
import android.view.MotionEvent;

import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.IDocumentLifecycleListener;
import com.amazon.apl.android.dependencies.IOnAplFinishCallback;

/**
 * The presenter interface.
 */
public interface IPresenter
        extends IAPLContentListener, IOnAplFinishCallback, IAPLOptionsBuilderProvider, IDocumentLifecycleListener {
    /**
     * Indicates that the user has touched the screen.
     * @param event the touch event.
     *
     * @return whether the touch event was handled in APL layout.
     */
    void onTouchEvent(MotionEvent event);

    /**
     * Indicates that the user has pressed a keypad button.
     * @param event the key event.
     *
     * @return whether the key event was handled in APL layout.
     */
    boolean onKeyEvent(KeyEvent event);

    /**
     * @return the APLLayout where the APL document is to be rendered.
     */
    APLLayout getAplLayout();

    /**
     * Return the APL event sender.
     * @return
     */
    IAPLEventSender getAplEventSender();

    /**
     * Saves a reference to the local info data consumer.
     *
     * @param consumer The object that consume local info data events.
     */
    void setLocalInfoDataConsumer(ILocalInfoDataConsumer consumer);

    /**
     * Callback to notify when APL window should be dismissed.
     * @param dismissibleCallback
     */
    void setDismissibleCallback(IDismissible dismissibleCallback);

    /**
     * This should be called to stop execution when there is a barge in.
     */
    void cancelExecution();
}
