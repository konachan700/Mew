package ru.mewhpm;

import com.jneko.jnekouilib.appmenu.AppMenuGroup;
import com.jneko.jnekouilib.appmenu.AppMenuItem;
import com.jneko.jnekouilib.editor.Editor;
import com.jneko.jnekouilib.utils.MessageBus;
import com.jneko.jnekouilib.utils.MessageBusActions;
import com.jneko.jnekouilib.windows.UIDialog;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import javafx.application.Application;
import static javafx.application.Application.launch;
import javafx.application.Platform;
import javafx.stage.Stage;
import org.apache.commons.lang3.SystemUtils;
import ru.mewhpm.utils.HibernateUtil;

public class MainApp extends Application {
    private volatile boolean 
            app_exit = false;
    
    private final MeWHID 
            mewHID = new MeWHID();

    private final Runnable 
            hid_req_thread = new Runnable() {
        @Override
        public void run() {
           
            while (true) {
                

            }
        }
    };
    
    private final UIDialog 
            uWin = new UIDialog(950, 700, true, true, "MeW HPM settings tool");
    
    private final Editor 
            eMeWConfigUI = new Editor();
    
    @Override
    public void start(Stage stage) {
        try {
            HibernateUtil.init("mew", "mew", "mew");
            mewHID.init();
        } catch (IOException ex) {
            Logger.getLogger(MainApp.class.getName()).log(Level.SEVERE, null, ex);
            Platform.exit();
        }
        
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
            
        });
        
        uWin.addCustomStyle("/styles/style.css");
        uWin.addLogoFromResources("/styles/hpm-app-logo.png");
        
/*        eMeWConfigUI.setHost(uWin.getRootFragment()); 
        eMeWConfigUI.addCollectionHelper("serialPort", alSerialPorts);
//        eMeWConfigUI.readObject(mewConf);
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
//                    HibernateUtil.saveObject(mewConf);
                })
        );*/
        
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
        
        new Thread(hid_req_thread).start();

        
        
        
        mewHID.waitForDevice(10 * 1000);
        
        MeWCommands.sendCmdPing(mewHID);
        MeWCommands.waitForData();
        
        System.out.println("sendCmdPing ok");
        
                MeWCommands.sendCmdPing(mewHID);
        MeWCommands.waitForData();
        
        System.out.println("sendCmdPing ok");
        
        MeWCommands.sendCmdPing(mewHID);
        MeWCommands.waitForData();
        
        System.out.println("sendCmdPing ok");
        
        MeWCommands.sendCmdPasswordsRead(mewHID, 0);
        MeWCommands.waitForData();
        
        System.out.println("sendCmdPasswordsRead ok");
/*
        
        try {
            MeWCommands.sendCmdPing(mewDevice);
            MeWCommands.waitForData();

            MeWCommands.sendCmdConfigRead(mewDevice, 0);
            if (MeWCommands.waitForData() && (MeWCommands.getCommandResult() == MeWRetCode.OK)) {                       
                System.out.println(Arrays.toString((MeWCommands.getData().array())));

            }

            MeWCommands.sendCmdPasswordsRead(mewDevice, 0);
            if (MeWCommands.waitForData() && (MeWCommands.getCommandResult() == MeWRetCode.OK)) {                       
                System.out.println(Arrays.toString((MeWCommands.getData().array())));

            }
            
            
        } catch (IOException ex) {
            Logger.getLogger(MainApp.class.getName()).log(Level.SEVERE, null, ex);
        }
*/
        
        uWin.showAndWait();
        
        app_exit = true;
        
        mewHID.dispose();
        HibernateUtil.dispose();
    }

    public static void main(String[] args) {
        launch(args);
    }



}
