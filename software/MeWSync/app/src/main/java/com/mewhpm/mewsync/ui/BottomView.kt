package com.mewhpm.mewsync.ui

import android.content.Context
import android.graphics.Color
import android.graphics.drawable.Icon
import android.util.AttributeSet
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.mikepenz.iconics.IconicsDrawable
import com.mikepenz.iconics.typeface.IIcon

class BottomView : BottomNavigationView {
    companion object {
        var iconSizeDp = 40
        var iconColor = Color.WHITE
    }

    constructor(context: Context) : super(context)
    constructor(context: Context, attributeSet: AttributeSet?) : super(context, attributeSet)
    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr)

    fun addIcon(icon: IIcon, text: String, onClick: () -> Unit) : BottomView {
        val item = menu.add(text)
        item.icon = Icon.createWithBitmap(
            IconicsDrawable(context).icon(icon).sizeDp(iconSizeDp).color(iconColor)
                .toBitmap()).loadDrawable(context)
        item.setOnMenuItemClickListener {
            onClick.invoke()
            false
        }
        return this
    }
}