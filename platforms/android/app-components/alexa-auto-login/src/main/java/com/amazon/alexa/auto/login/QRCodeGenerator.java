package com.amazon.alexa.auto.login;

import static android.graphics.Color.BLACK;
import static android.graphics.Color.WHITE;

import android.graphics.Bitmap;
import android.util.Log;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.EncodeHintType;
import com.google.zxing.MultiFormatWriter;
import com.google.zxing.WriterException;
import com.google.zxing.common.BitMatrix;

import java.util.EnumMap;
import java.util.Map;

/**
 * This class provides qrcode bitmap given an amazon url with code. It uses google's zxing library
 * to generate a bitmap.
 */
public class QRCodeGenerator {
    private static final String TAG = QRCodeGenerator.class.getSimpleName();
    // bitmap size DIMENSION = height = width = 150
    private static final int DIMENSION = 150;

    public Bitmap generateQRCode(String url) {
        Bitmap bitmap = null;
        try {
            // Getting QR-Code as Bitmap
            bitmap = encodeAsBitmap(url);
        } catch (WriterException e) {
            Log.w(TAG, "Failed to get QRCode as a bitmap.");
        }
        return bitmap;
    }

    private static String guessAppropriateEncoding(CharSequence contents) {
        for (int i = 0; i < contents.length(); i++) {
            if (contents.charAt(i) > 0xFF) {
                return "UTF-8";
            }
        }
        return null;
    }

    private static Bitmap encodeAsBitmap(String url) throws WriterException {
        if (url.isEmpty())
            return null;

        Map<EncodeHintType, Object> hints = null;
        String encoding = guessAppropriateEncoding(url);
        if (encoding != null) {
            hints = new EnumMap<>(EncodeHintType.class);
            hints.put(EncodeHintType.CHARACTER_SET, encoding);
        }
        MultiFormatWriter writer = new MultiFormatWriter();
        BitMatrix result = writer.encode(url, BarcodeFormat.QR_CODE, DIMENSION, DIMENSION, hints);
        int width = result.getWidth();
        int height = result.getHeight();
        int[] pixels = new int[width * height];
        // All are 0, or black, by default
        for (int y = 0; y < height; y++) {
            int offset = y * width;
            for (int x = 0; x < width; x++) {
                pixels[offset + x] = result.get(x, y) ? BLACK : WHITE;
            }
        }

        Bitmap bitmap = Bitmap.createBitmap(width, height, Bitmap.Config.ARGB_8888);
        bitmap.setPixels(pixels, 0, width, 0, 0, width, height);
        return bitmap;
    }
}
