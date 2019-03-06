package com.mewhpm.mewsync.ui.recyclerview.impl

import android.content.Context
import android.util.AttributeSet
import androidx.core.content.ContextCompat
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.data.DisplayablePassRecord
import com.mewhpm.mewsync.data.LevelUpRecord
import com.mewhpm.mewsync.data.PassRecord
import com.mewhpm.mewsync.data.mappers.PassRecordMapper
import com.mewhpm.mewsync.ui.recyclerview.RecyclerViewAbstract
import com.mewhpm.mewsync.ui.recyclerview.data.TextPairWithIcon
import com.mikepenz.google_material_typeface_library.GoogleMaterial
import org.jetbrains.anko.selector
import java.util.*

class RecyclerViewPasswordsTreeImpl : RecyclerViewAbstract<DisplayablePassRecord> {
    constructor(context: Context) : super(context)
    constructor(context: Context, attributeSet: AttributeSet?) : super(context, attributeSet)
    constructor(context: Context, attrs: AttributeSet?, defStyleAttr: Int) : super(context, attrs, defStyleAttr)

    companion object {
        const val ICON_SIZE_DP = 48
        const val ICON_COLOR_RES_ID = R.color.colorBrandDarkGray
    }

    private val backToParentItem = TextPairWithIcon(
        icon = GoogleMaterial.Icon.gmd_subdirectory_arrow_left,
        iconColor = ContextCompat.getColor(context, R.color.colorBrandBlack),
        iconSize = ICON_SIZE_DP,
        text = "Back to parent element",
        textColor = ContextCompat.getColor(context, ICON_COLOR_RES_ID),
        title = "...",
        titleColor = ContextCompat.getColor(context, R.color.colorBrandBlack)
    )
    private val passRecordForBackItem = LevelUpRecord()
    private val list = ArrayList<Pair<TextPairWithIcon, DisplayablePassRecord>>()

    var onDeleteEvent : (record : PassRecord) -> Unit = {}
    var onEditEvent : (record : PassRecord) -> Unit = {}

    var onItemClickEvent : (record : PassRecord) -> Unit = {}
    var onBackEvent : () -> Unit = {}

    fun addItems(items: List<PassRecord>, isRoot: Boolean = false) {
        list.clear()
        val endList : List<Pair<TextPairWithIcon, DisplayablePassRecord>> = items.map { PassRecordMapper.toTextPairWithIconForPasswordsList(it, context) }


        if (!isRoot) list.add(Pair(backToParentItem, passRecordForBackItem))
        list.addAll(endList)
        this.adapter?.notifyDataSetChanged()
    }

    override fun requestList(): ArrayList<Pair<TextPairWithIcon, DisplayablePassRecord>> = list
    override fun onElementClick(position: Int, item: TextPairWithIcon, obj: DisplayablePassRecord) {
        if (obj is LevelUpRecord) {
            onBackEvent.invoke()
            return
        }

        if (obj is PassRecord) onItemClickEvent.invoke(obj)
    }

    override fun onElementLongClick(position: Int, item: TextPairWithIcon, obj: DisplayablePassRecord) {
        if (obj !is PassRecord) return

        val actions = listOf("Edit", "Delete")
        context.selector("Actions", actions) { _, index ->
            when (index) {
                0 -> { onEditEvent.invoke(obj) }
                1 -> { onDeleteEvent.invoke(obj) }
            }
        }
    }

    override fun create() {
        super.create()
        this.adapter = TextPairWithIconAdapterImpl()
    }
}