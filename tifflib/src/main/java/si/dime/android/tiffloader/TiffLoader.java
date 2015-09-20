package si.dime.android.tiffloader;

import android.content.Context;
import android.graphics.Bitmap;
import android.util.Log;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;

/**
 * Tiff loader - Reads the tiff image, and can return Bitmaps for every page
 * of the tiff in different sizes.
 *
 * Created by dime on 01/09/15.
 */
public class TiffLoader {
    // Load the native library
    static {
        System.loadLibrary("tifffactory");
    }

    // A pointer to the TiffImage C++ object
    private long pointer;

    // A temp file (not null just if called the constructor with the input stream)
    private File tempFile;

    // The current getBitmap() calls running
    private int nativeGetBitmapCounter = 0;

    // Did somebody already called destroy?
    private boolean destroyed = false;

    // The semaphore
    private Object lock = new Object();

    // region Native methods

    /**
     * Reads the file and return a pointer to the native C++ TiffImage object
     *
     * @param path
     * @return
     */
    private native long read(String path);

    /**
     * Returns true if we successfully loaded the file.
     *
     * @return
     */
    private native boolean successfullyLoaded();

    /**
     * Releases the native C++ object
     */
    private native void close();

    /**
     * Returns the Bitmap for the given directory.
     *
     * @param dir
     * @param sampleSize
     * @return
     */
    private native Bitmap nativeGetBitmap(int dir, int sampleSize);


    /**
     * Returns the number of directories in the TIFF file (number of pages)
     *
     * @return
     */
    public native int getDirectoryCount();

    /**
     * Returns the size of the given directory (width, height)
     *
     * @param dir
     * @return
     */
    public native int[] getSizeForDirectory(int dir);

    // endregion


    /**
     * Construct the loader with the given file.
     *
     * @param file
     * @throws TiffLoadFailedException
     */
    public TiffLoader(File file) throws TiffLoadFailedException {
        init(file);
    }

    /**
     * Construct the loader with the given input stream.
     * In the background a temp file (in the cache dir) is created
     * and the input stream is written in that file.
     *
     * @param context
     * @param inputStream
     * @throws TiffLoadFailedException
     */
    public TiffLoader(Context context, InputStream inputStream) throws TiffLoadFailedException {
        try {
            // Create the temp file
            writeStreamToTempFile(context, inputStream);
        } catch (IOException e) {
            throw new TiffLoadFailedException();
        }

        // Init everything
        init(tempFile);
    }

    /**
     * Destroys the objects and releases memory.
     */
    public void destroy() {
        synchronized (lock) {
            // Set the flag
            destroyed = true;

            // Check for running getBitmap()
            if (nativeGetBitmapCounter == 0) {
                actualDestroy();
            }
        }
    }

    /**
     * Actually destroys the object
     */
    private void actualDestroy() {
        // Destroy the native objects
        close();

        // Delete the temp file (if exists)
        if (tempFile != null) {
            tempFile.delete();
        }
    }


    /**
     * Returns the bitmap for the given directory.
     *
     * NOTE!!!: The caller is responsible for recycling the Bitmap!
     *
     * @param dir
     *      Which directory (page) to get
     * @param sampleSize
     *      The size of the returned image
     *      1 = original size
     *      2 = half the size (original / 2)
     *      ... originalSize / sampleSize
     *
     * @throws OutOfMemoryError
     *
     * @return
     */
    public Bitmap getBitmap(int dir, int sampleSize) throws OutOfMemoryError {
        // Check if we are destroyed
        synchronized (lock) {
            if (destroyed) {
                throw new IllegalStateException("The loader is already destroyed!");
            }

            // If not - increase the counter
            nativeGetBitmapCounter++;
        }

        Bitmap bitmap = nativeGetBitmap(dir, sampleSize);

        // Decrease the counter
        synchronized (lock) {
            // Decrease the counter & check if we need to actually destroy the object
            if (--nativeGetBitmapCounter == 0 && destroyed) {
                actualDestroy();
            }
        }

        // Check for OutOfMemory error
        if (bitmap == null) {
            throw new OutOfMemoryError();
        }
        // Return the bitmap
        return bitmap;
    }


    // region Private methods

    /**
     * Writes the input stream to a temp file (in the cache dir)
     *
     * @param is
     * @throws IOException
     */
    private void writeStreamToTempFile(Context context, InputStream is) throws IOException {
        // Create the temp file
        tempFile = new File(context.getCacheDir(), "tiff-" + System.currentTimeMillis());

        FileOutputStream fos = null;
        try {
            byte[] data = new byte[2048];
            int nbread;
            fos = new FileOutputStream(tempFile);
            while((nbread = is.read(data)) > -1){
                fos.write(data,0,nbread);
            }
        }
        catch (Exception ex) {
            Log.e("TiffLoader", "Error!" + ex.getMessage());
        }
        finally{
            if (fos != null) {
                fos.close();
            }
        }
    }

    /**
     * Does the actual init
     *
     * @param file
     * @throws TiffLoadFailedException
     *
     */
    private void init(File file) throws TiffLoadFailedException {
        // Try to load the file
        pointer = read(file.getAbsolutePath());
        // Check for errors
        if (!successfullyLoaded()) {
            throw new TiffLoadFailedException();
        }
    }

    // endregion

}
