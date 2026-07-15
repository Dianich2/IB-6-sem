#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string>
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
#include <vector>
#include <string.h>

#define NULL 0
#define TRUE 1

#define COUNT_OF_COLS 20

#define NAME "Diana"
#define SURNAME "Podshivalenko"

#define NAME_LEN 5
#define SURNAME_LEN 13
#define BLOCK_SIZE (NAME_LEN * SURNAME_LEN)

#define NANOSECOND 1000000000.0

int indexForName[] = { 2, 4, 0, 1, 3 };
int indexForSurname[] = { 7, 2, 9, 4, 5, 11, 8, 10, 1, 12, 0, 3, 6 };

void MarshrutCipherDecipher(char* inputFileName, char* outputFileName, char* decipherFileName) {
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

	std::wstring text = L"";

	int i = 0, j = 0;
	while ((ch = fgetwc(iF)) != WEOF) {
		text += ch;
	}

	int fileLen = text.length();
	int countOfRows = (fileLen + COUNT_OF_COLS - 1) / COUNT_OF_COLS;

	std::vector<std::vector<wchar_t>> table(countOfRows, std::vector<wchar_t>(COUNT_OF_COLS));

	int indexInFile = 0;
	for (int i = 0; i < COUNT_OF_COLS; i++) {
		for (int j = 0; j < countOfRows; j++) {
			if (indexInFile < fileLen) {
				table[j][i] = text[indexInFile++];
			}
			else {
				table[j][i] = L'&';
			}
		}
	}

	fclose(iF);

	//зашифровка

	for (int i = 0; i < countOfRows; i++) {
		for (int j = 0; j < COUNT_OF_COLS; j++) {
			fputwc(table[i][j], oF);
		}
	}

	fclose(oF);

	// расшифровка

	oF = fopen(decipherFileName, "w");
	if (!oF) {
		printf("Error: file not opened!");
		return;
	}


	for (int i = 0; i < COUNT_OF_COLS; i++) {
		for (int j = 0; j < countOfRows; j++) {
			if (table[j][i] != '&') {
				fputwc(table[j][i], oF);
			}
		}
	}

	fclose(oF);
}


void MultiplyCipherDecipher(char* inputFileName, char* outputFileName, char* decipherFileName) {
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

	std::wstring text = L"";

	int i = 0, j = 0;
	while ((ch = fgetwc(iF)) != WEOF) {
		text += ch;
	}

	int fileLen = text.length();

	std::vector<std::vector<wchar_t>> table(SURNAME_LEN, std::vector<wchar_t>(NAME_LEN));

	int blocks = (fileLen + BLOCK_SIZE - 1) / BLOCK_SIZE;
	int indexInFile = 0;
	for (int b = 0; b < blocks; b++) {
		for (int i = 0; i < SURNAME_LEN; i++) {
			for (int j = 0; j < NAME_LEN; j++) {
				if (indexInFile < fileLen) {
					table[i][j] = text[indexInFile++];
				}
				else {
					table[i][j] = L'&';
				}
			}
		}

		//зашифровка
		for (int i = 0; i < NAME_LEN; i++) {
			for (int j = 0; j < SURNAME_LEN; j++) {
				fputwc(table[indexForSurname[j]][indexForName[i]], oF);
			}
		}
	}

	fclose(iF);
	fclose(oF);

	// расшифровка

	iF = fopen(outputFileName, "r");
	if (!iF) {
		printf("Error: file not opened!");
		return;
	}

	text = L"";

	i = 0;
	j = 0;
	while ((ch = fgetwc(iF)) != WEOF) {
		text += ch;
	}

	fileLen = text.length();

	oF = fopen(decipherFileName, "w");
	if (!oF) {
		printf("Error: file not opened!");
		return;
	}

	indexInFile = 0;
	for (int b = 0; b < blocks; b++) {
		for (int i = 0; i < NAME_LEN; i++) {
			for (int j = 0; j < SURNAME_LEN; j++) {
				table[indexForSurname[j]][indexForName[i]] = text[indexInFile++];

			}
		}

		for (int i = 0; i < SURNAME_LEN; i++) {
			for (int j = 0; j < NAME_LEN; j++) {
				if (table[i][j] != L'&')
					fputwc(table[i][j], oF);

			}
		}
	}

	fclose(oF);
}

struct timespec beginRealTime, endRealTime;

int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "");
	if (argc < 5) {
		printf("Error: you must input source fileName, dist fileName, filename for decipher text and key!\n");
		return EXIT_FAILURE;
	}

	if (strcmp(argv[4], "-ma") == 0) {
		clock_gettime(CLOCK_REALTIME, &beginRealTime);
		MarshrutCipherDecipher(argv[1], argv[2], argv[3]);
		clock_gettime(CLOCK_REALTIME, &endRealTime);
		double diffRealTime = (endRealTime.tv_sec - beginRealTime.tv_sec) + (endRealTime.tv_nsec - beginRealTime.tv_nsec) / NANOSECOND;
		printf("Time marshrut: %.3f\n", diffRealTime);
	}
	else if (strcmp(argv[4], "-mu") == 0) {
		clock_gettime(CLOCK_REALTIME, &beginRealTime);
		MultiplyCipherDecipher(argv[1], argv[2], argv[3]);
		clock_gettime(CLOCK_REALTIME, &endRealTime);
		double diffRealTime = (endRealTime.tv_sec - beginRealTime.tv_sec) + (endRealTime.tv_nsec - beginRealTime.tv_nsec) / NANOSECOND;
		printf("Time multiply: %.3f\n", diffRealTime);
	}
	return EXIT_SUCCESS;
}