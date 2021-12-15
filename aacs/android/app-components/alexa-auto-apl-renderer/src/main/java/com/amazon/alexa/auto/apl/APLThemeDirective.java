package com.amazon.alexa.auto.apl;

import androidx.annotation.NonNull;

public class APLThemeDirective {
    @NonNull
    public String themePayload;

    public APLThemeDirective(@NonNull String themePayload) {
        this.themePayload = themePayload;
    }
}
