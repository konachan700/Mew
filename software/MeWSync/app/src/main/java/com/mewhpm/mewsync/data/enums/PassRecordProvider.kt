package com.mewhpm.mewsync.data.enums

enum class PassRecordProvider (val type: Int) {
    LOCAL(1 shl 1),
    CLOUD(1 shl 2),
    HARDWARE(1 shl 3)
}