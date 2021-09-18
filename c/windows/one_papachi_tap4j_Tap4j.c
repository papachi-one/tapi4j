#include "one_papachi_tap4j_Tap4j.h"

#include <stdio.h>
#include <Windows.h>
#include <winnt.h>
#include <winreg.h>
#include <string.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#include <netioapi.h>

#define TAP_WIN_IOCTL_SET_MEDIA_STATUS 2228248
#define TAP_WIN_IOCTL_GET_MAC 2228228
#define ADAPTER_KEY "SYSTEM\\CurrentControlSet\\Control\\Class\\{4D36E972-E325-11CE-BFC1-08002BE10318}"
// #define NETWORK_KEY "SYSTEM\\CurrentControlSet\\Control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}"

char* concat(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1);
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

void throwException(JNIEnv *env, const char *exceptionClassName, DWORD error) {
    LPTSTR message;
    FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL, error, 0, (LPSTR) &message, 0, NULL);
    message[strlen(message) - 2] = '\0';
    jclass exceptionClass;
    exceptionClass = (*env)->FindClass(env, exceptionClassName);
    (*env)->ThrowNew(env, exceptionClass, message);
    free(message);
}

NET_IFINDEX getIfIndex(JNIEnv *env, const char *deviceName) {
    WCHAR wDeviceName[39];
    mbstowcs(wDeviceName, deviceName, 39);
    wDeviceName[38] = 0;
    const WCHAR *guidString = wDeviceName;
    GUID guid;
    HRESULT hResult = CLSIDFromString(guidString, (LPCLSID)&guid);
    if (hResult != S_OK) {
        throwException(env, "java/lang/IllegalArgumentException", hResult);
        return -1;
    }
    NET_LUID luid;
    ConvertInterfaceGuidToLuid(&guid, &luid);
    NET_IFINDEX ifIndex;
    ConvertInterfaceLuidToIndex(&luid, &ifIndex);
    return ifIndex;
}

ULONG getAddr(JNIEnv *env, const char *input) {
    typedef LONG (NTAPI *RtlIpv4StringToAddressA)(PCSTR, BOOLEAN, PCSTR *, struct in_addr *);
    RtlIpv4StringToAddressA pRtlIpv4StringToAddressA = (RtlIpv4StringToAddressA)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlIpv4StringToAddressA");
    struct in_addr addr;
    char c;
    char *cc = &c;
    NTSTATUS status = pRtlIpv4StringToAddressA(input, TRUE, &cc, &addr);
    if (status) {
        typedef LONG (NTAPI *RtlNtStatusToDosError)(NTSTATUS);
        RtlNtStatusToDosError pRtlNtStatusToDosError = (RtlNtStatusToDosError)GetProcAddress(GetModuleHandle("ntdll.dll"), "RtlNtStatusToDosError");
        DWORD error = pRtlNtStatusToDosError(status);
        throwException(env, "java/lang/IllegalArgumentException", error);
        return -1;
    }
    ULONG addrI = addr.S_un.S_addr;
    return addrI;
}

JNIEXPORT jlong JNICALL Java_one_papachi_tap4j_Tap4j_open(JNIEnv *env, jclass class, jstring deviceName) {
    const char *_deviceName = (*env)->GetStringUTFChars(env, deviceName, NULL);
    char *tmp1 = concat("\\\\.\\", _deviceName);
    char *tmp2 = concat(tmp1, ".tap");
    HANDLE handle = CreateFileA(tmp2, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_SYSTEM, 0);
    free(tmp1);
    free(tmp2);
    (*env)->ReleaseStringUTFChars(env, deviceName, _deviceName);
    if (handle == INVALID_HANDLE_VALUE) {
        DWORD error = GetLastError();
        throwException(env, "java/io/IOException", error);
    }
    return (jlong) handle;
}

JNIEXPORT jint JNICALL Java_one_papachi_tap4j_Tap4j_read(JNIEnv *env, jclass class, jlong deviceHandle, jobject dst) {
    jbyte *_dst = (*env)->GetDirectBufferAddress(env, dst);
    const jint capacity = (*env)->GetDirectBufferCapacity(env, dst);
    jclass class1 = (*env)->GetObjectClass(env, dst);
    jmethodID methodIdGetPosition = (*env)->GetMethodID(env, class1, "position", "()I");
    jint position = (*env)->CallIntMethod(env, dst, methodIdGetPosition);
    jint remaining = capacity - position;
    jint bytesRead = 0;
    BOOL errorFlag = ReadFile((HANDLE) deviceHandle, _dst + position, remaining, &bytesRead, NULL);
    if (errorFlag != FALSE) {
        jmethodID methodIdSetPosition = (*env)->GetMethodID(env, class1, "position", "(I)Ljava/nio/Buffer;");
        (*env)->CallObjectMethod(env, dst, methodIdSetPosition, position + bytesRead);
    } else {
        DWORD error = GetLastError();
        throwException(env, "java/io/IOException", error);
    }
    return bytesRead;
}

JNIEXPORT jint JNICALL Java_one_papachi_tap4j_Tap4j_write(JNIEnv *env, jclass class, jlong deviceHandle, jobject src) {
    const jbyte *_src = (*env)->GetDirectBufferAddress(env, src);
    const jint capacity = (*env)->GetDirectBufferCapacity(env, src);
    jclass class1 = (*env)->GetObjectClass(env, src);
    jmethodID methodIdGetPosition = (*env)->GetMethodID(env, class1, "position", "()I");
    jint position = (*env)->CallIntMethod(env, src, methodIdGetPosition);
    jint remaining = capacity - position;
    jint bytesWritten = 0;
    BOOL errorFlag = WriteFile((HANDLE) deviceHandle, _src + position, remaining, &bytesWritten, NULL);
    if (errorFlag != FALSE) {
        jmethodID methodIdSetPosition = (*env)->GetMethodID(env, class1, "position", "(I)Ljava/nio/Buffer;");
        (*env)->CallObjectMethod(env, src, methodIdSetPosition, position + bytesWritten);
    } else {
        DWORD error = GetLastError();
        throwException(env, "java/io/IOException", error);
    }
    return bytesWritten;
}

JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_close(JNIEnv *env, jclass class, jlong deviceHandle) {
    BOOL errorFlag = CloseHandle((HANDLE) deviceHandle);
    if (errorFlag == FALSE) {
        DWORD error = GetLastError();
        throwException(env, "java/io/IOException", error);
    }
}

JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_setIPAddress(JNIEnv *env, jclass class1, jstring deviceName, jlong deviceHandle, jstring ipAddress, jstring ipMask) {
    const char *_deviceName = (*env)->GetStringUTFChars(env, deviceName, NULL);
    const char *_ipAddress = (*env)->GetStringUTFChars(env, ipAddress, NULL);
    const char *_ipMask = (*env)->GetStringUTFChars(env, ipMask, NULL);
    NET_IFINDEX ifIndex = getIfIndex(env, _deviceName);
    if ((*env)->ExceptionCheck(env) == JNI_FALSE) {
        ULONG Address = getAddr(env, _ipAddress);
        if ((*env)->ExceptionCheck(env) == JNI_FALSE) {
            ULONG IpMask = getAddr(env,_ipMask);
            if ((*env)->ExceptionCheck(env) == JNI_FALSE) {
                ULONG NTEContext = 0;
                ULONG NTEInstance = 0;
                ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
                PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));
                if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
                    free(pAdapterInfo);
                    pAdapterInfo = (IP_ADAPTER_INFO *) malloc(ulOutBufLen);
                }
                ULONG error = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
                if (error) {
                    throwException(env, "java/io/IOException", error);
                } else {
                    PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
                    while (pAdapter) {
                        if (pAdapterInfo->Index == ifIndex) {
                            PIP_ADDR_STRING addr = &pAdapter->IpAddressList;
                            while (addr) {
                                DeleteIPAddress(addr->Context);
                                addr = addr->Next;
                            }
                        }
                        pAdapter = pAdapter->Next;
                    }
                    free(pAdapterInfo);
                    DWORD err = AddIPAddress(Address, IpMask, ifIndex, &NTEContext, &NTEInstance);
                    if (err) {
                        throwException(env, "java/io/IOException", err);
                    }
                }
            }
        }
    }
    (*env)->ReleaseStringUTFChars(env, ipMask, _ipMask);
    (*env)->ReleaseStringUTFChars(env, ipAddress, _ipAddress);
    (*env)->ReleaseStringUTFChars(env, deviceName, _deviceName);
}

JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_setStatus(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle, jboolean isUp) {
    ULONG _isUp = isUp == JNI_TRUE ? 1 : 0;
    DWORD len;
    BOOL result = DeviceIoControl((HANDLE) deviceHandle, TAP_WIN_IOCTL_SET_MEDIA_STATUS, &_isUp, sizeof _isUp, &_isUp, sizeof _isUp, &len, NULL);
    if (result == FALSE) {
        DWORD error = GetLastError();
        throwException(env, "java/io/IOException", error);
    }
}

JNIEXPORT jbyteArray JNICALL Java_one_papachi_tap4j_Tap4j_getMACAddress(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle) {
    char macAddress[] = {0, 0, 0, 0, 0, 0};
    DWORD len;
    BOOL result = DeviceIoControl((HANDLE) deviceHandle, TAP_WIN_IOCTL_GET_MAC, 0, 0, &macAddress, 6, &len, NULL);
    if (result == FALSE) {
        DWORD error = GetLastError();
        throwException(env, "java/io/IOException", error);
    }
    jbyteArray array = (*env)->NewByteArray(env, 6);
    (*env)->SetByteArrayRegion(env, array, 0, 6, macAddress);
    return array;
}

JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_setMACAddress(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle, jbyteArray mac) {
    throwException(env, "java/lang/UnsupportedOperationException", ERROR_CALL_NOT_IMPLEMENTED);
}

JNIEXPORT jint JNICALL Java_one_papachi_tap4j_Tap4j_getMTU(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle) {
    jint mtu = 1500;
    const char *_netCfgInstanceId = (*env)->GetStringUTFChars(env, deviceName, NULL);
    const char adapterKey[] = ADAPTER_KEY;
    HKEY hKey;
    LSTATUS status;
    if ((status = RegOpenKeyA(HKEY_LOCAL_MACHINE, adapterKey, &hKey)) == ERROR_SUCCESS) {
        DWORD lpcSubKeys = 0;
        DWORD lpcMaxSubKeyLen = 0;
        if ((status = RegQueryInfoKeyA(hKey, NULL, NULL, NULL, &lpcSubKeys, &lpcMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS) {
            char name[255];
            DWORD nameLength = 255;
            for (int i = 0; i < lpcSubKeys; i++) {
                RegEnumKeyA(hKey, i, name, nameLength);
                DWORD dataTypeNetCfgInstanceId;
                char valueNetCfgInstanceId[255];
                PVOID pvDataNetCfgInstanceId = valueNetCfgInstanceId;
                DWORD sizeNetCfgInstanceId = sizeof(valueNetCfgInstanceId);
                if ((status = RegGetValueA(hKey, name, "NetCfgInstanceId", RRF_RT_REG_SZ | RRF_ZEROONFAILURE, &dataTypeNetCfgInstanceId, pvDataNetCfgInstanceId, &sizeNetCfgInstanceId)) == ERROR_SUCCESS) {
                    if (strcmp(valueNetCfgInstanceId, _netCfgInstanceId) == 0) {
                        DWORD dataTypeMTU;
                        char valueMTU[255];
                        PVOID pvDataMTU = valueMTU;
                        DWORD sizeMTU = sizeof(valueMTU);
                        if ((status = RegGetValueA(hKey, name, "MTU", RRF_RT_REG_SZ | RRF_ZEROONFAILURE, &dataTypeMTU, pvDataMTU, &sizeMTU)) == ERROR_SUCCESS) {
                            mtu = atoi(valueMTU);
                            break;
                        } else {
                            throwException(env, "java/io/IOException", status);
                            break;
                        }
                    }
                } else {
                    throwException(env, "java/io/IOException", status);
                    break;
                }
            }
        } else {
            throwException(env, "java/io/IOException", status);
        }
        RegCloseKey(hKey);
    } else {
        throwException(env, "java/io/IOException", status);
    }
    (*env)->ReleaseStringUTFChars(env, deviceName, _netCfgInstanceId);
    return mtu;
}

JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_setMTU(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle, jint mtu) {
    const char *_netCfgInstanceId = (*env)->GetStringUTFChars(env, deviceName, NULL);
    const char adapterKey[] = ADAPTER_KEY;
    char _mtu[5];
    _itoa_s(mtu, _mtu, 5, 10);
    HKEY hKey;
    LSTATUS status;
    if ((status = RegOpenKeyA(HKEY_LOCAL_MACHINE, adapterKey, &hKey)) == ERROR_SUCCESS) {
        DWORD lpcSubKeys = 0;
        DWORD lpcMaxSubKeyLen = 0;
        if ((status = RegQueryInfoKeyA(hKey, NULL, NULL, NULL, &lpcSubKeys, &lpcMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL)) == ERROR_SUCCESS) {
            char name[255];
            DWORD nameLength = 255;
            for (int i = 0; i < lpcSubKeys; i++) {
                RegEnumKeyA(hKey, i, name, nameLength);
                DWORD dataTypeNetCfgInstanceId;
                char valueNetCfgInstanceId[255];
                PVOID pvDataNetCfgInstanceId = valueNetCfgInstanceId;
                DWORD sizeNetCfgInstanceId = sizeof(valueNetCfgInstanceId);
                if ((status = RegGetValueA(hKey, name, "NetCfgInstanceId", RRF_RT_REG_SZ | RRF_ZEROONFAILURE, &dataTypeNetCfgInstanceId, pvDataNetCfgInstanceId, &sizeNetCfgInstanceId)) == ERROR_SUCCESS) {
                    if (strcmp(valueNetCfgInstanceId, _netCfgInstanceId) == 0) {
                        char * tmp1 = concat(adapterKey, "\\");
                        char * tmp2 = concat(tmp1, name);
                        HKEY hKey2;
                        if ((status = RegOpenKeyExA(HKEY_LOCAL_MACHINE, tmp2, 0, KEY_SET_VALUE, &hKey2)) == ERROR_SUCCESS) {
                            RegSetValueExA(hKey2, "MTU", 0, REG_SZ, _mtu, strlen(_mtu) + 1);
                            RegCloseKey(hKey2);
                        } else {
                            throwException(env, "java/io/IOException", status);
                        }
                        free(tmp1);
                        free(tmp2);
                    }
                } else {
                    throwException(env, "java/io/IOException", status);
                }
            }
        } else {
            throwException(env, "java/io/IOException", status);
        }
        RegCloseKey(hKey);
    } else {
        throwException(env, "java/io/IOException", status);
    }
    (*env)->ReleaseStringUTFChars(env, deviceName, _netCfgInstanceId);
}

JNIEXPORT jobject JNICALL Java_one_papachi_tap4j_Tap4j_nativeList(JNIEnv *env, jclass _class) {
    jclass class = (*env)->FindClass(env, "java/util/ArrayList");
    jmethodID constructor = (*env)->GetMethodID(env, class, "<init>", "()V");
    jmethodID add = (*env)->GetMethodID(env, class, "add", "(Ljava/lang/Object;)Z");
    jobject result = (*env)->NewObject(env, class, constructor);

    jclass class1 = (*env)->FindClass(env, "one/papachi/tap4j/TapDevice");
    jmethodID constructor1 = (*env)->GetMethodID(env, class1, "<init>", "(Ljava/lang/String;)V");

    const char adapterKey[] = ADAPTER_KEY;
    HKEY hKey;

    if (RegOpenKeyA(HKEY_LOCAL_MACHINE, adapterKey, &hKey) == ERROR_SUCCESS) {
        DWORD lpcSubKeys = 0;
        DWORD lpcMaxSubKeyLen = 0;
        if (RegQueryInfoKeyA(hKey, NULL, NULL, NULL, &lpcSubKeys, &lpcMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            char name[255];
            DWORD nameLength = 255;
            for (int i = 0; i < lpcSubKeys; i++) {
                RegEnumKeyA(hKey, i, name, nameLength);
                DWORD dataType;
                char value[255];
                PVOID pvData = value;
                DWORD size = sizeof(value);
                RegGetValueA(hKey, name, "ComponentId", RRF_RT_REG_SZ | RRF_ZEROONFAILURE, &dataType, pvData, &size);
                if (strcmp(value, "tap0901") == 0) {
                    DWORD dataTypeNetCfgInstanceId;
                    char valueNetCfgInstanceId[255];
                    PVOID pvDataNetCfgInstanceId = valueNetCfgInstanceId;
                    DWORD sizeNetCfgInstanceId = sizeof(valueNetCfgInstanceId);
                    RegGetValueA(hKey, name, "NetCfgInstanceId", RRF_RT_REG_SZ | RRF_ZEROONFAILURE, &dataTypeNetCfgInstanceId, pvDataNetCfgInstanceId, &sizeNetCfgInstanceId);
                    jstring netCfgInstanceId = (*env)->NewStringUTF(env, valueNetCfgInstanceId);

                    jobject device = (*env)->NewObject(env, class1, constructor1, netCfgInstanceId);
                    (*env)->CallBooleanMethod(env, result, add, device);

                    (*env)->DeleteLocalRef(env, device);
                    (*env)->DeleteLocalRef(env, netCfgInstanceId);
                }
            }
        }
        RegCloseKey(hKey);
    }
    return result;
}

/*
JNIEXPORT jstring JNICALL Java_one_papachi_tap4j_win_Tap4jWin_getName(JNIEnv *env, jobject thisObj, jstring netCfgInstanceId) {
    const char * _netCfgInstanceId = (*env)->GetStringUTFChars(env, netCfgInstanceId, NULL);
    const char networkKey[] = NETWORK_KEY;
    char * tmp1 = concat(networkKey, "\\");
    char * tmp2 = concat(tmp1, _netCfgInstanceId);
    char * tmp3 = concat(tmp2, "\\Connection");
    DWORD dataTypeName;
    char valueName[255];
    PVOID pvDataName = valueName;
    DWORD sizeName = sizeof(valueName);
    RegGetValueA(HKEY_LOCAL_MACHINE, tmp3, "Name", RRF_RT_REG_SZ | RRF_ZEROONFAILURE, &dataTypeName, pvDataName, &sizeName);
    jstring name = (*env)->NewStringUTF(env, valueName);
    (*env)->ReleaseStringUTFChars(env, netCfgInstanceId, _netCfgInstanceId);
    free(tmp1);
    free(tmp2);
    free(tmp3);
    return name;
}

JNIEXPORT void JNICALL Java_one_papachi_tap4j_win_Tap4jWin_setName(JNIEnv *env, jobject thisObj, jstring netCfgInstanceId, jstring name) {
    const char * _netCfgInstanceId = (*env)->GetStringUTFChars(env, netCfgInstanceId, NULL);
    const char * _name = (*env)->GetStringUTFChars(env, name, NULL);
    const char networkKey[] = NETWORK_KEY;
    char * tmp1 = concat(networkKey, "\\");
    char * tmp2 = concat(tmp1, _netCfgInstanceId);
    char * tmp3 = concat(tmp2, "\\Connection");
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, tmp3, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        RegSetValueExA(hKey, "Name", 0, REG_SZ, _name, strlen(_name) + 1);
        RegCloseKey(hKey);
    }
    (*env)->ReleaseStringUTFChars(env, netCfgInstanceId, _netCfgInstanceId);
    (*env)->ReleaseStringUTFChars(env, name, _name);
    free(tmp1);
    free(tmp2);
    free(tmp3);
}

JNIEXPORT jstring JNICALL Java_one_papachi_tap4j_win_Tap4jWin_getDeviceInstanceId(JNIEnv *env, jobject thisObj, jstring netCfgInstanceId) {
    jstring deviceInstanceId = NULL;
    const char *_netCfgInstanceId = (*env)->GetStringUTFChars(env, netCfgInstanceId, NULL);
    const char adapterKey[] = ADAPTER_KEY;
    HKEY hKey;
    if (RegOpenKeyA(HKEY_LOCAL_MACHINE, adapterKey, &hKey) == ERROR_SUCCESS) {
        DWORD lpcSubKeys = 0;
        DWORD lpcMaxSubKeyLen = 0;
        if (RegQueryInfoKeyA(hKey, NULL, NULL, NULL, &lpcSubKeys, &lpcMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            char name[255];
            DWORD nameLength = 255;
            for (int i = 0; i < lpcSubKeys; i++) {
                RegEnumKeyA(hKey, i, name, nameLength);
                DWORD dataTypeNetCfgInstanceId;
                char valueNetCfgInstanceId[255];
                PVOID pvDataNetCfgInstanceId = valueNetCfgInstanceId;
                DWORD sizeNetCfgInstanceId = sizeof(valueNetCfgInstanceId);
                RegGetValueA(hKey, name, "NetCfgInstanceId", RRF_RT_REG_SZ | RRF_ZEROONFAILURE, &dataTypeNetCfgInstanceId, pvDataNetCfgInstanceId, &sizeNetCfgInstanceId);
                if (strcmp(valueNetCfgInstanceId, _netCfgInstanceId) == 0) {
                    DWORD dataType;
                    char value[255];
                    PVOID pvData = value;
                    DWORD size = sizeof(value);
                    RegGetValueA(hKey, name, "DeviceInstanceId", RRF_RT_REG_SZ | RRF_ZEROONFAILURE, &dataType, pvData, &size);
                    deviceInstanceId = (*env)->NewStringUTF(env, value);
                }
            }
        }
        RegCloseKey(hKey);
    }
    (*env)->ReleaseStringUTFChars(env, netCfgInstanceId, _netCfgInstanceId);
    return deviceInstanceId;
}
*/