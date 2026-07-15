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

#define A 421
#define C 1663
#define N 7875

void generateLCG(int initialValue, int countOfNumbers) {
  int x = initialValue; //не учитывается в последовательности начальное значение

  for (int i = 0; i < countOfNumbers; i++) {
    x = (A * x + C) % N;
    printf("%d ", x);
  }
  printf("\n");
}

int main(int argc, char* argv[]) {
  if (argc < 3) {
    printf("You must input initial value and count of nums\n");
    return 1;
  }

  generateLCG(atoi(argv[1]), atoi(argv[2]));
  return 0;
}
