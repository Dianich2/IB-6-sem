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
#include <ctype.h>

#define NULL 0
#define TRUE 1

int max(int n, int m) {
	return n > m ? n : m;
}

int min(int n, int m) {
	return n < m ? n : m;
}

int calcGCD(int n, int m) {
	int ma = max(n, m);
	int mi = min(n, m);
	int ost = ma % mi;
	while (ost != 0) {
		ma = mi;
		mi = ost;
		ost = ma % mi;
	}
	return mi;
}

int* primeNums = NULL;

void searchPrimeNums(int m, int n) {
	primeNums = (int*)calloc((n + 1), sizeof(int));
	for (int i = 0; i <= n; i++) {
        primeNums[i] = 1;
    }
	primeNums[0] = primeNums[1] = 0;
	for (int i = 2; i * i <= n; i++) {
		if (primeNums[i]) {
			for (int j = i * i; j <= n; j += i) {
				primeNums[j] = 0;
			}
		}
	}

	int countOfPrimeNums = 0;
	for (int i = m; i <= n; i++) {
		if (primeNums[i] == 1) {
			printf("%d ", i);
			countOfPrimeNums++;
		}
	}
	printf("\nTotal count of prime nums = %d\n", countOfPrimeNums);
	printf("\nCount of nums by formula = %.2f\n", (n / log(n)) - (m / log(m)));
    free(primeNums);
}

int main(int argc, char* argv[]) {
	printf("=== MENU ===\n\n");
	printf("1. GCD\n");
	printf("2. Search prime numbers\n");
	int inp;
	scanf("%d", &inp);
	switch (inp) {
		case 1: {
            int count = 0;
            printf("Input count of nums for calc GCD\n");
            scanf("%d", &count);
            if(count == 2){
                int gcd, n, m;
			    printf("Input first num\n");
			    scanf("%d", &n);
			    printf("Input second num\n");
			    scanf("%d", &m);
			    gcd = calcGCD(n, m);
			    printf("GCD(%d, %d) = %d\n", n, m, gcd);
            }
            else{
                int gcd, n, m, k;
			    printf("Input first num\n");
			    scanf("%d", &n);
			    printf("Input second num\n");
			    scanf("%d", &m);
                printf("Input third num\n");
			    scanf("%d", &k);
			    gcd = calcGCD(n, m);
                gcd = calcGCD(gcd, k);
			    printf("GCD(%d, %d, %d) = %d\n", n, m, k, gcd);
            }
			break;
		}
		case 2: {
			int n, m;
			printf("Input left border\n");
			scanf("%d", &m);
			printf("Input right border\n");
			scanf("%d", &n);
			searchPrimeNums(m, n);
			break;
		}
	}

	return EXIT_SUCCESS;
}