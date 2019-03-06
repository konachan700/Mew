package com.mewhpm.mewsync.data

import com.mewhpm.mewsync.data.enums.PassRecordType

class LevelUpRecord : DisplayablePassRecord {
    override var text: String = ""
    override var title : String = ""
    override var recordType : PassRecordType = PassRecordType.DIRECTORY
}