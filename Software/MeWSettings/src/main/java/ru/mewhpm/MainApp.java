package ru.mewhpm;

import com.jneko.jnekouilib.appmenu.AppMenuGroup;
import com.jneko.jnekouilib.appmenu.AppMenuItem;
import com.jneko.jnekouilib.editor.Editor;
import com.jneko.jnekouilib.panel.PanelButton;
import com.jneko.jnekouilib.utils.MessageBus;
import com.jneko.jnekouilib.utils.MessageBusActions;
import com.jneko.jnekouilib.windows.UIDialog;
import java.io.File;
import java.io.IOException;
import java.nio.file.DirectoryStream;
import java.nio.file.Files;
import java.nio.file.Path;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import static javafx.application.Application.launch;
import javafx.application.Platform;
import javafx.stage.Stage;
import jssc.SerialPort;
import jssc.SerialPortException;
import org.apache.commons.lang3.SystemUtils;
import ru.mewhpm.dao.MeWConfig;
import ru.mewhpm.dao.MeWSerialPort;
import ru.mewhpm.utils.HibernateUtil;

public class MainApp extends Application {
    private MeWConfig mewConf = null;
    private volatile boolean 
            app_exit = false,
            port_changed = true,
            mew_not_detected = false;
    
    private final Runnable serial_port_thread = () -> {
        SerialPort serialPort = null;
        while (true) {
            try{
                if (app_exit) {
                    if (serialPort != null) serialPort.closePort();
                    return;
                }

                if (mewConf == null) continue;
                if (mewConf.getLastSerialPort() == null) continue;
                if (mewConf.getLastSerialPort().length() <= 1) continue;

                if (port_changed) {
                    port_changed = false;
                    serialPort = new SerialPort(mewConf.getLastSerialPort());
                    serialPort.setParams(SerialPort.BAUDRATE_115200, SerialPort.DATABITS_8, SerialPort.STOPBITS_1, SerialPort.PARITY_NONE);
                    serialPort.setFlowControlMode(SerialPort.FLOWCONTROL_NONE);
                    serialPort.addEventListener((event) -> {

                        
                        
                        
                        

                    }, SerialPort.MASK_RXCHAR);
                    
                    
                    
                    
                    
                }
            } catch (SerialPortException ex) {
                
            }
        }
    };
    
    private final UIDialog 
            uWin = new UIDialog(950, 700, true, true, "MeW HPM settings tool");
    
    private final Set<MeWSerialPort> 
            alSerialPorts = new HashSet<>();
    
    private final Editor 
            eMeWConfigUI = new Editor();

    private void linuxGetSerialPortList() throws IOException {
        alSerialPorts.clear();
        final DirectoryStream<Path> stream = Files.newDirectoryStream(new File("/dev/").toPath());
        stream.forEach(path -> {
            if (path.toString().contains("ttyUSB") || path.toString().contains("ttyACM")) {
                final MeWSerialPort port = new MeWSerialPort();
                port.setPortName(path.getFileName().toString()); 
                alSerialPorts.add(port);
            }
        });
    }
    
    @Override
    public void start(Stage stage) {
        try {
            if (SystemUtils.IS_OS_LINUX) linuxGetSerialPortList();
            HibernateUtil.init("mew", "mew", "mew");
        } catch (IOException ex) {
            Logger.getLogger(MainApp.class.getName()).log(Level.SEVERE, null, ex);
            Platform.exit();
        }
        
        final List configs = HibernateUtil.getFullList(MeWConfig.class);
        
        mewConf = ((configs == null) || (configs.isEmpty())) ? new MeWConfig() : (MeWConfig) configs.get(0);
        mewConf.setSerialPort(alSerialPorts); 
        
        MessageBus.registerMessageReceiver(MessageBusActions.HibernateAddNew, (msg, object) -> {
            if (object[0] != null) HibernateUtil.saveObject(object[0]);
        });
        
        MessageBus.registerMessageReceiver(MessageBusActions.HibernateEdit, (msg, object) -> {
            if (object[0] != null) HibernateUtil.saveObject(object[0]);
        });
        
        MessageBus.registerMessageReceiver(MessageBusActions.HibernateDelete, (msg, object) -> {
            if (object[0] != null) HibernateUtil.removeObject(object[0]);
        });
        
        MessageBus.registerMessageReceiver(MessageBusActions.EFLElementClick, (msg, object) -> {
            final String serialPort = (String) object[0];
            //System.out.println(serialPort); 
            if (SystemUtils.IS_OS_LINUX) mewConf.setLastSerialPort("/dev/"+serialPort); 
            eMeWConfigUI.readObject(mewConf);
        });
        
        uWin.addCustomStyle("/styles/style.css");
        uWin.addLogoFromResources("/styles/hpm-app-logo.png");
        
        eMeWConfigUI.setHost(uWin.getRootFragment()); 
        eMeWConfigUI.addCollectionHelper("serialPort", alSerialPorts);
        eMeWConfigUI.readObject(mewConf);
        eMeWConfigUI.getPanel().addNodes(
                new PanelButton("iconRefresh", "Serial posts list refresh", e -> {
                    
                }),
                new PanelButton("iconFind", "Find MeW", e -> {
                    
                })
        );
        eMeWConfigUI.getPanel().addSeparator();
        eMeWConfigUI.getPanel().addNodes(
                new PanelButton("iconSave", "Save settings", e -> {
                    eMeWConfigUI.saveObject();
                    HibernateUtil.saveObject(mewConf);
                })
        );
        
        
        uWin.addMenu(
                new AppMenuGroup(),
                new AppMenuGroup(
                        "MeW HPM", "menuHeaderBlack", "menuHeaderIcon",
                        new AppMenuItem("Passwords", (c) -> {
                            
                        }).defaultSelected()
                        ,
                        new AppMenuItem("Settings", (c) -> {
                            uWin.showFragment(eMeWConfigUI, true);
                        })
                ),
                new AppMenuGroup(),
                new AppMenuGroup(
                        "MeW Cloud", "menuHeaderBlack", "menuHeaderIcon",
                        new AppMenuItem("Backup MeW", (c) -> {
                            
                        })
                        ,
                        new AppMenuItem("Restore MeW", (c) -> {
                            
                        })
                        ,
                        new AppMenuItem("MeW Account", (c) -> {
                            
                        })
                )
        );
        
        if (SystemUtils.IS_OS_WINDOWS) {
            System.err.println("Windows detected");
            
        } else if (SystemUtils.IS_OS_LINUX) {
            System.err.println("Linux detected");
            
        } else {
            System.err.println("Unsupported OS!");
            Platform.exit();
        }
        
        
        uWin.showAndWait();
        HibernateUtil.dispose();
    }

    public static void main(String[] args) {
        launch(args);
    }

}
