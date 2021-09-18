package one.papachi.tapi4j;

import java.io.IOException;
import java.nio.ByteBuffer;

public class TapDevice {

	protected final String deviceName;// "\\\\.\\{A6215D55-1B39-4C4E-B56E-250AB857A90A}.tap"

	protected long deviceHandle = -1;

	protected TapDevice(String deviceName) {
		this.deviceName = deviceName;
	}

	public String getDeviceName() {
		return deviceName;
	}

	public void open() throws IOException {
		deviceHandle = TapI4j.open(deviceName);
	}

	public boolean isOpen() {
		return deviceHandle != -1;
	}

	public int read(ByteBuffer dst) throws IOException {
		return TapI4j.read(deviceHandle, dst);
	}

	public int write(ByteBuffer src) throws IOException {
		return TapI4j.write(deviceHandle, src);
	}

	public void close() throws IOException {
		TapI4j.close(deviceHandle);
		deviceHandle = -1;
	}

	public void setIPAddress(String ipAddress, String ipMask) throws IOException {
		TapI4j.setIPAddress(deviceName, deviceHandle, ipAddress, ipMask);
	}

	public void setStatus(boolean isUp) throws IOException {
		TapI4j.setStatus(deviceName, deviceHandle, isUp);
	}

	public byte[] getMAC() throws IOException {
		return TapI4j.getMACAddress(deviceName, deviceHandle);
	}

	public void setMAC(byte[] mac) throws IOException {
		TapI4j.setMACAddress(deviceName, deviceHandle, mac);
	}

	public int getMTU() throws IOException {
		return TapI4j.getMTU(deviceName, deviceHandle);
	}

	public void setMTU(int mtu) throws IOException {
		TapI4j.setMTU(deviceName, deviceHandle, mtu);
	}

}
