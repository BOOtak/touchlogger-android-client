package org.leyfer.thesis.touchlogger_dirty.utils.crypto;

import android.util.Base64;

import org.leyfer.thesis.touchlogger_dirty.exception.EncryptAESException;
import org.leyfer.thesis.touchlogger_dirty.exception.EncryptRSAException;
import org.leyfer.thesis.touchlogger_dirty.exception.GenerateKeyException;
import org.leyfer.thesis.touchlogger_dirty.exception.PublicKeyException;

import java.security.InvalidKeyException;
import java.security.KeyFactory;
import java.security.NoSuchAlgorithmException;
import java.security.PublicKey;
import java.security.spec.InvalidKeySpecException;
import java.security.spec.X509EncodedKeySpec;

import javax.crypto.BadPaddingException;
import javax.crypto.Cipher;
import javax.crypto.IllegalBlockSizeException;
import javax.crypto.KeyGenerator;
import javax.crypto.NoSuchPaddingException;
import javax.crypto.SecretKey;

public class CryptoUtils {
    private static final String publicKeyPem =
            "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEA3RTlanDGDOcGuDp/SQBc" +
                    "e4Qi3IipasyS7gk0JLMDYWB9Qql6/By7d2enhErMAGTnPcA2mIaJdINAFO+rXcw/" +
                    "ANQ158XhqFRn+zKXdpw2nw8SV9s1iZEY33Wg8NNXKA2g6bwPXfywVEaQVM2lePW7" +
                    "MY9Sdus7w9cdtOUv+DYAZouZt1u3F0sKkvxFaGxVQYYvV6CbosAM8lnZzzYIaid/" +
                    "z6lhviBxN+q+nq2aDDxwkOJvaO+oWN/WI/aq66pVV3Xvp4+l86P4B3BNbFIci/U5" +
                    "fuQfxKF1QCSB1R/yj/BEhojAAFQuOEPpTNAwRyBeyS0yEjIzShdwmDlraCexrpcH" +
                    "oQIDAQAB";

    private static final int AES_KEY_SIZE = 128;

    public static PublicKey getPublicKey() {
        byte[] encoded = Base64.decode(publicKeyPem, Base64.NO_WRAP);
        X509EncodedKeySpec keySpec = new X509EncodedKeySpec(encoded);
        try {
            KeyFactory kf = KeyFactory.getInstance("RSA");
            return kf.generatePublic(keySpec);
        } catch (NoSuchAlgorithmException | InvalidKeySpecException e) {
            throw new PublicKeyException("Unable to get public key from PEM data!", e);
        }
    }

    public static byte[] encryptRSA(PublicKey publicKey, byte[] data) {
        try {
            Cipher cipher = Cipher.getInstance("RSA/ECB/PKCS1Padding");
            cipher.init(Cipher.ENCRYPT_MODE, publicKey);
            return cipher.doFinal(data);
        } catch (NoSuchAlgorithmException | NoSuchPaddingException | InvalidKeyException |
                BadPaddingException | IllegalBlockSizeException e) {
            throw new EncryptRSAException("Unable to encrypt data with RSA!", e);
        }
    }

    public static SecretKey generateAESKey() {
        KeyGenerator keyGen;

        try {
            keyGen = KeyGenerator.getInstance("AES");
        } catch (NoSuchAlgorithmException e) {
            throw new GenerateKeyException("Unable to generate AES key!", e);
        }

        keyGen.init(AES_KEY_SIZE);
        return keyGen.generateKey();
    }

    public static SymmetricEncryptionResult encryptAES(SecretKey sessionKey, byte[] data) {
        try {
            Cipher cipher = Cipher.getInstance("AES/CBC/PKCS5Padding");
            cipher.init(Cipher.ENCRYPT_MODE, sessionKey);
            return new SymmetricEncryptionResult(cipher.doFinal(data), cipher.getIV());
        } catch (NoSuchAlgorithmException | NoSuchPaddingException | InvalidKeyException
                | BadPaddingException | IllegalBlockSizeException e) {
            throw new EncryptAESException("Unable to encrypt data with AES algorithm!", e);
        }
    }
}
