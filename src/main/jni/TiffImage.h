//
// Created by Dimitar Kotevski on 01/09/15.
//

#ifndef TIFFIMAGE_H
#define TIFFIMAGE_H

#include <jni.h>
#include <android/log.h>
#include <android/bitmap.h>
#include <stdlib.h>
#include <stdio.h>
#include <tiffio.h>

#define LOGE(x) __android_log_print(ANDROID_LOG_ERROR, "TiffLoader", "%s", x)
#define LOGI(x) __android_log_print(ANDROID_LOG_DEBUG, "TiffLoader", "%s", x)
#define LOGII(x, y) __android_log_print(ANDROID_LOG_DEBUG, "TiffLoader", "%s %d", x, y)
#define LOGIS(x, y) __android_log_print(ANDROID_LOG_DEBUG, "TiffLoader", "%s %s", x, y)
#define LOGES(x, y) __android_log_print(ANDROID_LOG_ERROR, "TiffLoader", "%s %s", x, y)

class TiffImage {

public:
    TiffImage(JNIEnv *env, jstring path);
    bool successfullyLoaded();

private:
    TIFF *image;
    bool errorLoading;
};


#endif //TIFFIMAGE_H
