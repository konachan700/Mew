package com.mewhpm.mewsync

import android.content.Context
import android.graphics.drawable.Icon
import android.os.Bundle
import android.view.MenuItem
import androidx.appcompat.app.AppCompatActivity
import androidx.appcompat.widget.Toolbar
import androidx.core.content.ContextCompat
import androidx.core.view.GravityCompat
import com.mewhpm.mewsync.dao.KnownDevicesDao
import com.mewhpm.mewsync.dao.connectionSource
import com.mewhpm.mewsync.fragments.PasswordsRootFragment
import com.mewhpm.mewsync.ui.fragmentpages.getFragmentBook
import com.mewhpm.mewsync.utils.fixPaddingTopForNavigationView
import com.mewhpm.mewsync.utils.setOnLogoClickEvent
import com.mikepenz.google_material_typeface_library.GoogleMaterial
import com.mikepenz.iconics.IconicsDrawable
import com.mikepenz.iconics.utils.IconicsMenuInflaterUtil
import kotlinx.android.synthetic.main.x02_activity_device.*
import ru.ztrap.iconics.kt.wrapByIconics


class DeviceActivity : AppCompatActivity() {
    companion object {
        const val TAB_PASSWORDS_TREE_ROOT = "passwords_tree"
        var currentDeviceMac = ""
    }

    private val passwordFragment = PasswordsRootFragment()

    private fun generateIcon(icon: GoogleMaterial.Icon, colorResId: Int, size: Int = 28) : Icon {
        return Icon.createWithBitmap(
            IconicsDrawable(this@DeviceActivity.applicationContext)
                .icon(icon).sizeDp(size)
                .color(ContextCompat.getColor(this@DeviceActivity.applicationContext, colorResId))
                .toBitmap())
    }

    private fun onMenuClickSelector(menuItemResId : Int) {
        when (menuItemResId) {
            R.id.menuItemPasswords -> {
                this@DeviceActivity.getFragmentBook(R.id.fragment_holder_dev_1).showTopInGroup(TAB_PASSWORDS_TREE_ROOT) }
            R.id.menuSync -> {}
        }
    }

    private fun onMenuClickBase(menuItem : MenuItem) : Boolean {
        for (i in 0..(this@DeviceActivity.navView1.menu.size() - 1)) {
            this@DeviceActivity.navView1.menu.getItem(i).isChecked = false
        }
        menuItem.isChecked = true
        this@DeviceActivity.drawerLayout1.closeDrawers()
        onMenuClickSelector(menuItem.itemId)
        return true
    }

    override fun attachBaseContext(newBase: Context) {
        super.attachBaseContext(newBase.wrapByIconics())
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.x02_activity_device)
        val toolbar: Toolbar = this.activity_device_toolbar as Toolbar
        setSupportActionBar(toolbar)
        supportActionBar!!.setDisplayShowTitleEnabled(false)
        supportActionBar!!.setDisplayHomeAsUpEnabled(false)
        supportActionBar!!.setDisplayUseLogoEnabled(true)
        supportActionBar!!.setLogo(generateIcon(GoogleMaterial.Icon.gmd_apps, R.color.colorBrandWhite).loadDrawable(this))
        toolbar.setOnLogoClickEvent {
            this@DeviceActivity.drawerLayout1.openDrawer(GravityCompat.START)
        }

        getFragmentBook(R.id.fragment_holder_dev_1).groupTopFragmentRequest = { group ->
            when (group) {
                TAB_PASSWORDS_TREE_ROOT -> passwordFragment
                else -> throw IllegalArgumentException("Fragment group $group not exist")
            }
        }

        this.navView1.menu.clear()
        this.navView1.setNavigationItemSelectedListener { menuItem -> onMenuClickBase(menuItem) }
        IconicsMenuInflaterUtil.inflate(menuInflater, this, R.menu.device_navigation_view, this.navView1.menu)

        fixPaddingTopForNavigationView()

        this.logoutIcon1.setImageIcon(generateIcon(GoogleMaterial.Icon.gmd_exit_to_app, R.color.colorBrandWhite, 20))
        logoutTextView1.setOnClickListener {
            KnownDevicesDao.getInstance(connectionSource).clearDefault()
            this@DeviceActivity.finish()
        }

        this.navView1.menu.findItem(R.id.menuItemPasswords).isChecked = true
        getFragmentBook(R.id.fragment_holder_dev_1).showTopInGroup(TAB_PASSWORDS_TREE_ROOT)
    }
}
