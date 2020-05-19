package com.mewhpm.mewsync.security

import android.content.Context
import android.content.SharedPreferences
import android.preference.PreferenceManager
import android.util.Base64
import com.mewhpm.mewsync.data.meta.PassRecordMetadata
import com.mewhpm.mewsync.utils.CryptoUtils
import java.io.ByteArrayOutputStream
import java.security.MessageDigest
import java.security.SecureRandom

class LocalPasswordProviderImpl : PasswordProvider {
    companion object {
        const val MEW_DEVKEY = "mew_devkey"
        const val MEW_DEVKEY_SIZE = 1024

        const val ITERATION_COUNT = 64
        const val SALT = "wegr8h7^^%FR&%C5786rtv8i7o8b79yuvbwevf723v86fc5e5dx%$#@AZ@#%AXCTUVB*)N" // Please, change salt if fork me

        private val numbers = arrayListOf('1', '2', '3', '4', '5', '6', '7', '8', '9', '0')
        private val symbols = arrayListOf('~', '`', '!', '@', '#', '$', '%', '^', '&', '*', '<', '>',
            '(', ')', '-', '_', '+', '=', '.', ',', '"', '\'', '|', '/', '?', ' ', ':', ';')
        private val alphabetLower = arrayListOf(
            'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o',
            'p', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'z', 'x', 'c', 'v', 'b', 'n', 'm')
        private val alphabetUpper = alphabetLower.map { it.toUpperCase() }

        private var instance: LocalPasswordProviderImpl? = null
        fun getInstance(): LocalPasswordProviderImpl {
            if (instance == null) instance = LocalPasswordProviderImpl()
            return instance!!
        }
    }

    var meta: PassRecordMetadata? = null

    private val _uniqueDevkey = SecureRandom().generateSeed(MEW_DEVKEY_SIZE)

    private fun getUniqueKey(c : Context): ByteArray {
        val pref: SharedPreferences = PreferenceManager.getDefaultSharedPreferences(c)
        if (!pref.contains(MEW_DEVKEY)) {
            val sr = SecureRandom().generateSeed(MEW_DEVKEY_SIZE)
            val encrypted = CryptoUtils.encryptRSA(sr)
            val randomPool = Base64.encodeToString(encrypted, Base64.DEFAULT)
            pref.edit()
                .putString(MEW_DEVKEY, randomPool)
                .apply()
        }

        val devkey = Base64.decode(pref.getString(MEW_DEVKEY, null), Base64.DEFAULT)
        return CryptoUtils.decryptRSA(devkey)
    }

    private fun hash(prev: ByteArray?, uniqueDevkey: ByteArray?, passId: Long, deviceId: String, counter: Int): ByteArray {
        val md = MessageDigest.getInstance("SHA-512")
        if (prev != null) md.update(prev)
        md.update(SALT.toByteArray())
        md.update("passID:$passId;counter:$counter/${counter % 11}/${counter % 3}".toByteArray())
        md.update(deviceId.toByteArray())
        md.update(SALT.toByteArray())
        md.update(uniqueDevkey)
        return md.digest()
    }

    private fun dia(value : Byte, max : Int) : Byte {
        return (value.toDouble().minus(Byte.MIN_VALUE.toDouble()))
            .div(Byte.MAX_VALUE.toDouble() - Byte.MIN_VALUE.toDouble()).times(max.minus(1).toDouble()).toByte()
    }

    override fun generatePassword(c: Context?, passId: Long, deviceId: String, metadata: PassRecordMetadata?) : String {
        val uniqueDevkey = if (c == null)
            _uniqueDevkey // FOR TESTING ONLY
        else
            getUniqueKey(c)

        meta = metadata ?: PassRecordMetadata()

        val baos = ByteArrayOutputStream()
        var tmpHash: ByteArray? = null
        for (i in 0..ITERATION_COUNT) {
            tmpHash = hash(tmpHash, uniqueDevkey, passId, deviceId, i)
            baos.write(tmpHash)
        }

        val list = ArrayList<Char>()
        if (meta!!.containDigits) list.addAll(numbers)
        if (meta!!.containSymbols) list.addAll(symbols)
        if (meta!!.containLowercaseAlphabet) list.addAll(alphabetLower)
        if (meta!!.containUppercaseAlphabet) list.addAll(alphabetUpper)
        if (list.size > Byte.MAX_VALUE) throw IllegalStateException("Password may contain maximum ${Byte.MAX_VALUE} different chars!")

        val normIndexArray = baos.toByteArray().map { dia(it, list.size) }
        val count = dia(tmpHash!![0], (meta!!.maxLen - meta!!.minLen)).plus(meta!!.minLen)

        val sb = StringBuilder()
        for (i in 0..count) {
            sb.append(list[normIndexArray[i % normIndexArray.size].toInt()])
        }

        return sb.toString()
    }
}