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


#define NULL 0
#define TRUE 1

#define N 6
#define KEY_LEN 6
unsigned char key[] = {15, 14, 13, 12, 11, 10};

// инициализация массива S и его перемешивание нашим ключом
void KSA(
  unsigned char S[],
  unsigned char key[],
  int n
) {
  int i, j = 0;
  for (i = 0; i < n; i++) {
    S[i] = i;
  }
  for (i = 0; i < n; i++) {
    j = (j + S[i] + key[i % KEY_LEN]) % n;

    unsigned char buf = S[i];
    S[i] = S[j];
    S[j] = buf;
  }
}

void PRGA(
  unsigned char S[],
  unsigned char K[],
  int len,
  int n
) {
  int i = 0, j = 0;

  for (int k = 0; k < len; k++) {
    i = (i + 1) % n;
    j = (j + S[i]) % n;

    unsigned char buf = S[i];
    S[i] = S[j];
    S[j] = buf;

    int a = (S[i] + S[j]) % n;
    K[k] = S[a];
  }
}

void rc4(
  unsigned char *inp,
  unsigned char K[],
  unsigned char *out,
  int len
) {

  for (int i = 0; i < len; i++) {
    out[i] = inp[i] ^ K[i];
  }
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("You must input text\n");
    return 1;
  }

  unsigned char S[N];

  unsigned char* text = (unsigned char*)argv[1];
  int textLen = strlen((char*)text);

  unsigned char* K = (unsigned char*)malloc(textLen);
  unsigned char* encrypted = (unsigned char*)malloc(textLen);
  unsigned char* decrypted = (unsigned char*)malloc(textLen);

  KSA(S, key, N);

  clock_t start = clock();

  PRGA(S, K, textLen, N);

  clock_t end = clock();

  double time_ = (double)(end - start) / CLOCKS_PER_SEC;
  printf("Time: %f sec\n", time_);

  rc4(text, K, encrypted, textLen);

  printf("Encrypted (hex): ");
  for (int i = 0; i < textLen; i++) {
    printf("%02X ", encrypted[i]);
  }
  printf("\n");

  unsigned char* K_dec = (unsigned char*)malloc(textLen);
  unsigned char S_dec[N];

  KSA(S_dec, key, N);      
  PRGA(S_dec, K_dec, textLen, N);  
  rc4(encrypted, K_dec, decrypted, textLen);

  printf("Decrypted: ");
  for (int i = 0; i < textLen; i++) {
    printf("%c", decrypted[i]);
  }
  printf("\n");

  free(K);
  free(encrypted);
  free(decrypted);
  free(K_dec);
  return EXIT_SUCCESS;
}
