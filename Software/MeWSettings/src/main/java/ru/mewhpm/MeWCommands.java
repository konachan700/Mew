package ru.mewhpm;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import jssc.SerialPort;
import jssc.SerialPortException;
import ru.mewhpm.utils.EthernetCRC32;

public class MeWCommands {
    public static final int 
            MEW_CM_PING                    = 0xAA,
            MEW_CM_IDENT                   = 0x01,
            MEW_CM_READ_PASSWD             = 0x02,
            MEW_CM_WRITE_PASSWD            = 0x03,
            MEW_CM_CONFIG_READ             = 0x04,
            MEW_CM_CONFIG_WRITE            = 0x05,
            
            MEW_CM_RET_OK                  = 0x00,
            MEW_CM_RET_ERROR               = 0x01,
            MEW_CM_RET_PING                = 0x02,
            MEW_CM_RET_CRC_FAIL             = 0x03,
            MEW_CM_RET_DATA_ACCESS_FAIL     = 0x04,
            
            MEW_CRC_RET_OK                  = 1,
            MEW_CRC_RET_FAIL                = 0,
            
            MEW_CM_COMMAND_SIZE             = 3;
    
    private static final EthernetCRC32
            crc32 = new EthernetCRC32();
    
    private static final ByteBuffer 
            sendBuf = ByteBuffer.allocate(1024);
    
    private static final byte
            senBufX[] = new byte[1024];
    
    static {
        sendBuf.order(ByteOrder.LITTLE_ENDIAN);
    }
    

    
    private static void sendCmd(SerialPort sp, int cmd, byte[] data) throws IOException, SerialPortException {
        if ((sp == null) || (!sp.isOpened())) throw new IOException("port must be a ready!");
        
        if (data != null) {
            final ByteBuffer bb = ByteBuffer.wrap(data);
            bb.order(ByteOrder.LITTLE_ENDIAN);
            crc32.reset();
            crc32.update(bb);
            
            sendBuf
                    .putInt(cmd)
                    .putInt(crc32.getValue())
                    .putInt(data.length)
                    .put(bb); 
            for (int i=0; i<((MEW_CM_COMMAND_SIZE * Integer.BYTES)+data.length); i++) sp.writeByte(sendBuf.get(i));
        } else {
            sendBuf
                    .putInt(cmd)
                    .putInt(0)
                    .putInt(0);   
            for (int i=0; i<(MEW_CM_COMMAND_SIZE * Integer.BYTES); i++) sp.writeByte(sendBuf.get(i));
        }

        //sp.purgePort(PURGE_RXCLEAR | PURGE_TXCLEAR);
        sendBuf.clear();
    }
    
    public static void sendCmdPing(SerialPort sp) throws IOException, SerialPortException {
        if ((sp == null) || (!sp.isOpened())) throw new IOException("port must be a ready!");
        sendCmd(sp, MEW_CM_PING, null);
    }
    
    
    public static void sendCmdConfigRead(SerialPort sp, int configNumber) throws IOException, SerialPortException {
        if ((configNumber > 255) || (configNumber < 0)) throw new IOException("invalid config index!");
        sendCmd(sp, MEW_CM_CONFIG_READ, ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(configNumber).array());
    }
    
    
}
