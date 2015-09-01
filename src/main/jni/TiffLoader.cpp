//
// Created by Dimitar Kotevski on 01/09/15.
//
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#include "TiffLoader.h"
#include "TiffImage.h"

/**
 * Creates an instance of the TiffImage C++ object. The pointer should be saved
 * for all of the future calls on this object.
 *
 * Also it must be destroyed when no longer needed.
 */
jlong Java_si_dime_android_tiffloader_TiffLoader_read(JNIEnv *env, jobject, jstring path) {
    return (long)(new TiffImage(env, path));
}

/**
 * Releases the TiffImage C++ object
 */
void Java_si_dime_android_tiffloader_TiffLoader_close(JNIEnv *, jobject, jlong ptr) {
    delete (TiffImage *)(ptr);
}

/**
 * Returns true if the image was successfully loaded and
 * we can work with it.
 */
jboolean Java_si_dime_android_tiffloader_TiffLoader_successfullyLoaded(JNIEnv *, jobject, jlong ptr) {
    TiffImage *obj = (TiffImage *)ptr;
    return obj->successfullyLoaded();
}


#ifdef __cplusplus
}
#endif