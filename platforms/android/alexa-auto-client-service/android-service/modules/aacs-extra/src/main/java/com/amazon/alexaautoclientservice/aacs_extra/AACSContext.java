package com.amazon.alexaautoclientservice.aacs_extra;

import android.content.Context;

public interface AACSContext {
    String CERTS_DIR = "certsDir";
    String MODEL_DIR = "modelsDir";
    String PRODUCT_DSN = "productDsn";
    String APPDATA_DIR = "appDataDir";
    String JSON = "json";
    Context getContext();
    String getData(String key);
}
