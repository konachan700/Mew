package com.mewhpm.mewsync.services

import android.content.Context
import android.graphics.Paint
import android.inputmethodservice.InputMethodService
import android.view.Gravity
import android.view.MenuItem
import android.view.View
import android.view.inputmethod.EditorInfo
import android.view.inputmethod.InputMethodManager
import android.widget.PopupMenu
import com.google.gson.Gson
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.dao.KnownDevicesDao
import com.mewhpm.mewsync.dao.PasswordsDao
import com.mewhpm.mewsync.dao.connectionSource
import com.mewhpm.mewsync.data.BleDevice
import com.mewhpm.mewsync.data.PassRecord
import com.mewhpm.mewsync.data.meta.PassRecordMetadata
import com.mewhpm.mewsync.data.enums.PassRecordType
import com.mewhpm.mewsync.security.LocalPasswordProviderImpl
import com.mewhpm.mewsync.security.PasswordProvider
import com.mewhpm.mewsync.ui.FixedHeightLinearLayout
import com.mewhpm.mewsync.utils.setGmdIcon
import com.mikepenz.google_material_typeface_library.GoogleMaterial
import kotlinx.android.synthetic.main.x00_keyboard_view_real.view.*
import kotlin.collections.HashMap

class MewIMEService : InputMethodService() {
    companion object {
        const val ICON_SIZE = 28
    }

    private val gson = Gson()

    private var _myView : FixedHeightLinearLayout? = null
    private var _currentFolderId = 0L
    private var _currentMetadata : PassRecordMetadata? = null
    private var _currentPassword : PassRecord? = null
    private var _currentDevice : BleDevice? = null

    private val localPassProvider : PasswordProvider = LocalPasswordProviderImpl.getInstance()

    override fun onCreateInputView(): View {
        _myView = View.inflate(this, com.mewhpm.mewsync.R.layout.x00_keyboard_view_real, null) as FixedHeightLinearLayout

        val daoDev = KnownDevicesDao.getInstance(applicationContext.connectionSource)
        if (_currentDevice == null) {
            _currentDevice = daoDev.getDefault() ?: daoDev.getDeviceZero() ?: BleDevice()
        }

        with (_myView!!.textViewURL) {
            paintFlags = paintFlags or Paint.UNDERLINE_TEXT_FLAG
            setOnClickListener {
                input(_currentMetadata!!.url)
            }
        }

        with (_myView!!.textViewLogin) {
            paintFlags = paintFlags or Paint.UNDERLINE_TEXT_FLAG
            setOnClickListener {
                input(_currentMetadata!!.login)
            }
        }

        with (_myView!!.textViewPassword) {
            paintFlags = paintFlags or Paint.UNDERLINE_TEXT_FLAG
            setOnClickListener {
                if (KnownDevicesDao.isDeviceZero(_currentPassword!!.deviceAddr)) {
                    input(localPassProvider.generatePassword(context, _currentPassword!!.hwUID, _currentPassword!!.deviceAddr, _currentMetadata))
                }
            }
        }

        with (_myView!!.langSwitchBtn1) {
            setGmdIcon(GoogleMaterial.Icon.gmd_keyboard, ICON_SIZE, R.color.colorBrandBlack)
            setOnClickListener {
                val imeManager = applicationContext.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
                imeManager.showInputMethodPicker()
            }
        }

        with (_myView!!.deviceSelectBtn1) {
            setGmdIcon(GoogleMaterial.Icon.gmd_bluetooth, ICON_SIZE, R.color.colorBrandBlack)
            setOnClickListener {
                val refmap = HashMap<MenuItem, BleDevice>()
                val menu = PopupMenu(this.context, _myView!!.deviceSelectBtn1, Gravity.END)
                menu.setOnMenuItemClickListener {it2 ->
                    if (refmap[it2] != null) {
                        _currentDevice = refmap[it2]
                        _myView?.listRVPasswordsKeyboard?.addItems(fillList(), _currentFolderId == 0L)
                    }
                    true
                }

                daoDev.getAll().forEach {dev -> refmap[menu.menu.add(dev.name)] = dev }
                menu.show()
            }
        }

        with (_myView!!.listRVPasswordsKeyboard) {
            create()
            addItems(fillList(), true)
            onBackEvent = {
                val dao = PasswordsDao.getInstance(context.connectionSource)
                val parent = dao.getParent(_currentFolderId)
                _currentFolderId = parent?.id ?: 0
                addItems(fillList(), _currentFolderId == 0L)
            }
            onItemClickEvent = {
                _currentPassword = it
                when (_currentPassword!!.recordType) {
                    PassRecordType.DIRECTORY -> {
                        _currentFolderId = _currentPassword!!.id
                        addItems(fillList(), _currentFolderId == 0L)
                    }
                    PassRecordType.PASSWORD -> {
                        _currentMetadata = gson.fromJson<PassRecordMetadata>(_currentPassword!!.metadataJson, PassRecordMetadata::class.java)
                        _myView!!.textViewPassword.visibility = View.VISIBLE
                        _myView!!.textViewTitleKb.visibility = View.VISIBLE
                        _myView!!.textViewTitleNoItems.visibility = View.GONE
                        if (_currentMetadata != null) {
                            _myView!!.textViewLogin.visibility = if (_currentMetadata!!.login.isNotBlank()) View.VISIBLE else View.GONE
                            _myView!!.textViewURL.visibility = if (_currentMetadata!!.url.isNotBlank()) View.VISIBLE else View.GONE
                        }
                    }
                }
            }
        }

        return _myView!!
    }

    override fun onStartInputView(info: EditorInfo?, restarting: Boolean) {
        super.onStartInputView(info, restarting)
        if (restarting) _myView?.listRVPasswordsKeyboard?.addItems(fillList(), _currentFolderId == 0L)
    }

    override fun onEvaluateFullscreenMode(): Boolean {
        return false
    }

    private fun fillList() : List<PassRecord> {
        _myView!!.textViewTitleKb.visibility = View.GONE
        _myView!!.textViewLogin.visibility = View.GONE
        _myView!!.textViewURL.visibility = View.GONE
        _myView!!.textViewPassword.visibility = View.GONE
        _myView!!.textViewTitleNoItems.visibility = View.VISIBLE

        val daoPwd = PasswordsDao.getInstance(applicationContext.connectionSource)
        return daoPwd.getAllChild(parentId = _currentFolderId, mac = _currentDevice!!.mac)
    }

    private fun input(data: String) {
        currentInputConnection.commitText(data, 1)
    }
}