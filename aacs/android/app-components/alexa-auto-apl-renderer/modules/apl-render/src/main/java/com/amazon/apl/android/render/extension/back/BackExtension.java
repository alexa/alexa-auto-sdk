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

package com.amazon.apl.android.render.extension.back;

import android.text.TextUtils;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.amazon.apl.android.ExtensionCommandDefinition;
import com.amazon.apl.android.ExtensionEventHandler;
import com.amazon.apl.android.LegacyLocalExtension;
import com.amazon.apl.android.dependencies.IExtensionEventCallback;
import com.amazon.apl.android.dependencies.IOnAplFinishCallback;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * Implementation of the APL BackStack extension. Allows APL documents to navigate back to previously
 * rendered APL documents.
 *
 * APL-Spec https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-ext-backstack.html
 */
public class BackExtension extends LegacyLocalExtension {
    private static final String TAG = BackExtension.class.getSimpleName();

    public static final String URI = "aplext:backstack:10";
    public static final String COMMAND_GO_BACK_NAME = "GoBack";
    public static final String COMMAND_CLEAR_NAME = "Clear";
    public static final String SETTINGS_PROPERTY_BACKSTACK_ID = "backstackId";
    public static final String PROPERTY_BACK_TYPE = "backType";
    public static final String PROPERTY_BACK_VALUE = "backValue";

    private static final List<ExtensionCommandDefinition> COMMANDS = new ArrayList<>();

    //-------------------------------------------------------------------------
    // Supported commands
    //-------------------------------------------------------------------------
    static {
        COMMANDS.add(new ExtensionCommandDefinition(URI, COMMAND_GO_BACK_NAME)
                             .allowFastMode(true)
                             .property(PROPERTY_BACK_TYPE, BackType.COUNT.toString(), false)
                             .property(PROPERTY_BACK_VALUE, 1, false));
        COMMANDS.add(new ExtensionCommandDefinition(URI, COMMAND_CLEAR_NAME).allowFastMode(true));
    }

    @NonNull
    private final BackStack mBackStack;
    @NonNull
    private final IBackCallback mBackCallback;
    @NonNull
    private final IOnAplFinishCallback mOnFinishCallback;
    @Nullable
    private BackStackDocument mBackstackDocument;
    @Nullable
    private IExtensionEventCallbackResult mResultCallback;

    private boolean mResponsibleForBackButton;
    private String mCurrentDocumentId;

    /**
     * Instantiate a new BackExtension.
     * @param backStack             the backstack to use
     * @param callback              the callback for handling GoBack commands
     * @param onAplFinishCallback   the callback for handling GoBack when the stack is empty
     */
    public BackExtension(@NonNull BackStack backStack, @NonNull IBackCallback callback,
            @NonNull IOnAplFinishCallback onAplFinishCallback) {
        mBackStack = backStack;
        mBackCallback = callback;
        mOnFinishCallback = onAplFinishCallback;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    @NonNull
    public String getUri() {
        return URI;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public Object getEnvironment() {
        Map<String, Object> envObject = new HashMap<>();
        envObject.put("responsibleForBackButton", mResponsibleForBackButton);
        envObject.put("backstack", mBackStack.getDocumentIds());
        Log.i(TAG, "getEnvironment");
        return envObject;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    @NonNull
    public List<ExtensionCommandDefinition> getCommandDefinitions() {
        Log.i(TAG, "getCommandDefinitions");
        return COMMANDS;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    @NonNull
    public List<ExtensionEventHandler> getEventHandlers() {
        Log.i(TAG, "getEventHandlers");
        return Collections.emptyList();
    }

    /**
     * {@inheritDoc}
     */
    @Override
    @NonNull
    public IExtensionEventCallback getCallback() {
        return this;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void applySettings(Map<String, Object> settings) {
        Object backStackId = settings.get(SETTINGS_PROPERTY_BACKSTACK_ID);
        if (backStackId instanceof String) {
            mCurrentDocumentId = (String) backStackId;
        }
        Log.i(TAG, "applySettings: " + mCurrentDocumentId);
    }

    /**
     * Returns the current document id.
     * @return
     */
    public String getDocumentId() {
        return mCurrentDocumentId;
    }

    /**
     * Set the document id.
     * @param id
     */
    public void setDocumentId(String id) {
        mCurrentDocumentId = id;
    }

    /**
     * {@inheritDoc}
     */
    @Override
    public void onExtensionEvent(String name, String uri, Map<String, Object> source, Map<String, Object> custom,
            IExtensionEventCallbackResult resultCallback) {
        mBackstackDocument = null;
        mResultCallback = resultCallback;
        Log.v(TAG, "onExtensionEvent: " + name + " source: " + source);
        switch (name) {
            case COMMAND_GO_BACK_NAME:
                goBack(custom.get(PROPERTY_BACK_TYPE), custom.get(PROPERTY_BACK_VALUE));
                break;
            case COMMAND_CLEAR_NAME:
                eventHandled(true);
                clear();
                break;
            default:
                eventHandled(false);
        }
    }

    /**
     * Set the value of the Backstack environment property responsibleForBackButton.
     * @param isResponsible true if the document needs to display the back button
     */
    public void setResponsibleForBackButton(boolean isResponsible) {
        mResponsibleForBackButton = isResponsible;
    }

    /**
     * Return the current back stack document.
     * @return back stack document.
     */
    public BackStackDocument getBackstackDocument() {
        return mBackstackDocument;
    }

    /**
     * Add a document to the back stack.
     * @param backStackDocument the document to be added to the back stack.
     */
    public void addDocument(BackStackDocument backStackDocument) {
        mBackStack.addDocument(backStackDocument);
    }

    /**
     * Go back using document id.
     * @param documentId The id
     */
    public void goBack(String documentId) {
        if (TextUtils.isEmpty(documentId)) {
            mBackstackDocument = mBackStack.popDocuments(0);
        } else {
            mBackstackDocument = mBackStack.popDocuments(documentId);
        }
        triggerGoBack();
    }

    /**
     * Go back using index.
     * @param index The index to go back on the stack.
     */
    public void goBackIndex(int index) {
        mBackstackDocument = mBackStack.popDocumentsAtIndex(index);
        triggerGoBack();
    }

    /**
     * Go back using count.
     * @param count The number of elements that shoupd be popped off the stack.
     */
    public void goBackCount(int count) {
        mBackstackDocument = mBackStack.popDocuments(count);
        triggerGoBack();
    }

    /**
     * Clear the back stack.
     */
    public void clear() {
        mBackStack.clear();
    }

    /**
     * Handle the go back command.
     * @param objectBackType Back type: count, index, id
     * @param backValue
     */
    private void goBack(Object objectBackType, Object backValue) {
        try {
            int intBack = 0;
            if (backValue instanceof Double) {
                intBack = ((Double) backValue).intValue();
            } else if (backValue instanceof Integer) {
                intBack = (Integer) backValue;
            }
            BackType backType = BackType.valueOf(objectBackType);
            switch (backType) {
                case ID:
                    goBack((String) backValue);
                    break;
                case INDEX:
                    goBackIndex(intBack);
                    break;
                case COUNT:
                    goBackCount(intBack);
                    break;
            }
        } catch (IllegalArgumentException | NullPointerException | ClassCastException e) {
            Log.e(TAG, "Invalid keys in document. Doing nothing.", e);
        }
    }

    /**
     * Notified callback of the document to go back to or finish if last element was popped off.
     */
    private void triggerGoBack() {
        eventHandled(true);
        if (mBackstackDocument != null) {
            mBackCallback.goBack(mBackstackDocument);
        } else if (mBackStack.length() == 0) {
            // Finish if we have no documents in the stack
            mOnFinishCallback.onAplFinish();
        }
    }

    /**
     * Call back to signal if event was handled.
     * @param handled
     */
    private void eventHandled(boolean handled) {
        if (mResultCallback != null) {
            mResultCallback.onResult(handled);
        }
        mResultCallback = null;
    }

    /**
     * Supported back types.
     */
    enum BackType {
        COUNT,
        INDEX,
        ID;

        public static BackType valueOf(Object value) {
            if (value instanceof BackType) {
                return (BackType) value;
            }

            String name = ((String) value).toUpperCase();
            return BackType.valueOf(name);
        }
    }
}
