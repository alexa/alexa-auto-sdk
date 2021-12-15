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

import android.util.Log;

import androidx.annotation.NonNull;

import com.amazon.apl.android.DocumentState;

import java.util.HashMap;
import java.util.Map;
import java.util.Objects;

/**
 * Maintains a document with a specified ID in the backstack.
 */
public class BackStackDocument {
    private static final String TAG = BackStackDocument.class.getSimpleName();

    /**
     * The document's backstack id.
     */
    @NonNull
    private final String mDocumentId;

    /**
     * The document's cached state.
     */
    @NonNull
    private final DocumentState mDocumentState;

    /**
     * Map of metadata that can be attached to this document. Metadata
     * could be anything that the application can use to restore the
     * document.
     */
    @NonNull
    private final Map<String, Object> mDocumentExtras = new HashMap<>(8);

    /**
     * Creates a BackStack document to add to the backstack.
     * @param documentId    the backstackId
     * @param documentState the document's cached state
     */
    public BackStackDocument(@NonNull final String documentId, @NonNull final DocumentState documentState) {
        mDocumentId = documentId;
        mDocumentState = documentState;
    }

    /**
     * Return the document id.
     * @return the document's backstack id.
     */
    @NonNull
    public String getDocumentId() {
        return mDocumentId;
    }

    /**
     * Return the cached document.
     * @return the document's cached state.
     */
    @NonNull
    public DocumentState getDocumentState() {
        return mDocumentState;
    }

    /**
     * Return metadata associated with this document using a key.
     * @param key the key
     * @return An extra attached with this document.
     */
    @SuppressWarnings("unchecked")
    @NonNull
    public <T> T getExtra(@NonNull String key, @NonNull T fallbackValue) {
        try {
            return Objects.requireNonNull((T) mDocumentExtras.get(key));
        } catch (ClassCastException e) {
            Log.w(TAG, "Type mismatch for key: " + key, e);
        } catch (NullPointerException e) {
            Log.i(TAG, "Key not found: " + key);
        }
        return fallbackValue;
    }

    /**
     * Check if a document extra exists.
     * @param key the key
     * @return true if the document extra was added.
     */
    public boolean hasExtra(@NonNull String key) {
        return mDocumentExtras.containsKey(key);
    }

    /**
     * Attach additional information with this document.
     * @param key   the key
     * @param extra additional data
     * @return this for chaining
     */
    public <T> BackStackDocument putExtra(@NonNull String key, @NonNull T extra) {
        mDocumentExtras.put(key, extra);
        return this;
    }
}
