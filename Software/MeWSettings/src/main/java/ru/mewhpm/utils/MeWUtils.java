/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package ru.mewhpm.utils;

import java.nio.ByteBuffer;

/**
 *
 * @author misaki
 */
public class MeWUtils {
    private static final EthernetCRC32 crcUnit = new EthernetCRC32();
    private static final char[] hexArray = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
    
    public static int genCRC32(ByteBuffer data) {
        crcUnit.reset();
        crcUnit.update(data);
        return crcUnit.getValue();
    }
    
    public static String bytesToHex(byte[] bytes) {
        char[] hexChars = new char[bytes.length * 2];
        for ( int j = 0; j < bytes.length; j++ ) {
            int v = bytes[j] & 0xFF;
            hexChars[j * 2] = hexArray[v >>> 4];
            hexChars[j * 2 + 1] = hexArray[v & 0x0F];
        }
        return new String(hexChars);
    }
    
}
