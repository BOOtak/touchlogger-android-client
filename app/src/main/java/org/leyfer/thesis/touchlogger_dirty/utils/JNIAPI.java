package org.leyfer.thesis.touchlogger_dirty.utils;

/**
 * Created by kirill on 18.03.17.
 */

public class JNIAPI {

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public static native String stringFromJNI();

    /**
     * Copy file from source to destination.
     *
     * @param srcPath Path to source file.
     * @param dstPath Path to destination file.
     * @return True on success, false otherwise.
     */
    public static native boolean nativeCopy(String srcPath, String dstPath);

    /**
     * Oh, exploitable!
     * Temporarily replace read-only file with another one. Content of source file will be restored
     * after reboot.
     * This is done via CVE-2016-5195 vulnerability which works on all android devices
     * security updates installed before Dec. 2016.
     * WARNING! On some rare occasions this (and following) method may cause PERMANENT file changes.
     * I.e. files will stay same AFTER REBOOT. Mechanism of such behaviour is unknown. Remember to
     * restore all file contents when you're done playing with exploits.
     *
     * @param srcPath Path to replacement.
     * @param dstPath Path to file to be replaced.
     * @return True on success, false otherwise.
     */
    public static native boolean dirtyCopy(String srcPath, String dstPath);

    /**
     * Add new dependency to shared library. This is done via parsing ELF file and replacing
     * SONAME section with DT_NEEDED with new name. This works only for shared libraries and wont
     * work for executables as they don't contain SONAME section.
     *
     * @param path           Path to library to inject dependency to.
     * @param dependencyName Name of new dependency.
     * @return True on success, false otherwise.
     */
    public static native boolean injectDependencyIntoLinrary(String path, String dependencyName);

    /**
     * Replace one dependency in ELF file. This works through replacing DT_NEEDED value in ELF file
     * with given in arguments. This method works either for both shared libraries and executables.
     *
     * @param path          Path to ELF file.
     * @param oldDependency Name of dependency to be replaced.
     * @param newDependency Name of new dependency.
     * @return True on success, false otherwise.
     */
    public static native boolean replaceDependencyInBinary(String path, String oldDependency, String newDependency);

    public static native void restartZygote(String garbagePath, String backupPath);
}
