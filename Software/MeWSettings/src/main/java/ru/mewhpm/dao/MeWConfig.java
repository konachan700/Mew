package ru.mewhpm.dao;

import com.jneko.jnekouilib.anno.*;
import java.io.Serializable;
import java.util.Set;
import javax.persistence.*;

@Entity
@UIListItem
@UILibDataSource
public class MeWConfig implements Serializable {
	@Id
	@GeneratedValue(strategy=GenerationType.AUTO)
	@Column(name="ID", unique = true, nullable = false)
	private long ID;

	@Column(name="deviceName", unique = false, nullable = true, length = 255)
	private String deviceName;
        
	@Column(name="lastSerialPort", unique = false, nullable = true, length = 255)
	private String lastSerialPort;
        
        @Transient
	private Set serialPort;

	@UISortIndex(index=-100)
	@UILongField(name="DBID", type=UIFieldType.GETTER, readOnly=1, labelText="DB ID")
	public long getID() {
		return ID;
	}

	@UILongField(name="DBID", type=UIFieldType.SETTER)
	public void setID(long ID) {
		this.ID = ID;
	}

	@UISortIndex(index=1)
	@UIStringField(name="deviceName", type=UIFieldType.GETTER, readOnly=0, maxChars=255, helpText="Enter text here", labelText="MeW device name")
	public String getDeviceName() {
		return this.deviceName;
	}

	@UIStringField(name="deviceName", type=UIFieldType.SETTER)
	public void setDeviceName(String deviceName) {
		this.deviceName = deviceName;
	}

	@UISortIndex(index=3)
	@UICollection(name="serialPort", type=UIFieldType.GETTER, multiSelect=0, yesNoBoxPresent=1, text="MeW serial port")
	public Set getSerialPort() {
		return this.serialPort;
	}

	@UICollection(name="serialPort", type=UIFieldType.SETTER)
	public void setSerialPort(Set serialPort) {
		this.serialPort = serialPort;
	}

	@UISortIndex(index=2)
	@UIStringField(name="lastSerialPort", type=UIFieldType.GETTER, readOnly=0, maxChars=255, helpText="Enter text here", labelText="Selected serial port")
        public String getLastSerialPort() {
            return lastSerialPort;
        }

        @UIStringField(name="lastSerialPort", type=UIFieldType.SETTER)
        public void setLastSerialPort(String lastSerialPort) {
            this.lastSerialPort = lastSerialPort;
        }
}
