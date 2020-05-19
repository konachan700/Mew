package com.mewhpm.mewsync.ui

import android.content.Context
import android.util.AttributeSet
import android.util.DisplayMetrics
import android.view.WindowManager
import android.widget.LinearLayout

class FixedHeightLinearLayout : LinearLayout {
    constructor(context: Context) : super(context)
    constructor(context: Context, attributeSet: AttributeSet?) : super(context, attributeSet)
    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr)

    var precent = 3.0
    var outMetrics = DisplayMetrics()

    private val window = this.context.getSystemService(Context.WINDOW_SERVICE) as WindowManager

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        outMetrics = DisplayMetrics()
    }

    override fun onMeasure(widthMeasureSpec: Int, heightMeasureSpec: Int) {
        window.defaultDisplay.getMetrics(outMetrics)
        super.onMeasure(widthMeasureSpec, MeasureSpec.makeMeasureSpec(Math.round(outMetrics.heightPixels / precent).toInt(), MeasureSpec.EXACTLY))
    }
}