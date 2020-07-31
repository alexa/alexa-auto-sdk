package com.amazon.sampleapp.impl.AlexaSpeaker;

import android.app.Activity;
import android.view.MotionEvent;
import android.view.View;
import android.widget.SeekBar;
import android.widget.TextView;

import com.amazon.aace.alexa.AlexaSpeaker;
import com.amazon.sampleapp.R;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

public class AlexaSpeakerHandler extends AlexaSpeaker {
    private static final String TAG = AlexaSpeaker.class.getSimpleName();

    private final Activity mActivity;
    private final LoggerHandler mLogger;

    private boolean mIsMuted = false;
    private byte mAlexaVolume = 50;
    private byte mAlertsVolume = 50;

    public AlexaSpeakerHandler(Activity activity, LoggerHandler logger) {
        mActivity = activity;
        mLogger = logger;
        setupGUI();
    }

    @Override
    public void speakerSettingsChanged(SpeakerType type, boolean local, byte volume, boolean mute) {
        mLogger.postInfo(TAG,
                String.format("speakerSettingsChanged [type=%s,local=%b,volume=%d,mute=%b]", type.toString(), local,
                        volume, mute));

        if (type == SpeakerType.ALEXA_VOLUME) {
            mAlexaVolume = volume;
            mIsMuted = mute;
        } else if (type == SpeakerType.ALERTS_VOLUME) {
            mAlertsVolume = volume;
        }

        // update the ui controls
        updateUI();
    }

    //
    // GUI
    //
    private SeekBar mAlexaVolumeControl;
    private SeekBar mAlertsVolumeControl;
    private TextView mMuteButton;

    private void setupGUI() {
        mAlexaVolumeControl = mActivity.findViewById(R.id.speakerVolume);

        mAlexaVolumeControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {}

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                if (mIsMuted) {
                    mIsMuted = false;
                    localSetMute(SpeakerType.ALEXA_VOLUME, false);
                }

                localSetVolume(SpeakerType.ALEXA_VOLUME, (byte) seekBar.getProgress());
            }
        });

        // Prevent parent view from intercepting touch events
        mAlexaVolumeControl.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction();

                switch (action) {
                    case MotionEvent.ACTION_DOWN:
                        v.getParent().requestDisallowInterceptTouchEvent(true);
                        break;

                    case MotionEvent.ACTION_UP:
                        v.getParent().requestDisallowInterceptTouchEvent(false);
                        break;
                }

                // Handle SeekBar touch events
                v.onTouchEvent(event);
                return true;
            }
        });

        mAlertsVolumeControl = mActivity.findViewById(R.id.alertsVolume);

        mAlertsVolumeControl.setOnSeekBarChangeListener(new SeekBar.OnSeekBarChangeListener() {
            @Override
            public void onProgressChanged(SeekBar seekBar, int progress, boolean fromUser) {}

            @Override
            public void onStartTrackingTouch(SeekBar seekBar) {}

            @Override
            public void onStopTrackingTouch(SeekBar seekBar) {
                localSetVolume(SpeakerType.ALERTS_VOLUME, (byte) seekBar.getProgress());
            }
        });

        // Prevent parent view from intercepting touch events
        mAlertsVolumeControl.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                int action = event.getAction();

                switch (action) {
                    case MotionEvent.ACTION_DOWN:
                        v.getParent().requestDisallowInterceptTouchEvent(true);
                        break;

                    case MotionEvent.ACTION_UP:
                        v.getParent().requestDisallowInterceptTouchEvent(false);
                        break;
                }

                // Handle SeekBar touch events
                v.onTouchEvent(event);
                return true;
            }
        });

        mMuteButton = mActivity.findViewById(R.id.muteSpeakerButton);

        mMuteButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                localSetMute(SpeakerType.ALEXA_VOLUME, mIsMuted ? false : true);
            }
        });
    }

    private void updateUI() {
        mActivity.runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mMuteButton.setText(mIsMuted ? R.string.volume_unmute : R.string.volume_mute);
                mAlexaVolumeControl.setProgress(mAlexaVolume);
                mAlertsVolumeControl.setProgress(mAlertsVolume);
            }
        });
    }
}

/*
        private void setupUIVolumeControls( @Nullable final  Speaker.Type type ) {
            if ( type == Speaker.Type.AVS_SPEAKER ) {
                mVolumeControl = mActivity.findViewById( R.id.speakerVolume );
            } else {
                mVolumeControl = mActivity.findViewById( R.id.alertsVolume );
            }

            updateUIVolume( mVolume );
            updateMuteButton( mIsMuted );

            mVolumeControl.setOnSeekBarChangeListener( new SeekBar.OnSeekBarChangeListener() {
                @Override
                public void onProgressChanged( SeekBar seekBar, int progress, boolean fromUser ) {}

                @Override
                public void onStartTrackingTouch( SeekBar seekBar ) {}

                @Override
                public void onStopTrackingTouch( SeekBar seekBar ) {
                    int progress = seekBar.getProgress();
                    if ( type != Speaker.Type.AVS_SPEAKER ) {
                        localVolumeSet( ( byte ) progress );
                    } else {
                        // Unmute before setting volume
                        if ( mIsMuted ) {
                            updateMuteButton( false );
                            localMuteSet( false );
                        }
                        localVolumeSet( ( byte ) progress );
                    }
                }
            });

            // Prevent parent view from intercepting touch events
            mVolumeControl.setOnTouchListener( new View.OnTouchListener() {
                @Override
                public boolean onTouch( View v, MotionEvent event ) {
                    int action = event.getAction();
                    switch( action ) {
                        case MotionEvent.ACTION_DOWN:
                            v.getParent().requestDisallowInterceptTouchEvent( true );
                            break;

                        case MotionEvent.ACTION_UP:
                            v.getParent().requestDisallowInterceptTouchEvent( false );
                            break;
                    }

                    // Handle SeekBar touch events
                    v.onTouchEvent( event );
                    return true;
                }
            });

            if ( mMuteButton != null ) {
                mMuteButton.setOnClickListener( new View.OnClickListener() {
                    @Override
                    public void onClick( View v ) {
                        updateMuteButton( !mIsMuted );
                        mLogger.postVerbose( sTag, String.format( "Calling localMuteSet(%s)", !mIsMuted ) );
                        localMuteSet( !mIsMuted );
                    }
                });
            }
        }

        private void updateMuteButton( final boolean isMuted ) {
            if ( mMuteButton != null ) {
                mActivity.runOnUiThread( new Runnable() {
                    @Override
                    public void run() {
                        if ( !isMuted ) mMuteButton.setText( R.string.volume_mute );
                        else mMuteButton.setText( R.string.volume_unmute );
                    }
                });
            }
        }

        private void updateUIVolume( final byte vol ) {
            mActivity.runOnUiThread( new Runnable() {
                @Override
                public void run() { mVolumeControl.setProgress( vol ); }
            });
        }
    }
 */