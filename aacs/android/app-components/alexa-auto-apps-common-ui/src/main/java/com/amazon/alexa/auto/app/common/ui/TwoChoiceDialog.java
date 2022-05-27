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
import org.w3c.dom.Text;

/**
 * Dialog class to present confirmation screen with two choices.
 */
public class TwoChoiceDialog extends DialogFragment {
    @VisibleForTesting
    TextView mBtn1;

    @VisibleForTesting
    TextView mBtn2;

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setStyle(DialogFragment.STYLE_NO_TITLE, R.style.Alexa_DialogStyle);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        return inflater.inflate(R.layout.two_choice_dialog_layout, container, false);
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

        mBtn1 = view.findViewById(R.id.btn1);
        mBtn1.setText(params.button1Text);

        mBtn2 = view.findViewById(R.id.btn2);
        mBtn2.setText(params.button2Text);

        mBtn1.setOnClickListener(btn1View -> {
            EventBus.getDefault().post(new Button1Clicked(params.dialogKey));
            dismiss();
        });

        mBtn2.setOnClickListener(btn2View -> {
            EventBus.getDefault().post(new Button2Clicked(params.dialogKey));
            dismiss();
        });
    }

    /**
     * Creates an instance of {@link TwoChoiceDialog} with required parameters.
     *
     * @param params Parameters needed to construct the dialog object.
     * @return an instance of {@link TwoChoiceDialog}.
     */
    public static TwoChoiceDialog createDialog(Params params) {
        TwoChoiceDialog dialog = new TwoChoiceDialog();
        Bundle args = new Bundle();
        args.putParcelable("args", params);
        dialog.setArguments(args);

        return dialog;
    }

    // Note the choice selection events are raised through event bus so that
    // events could be delivered even after device is rotated.

    /**
     * Event bus event that is raised on click of button 1.
     */
    public static class Button1Clicked {
        @NonNull
        public final String dialogKey;

        public Button1Clicked(@NonNull String dialogKey) {
            this.dialogKey = dialogKey;
        }
    }

    /**
     * Event bus event that is raised on click of button 2.
     */
    public static class Button2Clicked {
        @NonNull
        public final String dialogKey;

        public Button2Clicked(@NonNull String dialogKey) {
            this.dialogKey = dialogKey;
        }
    }

    /**
     * Class to encapsulate parameters required for building the dialog object.
     */
    public static class Params implements Parcelable {
        @NonNull
        public final String dialogKey;
        @NonNull
        public final String title;
        @NonNull
        public final String content;
        @NonNull
        public final String button1Text;
        @NonNull
        public final String button2Text;

        /**
         * Construct the Parameters required for this dialog.
         *
         * @param dialogKey Key for the dialog that is delivered to listener along with
         *        following events raised from dialog: {@link Button1Clicked} and
         *        {@link Button2Clicked}.
         * @param title Title of the dialog.
         * @param content Content to be displayed in the dialog.
         * @param button1Text Text for first button.
         * @param button2Text Text for second button.
         */
        public Params(@NonNull String dialogKey, @NonNull String title, @NonNull String content,
                @NonNull String button1Text, @NonNull String button2Text) {
            this.title = title;
            this.content = content;
            this.button1Text = button1Text;
            this.button2Text = button2Text;
            this.dialogKey = dialogKey;
        }

        public Params(Parcel in) {
            this.dialogKey = in.readString();
            this.title = in.readString();
            this.content = in.readString();
            this.button1Text = in.readString();
            this.button2Text = in.readString();
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
            parcel.writeString(dialogKey);
            parcel.writeString(title);
            parcel.writeString(content);
            parcel.writeString(button1Text);
            parcel.writeString(button2Text);
        }
    }
}
