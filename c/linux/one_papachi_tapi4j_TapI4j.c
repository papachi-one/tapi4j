#include "one_papachi_tapi4j_TapI4j.h"

#include <sys/ioctl.h>
#include <fcntl.h>
#include <string.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <unistd.h>
#include <errno.h>
#include <net/if_arp.h>
#include <arpa/inet.h>

void throwException(JNIEnv *env, const char *exceptionClassName, int error) {
    jclass exceptionClass;
    exceptionClass = (*env)->FindClass(env, exceptionClassName);
    (*env)->ThrowNew(env, exceptionClass, strerror(error));
}

JNIEXPORT jlong JNICALL Java_one_papachi_tapi4j_TapI4j_open(JNIEnv *env, jclass class, jstring deviceName) {
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    int fd;
    if ((fd = open("/dev/net/tun", O_RDWR)) == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, _device, IFNAMSIZ);
        ifr.ifr_flags = IFF_TAP | IFF_NO_PI;
        if (ioctl(fd, TUNSETIFF, &ifr) == -1) {
            close(fd);
            throwException(env, "java/io/IOException", errno);
        }
    }
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
    if (bytesRead == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        jmethodID methodIdSetPosition = (*env)->GetMethodID(env, class, "position", "(I)Ljava/nio/Buffer;");
        (*env)->CallObjectMethod(env, dst, methodIdSetPosition, position + bytesRead);
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
    if (bytesWritten == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        jmethodID methodIdSetPosition = (*env)->GetMethodID(env, class, "position", "(I)Ljava/nio/Buffer;");
        (*env)->CallObjectMethod(env, src, methodIdSetPosition, position + bytesWritten);
    }
    return bytesWritten;
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_close(JNIEnv *env, jclass class, jlong deviceHandle) {
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
    if (sockfd == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        ifr.ifr_addr.sa_family = AF_INET;
        struct sockaddr_in* addr = (struct sockaddr_in*) &ifr.ifr_addr;
        inet_pton(AF_INET, _ipAddress, &addr->sin_addr);
        if (ioctl(sockfd, SIOCSIFADDR, &ifr) == -1) {
            throwException(env, "java/io/IOException", errno);
        } else {
            inet_pton(AF_INET, _ipMask, &addr->sin_addr);
            if (ioctl(sockfd, SIOCSIFNETMASK, &ifr) == -1) {
                throwException(env, "java/io/IOException", errno);
            }
        }
        close(sockfd);
    }
    (*env)->ReleaseStringUTFChars(env, ipMask, _ipMask);
    (*env)->ReleaseStringUTFChars(env, ipAddress, _ipAddress);
    (*env)->ReleaseStringUTFChars(env, deviceName, _deviceName);
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_setStatus(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle, jboolean isUp) {
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, _device, IFNAMSIZ);
    ifr.ifr_flags = JNI_TRUE == isUp ? IFF_UP : 0;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
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
    }
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
}

JNIEXPORT jbyteArray JNICALL Java_one_papachi_tapi4j_TapI4j_getMACAddress(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle) {
    jbyteArray mac = (*env)->NewByteArray(env, 6);
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, _device, IFNAMSIZ);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        if (ioctl(sockfd, SIOCGIFHWADDR, &ifr) == -1) {
            throwException(env, "java/io/IOException", errno);
        } else {
            (*env)->SetByteArrayRegion(env, mac, 0, 6, ifr.ifr_hwaddr.sa_data);
        }
        close(sockfd);
    }
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
    return mac;
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_setMACAddress(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle, jbyteArray mac) {
    const char *_device = (*env)->GetStringUTFChars(env, deviceName, NULL);
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, _device, IFNAMSIZ);
    ifr.ifr_ifru.ifru_hwaddr.sa_family = ARPHRD_ETHER;
    (*env)->GetByteArrayRegion(env, mac, 0, 6, (jbyte *) &ifr.ifr_ifru.ifru_hwaddr.sa_data);
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == -1) {
        throwException(env, "java/io/IOException", errno);
    } else {
        if (ioctl(sockfd, SIOCSIFHWADDR, &ifr) == -1) {
            throwException(env, "java/io/IOException", errno);
        }
        close(sockfd);
    }
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
}

JNIEXPORT jint JNICALL Java_one_papachi_tapi4j_TapI4j_getMTU(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle) {
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
        close(sockfd);
    }
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
    return result;
}

JNIEXPORT void JNICALL Java_one_papachi_tapi4j_TapI4j_setMTU(JNIEnv *env, jclass class, jstring deviceName, jlong deviceHandle, jint mtu) {
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
        close(sockfd);
    }
    (*env)->ReleaseStringUTFChars(env, deviceName, _device);
}

JNIEXPORT jobject JNICALL Java_one_papachi_tapi4j_TapI4j_nativeList(JNIEnv *env, jclass class) {
    throwException(env, "java/lang/UnsupportedOperationException", ENOSYS);
}
