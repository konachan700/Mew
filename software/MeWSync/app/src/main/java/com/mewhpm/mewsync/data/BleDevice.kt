package com.mewhpm.mewsync.data

import com.j256.ormlite.field.DatabaseField
import com.j256.ormlite.table.DatabaseTable

@DatabaseTable(tableName = "BleDevice")
class BleDevice {
    constructor()
    constructor(id: Long, mac: String, name: String) {
        this.id = id
        this.mac = mac
        this.name = name
    }

    companion object {
        const val EMPTY = ""
    }

    @DatabaseField(columnName = "id", generatedId = true)
    var id: Long = 0

    @DatabaseField(index = true)
    var mac: String = EMPTY

    @DatabaseField
    var name: String = EMPTY

    @DatabaseField
    var text: String = EMPTY

    @DatabaseField
    var default: Boolean = false
}