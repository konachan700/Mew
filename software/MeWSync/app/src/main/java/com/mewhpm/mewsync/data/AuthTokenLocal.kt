package com.mewhpm.mewsync.data

import java.util.*

data class AuthTokenLocal (
    val activityId : UUID,
    var date: Long = Date().time,
    var device: String
)