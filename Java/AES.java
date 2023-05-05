package Java;
import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

public class AES {
    public static byte[] encrypt(byte[] plaintext, byte[] key) throws Exception {
        SecretKeySpec secretKey = new SecretKeySpec(key, "AES");
        Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
        cipher.init(Cipher.ENCRYPT_MODE, secretKey);
        return cipher.doFinal(plaintext);
    }

    public static byte[] decrypt(byte[] ciphertext, byte[] key) throws Exception {
        SecretKeySpec secretKey = new SecretKeySpec(key, "AES");
        Cipher cipher = Cipher.getInstance("AES/ECB/PKCS5Padding");
        cipher.init(Cipher.DECRYPT_MODE, secretKey);
        System.out.println("ciphertext: " + cipher.doFinal(ciphertext));
        return cipher.doFinal(ciphertext);
    }
    
}