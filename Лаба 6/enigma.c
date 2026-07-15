#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
// #include<string>
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

#define ALPHABET_LEN 26

int startPosL, startPosM, startPosR;

// 0 - обычный алфавит(рефлектор)
// 1 - первый ротор
// 2 - второй ротор
// 3 - третий ротор

#define BASE_ALPHABET 0
#define BETA_ROTOR 1
#define III_ROTOR 2
#define GAMMA_ROTOR 3

char BetaRotor[] =  "LEYJVCNIXWPBQMDRTAKZGFUHOS";
char IIIRotor[] =   "BDFHJLCPRTXVZNYEIWGAKMUSQO";
char GammaRotor[] = "FSOKANUERHMBTIYCWLQPZXVGJD";

char reflektorFirst[] =  "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char reflektorSecond[] = "FVPJIAOYEDRZXWGCTKUQSBNMHL";

int getIndexInAlphabet(char sym, short alpNum) {
  if (alpNum == 0) {
    for (int i = 0; i < ALPHABET_LEN; i++) {
      if (reflektorFirst[i] == sym) {
        return i;
      }
    }
  }
  else if (alpNum == 1) {
    for (int i = 0; i < ALPHABET_LEN; i++) {
      if (BetaRotor[i] == sym) {
        return i;
      }
    }
  }
  else if (alpNum == 2) {
    for (int i = 0; i < ALPHABET_LEN; i++) {
      if (IIIRotor[i] == sym) {
        return i;
      }
    }
  }
  else if (alpNum == 3) {
    for (int i = 0; i < ALPHABET_LEN; i++) {
      if (GammaRotor[i] == sym) {
        return i;
      }
    }
  }
  return -1;
}


int main(int argc, char* argv[]) {
  if (argc < 5) {
    printf("You must input text and pos for 3 rotors\n");
    return EXIT_FAILURE;
  }

  startPosL = atoi(argv[2]);
  startPosM = atoi(argv[3]);
  startPosR = atoi(argv[4]);
  
  printf("Text to be encrypted: %s\n", argv[1]);
  int textLen = strlen(argv[1]);

  char* res = (char*)calloc(textLen, sizeof(char));

  for (int i = 0; i < textLen; i++) {
    startPosR += 4;
    if (startPosR >= ALPHABET_LEN) {
      startPosM += 1;
      startPosR %= ALPHABET_LEN;
    }
    if (startPosM >= ALPHABET_LEN) {
      startPosL += 1;
      startPosM %= ALPHABET_LEN;
    }

    char bufSym = argv[1][i];
    int idx = bufSym - 'A';

    // --- ПРЯМОЙ ХОД ---

    // R (Beta)
    idx = (idx + startPosR) % ALPHABET_LEN;
    idx = BetaRotor[idx] - 'A';
    idx = (idx - startPosR + ALPHABET_LEN) % ALPHABET_LEN;

    // M (III)
    idx = (idx + startPosM) % ALPHABET_LEN;
    idx = IIIRotor[idx] - 'A';
    idx = (idx - startPosM + ALPHABET_LEN) % ALPHABET_LEN;

    // L (Gamma)
    idx = (idx + startPosL) % ALPHABET_LEN;
    idx = GammaRotor[idx] - 'A';
    idx = (idx - startPosL + ALPHABET_LEN) % ALPHABET_LEN;

    // --- РЕФЛЕКТОР ---
    idx = reflektorSecond[idx] - 'A';

    // --- ОБРАТНЫЙ ХОД ---

    // L
    idx = (idx + startPosL) % ALPHABET_LEN;
    idx = getIndexInAlphabet(idx + 'A', GAMMA_ROTOR);
    idx = (idx - startPosL + ALPHABET_LEN) % ALPHABET_LEN;

    // M
    idx = (idx + startPosM) % ALPHABET_LEN;
    idx = getIndexInAlphabet(idx + 'A', III_ROTOR);
    idx = (idx - startPosM + ALPHABET_LEN) % ALPHABET_LEN;

    // R
    idx = (idx + startPosR) % ALPHABET_LEN;
    idx = getIndexInAlphabet(idx + 'A', BETA_ROTOR);
    idx = (idx - startPosR + ALPHABET_LEN) % ALPHABET_LEN;

    res[i] = idx + 'A';

  }

  printf("Result: %s\n", res);
}