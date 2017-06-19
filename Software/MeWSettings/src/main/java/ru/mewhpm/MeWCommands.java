package ru.mewhpm;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import ru.mewhpm.utils.EthernetCRC32;

public class MeWCommands {
    public static final int 
            MEW_CM_PING                    = 0xAA,
            MEW_CM_IDENT                   = 0x01,
            MEW_CM_READ_PASSWD             = 0x02,
            MEW_CM_WRITE_PASSWD            = 0x03,
            MEW_CM_CONFIG_READ             = 0x04,
            MEW_CM_CONFIG_WRITE            = 0x05,
            MEW_CM_LOGIN                   = 0x06,
            MEW_CM_LOGOUT                  = 0x07,
            
            MEW_CM_RET_OK                  = 0x00,
            MEW_CM_RET_ERROR               = 0x01,
            MEW_CM_RET_PING                = 0x02,
            MEW_CM_RET_CRC_FAIL            = 0x03,
            MEW_CM_RET_DATA_ACCESS_FAIL    = 0x04,
            
            MEW_CRC_RET_OK                 = 1,
            MEW_CRC_RET_FAIL               = 0,
            
            MEW_CM_COMMAND_SIZE            = 3,
            
            /********************************************/
            
            MEW_COMM_TIMEOUT               = 19500,
            STM32_INT_SIZE                 = 4;
    
    private static final EthernetCRC32
            crc32 = new EthernetCRC32();
    
    private static final ByteBuffer 
            sendBuf = ByteBuffer.allocate(1024);
    
    private static volatile ByteBuffer 
            dataBuf = null;
    
    private static volatile int 
            currentCommand = 0;
    
    private static volatile boolean
            commandWait = false;
    
    private static volatile MeWRetCode
            lastCommandResult = MeWRetCode.NO_ACTION;

    static {
        sendBuf.order(ByteOrder.LITTLE_ENDIAN);
    }
    
    public static MeWRetCode commandPoll(byte[] rawData) {
        ByteBuffer recvBuf = ByteBuffer.wrap(rawData);
        recvBuf.order(ByteOrder.LITTLE_ENDIAN);
        
        if (rawData.length < (MEW_CM_COMMAND_SIZE * STM32_INT_SIZE)) 
            return MeWRetCode.NO_ACTION;
        
        lastCommandResult = MeWRetCode.NO_ACTION;
            
        final int 
                cmd = recvBuf.getInt(0 * STM32_INT_SIZE),
                crc = recvBuf.getInt(1 * STM32_INT_SIZE),
                len = recvBuf.getInt(2 * STM32_INT_SIZE);
        
        System.out.println("CMD: "+cmd+"; CRC32: "+crc+"; LEN:"+len+"; rb="+rawData.length);
        
        switch (cmd) {
            case MEW_CM_RET_OK:
                if (rawData.length >= ((MEW_CM_COMMAND_SIZE * STM32_INT_SIZE) + len)) {
                    dataBuf = ByteBuffer.allocate(len);
                    for (int i=0; i<len; i++)
                        dataBuf.put(recvBuf.get(i+(MEW_CM_COMMAND_SIZE * STM32_INT_SIZE)));
                    
                    crc32.reset();
                    crc32.update(dataBuf);

                    if (crc == crc32.getValue()) {
                        lastCommandResult = MeWRetCode.OK;
                    } else {
                        lastCommandResult = MeWRetCode.CRC_FAIL;
                    }
                    commandWait = false;
                }
                break;
            case MEW_CM_RET_PING:
                commandWait = false;        
                lastCommandResult = MeWRetCode.PING;
            case MEW_CM_RET_ERROR:
            case MEW_CM_RET_CRC_FAIL:
            case MEW_CM_RET_DATA_ACCESS_FAIL:
                commandWait = false;
                lastCommandResult = MeWRetCode.ERROR;
        }

        return lastCommandResult;
    }
    
    public static ByteBuffer getData() {
        return dataBuf; 
    }
    
    public static MeWRetCode getCommandResult() {
        return lastCommandResult;
    }
    
    public static boolean waitForData() {
        long time = System.currentTimeMillis();
        while (commandWait) {
            if (System.currentTimeMillis() > (time + MEW_COMM_TIMEOUT)) {
                System.out.println("DATA TIMEOUT");
                return false;
            }
        }
        return true;
    }
    
    public static int getCurrentCommand() {
        return currentCommand;
    }
    
    private static void sendCmd(MeWHID hd, int cmd, byte[] data) {
        commandWait = true;
        currentCommand = cmd;

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
            hd.write((byte) 0x00, sendBuf.array(), ((MEW_CM_COMMAND_SIZE * STM32_INT_SIZE)+data.length));
        } else {
            sendBuf
                    .putInt(cmd)
                    .putInt(0)
                    .putInt(0);   
            hd.write((byte) 0x00, sendBuf.array(), (MEW_CM_COMMAND_SIZE * STM32_INT_SIZE));
        }

        sendBuf.clear();
    }
    
    public static void sendCmdPing(MeWHID hd) {
        sendCmd(hd, MEW_CM_PING, null);
    }
    
    
    public static void sendCmdConfigRead(MeWHID hd, int configNumber)  {
        sendCmd(hd, MEW_CM_CONFIG_READ, ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(configNumber).array());
    }
    
    public static void sendCmdPasswordsRead(MeWHID hd, int passNumber) {
        sendCmd(hd, MEW_CM_READ_PASSWD, ByteBuffer.allocate(4).order(ByteOrder.LITTLE_ENDIAN).putInt(passNumber).array());
    }
    
}
