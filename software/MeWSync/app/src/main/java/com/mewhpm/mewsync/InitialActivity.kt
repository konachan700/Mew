package com.mewhpm.mewsync

import android.content.Intent
import android.content.SharedPreferences
import android.os.Bundle
import android.preference.PreferenceManager
import android.view.View
import androidx.appcompat.app.AppCompatActivity
import com.mewhpm.mewsync.dao.KnownDevicesDao
import com.mewhpm.mewsync.dao.connectionSource
import com.mewhpm.mewsync.data.BleDevice
import com.mewhpm.mewsync.fragments.PincodeFragment
import com.mewhpm.mewsync.utils.CryptoUtils
import com.mewhpm.mewsync.utils.PinUtil
import kotlinx.android.synthetic.main.x00_initial_activity_view.*
import kotlinx.android.synthetic.main.x00_wait_fragment.view.*

class InitialActivity : AppCompatActivity() {
    private var _pinHash = ""
    private val pincodeFragment = PincodeFragment()

    private fun initPincode(defaultDevice: BleDevice) {
        with (pincodeFragment) {
            onBackPressEvent = { this@InitialActivity.finish() }
            onClearAllEvent = { _pinHash = "" }
            onNumberAddedEvent = { _pinHash = PinUtil.addNumber(this@InitialActivity, _pinHash, it) }
            onOKPressedEvent = {
                val pref: SharedPreferences = PreferenceManager.getDefaultSharedPreferences(applicationContext)
                val retVal = CryptoUtils.verifyPinCode(pref, _pinHash, defaultDevice)
                if (retVal) {
                    val act2 = Intent(applicationContext, DeviceActivity::class.java)
                    act2.addFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP)
                    DeviceActivity.currentDeviceMac = defaultDevice.mac
                    finish()
                    startActivityForResult(act2, 0)
                    null
                } else
                    getString(R.string.incorrect_pin_code)
            }
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.x00_initial_activity_view)

        this.waitDummy1.waitText1.text = getString(R.string.wait_for_keys_generating)

        CryptoUtils.checkGenerated {
            this@InitialActivity.runOnUiThread {
                val defaultDevice = KnownDevicesDao.getInstance(applicationContext.connectionSource).getDefault()
                if (defaultDevice == null) {
                    val act = Intent(applicationContext, MainActivity::class.java)
                    finish()
                    startActivityForResult(act, 0)
                    return@runOnUiThread
                }

                this.waitDummy1.visibility = View.GONE

                val transaction = supportFragmentManager.beginTransaction()
                transaction.replace(R.id.initContainerBox1, pincodeFragment)
                transaction.commit()

                initPincode(defaultDevice)
            }
        }
    }
}