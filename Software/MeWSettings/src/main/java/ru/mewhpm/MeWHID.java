package ru.mewhpm;

import java.io.ByteArrayOutputStream;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.Arrays;
import org.hid4java.HidDevice;
import org.hid4java.HidManager;
import org.hid4java.HidServices;
import org.hid4java.HidServicesListener;
import org.hid4java.event.HidServicesEvent;

public class MeWHID implements HidServicesListener, Runnable {
    private final HidServices hidSvc;
    
    private volatile boolean 
            threadStop = false;
    
    private volatile HidDevice
            devMeW = null;
    
    private final ByteArrayOutputStream
            devReadBuf = new ByteArrayOutputStream();
    
    private final Thread 
            thisThread = new Thread(this);
    
    private final int allowedCMDs[] = {
        MeWCommands.MEW_CM_RET_OK,
        MeWCommands.MEW_CM_RET_ERROR,
        MeWCommands.MEW_CM_RET_PING,
        MeWCommands.MEW_CM_RET_CRC_FAIL,
        MeWCommands.MEW_CM_RET_DATA_ACCESS_FAIL
    };
    
    public MeWHID() {
        hidSvc = HidManager.getHidServices();
    }
    
    public void init() {
        hidSvc.addHidServicesListener(this);
        hidSvc.start();
        thisThread.start();
    }
    
    public void dispose() {
        threadStop = true;
    }
    
    public boolean write(int reportID, byte[] data, int len) {
        if ((devMeW == null) || (!devMeW.isOpen())) return false;
        byte b[] = new byte[64];
        for (int i=0; i<64; i++) {
            if (i < data.length) b[i] = data[i]; else b[i] = 0;
        }

        return (devMeW.write(b, 64, (byte) 0) > 0);
    }
    
    public boolean waitForDevice(long maxMillis) {
        long time = System.currentTimeMillis() + maxMillis;
        while ((devMeW == null) || (!devMeW.isOpen())) {
            if (time < System.currentTimeMillis()) return false;
        }
        return true;
    }
    
    private boolean containCMD(byte[] data) {
        final ByteBuffer bb = ByteBuffer.allocate(4);
        bb.order(ByteOrder.LITTLE_ENDIAN);
        
        for (int cmd : allowedCMDs) {
            if ((data != null) && (data.length >= 4)) {
                bb.clear();
                bb.put(data, 0, 4);
          
                if (bb.getInt(0) == cmd) return true; 
            }
        }
        return false;
    }
    
    @Override
    public void hidDeviceAttached(HidServicesEvent hse) {
        System.err.println("hidDeviceAttached. Time:"+System.currentTimeMillis());
    }

    @Override
    public void hidDeviceDetached(HidServicesEvent hse) {
        System.err.println("hidDeviceDetached. Time:"+System.currentTimeMillis());
    }

    @Override
    public void hidFailure(HidServicesEvent hse) {
        System.err.println("hidFailure. Time:"+System.currentTimeMillis());
    }

    @Override
    public void run() {
        long time = 0;
        int lastBufSize = 0;
        
        while (true) {
            if (threadStop) {
                if ((devMeW != null) && (devMeW.isOpen())) devMeW.close();
                hidSvc.removeUsbServicesListener(this);
                hidSvc.stop();
                return;
            }
            
            if (devMeW == null) {
                if (time < System.currentTimeMillis()) {
                    time = System.currentTimeMillis() + 1000;
                    hidSvc.getAttachedHidDevices().stream().filter(
                            (hidDevice) -> ((hidDevice.getVendorId() == 0x4343) && (hidDevice.getProductId() == 0x4343))).forEach((hidDevice) -> {
                        devMeW = hidDevice;
                    });

                    if (devMeW != null) {
                        devMeW.open();
                    }
                }
            } else {
                if (devMeW.isOpen()) {
                    final byte buf[] = new byte[64];
                    final int len = devMeW.read(buf, 1);
                    if (len > 0) {
                        devReadBuf.write(buf, 0, len);

                        if (devReadBuf.size() > 768) {
                            devReadBuf.reset();
                            System.err.println("TOO MANY DATA. len: "+len+"; time:"+System.currentTimeMillis());
                        }
                        
                        if ((!containCMD(devReadBuf.toByteArray())) && (devReadBuf.size() >= 4)) {
                            System.err.println("BAD DATA INPUT. len: "+len+"; bs: "+devReadBuf.size()+"; time:"+System.currentTimeMillis()+"; arr = "+Arrays.toString(devReadBuf.toByteArray()));
                            devReadBuf.reset();
                        }
                    }
                } else {
                    //devMeW = null;
                }

                if (lastBufSize != devReadBuf.size()) {
                    System.err.println("New data. len: "+devReadBuf.size()+";");
                    
                    if (MeWCommands.commandPoll(devReadBuf.toByteArray()) != MeWRetCode.NO_ACTION) {
                        devReadBuf.reset();
                        System.err.println("Command executed. cmd: "+MeWCommands.getCurrentCommand()+"; time:"+System.currentTimeMillis());
                    }

                    lastBufSize = devReadBuf.size();
                }
            }
        }
    }
}
