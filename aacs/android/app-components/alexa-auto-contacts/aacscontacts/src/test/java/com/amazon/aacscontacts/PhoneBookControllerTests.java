package com.amazon.aacscontacts;

import static org.mockito.Matchers.eq;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.provider.ContactsContract;

import com.amazon.aacsconstants.Action;
import com.amazon.aacsconstants.Topic;
import com.amazon.alexa.auto.aacs.common.AACSMessageSender;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.modules.junit4.PowerMockRunner;

/**
 * Unit tests for {@link PhoneBookController}
 */
@RunWith(PowerMockRunner.class)
public class PhoneBookControllerTests {
    private final String TEST_ADDRESS_BOOK_SOURCE_ID = "74:B5:87:33:79:98";
    private final String TEST_ADDRESS_BOOK_NAME = "TestAddressBookName";
    private PhoneBookController mPhoneBookController;

    @Mock
    private AACSMessageSender mAACSMessageSender;

    @Mock
    private Context mMockedContext;

    @Mock
    private Cursor mMockedNameCursor;

    @Mock
    private Cursor mMockedNickCursor;

    @Mock
    private Cursor mMockedPhoneCursor;

    @Mock
    private ContentResolver mMockedContentResolver;

    @Before
    public void setUp() {
        mPhoneBookController = new PhoneBookController(mMockedContext, mAACSMessageSender);
    }

    @Test
    public void upload_valid_AddressBook() {
        PowerMockito.when(mMockedContext.getContentResolver()).thenReturn(mMockedContentResolver);
        Assert.assertNotNull(mMockedContentResolver);
        PowerMockito
                .when(mMockedContentResolver.query(ContactsContract.Data.CONTENT_URI, null,
                        ContactsContract.Data.MIMETYPE + " = ?",
                        new String[] {ContactsContract.CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE}, null))
                .thenReturn(mMockedNameCursor);
        PowerMockito
                .when(mMockedContentResolver.query(ContactsContract.Data.CONTENT_URI, null,
                        ContactsContract.Data.MIMETYPE + " = ?",
                        new String[] {ContactsContract.CommonDataKinds.Nickname.CONTENT_ITEM_TYPE}, null))
                .thenReturn(mMockedNickCursor);
        PowerMockito
                .when(mMockedContentResolver.query(eq(ContactsContract.CommonDataKinds.Phone.CONTENT_URI), eq(null),
                        eq(ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = ?"),
                        new String[] {Mockito.anyString()}, eq(null)))
                .thenReturn(mMockedPhoneCursor);
        PowerMockito.when(mMockedNameCursor.getCount()).thenReturn(1);
        PowerMockito
                .when(mMockedNameCursor.getString(
                        mMockedNameCursor.getColumnIndex(ContactsContract.RawContacts.ACCOUNT_NAME)))
                .thenReturn(TEST_ADDRESS_BOOK_SOURCE_ID);
        String messageId = mPhoneBookController.addAddressBook(TEST_ADDRESS_BOOK_SOURCE_ID, TEST_ADDRESS_BOOK_NAME);
        Assert.assertNotEquals("", messageId);
        Mockito.verify(mAACSMessageSender, Mockito.times(1))
                .sendMessageReturnID(
                        eq(Topic.ADDRESS_BOOK), eq(Action.AddressBook.ADD_ADDRESS_BOOK), Mockito.anyString());
    }

    @Test
    public void upload_empty_AddressBook() {
        PowerMockito.when(mMockedContext.getContentResolver()).thenReturn(mMockedContentResolver);
        Assert.assertNotNull(mMockedContentResolver);
        PowerMockito
                .when(mMockedContentResolver.query(ContactsContract.Data.CONTENT_URI, null,
                        ContactsContract.Data.MIMETYPE + " = ?",
                        new String[] {ContactsContract.CommonDataKinds.StructuredName.CONTENT_ITEM_TYPE}, null))
                .thenReturn(mMockedNameCursor);
        PowerMockito
                .when(mMockedContentResolver.query(ContactsContract.Data.CONTENT_URI, null,
                        ContactsContract.Data.MIMETYPE + " = ?",
                        new String[] {ContactsContract.CommonDataKinds.Nickname.CONTENT_ITEM_TYPE}, null))
                .thenReturn(mMockedNickCursor);
        PowerMockito
                .when(mMockedContentResolver.query(eq(ContactsContract.CommonDataKinds.Phone.CONTENT_URI), eq(null),
                        eq(ContactsContract.CommonDataKinds.Phone.CONTACT_ID + " = ?"),
                        new String[] {Mockito.anyString()}, eq(null)))
                .thenReturn(mMockedPhoneCursor);
        PowerMockito.when(mMockedNameCursor.getCount()).thenReturn(0);
        PowerMockito
                .when(mMockedNameCursor.getString(
                        mMockedNameCursor.getColumnIndex(ContactsContract.RawContacts.ACCOUNT_NAME)))
                .thenReturn(TEST_ADDRESS_BOOK_SOURCE_ID);
        String messageId = mPhoneBookController.addAddressBook(TEST_ADDRESS_BOOK_SOURCE_ID, TEST_ADDRESS_BOOK_NAME);
        Assert.assertEquals("", messageId);
        Mockito.verify(mAACSMessageSender, Mockito.times(0))
                .sendMessageReturnID(
                        eq(Topic.ADDRESS_BOOK), eq(Action.AddressBook.ADD_ADDRESS_BOOK), Mockito.anyString());
    }

    @Test
    public void remove_valid_AddressBook() {
        String messageId = mPhoneBookController.removeAddressBook(TEST_ADDRESS_BOOK_SOURCE_ID);
        Assert.assertNotEquals("", messageId);
        Mockito.verify(mAACSMessageSender, Mockito.times(1))
                .sendMessageReturnID(
                        eq(Topic.ADDRESS_BOOK), eq(Action.AddressBook.REMOVE_ADDRESS_BOOK), Mockito.anyString());
    }
}
