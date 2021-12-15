package com.amazon.alexa.auto.apl;

import static com.amazon.aacsconstants.AASBConstants.AlexaClient.DIALOG_STATE_LISTENING;
import static com.amazon.alexa.auto.apl.Constants.STATE;
import static com.amazon.alexa.auto.apps.common.Constants.APL_RUNTIME_PROPERTIES;
import static com.amazon.alexa.auto.apps.common.Constants.APL_RUNTIME_PROPERTY_DRIVING_STATE_NAME;
import static com.amazon.alexa.auto.apps.common.Constants.APL_RUNTIME_PROPERTY_THEME_NAME;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsipc.AACSSender;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;
import com.amazon.alexa.auto.apis.app.AlexaApp;
import com.amazon.alexa.auto.apis.session.SessionActivityController;
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
 * Fragment for Alexa Auto APL screen.
 */
public class APLFragment extends Fragment {
    private static final String TAG = APLFragment.class.getSimpleName();

    private static final double APL_FRAGMENT_MARGIN_RATIO = 0.05;

    @Nullable
    private APLHandler mAplHandler;
    @NonNull
    private final APLDirectiveReceiver mAPLReceiver;

    private JSONArray mVisualConfig;
    private String mDefaultWindowId;
    private int mAPLViewPortWidth;
    private int mAPLViewPortHeight;
    private Bundle mCreationArgs;

    private String mCurrentPayload = "";

    private APLLayout mAPLLayout;

    public APLFragment() {
        mAPLReceiver = new APLDirectiveReceiver();
    }

    @Override
    public void onStart() {
        Log.d(TAG, "onStart");
        super.onStart();
        EventBus.getDefault().register(mAPLReceiver);
    }

    @Override
    public void onStop() {
        Log.d(TAG, "onStop");
        super.onStop();
        if (mAplHandler != null) {
            mAplHandler.cancelExecution();
        }
        EventBus.getDefault().unregister(mAPLReceiver);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        handleClearDocumentIntent(mCurrentPayload);
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        Log.d(TAG, "onViewCreated");
        super.onViewCreated(view, savedInstanceState);

        Context context = getContext();
        Preconditions.checkNotNull(context);

        mCreationArgs = getArguments();
        if (mCreationArgs != null) {
            View fragmentView = requireView();
            mAPLLayout = fragmentView.findViewById(R.id.apl);

            WeakReference<Context> contextWk = new WeakReference<>(context);
            mAplHandler = new APLHandler(contextWk, new AACSMessageSender(contextWk, new AACSSender()), mAPLLayout);
        }

        FileUtil.readAACSConfigurationAsync(context).subscribe(this::buildAPLPresenter);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        Context context = getContext();
        Preconditions.checkNotNull(context);

        //---------------------------------------------------------------------
        // Initialize the APL Runtime. This must be called during
        // Activity.onCreate() or prior to APLLayout inflation.
        //---------------------------------------------------------------------
        APLPresenter.initialize(context);

        // Inflate the layout for this fragment
        return inflater.inflate(R.layout.fragment_apl, container, false);
    }

    private void handleRenderDocumentIntent(@NonNull Bundle creationArgs) {
        Preconditions.checkNotNull(mAplHandler);

        try {
            String payload = creationArgs.getString(Constants.PAYLOAD);
            Log.i(TAG, "handleRenderDocumentIntent payload: " + payload);
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
        if (mAplHandler != null) {
            try {
                JSONObject json = new JSONObject(payload);
                String token = json.getString(Constants.TOKEN);

                mAplHandler.clearDocument(token);
            } catch (Exception exception) {
                Log.w(TAG, "Failed to handle clear document. Error:" + exception);
            }
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

    private void handleUpdateAPLRuntimePropertiesIntent(@NonNull String payload) {
        Preconditions.checkNotNull(mAplHandler);
        try {
            // Construct APL runtime properties with local cached values.
            String aplRuntimeProperties = constructAPLRuntimeProperties();
            mAplHandler.handleAPLRuntimeProperties(aplRuntimeProperties);
        } catch (Exception exception) {
            Log.w(TAG, "Failed to handle update APL runtime properties commands. Error:" + exception);
        }
    }

    private void handleDialogStateChangedIntent(@NonNull String payload) {
        Preconditions.checkNotNull(mAplHandler);
        try {
            Log.v(TAG, "handleDialogStateChangedIntent: " + payload);
            JSONObject dialogState = new JSONObject(payload);
            // Cancel APL execution when we go into listening state
            if (dialogState.getString(STATE).equals(DIALOG_STATE_LISTENING)) {
                Log.v(TAG, "handleDialogStateChangedIntent: cancellingExecution");
                mAplHandler.cancelExecution();
            }
        } catch (Exception exception) {
            Log.w(TAG, "Failed to handle dialog state change. Error:" + exception);
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
                        JSONObject template = templates.getJSONObject(0);
                        mDefaultWindowId = template.getString("id");
                        mAPLViewPortWidth = template.getJSONObject("configuration")
                                                    .getJSONArray("sizes")
                                                    .getJSONObject(0)
                                                    .getJSONObject("value")
                                                    .getJSONObject("value")
                                                    .getInt("width");
                        mAPLViewPortHeight = template.getJSONObject("configuration")
                                                     .getJSONArray("sizes")
                                                     .getJSONObject(0)
                                                     .getJSONObject("value")
                                                     .getJSONObject("value")
                                                     .getInt("height");
                    }
                }
            }

            mAplHandler.buildAPLPresenter(mVisualConfig, mDefaultWindowId);

            setAPLFragmentLayout(mAPLLayout);

            mCurrentPayload = mCreationArgs.getString(Constants.PAYLOAD);

            String aplRuntimeProperties = constructAPLRuntimeProperties();
            if (!aplRuntimeProperties.isEmpty()) {
                mAplHandler.handleAPLRuntimeProperties(aplRuntimeProperties);
            }

            handleRenderDocumentIntent(mCreationArgs);
        } catch (JSONException e) {
            Log.w(TAG, "Failed to parse APL visual characteristics" + e);
        }
    }

    class APLDirectiveReceiver {
        @Subscribe
        public void OnReceive(APLDirective directive) {
            mCurrentPayload = directive.message.payload;
            switch (directive.message.action) {
                case Action.AlexaClient.DIALOG_STATE_CHANGED:
                    Preconditions.checkNotNull(directive.message.payload);
                    handleDialogStateChangedIntent(directive.message.payload);
                    break;
                case Action.APL.RENDER_DOCUMENT:
                    Preconditions.checkNotNull(directive.message.payload);
                    Bundle args = new Bundle();
                    args.putString(Constants.PAYLOAD, directive.message.payload);
                    handleRenderDocumentIntent(args);
                    break;
                case Action.APL.EXECUTE_COMMANDS:
                    Preconditions.checkNotNull(directive.message.payload);
                    handleExecuteCommandsIntent(directive.message.payload);
                    break;
                case Action.APL.CLEAR_DOCUMENT:
                    handleClearDocumentIntent(directive.message.payload);

                    Context context = getContext();
                    Preconditions.checkNotNull(context);

                    AlexaApp.from(context)
                            .getRootComponent()
                            .getComponent(SessionActivityController.class)
                            .ifPresent(SessionActivityController::removeFragment);
                    break;
                case Action.APL.UPDATE_APL_RUNTIME_PROPERTIES:
                    Preconditions.checkNotNull(directive.message.payload);
                    handleUpdateAPLRuntimePropertiesIntent(directive.message.payload);
                    break;
                default:
                    Log.d(TAG, "Unknown APL intent, action is " + directive.message.action);
                    break;
            }
        }
    }

    /**
     * Adjust APL fragment based on the APL Automotive viewport defined from the config, and set margins
     * based on device's screen size.
     * @param layout APL fragment layout
     */
    private void setAPLFragmentLayout(APLLayout layout) {
        requireActivity();

        ViewGroup.MarginLayoutParams marginLayoutParams = (ViewGroup.MarginLayoutParams) layout.getLayoutParams();

        DisplayMetrics displayMetrics = new DisplayMetrics();
        getActivity().getWindowManager().getDefaultDisplay().getMetrics(displayMetrics);
        int height = displayMetrics.heightPixels;
        int width = displayMetrics.widthPixels;

        marginLayoutParams.width = mAPLViewPortWidth;
        marginLayoutParams.height = mAPLViewPortHeight;
        marginLayoutParams.topMargin = (int) (height * APL_FRAGMENT_MARGIN_RATIO);
        marginLayoutParams.leftMargin = (int) (width * APL_FRAGMENT_MARGIN_RATIO);

        layout.setLayoutParams(marginLayoutParams);
    }

    /**
     * Construct APL runtime properties to render APL with updated properties value.
     * Sample APL runtime properties format: {"drivingState":"parked","theme":"dark-gray"}
     * @return APL runtime properties
     */
    private String constructAPLRuntimeProperties() {
        try {
            JSONObject properties = new JSONObject();

            SharedPreferences sharedPreferences = getContext().getSharedPreferences(APL_RUNTIME_PROPERTIES, 0);
            Preconditions.checkNotNull(sharedPreferences);
            String drivingStateValue = sharedPreferences.getString(APL_RUNTIME_PROPERTY_DRIVING_STATE_NAME, "");
            String themeValue = sharedPreferences.getString(APL_RUNTIME_PROPERTY_THEME_NAME, "");

            if (!drivingStateValue.isEmpty()) {
                properties.put(APL_RUNTIME_PROPERTY_DRIVING_STATE_NAME, drivingStateValue);
            }
            if (!themeValue.isEmpty()) {
                properties.put(APL_RUNTIME_PROPERTY_THEME_NAME, themeValue);
            }

            return properties.toString();
        } catch (JSONException e) {
            Log.d(TAG, "Failed to construct APL runtime properties");
            return "";
        }
    }
}
