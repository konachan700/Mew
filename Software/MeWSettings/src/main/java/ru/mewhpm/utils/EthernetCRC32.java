package ru.mewhpm.utils;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class EthernetCRC32 {
    int crc = 0xFFFFFFFF;
    
    private final int[] crcFastTable = { 
        0x00000000,0x04C11DB7,0x09823B6E,0x0D4326D9,
        0x130476DC,0x17C56B6B,0x1A864DB2,0x1E475005,
        0x2608EDB8,0x22C9F00F,0x2F8AD6D6,0x2B4BCB61,
        0x350C9B64,0x31CD86D3,0x3C8EA00A,0x384FBDBD
    };
     
    public void reset() {
        crc = 0xFFFFFFFF;
    }
     
    public void update(ByteBuffer bb) {
        if((bb.capacity() % 4) != 0 ) return;

        bb.position(0);
        bb.order(ByteOrder.LITTLE_ENDIAN);
        
        while(bb.hasRemaining()) crc = CRC32Round(crc, bb.getInt()); 
    }
     
    public void update(int[] data) {
        if((data.length % 4) != 0) {
            System.out.println("Bad CRC update size: " + data.length );
            reset();
            return;
        }
         
        int i, Hold;
         
        for (i = 0; i < data.length; i += 4) {
            Hold = data[i+0] + (data[i+1] << 8) + (data[i+2] << 16) + (data[i+3] << 24);
            crc = CRC32Round(crc, Hold);
        }     
    }
     
    public int getValue() {
        return crc;
    }
 
    private int CRC32Round(int Crc, int Data) {
      Crc = Crc ^ Data;
      
      Crc = (Crc << 4) ^ crcFastTable[(Crc >> 28) & 0x0F]; 
      Crc = (Crc << 4) ^ crcFastTable[(Crc >> 28) & 0x0F]; 
      Crc = (Crc << 4) ^ crcFastTable[(Crc >> 28) & 0x0F];
      Crc = (Crc << 4) ^ crcFastTable[(Crc >> 28) & 0x0F];
      Crc = (Crc << 4) ^ crcFastTable[(Crc >> 28) & 0x0F];
      Crc = (Crc << 4) ^ crcFastTable[(Crc >> 28) & 0x0F];
      Crc = (Crc << 4) ^ crcFastTable[(Crc >> 28) & 0x0F];
      Crc = (Crc << 4) ^ crcFastTable[(Crc >> 28) & 0x0F];
      
      return(Crc);
    }
}

