//
// Created by Dimitar Kotevski on 01/09/15.
//
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#include "TiffLoader.h"
#include "TiffImage.h"

// The pointer field (from the Java bridge class)
jfieldID pointerField;

/**
 * Returns the instance of the TiffImage object (from the pointer inside the Java instance)
 */
TiffImage* getInstance(JNIEnv *env, jobject thiz) {
    return (TiffImage *) env->GetLongField(thiz, pointerField);
}


/**
 * Creates an instance of the TiffImage C++ object. The pointer should be saved
 * for all of the future calls on this object.
 *
 * Also it must be destroyed when no longer needed.
 */
jlong Java_si_dime_android_tiffloader_TiffLoader_read(JNIEnv *env, jobject, jstring path) {
    // Get the pointer field
    jclass loaderClass = env->FindClass("si/dime/android/tiffloader/TiffLoader");
    pointerField = env->GetFieldID(loaderClass, "pointer", "J");

    return (long)(new TiffImage(env, path));
}

/**
 * Releases the TiffImage C++ object
 */
void Java_si_dime_android_tiffloader_TiffLoader_close(JNIEnv *env, jobject thiz) {
    delete getInstance(env, thiz);
}

/**
 * Returns true if the image was successfully loaded and
 * we can work with it.
 */
jboolean Java_si_dime_android_tiffloader_TiffLoader_successfullyLoaded(JNIEnv *env, jobject thiz) {
    TiffImage *obj = getInstance(env, thiz);
    return obj->successfullyLoaded();
}

/**
 * Returns the number of directories (pages)
 */
jint Java_si_dime_android_tiffloader_TiffLoader_getDirectoryCount(JNIEnv *env, jobject thiz) {
    TiffImage *obj = getInstance(env, thiz);
    return obj->getDirectoryCount();
}


/**
 * Returns the size of the given directory
 */
jintArray Java_si_dime_android_tiffloader_TiffLoader_getSizeForDirectory(JNIEnv *env, jobject thiz, jint dir) {
    TiffImage *obj = getInstance(env, thiz);

    // Create the java int[]
    jintArray result = env->NewIntArray(2);

    // Get the size
    int* size = obj->getSizeForDirectory(dir);

    // Copy the array
    env->SetIntArrayRegion(result, 0, 2, size);

    // Return the result
    return result;
}






#ifdef __cplusplus
}
#endif