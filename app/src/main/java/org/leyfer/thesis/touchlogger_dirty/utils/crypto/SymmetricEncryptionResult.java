package org.leyfer.thesis.touchlogger_dirty.utils.crypto;

public class SymmetricEncryptionResult {
    private final byte[] IV;
    private final byte[] encryptedData;

    public SymmetricEncryptionResult(byte[] encryptedData, byte[] IV) {
        this.encryptedData = encryptedData;
        this.IV = IV;
    }

    public byte[] getIV() {
        return IV;
    }

    public byte[] getEncryptedData() {
        return encryptedData;
    }
}
