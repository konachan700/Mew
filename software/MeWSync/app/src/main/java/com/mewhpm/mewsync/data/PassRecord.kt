package com.mewhpm.mewsync.data

import com.j256.ormlite.field.DatabaseField
import com.j256.ormlite.table.DatabaseTable
import com.mewhpm.mewsync.data.enums.PassRecordProvider
import com.mewhpm.mewsync.data.enums.PassRecordType
import java.util.*

@DatabaseTable(tableName = "PassRecord")
class PassRecord : DisplayablePassRecord {
    companion object {
        private const val EMPTY = ""
    }

    @DatabaseField(columnName = "id", generatedId = true)
    var id: Long = 0

    @DatabaseField
    var parentId: Long = 0

    @DatabaseField
    override var title: String = EMPTY

    @DatabaseField
    var timestamp: Long = Date().time

    @DatabaseField
    var metadataJson: String = EMPTY

    @DatabaseField
    var hwUID: Long = 0L

    @DatabaseField
    var deviceAddr: String = EMPTY

    @DatabaseField
    var provider: PassRecordProvider = PassRecordProvider.LOCAL

    @DatabaseField
    override var recordType: PassRecordType = PassRecordType.DIRECTORY

    /************************************* NOT IN DB *******************************************/

    override var text: String = EMPTY
}