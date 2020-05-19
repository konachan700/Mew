package com.mewhpm.mewsync.data.enums

enum class PassRecordType (val type: Int) {
    DIRECTORY(1 shl 1),
    PASSWORD(1 shl 2),
}