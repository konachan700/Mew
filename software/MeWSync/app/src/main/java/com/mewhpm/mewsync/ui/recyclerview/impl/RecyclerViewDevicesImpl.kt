package com.mewhpm.mewsync.ui.recyclerview.impl

import android.content.Context
import android.util.AttributeSet
import androidx.core.content.ContextCompat
import com.afollestad.materialdialogs.MaterialDialog
import com.afollestad.materialdialogs.input.input
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.dao.KnownDevicesDao
import com.mewhpm.mewsync.data.BleDevice
import com.mewhpm.mewsync.ui.recyclerview.RecyclerViewAbstract
import com.mewhpm.mewsync.ui.recyclerview.data.TextPairWithIcon
import com.mikepenz.google_material_typeface_library.GoogleMaterial
import org.jetbrains.anko.alert
import org.jetbrains.anko.cancelButton
import org.jetbrains.anko.okButton
import org.jetbrains.anko.selector

class RecyclerViewDevicesImpl : RecyclerViewAbstract<BleDevice> {
    constructor(context: Context) : super(context)
    constructor(context: Context, attributeSet: AttributeSet?) : super(context, attributeSet)
    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr)

    private val list = ArrayList<Pair<TextPairWithIcon, BleDevice>>()

    var deleteEvent : (position: Int, item: TextPairWithIcon, obj: BleDevice) -> Unit = { _, _, _ -> throw NotImplementedError("deleteEvent not set") }
    var setDefaultEvent : (position: Int, item: TextPairWithIcon, obj: BleDevice) -> Unit = { _, _, _ -> throw NotImplementedError("setDefaultEvent not set") }
    var setDescriptionEvent : (obj: BleDevice, desc: String) -> Unit = { _, _ -> throw NotImplementedError("setDescriptionEvent not set") }
    var deviceItemClickEvent : (dev : BleDevice) -> Unit = { throw NotImplementedError("deviceItemClickEvent not set") }

    override fun requestList(): ArrayList<Pair<TextPairWithIcon, BleDevice>> = list
    override fun onElementClick(position: Int, item: TextPairWithIcon, obj: BleDevice) {
        deviceItemClickEvent.invoke(obj)
    }

    override fun onElementLongClick(position: Int, item: TextPairWithIcon, obj: BleDevice) {
        if (KnownDevicesDao.isDeviceZero(obj.mac)) {
            context.alert (title = "Device Zero", message = "Do you really want setup a Device Zero as default?") {
                okButton {
                    sefDefault(position, item, obj)
                }
                cancelButton {  }
            }.show()
            return
        }

        val actions = listOf("Set default", "Change description", "Delete")
        context.selector("Actions", actions) { _, index ->
            when (index) {
                0 -> sefDefault(position, item, obj)
                1 -> {
                    MaterialDialog(this@RecyclerViewDevicesImpl.context).show {
                        input(allowEmpty = true, prefill = obj.text) { _, text ->
                            item.text = if (text.isBlank()) obj.mac else text.toString()
                            setDescriptionEvent.invoke(obj, text.toString())
                            this@RecyclerViewDevicesImpl.adapter?.notifyDataSetChanged()
                        }
                        title(R.string.change_desc)
                        positiveButton(R.string.change_description)
                        negativeButton(R.string.back)
                    }
                }
                2 -> {
                    context.alert (title = "Remove device", message = "Do you want delete the device with mac: \"${obj.mac}\" and name \"${obj.name}\"?") {
                        okButton {
                            remove(position, item, obj)
                        }
                        cancelButton {  }
                    }.show()
                }
            }
        }
    }

    private fun createDataTextPairWithIcon(dev : BleDevice) : TextPairWithIcon {
        return TextPairWithIcon(
            icon =
                if (KnownDevicesDao.isDeviceZero(dev.mac))
                    GoogleMaterial.Icon.gmd_sd_storage
                else
                    GoogleMaterial.Icon.gmd_bluetooth,
            iconColor = ContextCompat.getColor(context, if (dev.default) R.color.colorBrandDefaultElement else R.color.colorBrandDark1),
            iconSize = 48,
            text = if (dev.text.isBlank()) dev.mac else dev.text,
            textColor = ContextCompat.getColor(context, R.color.colorBrandDark2),
            title = dev.name,
            titleColor = ContextCompat.getColor(context, R.color.colorBrandBlack)
        )
    }

    fun add(dev : BleDevice) {
        val pair = Pair(createDataTextPairWithIcon(dev), dev)
        list.add(pair)
        this.adapter?.notifyDataSetChanged()
    }

    fun clear() {
        list.clear()
        this.adapter?.notifyDataSetChanged()
    }

    private fun sefDefault(position: Int, item: TextPairWithIcon, obj: BleDevice) {
        list.forEach { it.first.iconColor = ContextCompat.getColor(context, R.color.colorBrandDark1) }
        item.iconColor = ContextCompat.getColor(context, R.color.colorBrandDefaultElement)
        this.adapter?.notifyDataSetChanged()
        setDefaultEvent.invoke(position, item, obj)
    }

    private fun remove(position: Int, item: TextPairWithIcon, obj: BleDevice) {
        list.removeAt(position)
        this.adapter?.notifyDataSetChanged()
        deleteEvent(position, item, obj)
    }

    fun reload() {
        this.adapter?.notifyDataSetChanged()
    }

    override fun create() {
        super.create()
        this.adapter = TextPairWithIconAdapterImpl()
    }
}