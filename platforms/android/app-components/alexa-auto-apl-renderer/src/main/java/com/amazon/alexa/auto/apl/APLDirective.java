package com.amazon.alexa.auto.apl;

import androidx.annotation.NonNull;

import com.amazon.alexa.auto.aacs.common.AACSMessage;

public class APLDirective {
    @NonNull
    public AACSMessage message;

    public APLDirective(@NonNull AACSMessage message) {
        this.message = message;
    }
}
