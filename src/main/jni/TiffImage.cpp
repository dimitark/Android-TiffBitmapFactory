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