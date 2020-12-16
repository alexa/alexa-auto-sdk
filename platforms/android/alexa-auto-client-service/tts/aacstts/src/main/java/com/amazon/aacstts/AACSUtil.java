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

package com.amazon.aacstts;

import static com.amazon.aacstts.TTSConstants.AACS_TTS_LOG_PREFIX;

import android.content.Context;
import android.util.Log;

import com.amazon.aacsconstants.AACSConstants;
import com.amazon.aacsipc.AACSPinger;
import com.amazon.aacsipc.AACSPinger.AACSPingResponse;
import com.amazon.aacstts.handler.AlexaClientHandler;

import java.util.Optional;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Future;

public class AACSUtil {
    private static final String TAG = AACS_TTS_LOG_PREFIX + AACSUtil.class.getSimpleName();

    /**
     * Ping AACS using the provided {@link AACSPinger} and wait for the ping response.
     * @param aacsPinger - AACSPinger used to perform ping
     * @return if AACS is in CONNECTED state
     */
    public static Boolean pingAACSToCheckIfConnected(AACSPinger aacsPinger) {
        AACSPingResponse response = null;
        try {
            Future<AACSPingResponse> fut = aacsPinger.pingAACS();
            response = fut.get();
        } catch (Exception e) {
            Log.e(TAG,
                    String.format(
                            "pingAACSToCheckIfConnected: failed to receive ping response. Exception encountered: %s",
                            e.getMessage()));
            return false;
        }
        return response.AACSState.equals("CONNECTED");
    }

    /**
     * Check if AACS is in CONNECTED state. It first checks if {@link AlexaClientHandler} has received the
     * ConnectionStatusChanged message with CONNECTED payload. If not, it checks if there is a ping completed
     * and if {@link AACSPingResponse} shows AACS is connected. If ping is not yet performed, it sends a ping
     * using the given {@link AACSPinger} and waits for {@link AACSPingResponse} in the executor thread.
     * @param isAlexaClientConnected - Optional Boolean indicating if AlexaClient is in CONNECTED status
     * @param isAACSRunningPingResponse - Future of Boolean indicating if the ping response is received and
     *                                  contains the CONNECTED metadata
     * @param executor - Executor used to submit the ping task
     * @param aacsPinger - AACSPinger used to perform ping
     * @return CompletableFuture indicating if the check is complete and if AACS is in CONNECTED status
     */
    public static CompletableFuture<Boolean> checkIfAACSIsConnected(Optional<Boolean> isAlexaClientConnected,
            CompletableFuture<Boolean> isAACSRunningPingResponse, ExecutorService executor, AACSPinger aacsPinger) {
        CompletableFuture<Boolean> result = new CompletableFuture<>();

        if (isAlexaClientConnected.isPresent()) {
            result.complete(isAlexaClientConnected.get());
            return result;
        }

        if (isAACSRunningPingResponse.isDone()) {
            try {
                result.complete(isAACSRunningPingResponse.get());
            } catch (InterruptedException | ExecutionException e) {
                Log.e(TAG,
                        String.format("failed to get isAACSRunningPingResponse because of exception encountered: %s. ",
                                e.getMessage()));
                result.complete(false);
            }
        } else {
            executor.submit(() -> {
                if (pingAACSToCheckIfConnected(aacsPinger)) {
                    isAACSRunningPingResponse.complete(true);
                    result.complete(true);
                } else {
                    isAACSRunningPingResponse.complete(false);
                    result.complete(false);
                }
            });
        }
        return result;
    }
}
