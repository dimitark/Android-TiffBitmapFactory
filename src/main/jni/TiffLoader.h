//
// Created by Dimitar Kotevski on 01/09/15.
//

#include <jni.h>

#ifndef TIFFLOADER_H
#define TIFFLOADER_H

#ifndef _Included_si_dime_android_tiffloader_TiffLoader
#define _Included_si_dime_android_tiffloader_TiffLoader
#ifdef __cplusplus
extern "C" {
#endif


/**
 * Creates an instance of the TiffImage C++ object. The pointer should be saved
 * for all of the future calls on this object.
 *
 * Also it must be closed when no longer needed.
 */
jlong Java_si_dime_android_tiffloader_TiffLoader_read(JNIEnv *env, jobject, jstring path);

/**
 * Releases the TiffImage C++ object
 */
void Java_si_dime_android_tiffloader_TiffLoader_close(JNIEnv *, jobject, jlong ptr);

/**
 * Returns true if the image was successfully loaded and
 * we can work with it.
 */
jboolean Java_si_dime_android_tiffloader_TiffLoader_successfullyLoaded(JNIEnv *, jobject, jlong ptr);


#ifdef __cplusplus
}
#endif
#endif
#endif //TIFFLOADER_H
