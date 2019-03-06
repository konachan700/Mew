package com.mewhpm.mewsync.fragments
import android.graphics.Color
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.fragment.app.Fragment
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.dao.KnownDevicesDao
import com.mewhpm.mewsync.dao.connectionSource
import com.mewhpm.mewsync.data.BleDevice
import com.mewhpm.mewsync.ui.recyclerview.impl.RecyclerViewBleDiscoveryImpl
import com.mikepenz.google_material_typeface_library.GoogleMaterial
import com.mikepenz.iconics.IconicsDrawable
import com.polidea.rxandroidble2.RxBleClient
import com.polidea.rxandroidble2.scan.ScanSettings
import io.reactivex.disposables.Disposable
import kotlinx.android.synthetic.main.x01_ble_disovery_fragment.view.*
import org.jetbrains.anko.support.v4.toast

class BleDiscoveryFragment : Fragment() {
    var onExitEvent : () -> Unit = {}
    var onDeviceSelectEvent : (ble: BleDevice) -> Unit = {}

    private var _rxBleClient: RxBleClient? = null
    private var _bleScannerDisposable: Disposable? = null

    private var _rvList: RecyclerViewBleDiscoveryImpl? = null
    private var _view: View? = null

    private var colorCounter = 0
    private var colorArray = arrayOf("#FF9999", "#99FF99", "#FFFF99")

    override fun onPause() {
        super.onPause()
        _bleScannerDisposable?.dispose()
        _rxBleClient = null
        onExitEvent.invoke()
    }

    override fun onResume() {
        super.onResume()
        _rxBleClient = RxBleClient.create(requireContext())
        scan()
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        _view = inflater.inflate(R.layout.x01_ble_disovery_fragment, container, false)
        _rvList = _view!!.discoveryList

        with (_view!!) {
            button.setCompoundDrawables(IconicsDrawable(context)
                .icon(GoogleMaterial.Icon.gmd_bluetooth).sizeDp(32).color(Color.WHITE),null,null,null)
            button.setOnClickListener {
                onExitEvent.invoke()
            }
        }

        with (_rvList!!) {
            create()
            deviceExistRequestEvent = { dev ->
                KnownDevicesDao.getInstance(this@BleDiscoveryFragment.requireContext().connectionSource).isExist(dev) }
            deviceItemClickEvent = { dev -> onDeviceSelectEvent.invoke(dev) }
        }
        return _view
    }

    private fun scan() {
        _bleScannerDisposable = _rxBleClient!!.scanBleDevices(
            ScanSettings.Builder()
                .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
                .setCallbackType(ScanSettings.CALLBACK_TYPE_ALL_MATCHES)
                .build()
        ).subscribe(
            { scanResult ->
                colorCounter++
                if (colorCounter >= colorArray.size) colorCounter = 0
                view?.button?.setCompoundDrawables(
                    IconicsDrawable(context).icon(GoogleMaterial.Icon.gmd_bluetooth).sizeDp(32)
                        .color(Color.parseColor(colorArray[colorCounter])), null, null, null)

                if (scanResult.bleDevice.name != null && scanResult.bleDevice.macAddress != null)
                    _rvList?.add(BleDevice(0, scanResult.bleDevice.macAddress!!, scanResult.bleDevice.name!!))
            },
            { throwable ->
                toast("Detected an error: ${throwable.message}")
            }
        )
    }
}