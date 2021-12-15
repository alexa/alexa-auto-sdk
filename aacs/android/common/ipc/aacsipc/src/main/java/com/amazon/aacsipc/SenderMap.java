/*
 * Copyright 2020 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

package com.amazon.aacsipc;

import android.util.Log;

import java.util.HashMap;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.LinkedBlockingDeque;

class SenderMap {
    private static final String TAG = "AACS-" + SenderMap.class.getSimpleName();
    private HashMap<Integer, ResourceBundle> mMap;
    private LinkedBlockingDeque<Integer> mQueue;
    private int mResourceId;
    private int mCapacity;

    SenderMap(int capacity) {
        mMap = new HashMap<>(capacity);
        mQueue = new LinkedBlockingDeque<>(capacity);
        mResourceId = 0;
        mCapacity = capacity;
        Log.i(TAG, "IPC: sender map created with capacity " + capacity);
    }

    synchronized int put(String message, CompletableFuture<Boolean> future, int targetCount) {
        ResourceBundle resource = new ResourceBundle(mResourceId, message, future, targetCount);
        if (!mQueue.isEmpty() && mQueue.size() == mCapacity) {
            int evictedId = mQueue.poll();
            ResourceBundle evictedResource = mMap.get(evictedId);
            if (evictedResource != null) {
                Log.w(TAG,
                        "IPC: Cache capacity reached. Evict the least recently used resource: "
                                + "| ResourceId = " + evictedResource.mResourceId
                                + "\n| Message = " + evictedResource.getMessage()
                                + "\n| Future isDone = " + evictedResource.getFuture().isDone());
                mMap.remove(evictedId);
            } else {
                Log.e(TAG, "IPC: Trying to evict an invalid resource from cache");
            }
        }
        mMap.put(mResourceId, resource);
        mQueue.offer(mResourceId);

        // Increment resource id
        mResourceId += 1;
        return mResourceId - 1;
    }

    synchronized void remove(int resourceId) {
        ResourceBundle resource = mMap.remove(resourceId);
        if (resource != null) {
            mQueue.remove(resourceId);
            Log.i(TAG,
                    "IPC: Removed ResourceId " + resourceId + " | Message = " + resource.getMessage()
                            + " | Future isDone = " + resource.getFuture().isDone());
        } else {
            Log.e(TAG, "IPC: ResourceId not found in Message Hash Map");
        }
    }

    synchronized ResourceBundle get(int resourceId) {
        if (mMap.containsKey(resourceId)) {
            moveToTail(resourceId);
            return mMap.get(resourceId);
        } else {
            Log.e(TAG, "IPC: ResourceId not found in Message Hash Map");
        }

        return null;
    }

    synchronized void decrementCount(int resourceId) {
        ResourceBundle resource = mMap.get(resourceId);
        if (resource == null) {
            Log.w(TAG, "IPC: ResourceId not found in Message Hash Map.");
            return;
        }
        if (resource.getTargetCount() > 0) {
            resource.decrementCount();
        } else {
            Log.e(TAG, "IPC: cannot decrement 0 target count");
        }
    }

    private synchronized void moveToTail(int resourceId) {
        if (mQueue.remove(resourceId)) {
            mQueue.offer(resourceId);
        } else {
            Log.e(TAG, "IPC: ResourceId not found in queue");
        }
    }

    public static class ResourceBundle {
        int mResourceId;
        String mMessage;
        CompletableFuture<Boolean> mFuture;
        int mTargetCount;

        ResourceBundle(int resourceId, String message, CompletableFuture<Boolean> future, int targetCount) {
            mResourceId = resourceId;
            mMessage = message;
            mFuture = future;
            mTargetCount = targetCount;
        }

        synchronized String getMessage() {
            return mMessage;
        }

        synchronized CompletableFuture<Boolean> getFuture() {
            return mFuture;
        }

        synchronized int getTargetCount() {
            return mTargetCount;
        }

        synchronized void decrementCount() {
            mTargetCount -= 1;
            Log.v(TAG, "IPC: decrement target count by 1. Current value: " + mTargetCount);
        }
    }
}
