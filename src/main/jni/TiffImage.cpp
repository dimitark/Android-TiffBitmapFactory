//
// Created by Dimitar Kotevski on 01/09/15.
//

#include "TiffImage.h"

/**
 * Default constructor
 */
TiffImage::TiffImage(JNIEnv *env, jstring path) {
    // Convert the path from jstring to char*
    const char *strPath = NULL;
    strPath = env->GetStringUTFChars(path, 0);

    // Log info
    LOGIS("Trying to load the tiff file: ", strPath);

    // Load the TIFF image
    image = TIFFOpen(strPath, "r");
    // Check for errors
    if (image == NULL) {
        LOGES("Error while loading the tiff file: ", strPath);
        errorLoading = true;
    }

    // Release the char*
    env->ReleaseStringUTFChars(path, strPath);
};


/**
 * Returns true if the image was successfully loaded and
 * we can work with it.
 */
bool TiffImage::successfullyLoaded() {
    return !errorLoading;
}