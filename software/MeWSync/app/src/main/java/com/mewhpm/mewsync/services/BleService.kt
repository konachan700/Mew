package com.mewhpm.mewsync.services

import android.app.Service
import android.bluetooth.*
import android.content.Intent
import android.os.IBinder
import java.util.*

class BleService: Service() {
    companion object {
        val serialUUID = UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")!!
        val txUUID = UUID.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")!!
        val rxUUID = UUID.fromString("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")!!
        val clientUUID = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")!!

        const val EXTRA_ACTION = "action"
        const val EXTRA_ACTION_CONNECT = 1
        const val EXTRA_ACTION_DISCONNECT = 2

        const val EXTRA_RESULT_CODE = "result"
        const val EXTRA_RESULT_CODE_OK = 1
        const val EXTRA_RESULT_CODE_ERROR = 2
        const val EXTRA_RESULT_CODE_IN_PROGRESS = 3

        const val BLE_INTENT_ERR_CODE = "errorCode"
        const val BLE_INTENT_ERR_MESSAGE = "errorMessage"

        const val EXTRA_DATA_MAC = "mac"
        const val EXTRA_DATA_NAME = "name"
        const val EXTRA_DATA_TIMESTAMP = "timestamp"
    }

    inner class BleCallback: BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt?, status: Int, newState: Int) {
            super.onConnectionStateChange(gatt, status, newState)
            when(newState) {
                BluetoothGatt.STATE_CONNECTED -> {

                }
                BluetoothGatt.STATE_DISCONNECTED -> {

                }
            }
        }

        override fun onServicesDiscovered(gatt: BluetoothGatt?, status: Int) {
            super.onServicesDiscovered(gatt, status)

            _serialTx = gatt?.getService(serialUUID)?.getCharacteristic(txUUID)
            _serialRx = gatt?.getService(serialUUID)?.getCharacteristic(rxUUID)

            if (gatt?.setCharacteristicNotification(_serialRx, true) != true) {
                sendErrorIntent(1000, "Couldn't enable the Characteristic Notification")
                return
            }

            if (_serialRx?.getDescriptor(clientUUID) == null) {
                sendErrorIntent(1001,"Couldn't get RX client descriptor!")
                return
            }

            val desc = _serialRx?.getDescriptor(clientUUID)
            desc?.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
            if (!gatt.writeDescriptor(desc)) {
                sendErrorIntent(1002, "Couldn't write RX descriptor!")
                return
            }

            sendOkIntent(2001)
        }
    }

    private var _serialTx: BluetoothGattCharacteristic? = null
    private var _serialRx: BluetoothGattCharacteristic? = null
    private var _gatt: BluetoothGatt? = null

    private val _callback = BleCallback()

    override fun onBind(intent: Intent?): IBinder? {
        return null
    }

    override fun onStartCommand(intent: Intent?, flags: Int, startId: Int): Int {
        if (intent == null) return START_STICKY

        when (intent.getIntExtra(EXTRA_ACTION, 0)) {
            EXTRA_ACTION_CONNECT -> {
                if (!intent.hasExtra(EXTRA_DATA_MAC)) return START_STICKY

                val device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(intent.getStringExtra(EXTRA_DATA_MAC))
                _gatt = device.connectGatt(this, false, _callback)
            }
            EXTRA_ACTION_DISCONNECT -> {
                _gatt?.disconnect()
                _gatt?.close()
                _gatt = null
                _serialTx = null
                _serialRx = null

                sendOkIntent(2000)
            }
        }
        return START_STICKY
    }

    private fun sendOkIntent(code: Int = 0) {
        val intentAnswer = Intent()
        intentAnswer.putExtra(EXTRA_RESULT_CODE, EXTRA_RESULT_CODE_OK)
        intentAnswer.putExtra(BLE_INTENT_ERR_CODE, code)
        sendBroadcast(intentAnswer)
    }

    private fun sendErrorIntent(code: Int, msg: String) {
        val intentAnswer = Intent()
        intentAnswer.putExtra(EXTRA_RESULT_CODE, EXTRA_RESULT_CODE_ERROR)
        intentAnswer.putExtra(BLE_INTENT_ERR_MESSAGE, msg)
        intentAnswer.putExtra(BLE_INTENT_ERR_CODE, code)
        sendBroadcast(intentAnswer)
    }
}