package com.mewhpm.mewsync.dao

import android.util.Log
import com.j256.ormlite.dao.Dao
import com.j256.ormlite.dao.DaoManager
import com.j256.ormlite.support.ConnectionSource
import com.j256.ormlite.table.TableUtils
import com.mewhpm.mewsync.DeviceActivity
import com.mewhpm.mewsync.data.PassRecord
import com.mewhpm.mewsync.data.enums.PassRecordType
import com.mewhpm.mewsync.utils.CryptoUtils
import java.lang.Exception
import java.sql.SQLException

class PasswordsDao private constructor (val connectionSource : ConnectionSource) {
    companion object {
        private var instance: PasswordsDao? = null
        fun getInstance(_connectionSource : ConnectionSource) : PasswordsDao {
            if (instance == null) instance = PasswordsDao(_connectionSource)
            return instance!!
        }
    }

    private val dao : Dao<PassRecord, Long> = DaoManager.createDao(connectionSource, PassRecord::class.java)
    init {
        TableUtils.createTableIfNotExists(connectionSource, PassRecord::class.java)
    }

    fun getAllChild(parentId: Long, nodeType: Long, mac: String = DeviceActivity.currentDeviceMac) : List<PassRecord> {
        val unsorted = dao.queryForFieldValuesArgs(mapOf(
            "parentId" to parentId,
            "nodeType" to nodeType,
            "deviceAddr" to mac
        ))
        return decrypt(unsorted.sortedWith(compareBy(PassRecord::title)))
    }

    fun getAllChild(parentId: Long, mac: String = DeviceActivity.currentDeviceMac) : List<PassRecord> {
        val unsorted = dao.queryForFieldValuesArgs(mapOf(
            "parentId" to parentId,
            "deviceAddr" to mac
        ))
        return decrypt(unsorted.sortedWith(compareBy(PassRecord::recordType, PassRecord::title)))
    }

    fun getById(nodeId: Long) : PassRecord? {
        if (nodeId == 0L) return null
        return try {
            return dao.queryForId(nodeId)
        } catch (e : SQLException) {
            null
        }
    }

    fun getParent(nodeId: Long) : PassRecord? {
        if (nodeId == 0L) return null
        return try {
            val current = dao.queryForId(nodeId)
            if (current.parentId == 0L) return null
            val parent = dao.queryForId(current.parentId)
            parent
        } catch (e : SQLException) {
            null
        }
    }

    fun createOrSave(passRecord: PassRecord) {
        if (passRecord.id == 0L) create(passRecord) else save(passRecord)
    }

    fun save(passRecord: PassRecord) {
        encrypt(passRecord)
        dao.update(passRecord)
    }

    fun create(passRecord: PassRecord) {
        encrypt(passRecord)
        dao.create(passRecord)
    }

    fun remove(id: Long) {
        dao.deleteById(id)
    }

    fun remove(passRecord: PassRecord) {
        try {
            dao.delete(passRecord)
        } catch (e : SQLException) {
            Log.w("PasswordsDao:remove", "Cant't delete node with id=${passRecord.id}")
        }
    }

    fun removeDir(passRecord: PassRecord) {
        if (passRecord.recordType != PassRecordType.DIRECTORY) return
        getAllChild(passRecord.id).forEach { child ->
            if (child.recordType == PassRecordType.DIRECTORY) {
                removeDir(child)
            } else {
                remove(child)
            }
        }
        remove(passRecord)
    }

    private fun encrypt(passRecords: List<PassRecord>) : List<PassRecord> {
        passRecords.forEach { encrypt(it) }
        return passRecords
    }

    private fun decrypt(passRecords: List<PassRecord>) : List<PassRecord> {
        passRecords.forEach { decrypt(it) }
        return passRecords
    }

    private fun encrypt(passRecord: PassRecord) : PassRecord {
        try {
            passRecord.metadataJson = CryptoUtils.encryptRSA(passRecord.metadataJson)
        } catch (e : Exception) {
            Log.d("encrypt", "Bad json found in database")
            Log.d("encrypt", "\tid#${passRecord.id};\tjson: \"${passRecord.metadataJson}\"")
            Log.d("encrypt", "\terror: \"${e.message}\"")
        }
        return passRecord
    }

    private fun decrypt(passRecord: PassRecord) : PassRecord {
        try {
            passRecord.metadataJson = CryptoUtils.decryptRSA(passRecord.metadataJson)
        } catch (e : Exception) {
            Log.d("decrypt", "Bad json found in database")
            Log.d("decrypt", "\tid#${passRecord.id};\tjson: \"${passRecord.metadataJson}\"")
            Log.d("encrypt", "\terror: \"${e.message}\"")
        }
        return passRecord
    }
}