package com.mewhpm.mewsync.utils

import android.content.Context
import android.content.res.ColorStateList
import android.graphics.drawable.Icon
import android.util.Log
import android.view.Menu
import android.view.MenuItem
import android.view.View
import android.view.inputmethod.InputMethodManager
import android.widget.*
import androidx.core.content.ContextCompat
import com.mewhpm.mewsync.R
import com.mikepenz.iconics.IconicsDrawable
import com.mikepenz.iconics.typeface.IIcon
import kotlinx.android.synthetic.main.x02_activity_device.*
import org.jetbrains.anko.sdk27.coroutines.onClick
import org.jetbrains.anko.textColor

fun androidx.appcompat.app.AppCompatActivity.fixPaddingTopForNavigationView() {
    val sizeRes = resources.getIdentifier("status_bar_height", "dimen", "android")
    val res = if (sizeRes > 0) resources.getDimensionPixelSize(sizeRes) else 0
    this.navView1.setPadding(this.navView1.paddingLeft, res, this.navView1.paddingRight, this.navView1.paddingBottom)
}

private fun <T> reflectionGetItem(obj: Any, field: String, clazz: Class<T>) : T {
    val rfield = obj::class.java.getDeclaredField(field)
    rfield.isAccessible = true
    return rfield.get(obj) as T
}

// SearchView is not a simple-styleable element. I don't want to dig android sources for searching, what styles it can be used...
fun androidx.fragment.app.Fragment.fixColorOfSearchBar(menu: Menu, itemResId: Int) {
    try {
        val sView = menu.findItem(itemResId).actionView as SearchView
        reflectionGetItem(sView, "mSearchButton", ImageView::class.java).imageTintList = ColorStateList.valueOf(
            ContextCompat.getColor(this.requireContext(), R.color.colorBrandWhite))
        reflectionGetItem(sView, "mCloseButton", ImageView::class.java).imageTintList = ColorStateList.valueOf(
            ContextCompat.getColor(this.requireContext(), R.color.colorBrandWhite))
        reflectionGetItem(sView, "mGoButton", ImageView::class.java).imageTintList = ColorStateList.valueOf(
            ContextCompat.getColor(this.requireContext(), R.color.colorBrandWhite))
        reflectionGetItem(sView, "mVoiceButton", ImageView::class.java).imageTintList = ColorStateList.valueOf(
            ContextCompat.getColor(this.requireContext(), R.color.colorBrandWhite))
        reflectionGetItem(sView, "mSearchSrcTextView", AutoCompleteTextView::class.java).textColor =
            ContextCompat.getColor(this.requireContext(), R.color.colorBrandWhite)
    } catch (e: Throwable) {
        Toast.makeText(this.requireContext(), "Can't use reflection for paint the search bar...", Toast.LENGTH_SHORT).show()
    }
}

fun androidx.appcompat.widget.Toolbar.setOnLogoClickEvent(ev : () -> Unit) {
    try {
        reflectionGetItem(this, "mLogoView", ImageView::class.java).onClick { ev.invoke() }
    } catch (e: Throwable) {
        Log.e("onLogoClickEvent", "Can't use reflection for set action for the toolbar logo.\n\tERROR: ${e.message}")
    }
}

fun View.hideKeyboard() {
    val imm = context.getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
    imm.hideSoftInputFromWindow(windowToken, 0)
}

fun ImageButton.setGmdIcon(icon : IIcon, sizeDp : Int, colorResId : Int) {
    setImageIcon(
        Icon.createWithBitmap(
            IconicsDrawable(context)
                .icon(icon)
                .sizeDp(sizeDp)
                .color(ContextCompat.getColor(context, colorResId))
                .toBitmap()
        )
    )
}
