package com.mewhpm.mewsync.data

import com.mewhpm.mewsync.data.enums.PassRecordType

interface DisplayablePassRecord {
    var title : String
    var text : String
    var recordType : PassRecordType
}