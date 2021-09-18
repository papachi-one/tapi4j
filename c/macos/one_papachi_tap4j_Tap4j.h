/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class one_papachi_tap4j_Tap4j */

#ifndef _Included_one_papachi_tap4j_Tap4j
#define _Included_one_papachi_tap4j_Tap4j
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    open
 * Signature: (Ljava/lang/String;)J
 */
JNIEXPORT jlong JNICALL Java_one_papachi_tap4j_Tap4j_open
        (JNIEnv *, jclass, jstring);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    read
 * Signature: (JLjava/nio/ByteBuffer;)I
 */
JNIEXPORT jint JNICALL Java_one_papachi_tap4j_Tap4j_read
        (JNIEnv *, jclass, jlong, jobject);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    write
 * Signature: (JLjava/nio/ByteBuffer;)I
 */
JNIEXPORT jint JNICALL Java_one_papachi_tap4j_Tap4j_write
        (JNIEnv *, jclass, jlong, jobject);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    close
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_close
        (JNIEnv *, jclass, jlong);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    setIPAddress
 * Signature: (Ljava/lang/String;JLjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_setIPAddress
        (JNIEnv *, jclass, jstring, jlong, jstring, jstring);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    setStatus
 * Signature: (Ljava/lang/String;JZ)V
 */
JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_setStatus
        (JNIEnv *, jclass, jstring, jlong, jboolean);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    getMACAddress
 * Signature: (Ljava/lang/String;J)[B
 */
JNIEXPORT jbyteArray JNICALL Java_one_papachi_tap4j_Tap4j_getMACAddress
        (JNIEnv *, jclass, jstring, jlong);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    setMACAddress
 * Signature: (Ljava/lang/String;J[B)V
 */
JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_setMACAddress
        (JNIEnv *, jclass, jstring, jlong, jbyteArray);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    getMTU
 * Signature: (Ljava/lang/String;J)I
 */
JNIEXPORT jint JNICALL Java_one_papachi_tap4j_Tap4j_getMTU
        (JNIEnv *, jclass, jstring, jlong);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    setMTU
 * Signature: (Ljava/lang/String;JI)V
 */
JNIEXPORT void JNICALL Java_one_papachi_tap4j_Tap4j_setMTU
        (JNIEnv *, jclass, jstring, jlong, jint);

/*
 * Class:     one_papachi_tap4j_Tap4j
 * Method:    nativeList
 * Signature: ()Ljava/util/List;
 */
JNIEXPORT jobject JNICALL Java_one_papachi_tap4j_Tap4j_nativeList
        (JNIEnv *, jclass);

#ifdef __cplusplus
}
#endif
#endif
