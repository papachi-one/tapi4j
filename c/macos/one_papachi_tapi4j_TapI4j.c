#include "one_papachi_tapi4j_TapI4j.h"
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if_dl.h>
#include <stdlib.h>
#include <arpa/inet.h>

char* concat(const char *s1, const char *s2) {
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1 + len2 + 1);
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

void throwException(JNIEnv *env, const char *exceptionClassName, int error) {
    jclass exceptionClass;
    exceptionClass = (*env)->FindClass(env, exceptionClassName);
    (*env)->ThrowNew(env, exceptionClass, strerror(error));
}

JNIEXPORT jlong JNICALL Java_one_papachi_tapi4j_TapI4j_open(JNIEnv *env, jclass class1, jstring deviceName) {
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    char *devicePath = concat("/dev/", _device);
    int fd = open(devicePath, O_RDWR);
    if (fd == -1) {
        throwException(env, "java/io/IOexception", errno);
    }
    free(devicePath);
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
    return (jlong) fd;
}

JNIEXPORT jint JNICALL Java_one_papachi_tapi4j_TapI4j_read(JNIEnv *env, jclass class1, jlong deviceHandle, jobject dst) {
    jbyte *_dst = (*env)->GetDirectBufferAddress(env, dst);
    const jint capacity = (*env)->GetDirectBufferCapacity(env, dst);
    jclass class = (*env)->GetObjectClass(env, dst);
    jmethodID methodIdGetPosition = (*env)->GetMethodID(env, class, "position", "()I");
    jint position = (*env)->CallIntMethod(env, dst, methodIdGetPosition);
    jint remaining = capacity - position;
    jint bytesRead = read(deviceHandle, _dst, remaining);
    if (bytesRead > 0) {
        jmethodID methodIdSetPosition = (*env)->GetMethodID(env, class, "position", "(I)Ljava/nio/Buffer;");
        (*env)->CallObjectMethod(env, dst, methodIdSetPosition, position + bytesRead);
    } else {
        throwException(env, "java/io/IOException", errno);
    }
    return bytesRead;
}

JNIEXPORT jint JNICALL Java_one_papachi_tapi4j_TapI4j_write(JNIEnv *env, jclass class1, jlong deviceHandle, jobject src) {
    const jbyte *_src = (*env)->GetDirectBufferAddress(env, src);
    const jint capacity = (*env)->GetDirectBufferCapacity(env, src);
    jclass class = (*env)->GetObjectClass(env, src);
    jmethodID methodIdGetPosition = (*env)->GetMethodID(env, class, "position", "()I");
    jint position = (*env)->CallIntMethod(env, src, methodIdGetPosition);
    jint remaining = capacity - position;
    jint bytesWritten = write(deviceHandle, _src, remaining);
    if (bytesWritten > 0) {
        jmethodID methodIdSetPosition = (*env)->GetMethodID(env, class, "position", "(I)Ljava/nio/Buffer;");
        (*env)->CallObjectMethod(env, src, methodIdSetPosition, position + bytesWritten);
    } else {
        throwException(env, "java/io/IOException", errno);
    }
    return bytesWritten;
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_close(JNIEnv *env, jclass class1, jlong deviceHandle) {
    if (close(deviceHandle) == -1) {
        throwException(env, "java/io/IOException", errno);
    }
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_setIPAddress(JNIEnv *env, jclass class1, jstring deviceName, jlong deviceHandle, jstring ipAddress, jstring ipMask) {
    const char *_deviceName = (*env)->GetStringUTFChars(env, deviceName, NULL);
    const char *_ipAddress = (*env)->GetStringUTFChars(env, ipAddress, NULL);
    const char *_ipMask = (*env)->GetStringUTFChars(env, ipMask, NULL);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, _deviceName, IFNAMSIZ);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    struct sockaddr_in* addr = (struct sockaddr_in*) &ifr.ifr_addr;
    inet_pton(AF_INET, _ipMask, &addr->sin_addr);
    if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) == -1) {
        throwException(env, "java/io/IOException", errno);
    }
    close(sockfd);
    (*env)->ReleaseStringUTFChars(env, ipMask, _ipMask);
    (*env)->ReleaseStringUTFChars(env, ipAddress, _ipAddress);
    (*env)->ReleaseStringUTFChars(env, deviceName, _deviceName);
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_setStatus(JNIEnv *env, jclass class1, jstring deviceName, jlong deviceHandle, jboolean isUp) {
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, _device, IFNAMSIZ);
    ifr.ifr_flags = JNI_TRUE == isUp ? IFF_UP : 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (ioctl(sockfd, SIOCGIFFLAGS, &ifr) == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        if (isUp == JNI_TRUE) {
            ifr.ifr_ifru.ifru_flags |= IFF_UP;
        } else {
            ifr.ifr_ifru.ifru_flags &= ~IFF_UP;
        }
        if (ioctl(sockfd, SIOCSIFFLAGS, &ifr) == -1) {
            throwException(env, "java/io/IOException", errno);
        }
    }
    close(sockfd);
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
}

JNIEXPORT jbyteArray JNICALL Java_one_papachi_tapi4j_TapI4j_getMACAddress(JNIEnv *env, jclass class1, jstring deviceName, jlong deviceHandle) {
    jbyteArray mac = (*env)->NewByteArray(env, 6);
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    int mib[6];
    size_t len;
    char *buf;
    unsigned char *ptr;
    struct if_msghdr *ifm;
    struct sockaddr_dl *sdl;
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;
    mib[5] = if_nametoindex("tap0");
    if (sysctl(mib, 6, NULL, &len, NULL, 0) == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        if ((buf = malloc(len)) == NULL) {
            throwException(env, "java/io/IOException", errno);
        } else {
            if (sysctl(mib, 6, buf, &len, NULL, 0) < 0) {
                throwException(env, "java/io/IOException", errno);
            } else {
                ifm = (struct if_msghdr *)buf;
                sdl = (struct sockaddr_dl *)(ifm + 1);
                ptr = (unsigned char *)LLADDR(sdl);
                (*env)->SetByteArrayRegion(env, mac, 0, 6, (const jbyte *)ptr);
            }
        }
    }
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
    return mac;
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_setMACAddress(JNIEnv *env, jclass class1, jstring deviceName, jlong deviceHandle, jbyteArray mac) {
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, _device, IFNAMSIZ);
    (*env)->GetByteArrayRegion(env, mac, 0, 6, (jbyte *) &ifr.ifr_ifru.ifru_addr.sa_data);
    ifr.ifr_ifru.ifru_addr.sa_len = 6;
    ifr.ifr_ifru.ifru_addr.sa_family = AF_LINK;
    int sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        if (ioctl(sockfd, SIOCSIFLLADDR, &ifr) == -1) {
            throwException(env, "java/io/IOException", errno);
        }
    }
    close(sockfd);
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
}

JNIEXPORT jint JNICALL Java_one_papachi_tapi4j_TapI4j_getMTU(JNIEnv *env, jclass class1, jstring deviceName, jlong deviceHandle) {
    jint result = -1;
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, _device, IFNAMSIZ);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        if (ioctl(sockfd, SIOCGIFMTU, &ifr) == -1) {
            throwException(env, "java/io/IOException", errno);
        } else {
            result = ifr.ifr_ifru.ifru_mtu;
        }
    }
    close(sockfd);
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
    return result;
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_setMTU(JNIEnv *env, jclass class1, jstring deviceName, jlong deviceHandle, jint mtu) {
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, _device, IFNAMSIZ);
    ifr.ifr_ifru.ifru_mtu = mtu;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        if (ioctl(sockfd, SIOCSIFMTU, &ifr) == -1) {
            throwException(env, "java/io/IOException", errno);
        }
    }
    close(sockfd);
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
}

JNIEXPORT jobject JNICALL Java_one_papachi_tapi4j_TapI4j_nativeList(JNIEnv *env, jclass class1) {
    throwException(env, "java/lang/UnsupportedOperationException", ENOSYS);
}
