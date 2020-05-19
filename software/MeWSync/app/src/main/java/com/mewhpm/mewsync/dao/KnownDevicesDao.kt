package com.mewhpm.mewsync.dao

import com.j256.ormlite.dao.Dao
import com.j256.ormlite.dao.DaoManager
import com.j256.ormlite.support.ConnectionSource
import com.j256.ormlite.table.TableUtils
import com.mewhpm.mewsync.data.BleDevice

class KnownDevicesDao private constructor (val connectionSource : ConnectionSource) {
    companion object {
        const val ZERO_MAC = "00:00:00:00:00:00"

        private var instance: KnownDevicesDao? = null
        fun getInstance(_connectionSource : ConnectionSource) : KnownDevicesDao {
            if (instance == null) instance = KnownDevicesDao(_connectionSource)
            return instance!!
        }

        fun isDeviceZero(mac: String) : Boolean {
            return ZERO_MAC.contentEquals(mac)
        }
    }

    private val dao : Dao<BleDevice, Long> = DaoManager.createDao(connectionSource, BleDevice::class.java)
    init {
        TableUtils.createTableIfNotExists(connectionSource, BleDevice::class.java)

        val devZero = BleDevice()
        devZero.id = 0
        devZero.default = false
        devZero.name = "MeW Local"
        devZero.text = "For testing without MeW HPM"
        devZero.mac = KnownDevicesDao.ZERO_MAC
        if (!isExist(devZero)) {
            addNew(devZero)
        }
    }

    fun getAll() : List<BleDevice> {
        return dao.queryForAll().sortedWith(compareBy(BleDevice::mac))
    }

    fun isExist(dev: BleDevice): Boolean {
        return !dao.queryForEq("mac", dev.mac).isEmpty()
    }

    fun getByMac(mac: String): BleDevice? {
        val list = dao.queryForEq("mac", mac)
        return if (list.isEmpty()) null else list[0]
    }

    fun getDeviceZero(): BleDevice? {
        val list = dao.queryForEq("mac", KnownDevicesDao.ZERO_MAC)
        return if (list.isEmpty()) null else list[0]
    }

    fun addNew(dev: BleDevice) {
        dao.create(dev)
    }

    fun remove(dev: BleDevice) {
        dao.delete(dev)
    }

    fun save(dev: BleDevice) {
        dao.update(dev)
    }

    fun clearDefault() {
        dao.updateBuilder().updateColumnValue("default", false).update()
    }

    fun setDefault(dev: BleDevice) {
        dao.updateBuilder().updateColumnValue("default", false).update()
        dev.default = true
        dao.update(dev)
    }

    fun getDefault() : BleDevice? {
        val list = dao.queryForEq("default", true)
        return if (list.isNotEmpty()) list[0] else null
    }
}