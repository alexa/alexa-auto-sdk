package com.amazon.sampleapp.impl.Audio;

import android.app.Activity;

import com.amazon.aace.audio.AudioInput;
import com.amazon.aace.audio.AudioInputProvider;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class AudioInputProviderHandler extends AudioInputProvider
{
    private static final String sTag = "AudioInputProviderHandler";

    private AudioInput mDefaultAudioInput = null;
    private final Activity mActivity;
    private final LoggerHandler mLogger;

    public AudioInputProviderHandler(  Activity activity,
                                       LoggerHandler logger ) {
        mActivity = activity;
        mLogger = logger;
    }

    @Override
    public AudioInput openChannel( String name, AudioInputType type )
    {
        mLogger.postInfo( sTag, String.format( "openChannel() for type %s", type ) );

        if( type == AudioInputType.VOICE || type == AudioInputType.COMMUNICATION ) {
            return getDefaultAudioInput();
        }
        else {
            return null;
        }
    }

    private AudioInput getDefaultAudioInput() {
        if( mDefaultAudioInput == null ) {
            mDefaultAudioInput = new AudioInputHandler( mActivity, mLogger );
        }
        return mDefaultAudioInput;
    }
}
