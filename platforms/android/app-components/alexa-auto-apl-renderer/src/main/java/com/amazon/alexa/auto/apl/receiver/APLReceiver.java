package com.amazon.alexa.auto.apl.receiver;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.util.Log;

import androidx.annotation.NonNull;
import androidx.annotation.RequiresApi;

import com.amazon.aacsconstants.Action;
import com.amazon.alexa.auto.aacs.common.AACSMessageBuilder;
import com.amazon.alexa.auto.apl.APLActivity;
import com.amazon.alexa.auto.apl.APLDirective;
import com.amazon.alexa.auto.apl.Constants;

import org.greenrobot.eventbus.EventBus;

public class APLReceiver extends BroadcastReceiver {
    private static final String TAG = APLReceiver.class.getSimpleName();

    @RequiresApi(api = Build.VERSION_CODES.N)
    @Override
    public void onReceive(Context context, Intent intent) {
        if (intent == null || intent.getAction() == null) {
            return;
        }

        AACSMessageBuilder.parseEmbeddedIntent(intent).ifPresent(message -> {
            Log.d(TAG, "APL msg.action " + message.action + " payload: " + message.payload);
            if (Action.APL.RENDER_DOCUMENT.equals(message.action)) {
                handleRenderDocument(context, message.payload);
            } else {
                APLDirective directive = new APLDirective(message);
                EventBus.getDefault().post(directive);
            }
        });
    }

    private void handleRenderDocument(@NonNull Context context, @NonNull final String payload) {
        Intent intent = new Intent(context, APLActivity.class);
        intent.putExtra(Constants.PAYLOAD, payload);
        intent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        context.startActivity(intent);
    }
}
