/*
 * Copyright 2017-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl.Communication;

import android.content.Context;
import android.os.Build;
import android.os.Looper;
import android.support.annotation.Nullable;
import android.support.annotation.RequiresApi;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.amazon.sampleapp.R;


public class AlexaCommsView extends LinearLayout implements AlexaCommsHandler.AlexaCommsObserver {
    private TextView m_statusText;
    private Button m_acceptCallBtn;
    private Button m_stopCallBtn;

    private AlexaCommsHandler m_commsHandler;

    public AlexaCommsView(Context context) {
        super(context);
        init();
    }

    public AlexaCommsView(Context context, @Nullable AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public AlexaCommsView(Context context, @Nullable AttributeSet attrs, int defStyleAttr) {
        super(context, attrs, defStyleAttr);
        init();
    }

    @RequiresApi(api = Build.VERSION_CODES.LOLLIPOP)
    public AlexaCommsView(Context context, @Nullable AttributeSet attrs, int defStyleAttr, int defStyleRes) {
        super(context, attrs, defStyleAttr, defStyleRes);
        init();
    }

    /**
     * Initialize view
     */
    private void init() {
        inflate(getContext(), R.layout.alexa_comms_view, this);
        m_statusText = findViewById(R.id.alexacomms_status);
        m_acceptCallBtn = findViewById(R.id.alexa_comms_accept_btn);
        m_stopCallBtn = findViewById(R.id.alexa_comms_stop_btn);

        m_acceptCallBtn.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                m_commsHandler.acceptCall();
            }
        });

        m_stopCallBtn.setOnClickListener(new OnClickListener() {
            @Override
            public void onClick(View view) {
                m_commsHandler.stopCall();
            }
        });
    }

    public void setupUI(AlexaCommsHandler commsHandler) {
        m_commsHandler = commsHandler;
        m_commsHandler.addObserver(this);
    }

    @Override
    public void onDetachedFromWindow() {
        super.onDetachedFromWindow();
        if (m_commsHandler != null) {
            m_commsHandler.removeObserver(this);
        }
    }

    @Override
    public void onAttachedToWindow() {
        super.onAttachedToWindow();
        if (m_commsHandler != null) {
            m_commsHandler.addObserver(this);
        }
    }

    @Override
    public void onStateChanged(final AlexaCommsState newState) {
        if (Looper.getMainLooper().getThread() == Thread.currentThread()) {
            onCommsStateChanged(newState);
        } else {
            this.post(new Runnable() {
                @Override
                public void run() {
                    onCommsStateChanged(newState);
                }
            });
        }
    }

    private void onCommsStateChanged(AlexaCommsState commsState) {
        String statusText = getResources().getString(R.string.alexa_comms_status_noactive_call);
        boolean canAcceptCall = false;
        boolean canStopCall = false;

        switch (commsState.getCurrentCallState()) {
            case CONNECTING:
                statusText = getResources().getString(R.string.alexa_comms_status_connecting);
                canStopCall = true;
                break;
            case CALL_CONNECTED:
                statusText = getResources().getString(R.string.alexa_comms_status_connected);
                canStopCall = true;
                break;
            case CALL_DISCONNECTED:
                statusText = getResources().getString(R.string.alexa_comms_status_disconnected);
                break;
            case INBOUND_RINGING:
                statusText = getResources().getString(R.string.alexa_comms_status_inbound_ringing);
                canAcceptCall = true;
                canStopCall = true;
                break;
            case NONE:
                statusText = getResources().getString(R.string.alexa_comms_status_noactive_call);
                break;
        }

        m_statusText.setText(statusText);
        m_stopCallBtn.setEnabled(canStopCall);
        m_acceptCallBtn.setEnabled(canAcceptCall);
    }
}
