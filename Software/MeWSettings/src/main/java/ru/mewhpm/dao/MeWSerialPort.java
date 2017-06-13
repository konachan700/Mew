package ru.mewhpm.dao;

import com.jneko.jnekouilib.anno.*;
import java.io.Serializable;

@UIListItem
@UILibDataSource
public class MeWSerialPort implements Serializable {
    private long ID;
    private String portName;

    @UISortIndex(index=-100)
    @UILongField(name="DBID", type=UIFieldType.GETTER, readOnly=1, labelText="DB ID")
    public long getID() {
            return ID;
    }

    @UILongField(name="DBID", type=UIFieldType.SETTER)
    public void setID(long ID) {
            this.ID = ID;
    }

    @UIListItemHeader
    @UISortIndex(index=1)
    @UITextArea(name="portName", type=UIFieldType.GETTER, readOnly=0, maxChars=4096, helpText="Serial port name", labelText="Serial port name")
    public String getPortName() {
            return this.portName;
    }

    @UITextArea(name="portName", type=UIFieldType.SETTER)
    public void setPortName(String portName) {
            this.portName = portName;
    }
}
