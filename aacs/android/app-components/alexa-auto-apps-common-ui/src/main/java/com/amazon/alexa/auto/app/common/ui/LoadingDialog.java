/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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
package com.amazon.alexa.auto.app.common.ui;

import android.app.Dialog;
import android.graphics.drawable.ColorDrawable;
import android.os.Bundle;
import android.os.Parcel;
import android.os.Parcelable;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.Window;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.annotation.VisibleForTesting;
import androidx.fragment.app.DialogFragment;

import com.amazon.alexa.auto.apps.common.ui.R;
import com.amazon.alexa.auto.apps.common.util.Preconditions;

import org.greenrobot.eventbus.EventBus;

/**
 * Dialog class to present loading screen.
 */
public class LoadingDialog extends DialogFragment {
    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setStyle(DialogFragment.STYLE_NO_TITLE, R.style.Alexa_DialogStyle);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.loading_dialog_layout, container, false);
    }

    @Override
    public void onResume() {
        super.onResume();
        Dialog dialog = getDialog();
        if (dialog != null) {
            Window window = dialog.getWindow();
            if (window != null) {
                window.setBackgroundDrawable(new ColorDrawable(android.graphics.Color.TRANSPARENT));
            }
        }
    }

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);

        Bundle args = getArguments();
        Preconditions.checkNotNull(args);

        Params params = args.getParcelable("args");
        Preconditions.checkNotNull(params);

        TextView headerTxt = view.findViewById(R.id.header_text);
        headerTxt.setText(params.title);

        TextView contentTxt = view.findViewById(R.id.content_text);
        contentTxt.setText(params.content);
    }

    /**
     * Creates an instance of {@link LoadingDialog} with required parameters.
     *
     * @param params Parameters needed to construct the dialog object.
     * @return an instance of {@link LoadingDialog}.
     */
    public static LoadingDialog createDialog(Params params) {
        LoadingDialog dialog = new LoadingDialog();
        Bundle args = new Bundle();
        args.putParcelable("args", params);
        dialog.setArguments(args);

        return dialog;
    }

    /**
     * Class to encapsulate parameters required for building the dialog object.
     */
    public static class Params implements Parcelable {
        @NonNull
        public final String title;
        @NonNull
        public final String content;

        /**
         * Construct the Parameters required for this dialog.
         *
         * @param title Title of the dialog.
         * @param content Content to be displayed in the dialog.
         */
        public Params(@NonNull String title, @NonNull String content) {
            this.title = title;
            this.content = content;
        }

        public Params(Parcel in) {
            this.title = in.readString();
            this.content = in.readString();
        }

        public static final Creator<Params> CREATOR = new Creator<Params>() {
            @Override
            public Params createFromParcel(Parcel in) {
                return new Params(in);
            }

            @Override
            public Params[] newArray(int size) {
                return new Params[size];
            }
        };

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel parcel, int i) {
            parcel.writeString(title);
            parcel.writeString(content);
        }
    }
}
