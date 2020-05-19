package com.mewhpm.mewsync.data.meta

import java.io.Serializable

data class PassRecordMetadata (
    var url: String = "",
    var login: String = "",
    var minLen: Int = 16,
    var maxLen: Int = 24,
    var containDigits: Boolean = true,
    var containSymbols: Boolean = true,
    var containLowercaseAlphabet: Boolean = true,
    var containUppercaseAlphabet: Boolean = true,
    var text : String = ""
) : Serializable