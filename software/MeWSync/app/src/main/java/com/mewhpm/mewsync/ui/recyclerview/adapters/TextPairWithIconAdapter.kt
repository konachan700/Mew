package com.mewhpm.mewsync.ui.recyclerview.adapters

import android.content.Context
import android.graphics.drawable.Icon
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.ImageView
import android.widget.TextView
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.ui.recyclerview.adapters.TextPairWithIconAdapter.ViewHolder
import com.mewhpm.mewsync.ui.recyclerview.data.TextPairWithIcon
import com.mikepenz.iconics.IconicsDrawable
import kotlinx.android.synthetic.main.x01_recyclerview_pair_element.view.*

abstract class TextPairWithIconAdapter: androidx.recyclerview.widget.RecyclerView.Adapter<ViewHolder>() {
    inner class ViewHolder(val mView: View) : androidx.recyclerview.widget.RecyclerView.ViewHolder(mView) {
        val elementIcon: ImageView = mView.element_icon
        val elementText: TextView = mView.element_text
        val elementTitle: TextView = mView.element_title
    }

    abstract fun requestDataTextPairWithIcon(position: Int) : TextPairWithIcon
    abstract fun requestListSize() : Int
    abstract fun requestContext() : Context

    abstract fun onElementClick(position: Int)
    abstract fun onElementLongClick(position: Int)

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.x01_recyclerview_pair_element, parent, false)
        return ViewHolder(view)
    }

    override fun getItemCount(): Int {
        return requestListSize()
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val element = requestDataTextPairWithIcon(position)
        val context = requestContext()

        with(holder) {
            elementText.text = element.text
            elementText.setTextColor(element.textColor)

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

            mView.setOnClickListener { onElementClick(position) }
            mView.setOnLongClickListener { onElementLongClick(position); true }
        }
    }
}