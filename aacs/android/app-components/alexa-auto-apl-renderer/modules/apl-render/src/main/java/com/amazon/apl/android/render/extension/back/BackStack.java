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
import androidx.annotation.Nullable;

import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

/**
 * This object contains {@link BackStackDocument} objects and methods to support the apl Backstack.
 *
 * APL-Spec https://developer.amazon.com/en-US/docs/alexa/alexa-presentation-language/apl-ext-backstack.html
 *
 * Note:
 * The top of the stack is {@link BackStack#length()} - 1.
 */
public class BackStack {
    private static final String TAG = BackStack.class.getSimpleName();

    private ArrayDeque<BackStackDocument> mDocuments;
    public BackStack() {
        mDocuments = new ArrayDeque<>();
    }

    /**
     * Adds a document to the BackStack.
     * @param backStackDocument the document to add.
     */
    void addDocument(@NonNull BackStackDocument backStackDocument) {
        Log.v(TAG, "addDocument: " + backStackDocument.hashCode());
        mDocuments.addLast(backStackDocument);
    }

    /**
     * @return the length of the backstack.
     */
    public int length() {
        Log.v(TAG, "length: " + mDocuments.size());
        return mDocuments.size();
    }

    /**
     * Clears the stack of documents.
     */
    public void clear() {
        Log.v(TAG, "clear: " + mDocuments.size());
        mDocuments.clear();
    }

    /**
     * @return the list of document ids in the backstack.
     */
    public List<String> getDocumentIds() {
        Log.v(TAG, "getDocumentIds: size: " + mDocuments.size());
        List<String> documentIds = new ArrayList<>();
        for (BackStackDocument backStackDocument : mDocuments) {
            documentIds.add(backStackDocument.getDocumentId());
        }
        return documentIds;
    }

    /**
     * Gets the index of the most recent document with the id documentId.
     *
     * Note: documents are stored in ascending-recency order. That is, the order ['A','B','C'] means
     * that 'C' is the most recent document.
     *
     * @param documentId the id to search for.
     * @return the index of the most recent document in the stack matching documentId,
     *      or -1 if not found.
     */
    int indexOf(@NonNull String documentId) {
        int index = mDocuments.size() - 1;
        for (Iterator<BackStackDocument> itr = mDocuments.descendingIterator(); itr.hasNext();) {
            if (documentId.equals(itr.next().getDocumentId())) {
                return index;
            }
            index--;
        }

        return -1;
    }

    /**
     * Removes all documents in the stack more recent than the most recent document with matching
     * documentId and removes and returns that document.
     *
     * For example, if the stack is ['A','B','B','C'], then popDocuments('B') would return the document
     * at index 2 and the stack would be ['A','B'].
     *
     * @param documentId the id of the document to return
     * @return the most recent document in the stack whose document id matches the parameter.
     */
    @Nullable
    BackStackDocument popDocuments(@NonNull String documentId) {
        Log.v(TAG, "popDocuments id: " + documentId);
        int indexOfDocument = indexOf(documentId);
        if (indexOfDocument == -1) {
            return null;
        }
        return popDocumentsAtIndex(indexOfDocument);
    }

    /**
     * Removes all documents more recent than and including index and returns the document at index.
     *
     * For example, if the stack has ['A','B','C'], then both popDocumentsAtIndex(0) and
     * popDocumentsAtIndex(-3) would return 'A' and the stack would be [].
     *
     * @param index the index of the document to return (can be negative to count backwards)
     * @return the document at index.
     */
    @Nullable
    BackStackDocument popDocumentsAtIndex(int index) {
        Log.v(TAG, "popDocuments index: " + index);
        // Convert negative indexes to positive
        if (index < 0) {
            index = index + mDocuments.size();
        }

        if (index < 0 || index >= mDocuments.size()) {
            return null;
        }

        return popDocuments(mDocuments.size() - index);
    }

    /**
     * Removes count documents from the stack and returns the last one removed.
     *
     * For example, if the stack has documents ['A', 'B', 'C'], then popDocuments(2) would return 'B',
     * and the stack would be: ['A'].
     *
     * @param count the number of documents to remove
     * @return the count document in the stack
     */
    @Nullable
    BackStackDocument popDocuments(int count) {
        Log.v(TAG, "popDocuments count: " + count + " size: " + mDocuments.size());
        if (count < 0 || count > mDocuments.size() || mDocuments.size() == 0) {
            return null;
        }

        for (int i = 0; i < count - 1; i++) {
            mDocuments.removeLast();
        }

        return mDocuments.removeLast();
    }
}
