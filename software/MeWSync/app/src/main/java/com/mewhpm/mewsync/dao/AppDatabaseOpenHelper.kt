package com.mewhpm.mewsync.dao

import android.content.Context
import android.database.sqlite.SQLiteDatabase
import org.jetbrains.anko.db.*
import com.j256.ormlite.android.AndroidConnectionSource
import com.j256.ormlite.support.ConnectionSource

class AppDatabaseOpenHelper (ctx: Context) : ManagedSQLiteOpenHelper(ctx, "MeWSync", null, 1) {
    companion object {
        private var instance: AppDatabaseOpenHelper? = null
        private var connectionSource: ConnectionSource? = null

        @Synchronized
        fun getInstance(ctx: Context): AppDatabaseOpenHelper {
            if (instance == null) {
                instance = AppDatabaseOpenHelper(ctx.applicationContext)
            }
            return instance!!
        }

        @Synchronized
        fun getConnectionSource(ctx: Context): ConnectionSource {
            if (connectionSource == null) {
                connectionSource = AndroidConnectionSource(getInstance(ctx))
            }
            return connectionSource!!
        }
    }

    override fun onCreate(db: SQLiteDatabase) { }
    override fun onUpgrade(db: SQLiteDatabase, oldVersion: Int, newVersion: Int) {
        val c = db.rawQuery("SELECT name FROM sqlite_master WHERE type='table'", null)
        while (c.moveToNext()) db.execSQL("DROP TABLE IF EXISTS $c.getString(0)")
        c.close()
    }
}

val Context.database: AppDatabaseOpenHelper
    get() = AppDatabaseOpenHelper.getInstance(applicationContext)

val Context.connectionSource: ConnectionSource
    get() = AppDatabaseOpenHelper.getConnectionSource(applicationContext)