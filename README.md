# TiffLoader-Android
TiffLoader-Android is an Android library that allows you to load TIFF images.

This library is a fork of [Android-TiffBitmapFactory](https://github.com/Beyka/Android-TiffBitmapFactory)

For decoding the TIFF images it uses the native library [libtiff](https://github.com/dumganhar/libtiff)

### Supported platforms

At the moment the supported platforms are: **x86 armeabi armeabi-v7a**

### Usage
```Java
try {
    // Read the TIFF file
    TiffLoader loader = new TiffLoader(getApplicationContext(), getAssets().open("test.tif"));

    // Get the number of directories (pages)
    int pages = loader.getDirectoryCount();

    // Print the sizes of the pages
    for (int i = 0; i < pages; i++) {
        int[] size = loader.getSizeForDirectory(i);
        Log.d("App", String.format("The page %d has the size: %d x %d", i, size[0], size[1]));
    }

    // Get the second page (original size)
    Bitmap secondPageOriginalSize = loader.getBitmap(
            1, // The page index
            1); // Indicating we want the original size
    // Get the first page (half the size)
    Bitmap firstPageHalfSize = loader.getBitmap(
            0, // The page index
            2); // Indicating we want the originalSize / 2

    // Do magic with the bitmaps
    // ...
    // ...

    // Don't forget to RECYCLE!
    firstPageHalfSize.recycle();
    secondPageOriginalSize.recycle();

    // And ALWAYS destroy the object!
    loader.destroy();
} catch (IOException e) {
    Log.e("App", "Error while reading asset", e);
} catch (TiffLoadFailedException e) {
    Log.e("App", "Error while reading the TIFF file", e);
} catch (OutOfMemoryError e) {
    Log.e("App", "Oh no! You don't have enough memory to load the TIFF page!", e);
}
```

### Build
To build the native part of the library use [Android-NDK-bundle-10](https://developer.android.com/tools/sdk/ndk/index.html) or higher.

Go to the root directory and run
```
ndk-build NDK_PROJECT_PATH=src/main
```

This is mandatory until Google releases a stable version of the gradle android plugin with NDK support.


License
=======

    The MIT License (MIT)

    Copyright (c) 2015, Dimitar Kotevski

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.