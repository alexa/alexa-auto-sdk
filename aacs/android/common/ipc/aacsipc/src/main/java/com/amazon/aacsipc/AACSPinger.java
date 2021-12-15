package com.amazon.aacsipc;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.Log;

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;
import java.util.concurrent.TimeUnit;

public class AACSPinger {
    private static final String TAG = "AACS-" + AACSPinger.class.getSimpleName();
    private ExecutorService mExecutorService;

    public AACSPinger(Context context, String androidPermission) {
        mBroadcastReceiver = new ClientPingReceiver();

        IntentFilter filter = new IntentFilter();
        filter.addAction("com.amazon.aacs.pingreply");
        mContext = context;
        mExecutorService = Executors.newSingleThreadExecutor();
        mContext.registerReceiver(mBroadcastReceiver, filter, androidPermission, null);
    }

    public AACSPinger(Context context) {
        this(context, null);
    }

    private Context mContext;
    private ClientPingReceiver mBroadcastReceiver;
    private int pingId = 0;

    private class ClientPingReceiver extends BroadcastReceiver {
        private AACSPingResponse lastPingResponse;

        @Override
        public void onReceive(Context context, Intent intent) {
            AACSPingResponse response = new AACSPingResponse();
            response.timestamp = System.currentTimeMillis();
            if ("com.amazon.aacs.pingreply".equals(intent.getAction())) {
                response.pingId = pingId;
                response.hasResponse = true;
                Log.d(TAG, "Ping reply received for ping transaction #" + pingId);
                if (intent.hasExtra("state")) {
                    response.AACSState = intent.getStringExtra("state");
                }
                lastPingResponse = response;
            }
        }
    }

    public class AACSPingResponse {
        public String AACSState;
        public boolean hasResponse = false;
        public long timestamp;
        public long responseTimeInMS;
        public int pingId;
    }

    public class PingTask implements Callable<AACSPingResponse> {
        private long timeoutInMS;

        public PingTask(long timeoutInMS) {
            this.timeoutInMS = timeoutInMS;
        }

        public PingTask() {
            this(DEFAULT_PING_TIMEOUT_MS);
        }

        @Override
        public AACSPingResponse call() throws Exception {
            Intent intent = new Intent();
            intent.setAction("com.amazon.aacs.ping");
            intent.addCategory("com.amazon.aacs.pingtopic");
            intent.putExtra("replyType", "RECEIVER");
            long startTime = System.currentTimeMillis();
            mContext.sendBroadcast(intent, "com.amazon.alexaautoclientservice.ping");

            long expireTime = startTime + this.timeoutInMS;
            while (System.currentTimeMillis() < expireTime) {
                AACSPingResponse lastResponse = mBroadcastReceiver.lastPingResponse;
                if (lastResponse != null && lastResponse.pingId == pingId) {
                    lastResponse.responseTimeInMS = lastResponse.timestamp - startTime;
                    Log.i(TAG,
                            "Ping #" + pingId + " response was received in " + lastResponse.responseTimeInMS + "ms.");
                    return lastResponse;
                }
                try {
                    TimeUnit.MILLISECONDS.sleep(10);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            }
            Log.w(TAG, "No ping response received for ping id " + pingId);
            return new AACSPingResponse();
        }
    }

    private static final int DEFAULT_PING_TIMEOUT_MS = 1000;
    public Future<AACSPingResponse> pingAACS() {
        return pingAACS(DEFAULT_PING_TIMEOUT_MS);
    }

    public Future<AACSPingResponse> pingAACS(long timeoutInMS) {
        pingId++;
        Log.i(TAG, "Sending ping #" + pingId);
        if (!mExecutorService.isShutdown()) {
            return mExecutorService.submit(new PingTask(timeoutInMS));
        }
        Log.w(TAG, "Cannot send ping request. AACSPinger has already been shutdown!");
        return null;
    }

    public void shutdown() {
        Context context = mContext;
        if (context != null && mBroadcastReceiver != null) {
            context.unregisterReceiver(mBroadcastReceiver);
        }
        mExecutorService.shutdown();
        mContext = null;
    }
}
