#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
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


#define NULL 0
#define TRUE 1
#define KEY 28
#define NANOSECOND 1000000000.0

wchar_t polishAlphabet[] = L"aąbcćdeęfghijklłmnńoópqrstuvwxyzźż";

#define ALPHLEN (sizeof(polishAlphabet)/sizeof(wchar_t) - 1)


int getCharIndexInAlphabet(wchar_t ch) {
	for (int i = 0; i < ALPHLEN; i++) {
		if (ch == polishAlphabet[i]) {
			return i;
		}
	}
	return -1;
}

void CaesarCipher(char* inputFileName, char* outputFileName) {
	FILE* iF = NULL;
	FILE* oF = NULL;
	wchar_t ch;;
	ssize_t rB;

	iF = fopen(inputFileName, "r");
	if (!iF) {
		printf("Error: file not opened!");
		return;
	}

	oF = fopen(outputFileName, "w");
	if (!oF) {
		printf("Error: file not opened!");
		fclose(iF);
		return;
	}

	while ((ch = fgetwc(iF)) != WEOF) {
		int charIndex = getCharIndexInAlphabet(ch);
		if (charIndex != -1) {
			fputwc(polishAlphabet[(charIndex + KEY) % ALPHLEN], oF);
		}
		else {
			fputwc(ch, oF);
		}
	}

	fclose(iF);
	fclose(oF);
}

void CaesarDecipher(char* inputFileName, char* outputFileName) {
	FILE* iF = NULL;
	FILE* oF = NULL;
	wchar_t ch;
	ssize_t rB;

	iF = fopen(inputFileName, "r");
	if (!iF) {
		printf("Error: file not opened!");
		return;
	}

	oF = fopen(outputFileName, "w");
	if (!oF) {
		printf("Error: file not opened!");
		fclose(iF);
		return;
	}

	while ((ch = fgetwc(iF)) != WEOF) {
		int charIndex = getCharIndexInAlphabet(ch);
		if (charIndex != -1) {
			fputwc(polishAlphabet[(charIndex - KEY + ALPHLEN) % ALPHLEN], oF);
		}
		else {
			fputwc(ch, oF);
		}
	}

	fclose(iF);
	fclose(oF);
}


void PortCipher(char* inputFileName, char* outputFileName) {
	FILE* iF = fopen(inputFileName, "r");
	if (!iF) {
		printf("Error: file not opened!");
		return;
	}

	FILE* oF = fopen(outputFileName, "w");
	if (!oF) {
		printf("Error: file not opened!");
		fclose(iF);
		return;
	}

	wchar_t first = 0;
	int hasFirst = 0;
	wchar_t ch;

	while ((ch = fgetwc(iF)) != WEOF) {
		ch = tolower(ch);
		int index = getCharIndexInAlphabet(ch);
		if (index == -1)
			continue;

		if (!hasFirst) {
			first = ch;
			hasFirst = 1;
		}
		else {
			int r = getCharIndexInAlphabet(first);
			int c = index;

			int num = r * ALPHLEN + c + 1;
			fwprintf(oF, L"%03d ", num);

			hasFirst = 0;
		}
	}

	if (hasFirst) {
		int r = getCharIndexInAlphabet(first);
		int c = 0;

		int num = r * ALPHLEN + c + 1;
		fwprintf(oF, L"%03d ", num);
	}

	fclose(iF);
	fclose(oF);
}



void PortDecipher(char* inputFileName, char* outputFileName) {
	FILE* iF = fopen(inputFileName, "r");
	if (!iF) {
		printf("Error: file not opened!");
		return;
	}

	FILE* oF = fopen(outputFileName, "w");
	if (!oF) {
		printf("Error: file not opened!");
		fclose(iF);
		return;
	}

	int num;

	while (fwscanf(iF, L"%d", &num) == 1) {

		if (num <= 0 || num > ALPHLEN * ALPHLEN) {
			continue;
		}
		num -= 1;

		int r = num / ALPHLEN;
		int c = num % ALPHLEN;

		if (r < 0 || r >= ALPHLEN || c < 0 || c >= ALPHLEN) {
			continue;
		}

		fputwc(polishAlphabet[r], oF);
		fputwc(polishAlphabet[c], oF);
	}

	fclose(iF);
	fclose(oF);
}


struct timespec beginRealTime, endRealTime;

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");
	if (argc < 5) {
		printf("Error: you must input source fileName, dist fileName and second dist fileName for decipher and key!\n");
		return EXIT_FAILURE;
	}
	if (strcmp(argv[4], "-c") == 0) {
		clock_gettime(CLOCK_REALTIME, &beginRealTime);
		CaesarCipher(argv[1], argv[2]);
		clock_gettime(CLOCK_REALTIME, &endRealTime);
		double diffRealTime = (endRealTime.tv_sec - beginRealTime.tv_sec) + (endRealTime.tv_nsec - beginRealTime.tv_nsec) / NANOSECOND;
		printf("Time cipher: %.3f\n", diffRealTime);
		clock_gettime(CLOCK_REALTIME, &beginRealTime);
		CaesarDecipher(argv[2], argv[3]);
		clock_gettime(CLOCK_REALTIME, &endRealTime);
		diffRealTime = (endRealTime.tv_sec - beginRealTime.tv_sec) + (endRealTime.tv_nsec - beginRealTime.tv_nsec) / NANOSECOND;
		printf("Time decipher: %.3f\n", diffRealTime);
	}
	else if (strcmp(argv[4], "-p") == 0) {
		clock_gettime(CLOCK_REALTIME, &beginRealTime);
		PortCipher(argv[1], argv[2]);
		clock_gettime(CLOCK_REALTIME, &endRealTime);
		double diffRealTime = (endRealTime.tv_sec - beginRealTime.tv_sec) + (endRealTime.tv_nsec - beginRealTime.tv_nsec) / NANOSECOND;
		printf("Time cipher: %.3f\n", diffRealTime);
		clock_gettime(CLOCK_REALTIME, &beginRealTime);
		PortDecipher(argv[2], argv[3]);
		clock_gettime(CLOCK_REALTIME, &endRealTime);
		diffRealTime = (endRealTime.tv_sec - beginRealTime.tv_sec) + (endRealTime.tv_nsec - beginRealTime.tv_nsec) / NANOSECOND;
		printf("Time decipher: %.3f\n", diffRealTime);
	}
	return EXIT_SUCCESS;
}