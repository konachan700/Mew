package com.mewhpm.mewsync

import androidx.test.runner.AndroidJUnit4
import com.mewhpm.mewsync.utils.CryptoUtils
import org.junit.Assert.*
import org.junit.Test
import org.junit.runner.RunWith
import java.nio.charset.StandardCharsets

@RunWith(AndroidJUnit4::class)
class CryptoInstrumentedTest {
    @Test
    fun rsa4k_test() {
        val testString = "Test 12345 Test 12345 Test 12345 Test 12345 Test 12345 Test 12345 Test 12345 Test 12345"
        val encrypted = CryptoUtils.encryptRSA(testString.toByteArray())
        val decrypted = CryptoUtils.decryptRSA(encrypted)

        assertEquals(testString, decrypted.toString(StandardCharsets.UTF_8))
    }

    @Test
    fun sha256_test() {
        val originalHash = "9822b4afe3b5e3e15aa3b2145926f937dedb26c849414982b7d7b17a920bc2ac"
        val hash = CryptoUtils.sha256("83741623874782548235453284567354715635223457354234534")

        assertEquals(originalHash, hash)
    }

    @Test
    fun getUniqueSalt_test() {
        val salt1 = CryptoUtils.getUniqueSalt()
        val salt2 = CryptoUtils.getUniqueSalt()

        assertNotNull(salt1)
        assertTrue(salt1.isNotBlank())
        assertEquals(salt1, salt2)
    }
}
