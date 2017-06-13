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
public class Utils {
    private static EthernetCRC32 crcUnit = new EthernetCRC32();
    
    public static int genCRC32(ByteBuffer data) {
        crcUnit.reset();
        crcUnit.update(data);
        return crcUnit.getValue();
    }
    
    
}
