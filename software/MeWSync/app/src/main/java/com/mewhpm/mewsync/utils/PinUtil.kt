package com.mewhpm.mewsync.utils

import android.content.Context

class PinUtil {
    companion object {
        fun addNumber(c : Context, pinHash:String, number: String): String {
            VibroUtils.vibrate(c, 100)
            val pin = StringBuilder()
                .append(pinHash)
                .append(number)
                .append(CryptoUtils.getUniqueSalt())
                .append(number)
                .toString()
            return CryptoUtils.sha256(pin)
        }
    }
}