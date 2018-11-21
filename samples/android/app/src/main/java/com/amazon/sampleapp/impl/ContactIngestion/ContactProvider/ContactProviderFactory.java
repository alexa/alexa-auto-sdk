package com.amazon.sampleapp.impl.ContactIngestion.ContactProvider;

import android.app.Activity;

import com.amazon.sampleapp.impl.ContactIngestion.ContactInputSourceType;
import com.amazon.sampleapp.impl.Logger.LoggerHandler;

/**
 * A factory class to provide a contact provider.
 */
public class ContactProviderFactory {

    /**
     * Factory method to return a singleton instance of a {@link ContactProvider}.
     * @param activity the {@link com.amazon.sampleapp.MainActivity}
     * @param logger the {@link LoggerHandler}
     * @param contactInputSourceType the {@link ContactInputSourceType}
     * @return a singleton instance of a {@link ContactProvider}
     */
    public static final ContactProvider generateContactProvider(final Activity activity,
                                                                final LoggerHandler logger,
                                                                final ContactInputSourceType contactInputSourceType) {
        ContactProvider contactProvider = null;
        switch (contactInputSourceType) {
                case FILE :
                    contactProvider = new FileContactProviderHandler(activity, logger);
                    break;
            }
        return contactProvider;
    }
}
