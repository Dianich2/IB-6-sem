#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define Z 8

// создаем сверхвозрастающую последовательность
void generateSuperSequence(
    unsigned long long d[], 
    int n
){
    unsigned long long int sum = 0;
    for(int i = 0; i < n; i++){
        d[i] = sum + (rand() % 10 + 1);
        sum += d[i];
    }
}


// для НОД
unsigned long long gcd(
    unsigned long long int a,
    unsigned long long int b
){
    while(b != 0){
        unsigned long long buf = b;
        b = a % b;
        a = buf;
    }
    return a;
}


// для генерации открытого ключа
void generatePublic(
    unsigned long long d[],
    unsigned long long e[],
    int n,
    unsigned long long a,
    unsigned long long mod
){
    for(int i = 0; i < n; i++){
        e[i] = (d[i] * a) % mod;
    }
}


// шифрование
unsigned long long encrypt(
    char ch,
    unsigned long long e[]
){
    unsigned long long sum = 0;

    for(int i = 0; i < 8; i++){
        if(ch & (1 << (7 - i))){
            sum += e[i];
        }
    }
    return sum;
}

// поиск обратного по модулю
long long inverse(
    long long a,
    long long m
){
    long long m0 = m;
    long long t;
    long long q;
    long long x0 = 0;
    long long int x1 = 1;

    while(a > 1){
        q = a / m;
        t = m;
        m = a % m;
        a = t;
        t = x0;
        x0 = x1 - q * x0;
        x1 = t;
    }

    if(x1 < 0){
        x1 += m0;
    }

    return x1;
}


//расшифрование
void decrypt(
    unsigned long long S,
    unsigned long long d[],
    char *res
){
    *res = 0;

    for(int i = 7; i >= 0; i--){
        if(S >= d[i]){
            S -= d[i];
            *res |= (1 << (7 - i));
        }
    }
}


int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Error: You must input message\n");
        return EXIT_FAILURE;
    }
    srand(time(NULL));

    unsigned long long d[Z];
    unsigned long long e[Z];

    generateSuperSequence(d, Z);

    unsigned long long sum = 0;
    
    for(int i = 0; i < Z; i++){
        sum += d[i];
    }

    unsigned long long mod = sum + 50;
    unsigned long long a = 3;

    while(gcd(a, mod) != 1){
        a++;
    }

    generatePublic(d, e, Z, a, mod);

    printf("Private key:\n");
    for(int i = 0; i < Z; i++){
        printf("%llu ", d[i]);
    }

    printf("\nPublic key:\n");
    for(int i = 0; i < Z; i++) {
        printf("%llu ", e[i]);
    }
    printf("\n");

    unsigned long long encrypted[100];

    // шифруем
    for(int i = 0; argv[1][i]; i++){
        encrypted[i] = encrypt(argv[1][i], e);
        printf("Encrypted : %llu\n", encrypted[i]);
    }

    long long inverseA = inverse(a, mod);

    // расшифровываем
    for(int i = 0; argv[1][i]; i++){
        unsigned long long S = (encrypted[i] * inverseA) % mod;
        char res;
        decrypt(S, d, &res);
        printf("Decrypted : %c\n", res);
    }

    return EXIT_SUCCESS;
}





