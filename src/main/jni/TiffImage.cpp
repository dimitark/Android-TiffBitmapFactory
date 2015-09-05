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
    } else {
        // Count the directories
        directoryCount = -1;
        getDirectoryCount();

        // Reset the flags
        shouldReleaseSizeArray = false;
        colorMask = 0xFF;

        // Read the sizes
        readSizes();
    }

    // Release the char*
    env->ReleaseStringUTFChars(path, strPath);
};

/**
 * Destructor
 */
TiffImage::~TiffImage() {
    // Release everything
    if (shouldReleaseSizeArray) {
        free(lastReadSizeAddress);
    }

    if (image) {
        TIFFClose(image);
        image = NULL;
    }
}


/**
 * Returns true if the image was successfully loaded and
 * we can work with it.
 */
bool TiffImage::successfullyLoaded() {
    return !errorLoading;
}

/**
 * Returns the directory count of this TIFF (number of images)
 */
int TiffImage::getDirectoryCount() {
    if (directoryCount > 0) {
        return directoryCount;
    }

    // Count the directories
    directoryCount = 0;
    do {
        directoryCount++;
    } while (TIFFReadDirectory(image));

    // Set the directory to 0
    TIFFSetDirectory(image, 0);

    // Return the count
    return directoryCount;
}

/**
 * Returns the size of the given dir
 */
int* TiffImage::getSizeForDirectory(int dir) {
    if (shouldReleaseSizeArray) {
        // Free the previous address
        free(lastReadSizeAddress);
    }

    lastReadSizeAddress = (int*) malloc(sizeof (int) * 2);
    lastReadSizeAddress[0] = widths[dir];
    lastReadSizeAddress[1] = heights[dir];
    shouldReleaseSizeArray = true;
    return lastReadSizeAddress;
}

/**
 * Reads the sizes of all directories
 */
void TiffImage::readSizes() {
    // Initialize the arrays
    widths = new int[directoryCount];
    heights = new int[directoryCount];

    for (int i=0; i < directoryCount; i++) {
        // Set the directory
        TIFFSetDirectory(image, i);

        // Read the sizes
        int width, height;
        TIFFGetField(image, TIFFTAG_IMAGEWIDTH, &width);
        TIFFGetField(image, TIFFTAG_IMAGELENGTH, &height);
        // Save them
        widths[i] = width;
        heights[i] = height;
    }

    // Set the directory to 0
    TIFFSetDirectory(image, 0);
}


/**
 * Does the actual reading of the TIFF directory (page) and passes it as a Android Bitmap object
 */
jobject TiffImage::createBitmap(JNIEnv *env, int dir, int inSampleSize) {
    // Change the directory
    TIFFSetDirectory(image, dir);

    // Calculate the buffer size
    int origwidth = widths[dir];
    int origheight = heights[dir];
    int origBufferSize = origwidth * origheight;

    // Allocate the memory
    unsigned int *origBuffer = (unsigned int *) _TIFFmalloc(origBufferSize * sizeof(unsigned int));
    // Error check
    if (origBuffer == NULL) {
        LOGE("Can\'t allocate memory for origBuffer");
        return NULL;
    }

    // Read the image
    TIFFReadRGBAImageOriented(image, origwidth, origheight, origBuffer, ORIENTATION_TOPLEFT, 0);

    // Convert ABGR to ARGB
    int i = 0;
    int j = 0;
    int tmp = 0;
    for (i = 0; i < origheight; i++) {
        for (j = 0; j < origwidth; j++) {
            tmp = origBuffer[j + origwidth * i];
            origBuffer[j + origwidth * i] =
                    (tmp & 0xff000000) | ((tmp & 0x00ff0000) >> 16) | (tmp & 0x0000ff00) |
                    ((tmp & 0xff) << 16);
        }
    }

    // The Android Bitmap object size
    int bitmapwidth = origwidth;
    int bitmapheight = origheight;

    void *processedBuffer = createBitmapARGB8888(env, inSampleSize, origBuffer, &bitmapwidth, &bitmapheight, origwidth, origheight);

    //Create mutable bitmap
    jclass bitmapClass = env->FindClass("android/graphics/Bitmap");
    jmethodID methodid = env->GetStaticMethodID(bitmapClass, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
    // Get the prefered config
    jclass bitmapConfig = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID argb8888FieldID = env->GetStaticFieldID(bitmapConfig, "ARGB_8888", "Landroid/graphics/Bitmap$Config;");
    jobject config = env->GetStaticObjectField(bitmapConfig, argb8888FieldID);
    // Call Bitmap.createBitmap(...)
    jobject java_bitmap = env->CallStaticObjectMethod(bitmapClass, methodid, bitmapwidth, bitmapheight, config);
    // Remove everything that is not needed
    env->DeleteLocalRef(bitmapConfig);
    env->DeleteLocalRef(config);

    // Copy the data to bitmap
    int ret;
    void *bitmapPixels;
    if ((ret = AndroidBitmap_lockPixels(env, java_bitmap, &bitmapPixels)) < 0) {
        //error
        LOGE("Lock pixels failed");
        return NULL;
    }
    int pixelsCount = bitmapwidth * bitmapheight;

    // Do the copy
    memcpy(bitmapPixels, (jint *) processedBuffer, sizeof(jint) * pixelsCount);

    // Unlock the pixels
    AndroidBitmap_unlockPixels(env, java_bitmap);
    // Remove the array
    delete[] (jint *) processedBuffer;

    // Remove the Bitmap class object
    env->DeleteLocalRef(bitmapClass);

    return java_bitmap;
}

/**
 * Creates the ARGB8888 Bitmap
 */
jint* TiffImage::createBitmapARGB8888(JNIEnv *env, int inSampleSize, unsigned int *buffer, int *bitmapwidth, int *bitmapheight, int origwidth, int origheight) {
    jint *pixels = NULL;
    if (inSampleSize > 1) {
        *bitmapwidth = origwidth / inSampleSize;
        *bitmapheight = origheight / inSampleSize;
        int pixelsBufferSize = *bitmapwidth * *bitmapheight;
        pixels = (jint *) malloc(sizeof(jint) * pixelsBufferSize);
        if (pixels == NULL) {
            LOGE("Can\'t allocate memory for temp buffer");
            return NULL;
        }
        else {
            for (int i = 0, i1 = 0; i < *bitmapwidth; i++, i1 += inSampleSize) {
                for (int j = 0, j1 = 0; j < *bitmapheight; j++, j1 += inSampleSize) {

                    //Apply filter to pixel
                    jint crPix = buffer[j1 * origwidth + i1];
                    int sum = 1;

                    int alpha = colorMask & crPix >> 24;
                    int red = colorMask & crPix >> 16;
                    int green = colorMask & crPix >> 8;
                    int blue = colorMask & crPix;

                    //using kernel 3x3

                    //topleft
                    if (i1 - 1 >= 0 && j1 - 1 >= 0) {
                        crPix = buffer[(j1 - 1) * origwidth + i1 - 1];
                        red += colorMask & crPix >> 16;
                        green += colorMask & crPix >> 8;
                        blue += colorMask & crPix;
                        alpha += colorMask & crPix >> 24;
                        sum++;
                    }
                    //top
                    if (j1 - 1 >= 0) {
                        crPix = buffer[(j1 - 1) * origwidth + i1];
                        red += colorMask & crPix >> 16;
                        green += colorMask & crPix >> 8;
                        blue += colorMask & crPix;
                        alpha += colorMask & crPix >> 24;
                        sum++;
                    }
                    // topright
                    if (i1 + 1 < origwidth && j1 - 1 >= 0) {
                        crPix = buffer[(j1 - 1) * origwidth + i1 + 1];
                        red += colorMask & crPix >> 16;
                        green += colorMask & crPix >> 8;
                        blue += colorMask & crPix;
                        alpha += colorMask & crPix >> 24;
                        sum++;
                    }
                    //right
                    if (i1 + 1 < origwidth) {
                        crPix = buffer[j1 * origwidth + i1 + 1];
                        red += colorMask & crPix >> 16;
                        green += colorMask & crPix >> 8;
                        blue += colorMask & crPix;
                        alpha += colorMask & crPix >> 24;
                        sum++;
                    }
                    //bottomright
                    if (i1 + 1 < origwidth && j1 + 1 < origheight) {
                        crPix = buffer[(j1 + 1) * origwidth + i1 + 1];
                        red += colorMask & crPix >> 16;
                        green += colorMask & crPix >> 8;
                        blue += colorMask & crPix;
                        alpha += colorMask & crPix >> 24;
                        sum++;
                    }
                    //bottom
                    if (j1 + 1 < origheight) {
                        crPix = buffer[(j1 + 1) * origwidth + i1 + 1];
                        red += colorMask & crPix >> 16;
                        green += colorMask & crPix >> 8;
                        blue += colorMask & crPix;
                        alpha += colorMask & crPix >> 24;
                        sum++;
                    }
                    //bottomleft
                    if (i1 - 1 >= 0 && j1 + 1 < origheight) {
                        crPix = buffer[(j1 + 1) * origwidth + i1 - 1];
                        red += colorMask & crPix >> 16;
                        green += colorMask & crPix >> 8;
                        blue += colorMask & crPix;
                        alpha += colorMask & crPix >> 24;
                        sum++;
                    }
                    //left
                    if (i1 - 1 >= 0) {
                        crPix = buffer[j1 * origwidth + i1 - 1];
                        red += colorMask & crPix >> 16;
                        green += colorMask & crPix >> 8;
                        blue += colorMask & crPix;
                        alpha += colorMask & crPix >> 24;
                        sum++;
                    }

                    red /= sum;
                    if (red > 255) red = 255;
                    if (red < 0) red = 0;

                    green /= sum;
                    if (green > 255) green = 255;
                    if (green < 0) green = 0;

                    blue /= sum;
                    if (blue > 255) blue = 255;
                    if (blue < 0) blue = 0;

                    alpha /= sum;///= sum;
                    if (alpha > 255) alpha = 255;
                    if (alpha < 0) alpha = 0;

                    crPix = (alpha << 24) | (red << 16) | (green << 8) | (blue);

                    pixels[j * *bitmapwidth + i] = crPix;
                }
            }
        }
    }
    else {
        int bufferSize = *bitmapwidth * *bitmapheight;
        pixels = (jint *) malloc(sizeof(jint) * bufferSize);
        memcpy(pixels, buffer, bufferSize * sizeof(jint));
    }

    //Close Buffer
    if (buffer) {
        _TIFFfree(buffer);
        buffer = NULL;
    }
    return pixels;
}