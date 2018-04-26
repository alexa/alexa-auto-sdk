/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.sampleapp.impl;

import android.content.Context;
import android.content.SharedPreferences;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;
import android.preference.PreferenceManager;
import android.util.Log;

import com.amazon.aace.alexa.SpeechRecognizer;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

public class SpeechRecognizerHandler extends SpeechRecognizer
{
    private SharedPreferences m_preferences;

    private AudioRecord m_audioInput;
    private AudioReader m_reader;

    private LoggerHandler m_logger;

    // executor sevice
    private final ExecutorService m_executor = Executors.newSingleThreadExecutor();

    public SpeechRecognizerHandler( Context context, LoggerHandler logger, boolean wakewordEnabled )
    {
        super( wakewordEnabled );

        m_logger = logger;
        m_preferences = PreferenceManager.getDefaultSharedPreferences( context );
        m_audioInput = createAudioInput();
    }

    private AudioRecord createAudioInput()
    {
        AudioRecord audioRecord = null;

        try {
            audioRecord = new AudioRecord( MediaRecorder.AudioSource.MIC, 16000, AudioFormat.CHANNEL_IN_MONO, AudioFormat.ENCODING_PCM_16BIT, 1024 );
        }
        catch( Throwable ex ) {
            m_logger.post( ex );
        }

        return audioRecord;
    }

    @Override
    public boolean startAudioInput() {
        return m_audioInput != null ? startRecording() : false;
    }

    private boolean startRecording()
    {
        try
        {
            if( m_reader != null && m_reader.isRunning() ) {
                Log.e( "SpeechRecognizer", "startRecording() called but AudioRecorder thread is already running!" );
                return false;
            }
            else
            {
                m_audioInput.startRecording();

                // submit the audio reader thread
                m_executor.submit( m_reader = new AudioReader() );

                return true;
            }
        }
        catch( Throwable ex ) {
            m_logger.post( ex );
            return false;
        }
    }

    @Override
    public boolean stopAudioInput()
    {

        try
        {
            if( m_audioInput != null )
            {
                // cancel the audio reader
                if( m_reader != null ) {
                    m_reader.cancel();
                }
                m_audioInput.stop();

                return true;

            }
            else {
                return false;
            }
        }
        catch( Throwable ex ) {
            m_logger.post( ex );
            return false;
        }
    }

    //
    // AudioReader class
    //

    private class AudioReader implements Runnable
    {
        private boolean m_running = true;
        private byte[] m_buffer = new byte[300];

        public void cancel() {
            m_running = false;
        }

        public boolean isRunning() {
            return m_running;
        }

        @Override
        public void run()
        {
            int size = 0;

            while (m_running) {
                size = m_audioInput.read(m_buffer, 0, m_buffer.length);

                if (size > 0 && m_running) {
                    write(m_buffer, size);
                }
            }
        }
    }
}
