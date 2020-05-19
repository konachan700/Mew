package com.mewhpm.mewsync.data.mappers

import android.content.Context
import androidx.core.content.ContextCompat
import com.mewhpm.mewsync.R
import com.mewhpm.mewsync.data.DisplayablePassRecord
import com.mewhpm.mewsync.data.enums.PassRecordType
import com.mewhpm.mewsync.ui.recyclerview.data.TextPairWithIcon
import com.mikepenz.google_material_typeface_library.GoogleMaterial

class PassRecordMapper {
    companion object {
        fun toTextPairWithIconForKeyboard(
            passRecord: DisplayablePassRecord,
            context: Context
        ): Pair<TextPairWithIcon, DisplayablePassRecord> {
            val iconColor = R.color.colorBrandBlack
            val iconSizeDp = 24
            val titleColor = R.color.colorBrandBlack

            return when (passRecord.recordType) {
                PassRecordType.DIRECTORY -> {
                    Pair(
                        TextPairWithIcon(
                            icon = GoogleMaterial.Icon.gmd_folder,
                            iconColor = ContextCompat.getColor(context, iconColor),
                            iconSize = iconSizeDp,
                            text = passRecord.text,
                            textColor = ContextCompat.getColor(context, iconColor),
                            title = passRecord.title,
                            titleColor = ContextCompat.getColor(context, titleColor)
                        ), passRecord
                    )
                }
                PassRecordType.PASSWORD -> {
                    Pair(
                        TextPairWithIcon(
                            icon = GoogleMaterial.Icon.gmd_label_outline,
                            iconColor = ContextCompat.getColor(context, iconColor),
                            iconSize = iconSizeDp,
                            text = "",
                            textColor = 0,
                            title = passRecord.title,
                            titleColor = ContextCompat.getColor(context, titleColor)
                        ), passRecord
                    )
                }
            }
        }

        fun toTextPairWithIconForPasswordsList(
            passRecord: DisplayablePassRecord,
            context: Context
        ): Pair<TextPairWithIcon, DisplayablePassRecord> {
            val iconColor = R.color.colorBrandDarkGray
            val iconSizeDp = 48
            val titleColor = R.color.colorBrandBlack
            val textColor = R.color.colorBrandDarkGray
            return when (passRecord.recordType) {
                PassRecordType.DIRECTORY -> {
                    Pair(
                        TextPairWithIcon(
                            icon = GoogleMaterial.Icon.gmd_folder,
                            iconColor = ContextCompat.getColor(context, iconColor),
                            iconSize = iconSizeDp,
                            text = "Directory",
                            textColor = ContextCompat.getColor(context, textColor),
                            title = passRecord.title,
                            titleColor = ContextCompat.getColor(context, titleColor)
                        ), passRecord
                    )
                }
                PassRecordType.PASSWORD -> {
                    Pair(
                        TextPairWithIcon(
                            icon = GoogleMaterial.Icon.gmd_dialpad,
                            iconColor = ContextCompat.getColor(context, iconColor),
                            iconSize = iconSizeDp,
                            text = "Password",
                            textColor = ContextCompat.getColor(context, textColor),
                            title = passRecord.title,
                            titleColor = ContextCompat.getColor(context, titleColor)
                        ), passRecord
                    )
                }
            }
        }

    }
}