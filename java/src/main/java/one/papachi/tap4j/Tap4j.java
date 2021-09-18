package one.papachi.tapi4j;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.nio.ByteBuffer;
import java.nio.file.Paths;
import java.util.List;

public class TapI4j {

    public static void main(String[] args) throws Exception {
        String deviceName = "{A6215D55-1B39-4C4E-B56E-250AB857A90A}";
        long deviceHandle = TapI4j.open(deviceName);
        System.out.println(deviceHandle);
        TapI4j.setStatus(deviceName, deviceHandle, true);
        TapI4j.setIPAddress("{A6215D55-1B39-4C4E-B56E-250AB857A90A}", deviceHandle, "10.0.0.1", "255.0.0.0");
//        Thread.sleep(10000);
        TapI4j.close(deviceHandle);
    }

    private static final String temporaryLibraryPath = ".";

    static {
//        String libraryName = TapUtils.getOperatingSystemFamily().getLibraryName();
//        File file = Paths.get(temporaryLibraryPath, libraryName).toFile();
//        file.deleteOnExit();
//        try (InputStream is = Tap4j.class.getResourceAsStream(libraryName); OutputStream os = new FileOutputStream(file)) {
//            is.transferTo(os);
//        } catch (Exception e) {
//        }
        System.load("c:\\Users\\PC\\Projects\\tap4j\\c\\windows\\cmake-build-release\\tap4j.dll");
//        System.load(file.getAbsolutePath());
    }

    public static native long open(String device) throws IOException;

    public static native int read(long device, ByteBuffer dst) throws IOException;

    public static native int write(long device, ByteBuffer src) throws IOException;

    public static native void close(long device) throws IOException;

    public static native void setIPAddress(String deviceName, long deviceHandle, String ipAddress, String ipMask) throws IOException;

    public static native void setStatus(String deviceName, long deviceHandle, boolean isUp) throws IOException;

    public static native byte[] getMACAddress(String deviceName, long deviceHandle) throws IOException;

    public static native void setMACAddress(String deviceName, long deviceHandle, byte[] mac) throws IOException;

    public static native int getMTU(String deviceName, long deviceHandle) throws IOException;

    public static native void setMTU(String deviceName, long deviceHandle, int mtu) throws IOException;

    public static List<TapDevice> list() {
        return nativeList();
    }

    private static native List<TapDevice> nativeList();

}
