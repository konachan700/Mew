package com.mewhpm.mewsync.fragments

import android.graphics.drawable.Icon
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import androidx.core.content.ContextCompat
import androidx.fragment.app.Fragment
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.utils.VibroUtils
import com.mikepenz.google_material_typeface_library.GoogleMaterial
import com.mikepenz.iconics.IconicsDrawable
import kotlinx.android.synthetic.main.x00_pincode_fragment.view.*
import org.jetbrains.anko.sdk27.coroutines.onClick

class PincodeFragment : Fragment() {
    private var _view : View? = null
    private var _title = ""

    var onNumberAddedEvent : (number: String) -> Unit = {}
    var onClearAllEvent : () -> Unit = {}
    var onOKPressedEvent : () -> String? = { null }
    var onBackPressEvent : () -> Unit = {}

    private fun addNum(num : String) {
        _view!!.pincodeBox1.append("\u25cf")
        onNumberAddedEvent.invoke(num)
        VibroUtils.vibrate(this.requireContext(), 200)
    }

    fun clearInput() {
        _view!!.pincodeBox1.text.clear()
    }

    fun setTitle(title: String) {
        _title = title
    }

    override fun onCreateView(inflater: LayoutInflater, container: ViewGroup?, savedInstanceState: Bundle?): View? {
        _view = inflater.inflate(R.layout.x00_pincode_fragment, container, false)

        if (_title.isNotBlank()) _view?.pincodeFragmentTitle?.text = _title

        _view!!.pincodeDeleteButton.setImageIcon(
            Icon.createWithBitmap(
                IconicsDrawable(context!!)
                    .icon(GoogleMaterial.Icon.gmd_backspace)
                    .sizeDp(48)
                    .color(ContextCompat.getColor(context!!, R.color.colorBrandWhite))
                    .toBitmap()
            ))

        _view!!.button11.setOnClickListener { addNum("1") }
        _view!!.button12.setOnClickListener { addNum("2") }
        _view!!.button13.setOnClickListener { addNum("3") }
        _view!!.button21.setOnClickListener { addNum("4") }
        _view!!.button22.setOnClickListener { addNum("5") }
        _view!!.button23.setOnClickListener { addNum("6") }
        _view!!.button31.setOnClickListener { addNum("7") }
        _view!!.button32.setOnClickListener { addNum("8") }
        _view!!.button33.setOnClickListener { addNum("9") }
        _view!!.button42.setOnClickListener { addNum("0") }

        _view!!.button41.setOnClickListener {
            onBackPressEvent.invoke()
            VibroUtils.vibrate(this@PincodeFragment.requireContext(), 100)
        }

        _view!!.button43.setOnClickListener {
            val errorMessage = onOKPressedEvent.invoke()
            if (errorMessage == null) {
                _view!!.incorrectPinWarning.visibility = View.INVISIBLE
            } else {
                _view!!.incorrectPinWarning.visibility = View.VISIBLE
                _view!!.incorrectPinWarning.text = errorMessage
            }

            _view!!.pincodeBox1.text.clear()
            onClearAllEvent.invoke()
            VibroUtils.vibrate(this.requireContext(), 100)
        }

        _view!!.pincodeDeleteButton.onClick {
            _view!!.pincodeBox1.text.clear()
            _view!!.incorrectPinWarning.visibility = View.INVISIBLE
            onClearAllEvent.invoke()
            VibroUtils.vibrate(this@PincodeFragment.requireContext(), 100)
        }

        return _view
    }
}