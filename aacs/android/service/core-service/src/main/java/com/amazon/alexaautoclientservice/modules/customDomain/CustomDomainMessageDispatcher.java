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
package com.amazon.alexaautoclientservice.modules.customDomain;

import android.content.Context;
import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsconstants.AASBConstants;
import com.amazon.aacsconstants.Action;
import com.amazon.aacsipc.AACSSender;
import com.amazon.aacsipc.TargetComponent;
import com.amazon.alexaautoclientservice.ComponentRegistry;

import org.json.JSONException;
import org.json.JSONObject;
import org.json.JSONStringer;

import java.util.List;

public class CustomDomainMessageDispatcher {
    private static final String TAG = AACSConstants.AACS + "-" + CustomDomainMessageDispatcher.class.getSimpleName();
    private static final String CUSTOM_DOMAIN_INTENT_PREFIX = "com.amazon.aacs.customDomain.";

    private AACSSender mAACSSender;
    private Context mContext;

    public CustomDomainMessageDispatcher(@NonNull AACSSender aacsSender, @NonNull Context context) {
        mAACSSender = aacsSender;
        mContext = context;
    }

    public void handleCustomDomainMessage(@NonNull String aasbMessage, @NonNull String messageId, @NonNull String topic,
            @NonNull String action, @NonNull String payload) {
        JSONObject payloadJson;
        try {
            payloadJson = new JSONObject(payload);
        } catch (JSONException ex) {
            Log.e(TAG, "Exception occurred while parsing the payload: " + ex.getMessage());
            return;
        }

        switch (action) {
            case Action.CustomDomain.HANDLE_DIRECTIVE:
                dispatchHandleDirectiveMessage(aasbMessage, payloadJson);
                break;
            case Action.CustomDomain.CANCEL_DIRECTIVE:
                dispatchCancelDirectiveMessage(aasbMessage, payloadJson);
                break;
            case Action.CustomDomain.GET_CONTEXT:
                dispatchGetContextMessage(aasbMessage, payloadJson);
                break;
            default:
                Log.e(TAG, "Unexpected action received: " + action);
                break;
        }
    }

    private void dispatchHandleDirectiveMessage(String aasbMessage, JSONObject payload) {
        String directiveNamespace, directiveName;
        try {
            directiveNamespace = payload.getString(AASBConstants.CustomDomain.DIRECTIVE_NAMESPACE);
            directiveName = payload.getString(AASBConstants.CustomDomain.DIRECTIVE_NAME);

            Log.d(TAG,
                    String.format("dispatchHandleDirectiveMessage, namespace=%s, name=%s", directiveNamespace,
                            directiveName));

            List<TargetComponent> targets = ComponentRegistry.getInstance().findTargets(
                    mContext, directiveNamespace, directiveName, CUSTOM_DOMAIN_INTENT_PREFIX);
            dispatchMessage(aasbMessage, targets, CUSTOM_DOMAIN_INTENT_PREFIX + directiveName,
                    CUSTOM_DOMAIN_INTENT_PREFIX + directiveNamespace);
        } catch (JSONException ex) {
            Log.e(TAG, "Exception occurred while parsing the HandleDirective message payload: " + ex.getMessage());
        }
    }

    private void dispatchCancelDirectiveMessage(String aasbMessage, JSONObject payload) {
        String directiveNamespace;
        String directiveName;
        try {
            directiveNamespace = payload.getString(AASBConstants.CustomDomain.DIRECTIVE_NAMESPACE);
            directiveName = payload.getString(AASBConstants.CustomDomain.DIRECTIVE_NAME);

            Log.d(TAG,
                    String.format("dispatchCancelDirectiveMessage, namespace=%s, name=%s", directiveNamespace,
                            directiveName));

            List<TargetComponent> targets = ComponentRegistry.getInstance().findTargets(
                    mContext, directiveNamespace, Action.CustomDomain.CANCEL_DIRECTIVE, CUSTOM_DOMAIN_INTENT_PREFIX);
            dispatchMessage(aasbMessage, targets, CUSTOM_DOMAIN_INTENT_PREFIX + directiveName,
                    CUSTOM_DOMAIN_INTENT_PREFIX + directiveNamespace);
        } catch (JSONException ex) {
            Log.e(TAG, "Exception occurred while parsing the CancelDirective message payload: " + ex.getMessage());
        }
    }

    private void dispatchGetContextMessage(String aasbMessage, JSONObject payload) {
        String contextNamespace;
        try {
            contextNamespace = payload.getString(AASBConstants.CustomDomain.CONTEXT_NAMESPACE);

            Log.d(TAG, String.format("dispatchGetContextMessage, namespace=%s", contextNamespace));

            List<TargetComponent> targets = ComponentRegistry.getInstance().findTargets(
                    mContext, contextNamespace, Action.CustomDomain.GET_CONTEXT, CUSTOM_DOMAIN_INTENT_PREFIX);
            dispatchMessage(aasbMessage, targets, CUSTOM_DOMAIN_INTENT_PREFIX + Action.CustomDomain.GET_CONTEXT,
                    CUSTOM_DOMAIN_INTENT_PREFIX + contextNamespace);
        } catch (JSONException ex) {
            Log.e(TAG, "Exception occurred while parsing the GetContext message payload: " + ex.getMessage());
        }
    }

    private void dispatchMessage(
            String aasbMessage, List<TargetComponent> targets, String intentAction, String intentCategory) {
        if (targets != null && !targets.isEmpty()) {
            // Since the custom directive payload is opaque, use sendCustomDomainMessageAnySize() to send it with
            // arbitrary size
            mAACSSender.sendMessageAnySize(aasbMessage, intentAction, intentCategory, targets, mContext);
        }
    }
}
