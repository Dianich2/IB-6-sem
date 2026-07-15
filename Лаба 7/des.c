#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>
#include<sys/stat.h>
#include <time.h>
#include<sys/types.h>
#include<sys/sysmacros.h>
#include <sys/wait.h>
#include<dirent.h>
#include <sys/inotify.h>
#include <aio.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <math.h>
#include <ctype.h>
#include <wchar.h>
#include <locale.h>
#include <pthread.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <sched.h>
#include <stddef.h>
#include <semaphore.h>
#include <stdbool.h>
#include <string.h>
#include <openssl/des.h>
#include <openssl/rand.h>
#include <openssl/evp.h>


#define NULL 0
#define TRUE 1

#define ALPHABET_LEN 26

#define FIRST_KEY "PODSHIVA"
#define SECOND_KEY "LENKODIA"

#define BLOCK_SIZE 8

unsigned char* delTextInBlocks(
  char *inp, 
  size_t curLen, 
  size_t* fullLen
) {
  size_t dopLen = BLOCK_SIZE - (curLen % BLOCK_SIZE);
  *fullLen = curLen + dopLen;

  unsigned char* fullTextWithDop = (unsigned char*)malloc(*fullLen);
  memcpy(fullTextWithDop, inp, curLen);
  for (int i = curLen; i < *fullLen; i++) {
    fullTextWithDop[i] = dopLen; // PKCS#5 padding(каждый доп байт равен числу байтов, которые нужно дополнить)
  }
  return fullTextWithDop;
}

void encrypt(
  unsigned char *inp,
  unsigned char *out,
  size_t len
) {
  DES_cblock key1, key2;
  DES_key_schedule ks1, ks2;

  memcpy(key1, FIRST_KEY, BLOCK_SIZE);
  memcpy(key2, SECOND_KEY, BLOCK_SIZE);

  DES_set_key_unchecked(&key1, &ks1);
  DES_set_key_unchecked(&key2, &ks2);

  for (size_t i = 0; i < len; i += BLOCK_SIZE) {
    DES_ecb2_encrypt((DES_cblock*)(inp + i),
      (DES_cblock*)(out + i),
      &ks1, &ks2,
      DES_ENCRYPT);
  }
}

void decrypt(
  const unsigned char* inp, 
  unsigned char* out, 
  size_t len
) {
  DES_cblock key1, key2;
  DES_key_schedule ks1, ks2;

  memcpy(key1, FIRST_KEY, BLOCK_SIZE);
  memcpy(key2, SECOND_KEY, BLOCK_SIZE);

  DES_set_key_unchecked(&key1, &ks1);
  DES_set_key_unchecked(&key2, &ks2);

  for (size_t i = 0; i < len; i += BLOCK_SIZE) {
    DES_ecb2_encrypt((DES_cblock*)(inp + i),
      (DES_cblock*)(out + i),
      &ks1, &ks2,
      DES_DECRYPT);
  }
}

int count_changed_bits(
  const unsigned char* a, 
  const unsigned char* b, 
  size_t len
) {
  int count = 0;
  for (size_t i = 0; i < len; i++) {
    unsigned char diff = a[i] ^ b[i];
    for (int j = 0; j < 8; j++) {
      if (diff & (1 << j)) count++;
    }
  }
  return count;
}



int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("You must input text\n");
    return 1;
  }

  const char* input_text = argv[1];
  size_t input_len = strlen(input_text);

  size_t padded_len;
  unsigned char* padded_text = delTextInBlocks((unsigned char*)input_text, input_len, &padded_len);
  unsigned char* encrypted = (unsigned char*)malloc(padded_len);
  unsigned char* decrypted = (unsigned char*)malloc(padded_len);

  clock_t start = clock();
  encrypt(padded_text, encrypted, padded_len);
  clock_t end = clock();
  double enc_time = (double)(end - start) / CLOCKS_PER_SEC;
  printf("Encryption time: %.6f sec\n", enc_time);
  printf("Encrypted text: ");
  for (size_t i = 0; i < padded_len; i++) printf("%02X", encrypted[i]);
  printf("\n");

  decrypt(encrypted, decrypted, padded_len);
  printf("Decrypted text: ");
  for (size_t i = 0; i < input_len; i++) printf("%c", decrypted[i]);
  printf("\n");

  unsigned char* test_text = (unsigned char*)malloc(padded_len);
  memcpy(test_text, padded_text, padded_len);
  test_text[0] ^= 0x01;

  unsigned char* test_encrypted = (unsigned char*)malloc(padded_len);
  encrypt(test_text, test_encrypted, padded_len);
  int changed_bits = count_changed_bits(encrypted, test_encrypted, padded_len);
  printf("Changed bits for 1-bit input change: %d\n", changed_bits);

  free(padded_text);
  free(encrypted);
  free(decrypted);
  free(test_text);
  free(test_encrypted);

  return 0;
}
