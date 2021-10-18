#include "newplat.h"

void handleOpenSSLErrors(void)
{
    unsigned long errCode;

    printf("An openssl error occurred\n");
    while(errCode = ERR_get_error()) {
        char *err = ERR_error_string(errCode, NULL);
        printf("%s\n", err);
    }
}
/* AES_ECB_PKCS5_BASE64_Encrypt
* 入参:
* src:明文
* srcLen:明文长度
* key:密钥 长度只能是16/24/32字节 否则OPENSSL会对key进行截取或PKCS0填充
* keyLen:密钥长度
* outLen:密文base64后长度
* 返回值: 密文base64后的字符串，使用后请free
*/

#define AES_BLOCK_SIZE 16
unsigned char* AES_ECB_PKCS5_Encrypt(unsigned char *src, int srcLen, unsigned char *key, int keyLen, int *outLen, bool base64_en)
{
    EVP_CIPHER_CTX *ctx = NULL;
    int blockCount = 0;
    int quotient = srcLen / AES_BLOCK_SIZE;
    int mod = srcLen % AES_BLOCK_SIZE;
    blockCount = quotient + 1;

    int padding = AES_BLOCK_SIZE - mod;
    char *in = (char *)malloc(AES_BLOCK_SIZE*blockCount);
    memset(in, padding, AES_BLOCK_SIZE*blockCount);
    memcpy(in, src, srcLen);

    //out
    char *out = (char *)malloc(AES_BLOCK_SIZE*blockCount);
    memset(out, 0x00, AES_BLOCK_SIZE*blockCount);
    *outLen = AES_BLOCK_SIZE*blockCount;
    
    do {
        if(!(ctx = EVP_CIPHER_CTX_new())) {
            handleOpenSSLErrors();
            break;
        }   
        if(1 != EVP_EncryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
            handleOpenSSLErrors();
            break;
        }          
        if(1 != EVP_EncryptUpdate(ctx, (unsigned char*)out, outLen, in, AES_BLOCK_SIZE*blockCount)) {
            handleOpenSSLErrors();    
            break;
        }
		if(base64_en){
			//base64 encode 
			unsigned char* res = (unsigned char*)malloc(outLen * 2);
        	EVP_EncodeBlock(res, (const unsigned char*)out, outLen);
		}

    }while(0);
    if (ctx != NULL)
        EVP_CIPHER_CTX_free(ctx);
	if(base64_en){	    
    	free(in);
   		free(out);
    	return (unsigned char*)res;
	} else {
	    free(in);
    	return (unsigned char*)out;	
	}

}

/* AES_ECB_PKCS5_Decrypt
* 入参:
* src:base64编码后的密文
* srcLen:密文长度
* key:密钥 长度只能是16/24/32字节 否则OPENSSL会对key进行截取或PKCS0填充
* keyLen:密钥长度
* outLen:明文长度
* 返回值: 明文 需要free
*/
unsigned char* AES_ECB_PKCS5_Decrypt(unsigned char *src, int srcLen, unsigned char *key, int keyLen, int *outLen, bool base64_en)
{
    EVP_CIPHER_CTX *ctx = NULL;
	unsigned char* in = (unsigned char*)malloc(srcLen);
    char *out = (char*)malloc(srcLen);
    do {
		if(base64_en){	 
			if((int textlen = EVP_DecodeBlock(in, (const unsigned char*)src, srcLen)) < 0) {
            	handleOpenSSLErrors();
            	break;
        	}
		}
		if(!(ctx = EVP_CIPHER_CTX_new())) {
            handleOpenSSLErrors();
            break;
        }
        if(1 != EVP_DecryptInit_ex(ctx, EVP_aes_128_ecb(), NULL, key, NULL)) {
            handleOpenSSLErrors();
            break;
        }
		if(base64_en){
			if(1 != EVP_DecryptUpdate(ctx, out, outLen, in, srcLen*3/4)) {
				handleOpenSSLErrors();
				break;
			}
		} else {
			if(1 != EVP_DecryptUpdate(ctx, out, outLen, src, srcLen)) {
				handleOpenSSLErrors();
				break;
			}
		}
    }while(0);
    free(in);
    if (ctx != NULL)
        EVP_CIPHER_CTX_free(ctx);
    
    //PKCS5 UNPADDING
    int unpadding = out[*outLen - 1];
    *outLen = *outLen - unpadding;
    out[*outLen] = '\0';
    return (unsigned char*)out;
}