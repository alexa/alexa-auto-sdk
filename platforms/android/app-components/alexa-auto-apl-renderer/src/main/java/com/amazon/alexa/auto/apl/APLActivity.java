package com.amazon.alexa.auto.apl;

import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apl.handler.APLHandler;
import com.amazon.alexa.auto.apps.common.util.FileUtil;
import com.amazon.alexa.auto.apps.common.util.Preconditions;
import com.amazon.apl.android.APLLayout;
import com.amazon.apl.android.render.APLPresenter;

import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.lang.ref.WeakReference;

/**
 * Activity for Alexa APL screen.
 */
public class APLActivity extends AppCompatActivity {
    private static final String TAG = APLActivity.class.getSimpleName();

    @Nullable
    private APLHandler mAplHandler;
    @NonNull
    private final APLDirectiveReceiver mAPLReceiver;

    private JSONArray mVisualConfig;
    private String mDefaultWindowId;
    private Intent mCreationIntent;

    public APLActivity() {
        mAPLReceiver = new APLDirectiveReceiver();
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        //---------------------------------------------------------------------
        // Initialize the APL Runtime. This must be called during
        // Activity.onCreate() or prior to APLLayout inflation.
        //---------------------------------------------------------------------
        APLPresenter.initialize(this);

        setContentView(R.layout.activity_apl);

        mCreationIntent = getIntent();
        if (mCreationIntent != null) {
            APLLayout aplLayout = findViewById(R.id.apl);
            WeakReference<Context> contextWk = new WeakReference<>(this);
            mAplHandler = new APLHandler(contextWk, new AACSMessageSender(contextWk, new AACSSender()), aplLayout);
        }
        FileUtil.readAACSConfigurationAsync(getApplicationContext()).subscribe(this::buildAPLPresenter);
    }

    @Override
    protected void onStart() {
        super.onStart();
        EventBus.getDefault().register(mAPLReceiver);
    }

    @Override
    protected void onStop() {
        super.onStop();
        EventBus.getDefault().unregister(mAPLReceiver);
    }

    @Override
    protected void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        handleRenderDocumentIntent(intent);
    }

    private void handleRenderDocumentIntent(@NonNull Intent creationIntent) {
        Preconditions.checkNotNull(mAplHandler);
        try {
            String payload = creationIntent.getStringExtra(Constants.PAYLOAD);
            Preconditions.checkNotNull(payload);

            JSONObject json = new JSONObject(payload);
            String token = json.getString(Constants.TOKEN);
            String renderPayload = json.getString(Constants.PAYLOAD);

            mAplHandler.renderDocument(renderPayload, token, mDefaultWindowId);
        } catch (Exception exception) {
            Log.w(TAG, "Failed to handle render document. Error:" + exception);
        }
    }

    private void handleClearDocumentIntent(@NonNull String payload) {
        Preconditions.checkNotNull(mAplHandler);
        try {
            JSONObject json = new JSONObject(payload);
            String token = json.getString(Constants.TOKEN);

            mAplHandler.clearDocument(token);
        } catch (Exception exception) {
            Log.w(TAG, "Failed to handle clear document. Error:" + exception);
        }
    }

    private void handleExecuteCommandsIntent(@NonNull String payload) {
        Preconditions.checkNotNull(mAplHandler);
        try {
            JSONObject json = new JSONObject(payload);
            String token = json.getString(Constants.TOKEN);
            String executeCommandPayload = json.getString(Constants.PAYLOAD);

            mAplHandler.executeCommands(executeCommandPayload, token);
        } catch (Exception exception) {
            Log.w(TAG, "Failed to handle execute commands. Error:" + exception);
        }
    }

    /**
     * Initialize APLPresenter class to provide the orchestration logic in the APL rendering process.
     *
     * @param configs AACS configs.
     */
    private void buildAPLPresenter(@NonNull String configs) {
        Preconditions.checkNotNull(mAplHandler);
        try {
            JSONObject config = new JSONObject(configs);
            mVisualConfig =
                    config.getJSONObject("aacs.alexa").getJSONObject("gui").getJSONArray("visualCharacteristics");

            if (mVisualConfig.length() > 0) {
                for (int i = 0; i < mVisualConfig.length(); i++) {
                    JSONObject currentElement = mVisualConfig.getJSONObject(i);
                    if ("Alexa.Display.Window".equals(currentElement.getString("interface"))) {
                        JSONArray templates = currentElement.getJSONObject("configurations").getJSONArray("templates");
                        for (int j = 0; j < templates.length(); j++) {
                            JSONObject template = templates.getJSONObject(j);
                            mDefaultWindowId = template.getString("id");
                            break;
                        }
                    }
                }
            }

            mAplHandler.buildAPLPresenter(mVisualConfig, mDefaultWindowId);

            handleRenderDocumentIntent(mCreationIntent);
        } catch (JSONException e) {
            Log.w(TAG, "Failed to parse APL visual characteristics" + e);
        }
    }

    class APLDirectiveReceiver {
        @Subscribe
        public void OnReceive(APLDirective directive) {
            switch (directive.message.action) {
                case Action.APL.EXECUTE_COMMANDS:
                    Preconditions.checkNotNull(directive.message.payload);
                    handleExecuteCommandsIntent(directive.message.payload);
                    break;
                case Action.APL.CLEAR_DOCUMENT:
                    handleClearDocumentIntent(directive.message.payload);
                    finish();
                    break;
                default:
                    Log.d(TAG, "Unknown APL intent, action is " + directive.message.action);
                    break;
            }
        }
    }
}