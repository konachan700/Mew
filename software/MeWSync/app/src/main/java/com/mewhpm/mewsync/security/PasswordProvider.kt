package com.mewhpm.mewsync.security

import android.content.Context
import com.mewhpm.mewsync.data.meta.PassRecordMetadata

interface PasswordProvider {
    fun generatePassword(c: Context?, passId : Long, deviceId : String, metadata: PassRecordMetadata? = null) : String
}