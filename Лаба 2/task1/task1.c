#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<sys/stat.h>
#include <time.h>
#include<sys/types.h>
#include<sys/sysmacros.h>
#include<dirent.h>
#include <sys/inotify.h>
#include <aio.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <signal.h>
#include <errno.h>
#include <math.h>

#define NULL 0
#define TRUE 1


char englishAlphabet[] = "abcdefghijklmnopqrstuvwxyz";
char russianAlphabet[] = "àáâãäå¸æçèéêëìíîïðñòóôõö÷øùúûüýþÿ";
void calcSymbolsFrequency(char* alph, char* file, int* allCount, int* freq) {
  int f;
  int alphLen = strlen(alph);
  unsigned char buf[8192];
  ssize_t rB;

  f = open(file, O_RDONLY);
  if (f == -1) {
    printf("Error: file not opened!\n");
    return;
  }

  while ((rB = read(f, buf, sizeof(buf))) > 0) {
    for (int i = 0; i < rB; i++) {
      for (int j = 0; j < alphLen; j++) {
        if (buf[i] == (unsigned char)alph[j]) {
          freq[j]++;
          (*allCount)++;
          break;
        }
      }
    }
  }

  close(f);
}

double calcEntropy(int* freq, int alphLen, int allCount) {
  double entropy = 0;

  if (allCount == 0) {
    return 0;
  }

  for (int i = 0; i < alphLen; i++) {
    if (freq[i] > 0) {
      double p = (double)freq[i] / allCount;
      entropy += p * log2(p);
    }
  }

  return -entropy;
}

void printFrequency(char* alph, int* freq, int alphLen, int allCount) {
  printf("\nSymbol frequencies:\n");

  for (int i = 0; i < alphLen; i++) {
    double p = 0;

    if (allCount > 0) {
      p = (double)freq[i] / allCount;
    }

    printf("%c: %d  p = %.5f\n", alph[i], freq[i], p);
  }
}

void calcBinaryFrequency(char* file, int* zeros, int* ones) {
  int f;
  unsigned char buf[8192];
  ssize_t rB;

  f = open(file, O_RDONLY);
  if (f == -1) {
    printf("Error: file not opened!\n");
    return;
  }

  while ((rB = read(f, buf, sizeof(buf))) > 0) {
    for (int i = 0; i < rB; i++) {
      for (int bit = 0; bit < 8; bit++) {
        if (buf[i] & (1 << bit)) {
          (*ones)++;
        }
        else {
          (*zeros)++;
        }
      }
    }
  }

  close(f);
}

double calcBinaryEntropy(int zeros, int ones) {
  int total = zeros + ones;
  double entropy = 0;

  if (total == 0) {
    return 0;
  }

  if (zeros > 0) {
    double p0 = (double)zeros / total;
    entropy += p0 * log2(p0);
  }

  if (ones > 0) {
    double p1 = (double)ones / total;
    entropy += p1 * log2(p1);
  }

  return -entropy;
}

double calcErrorEntropy(double p) {
  double q = 1 - p;
  double h = 0;

  if (p > 0) {
    h += p * log2(p);
  }

  if (q > 0) {
    h += q * log2(q);
  }

  return -h;
}

int countLetters(char* text) {
  int count = 0;

  for (int i = 0; text[i] != '\0'; i++) {
    if (text[i] != ' ') {
      count++;
    }
  }

  return count;
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Please, input filename!\n");
    return -1;
  }

  char fio[] = "Podshivalenko Diana Igorevna";

  int allCountEn = 0;
  int* freqEn = (int*)calloc(strlen(englishAlphabet), sizeof(int));

  calcSymbolsFrequency(englishAlphabet, argv[1], &allCountEn, freqEn);

  double entropyEn = calcEntropy(freqEn, strlen(englishAlphabet), allCountEn);

  printf("\n=== English alphabet ===\n");
  printf("English symbols count = %d\n", allCountEn);
  printf("English entropy = %.4f bit/symbol\n", entropyEn);
  printFrequency(englishAlphabet, freqEn, strlen(englishAlphabet), allCountEn);

  int allCountRu = 0;
  int* freqRu = (int*)calloc(strlen(russianAlphabet), sizeof(int));

  calcSymbolsFrequency(russianAlphabet, argv[1], &allCountRu, freqRu);

  double entropyRu = calcEntropy(freqRu, strlen(russianAlphabet), allCountRu);

  printf("\n=== Russian alphabet ===\n");
  printf("Russian symbols count = %d\n", allCountRu);
  printf("Russian entropy = %.4f bit/symbol\n", entropyRu);
  printFrequency(russianAlphabet, freqRu, strlen(russianAlphabet), allCountRu);

  int zeros = 0;
  int ones = 0;

  calcBinaryFrequency(argv[1], &zeros, &ones);

  double entropyBin = calcBinaryEntropy(zeros, ones);

  printf("\n=== Binary alphabet ===\n");
  printf("Zeros = %d\n", zeros);
  printf("Ones  = %d\n", ones);
  printf("Binary entropy = %.4f bit/bit\n", entropyBin);

  printf("\n=== Information amount for full name ===\n");
  printf("Message: %s\n", fio);

  int lettersCount = countLetters(fio);
  int asciiBits = strlen(fio) * 8;

  double infoByEnglish = lettersCount * entropyEn;
  double infoByBinary = asciiBits * entropyBin;

  printf("Symbols without spaces = %d\n", lettersCount);
  printf("ASCII bits = %d\n", asciiBits);

  printf("I by English alphabet entropy = %.4f bits\n", infoByEnglish);
  printf("I by binary entropy = %.4f bits\n", infoByBinary);

  printf("\n=== Binary channel with errors ===\n");

  double errors[] = { 0.1, 0.5, 1.0 };

  for (int i = 0; i < 3; i++) {
    double p = errors[i];
    double hyx = calcErrorEntropy(p);
    double he = 1 - hyx;
    double info = asciiBits * he;

    printf("p = %.1f: H(Y|X) = %.4f, He = %.4f, I = %.4f bits\n",
      p, hyx, he, info);
  }

  free(freqEn);
  free(freqRu);

  return 0;
}