package com.mewhpm.mewsync.ui.recyclerview.adapters

import android.content.Context
import android.graphics.drawable.Icon
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import androidx.core.content.ContextCompat
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.ui.recyclerview.data.TextPairWithIcon
import com.mikepenz.iconics.IconicsDrawable
import kotlinx.android.synthetic.main.x01_recyclerview_pair_element.view.*

abstract class SimpleTextIconAdapter: androidx.recyclerview.widget.RecyclerView.Adapter<SimpleTextIconAdapter.ViewHolder>() {
    inner class ViewHolder(val mView: View) : androidx.recyclerview.widget.RecyclerView.ViewHolder(mView) {
        val elementIcon: ImageView = mView.element_icon
        val elementTitle: TextView = mView.element_title
    }

    abstract fun requestDataTextPairWithIcon(position: Int) : TextPairWithIcon
    abstract fun requestListSize() : Int
    abstract fun requestContext() : Context

    abstract fun onElementClick(position: Int)
    abstract fun onElementLongClick(position: Int)

    var _index = -1

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.x00_recyclerview_simple_element, parent, false)
        return ViewHolder(view)
    }

    override fun getItemCount(): Int {
        return requestListSize()
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val element = requestDataTextPairWithIcon(position)
        val context = requestContext()

        with(holder) {
            elementTitle.text = element.title
            elementTitle.setTextColor(element.titleColor)

            elementIcon.setImageIcon(
                Icon.createWithBitmap(
                    IconicsDrawable(context)
                        .icon(element.icon)
                        .sizeDp(element.iconSize)
                        .color(element.iconColor)
                        .toBitmap()
                )
            )

            mView.background = ContextCompat.getDrawable(context,
                if (_index != position) R.drawable.x00_rv_keyboard_list_item_bg else R.drawable.x00_rv_keyboard_list_item_bg_selected)

            mView.setOnClickListener {
                _index = position
                notifyDataSetChanged()
                onElementClick(position)
            }
            mView.setOnLongClickListener { onElementLongClick(position); true }
        }
    }
}