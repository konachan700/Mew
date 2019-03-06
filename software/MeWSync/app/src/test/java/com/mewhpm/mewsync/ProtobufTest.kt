package com.mewhpm.mewsync

import com.mewhpm.mewsync.data.generated.PassRecordDTO
import org.junit.Assert
import org.junit.Test

class ProtobufTest {
    @Test
    fun testSerialize() {
        val bytes = PassRecordDTO.record.newBuilder()
            .setDisplayName("password for my forum")
            .setHwID(100L)
            .setPackageId(1)
            .setLogin("test-login-1")
            .setUrl("http://www.myforum.lan/4265345")
            .setHasAlphaLowercase(true)
            .setHasDigit(true)
            .setHasAlphaUppercase(true)
            .setHasSymbols(false)
            .setMaxLen(20)
            .setMinLen(10)
            .setType(1024)
            .build().toByteArray()
        Assert.assertNotNull(bytes)
        System.err.println("Serialized DTO size: ${bytes.size} bytes")

        val dto2 = PassRecordDTO.record.parseFrom(bytes)
        Assert.assertNotNull(dto2)
        Assert.assertEquals(dto2.displayName, "password for my forum")
        Assert.assertEquals(dto2.type, 1024)
    }
}