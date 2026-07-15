// поясним немножко что тут у нас получилось
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// библиотека для хеширования(по сути высокоуровневый интерфейс OpenSSL для хеширования)
#include <openssl/evp.h> 

#define MAX_INPUT 4096      // максимальная допустимая длина вводимого сообщения
// длина нашего хеша, который получим(256 / 8 = 32 байта)
// каждый в hex записывается 2 символами ну и еще на конец строки символ нужен (32 * 2 + 1)
#define HASH_HEX_LENGTH 65  


// как в предыдущей лабке функция для замера времени
// вернет текущее время в секундах(часики снова MONOTONIC)
double now_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

// функция для конвертации массива байтов в hex строку(для красоты)
void bytes_to_hex(
    const unsigned char *bytes, // исходный массив
    unsigned int len,           // кол-во байт
    char *hex                   // здесь будет результат
) {
    for (unsigned int i = 0; i < len; i++) {
        sprintf(
            hex + i * 2, // куда пишем(ну типо каждый байт по 2 символа)
            "%02x",      // для вывода числа в 16-ричном виде с ведущим нулем если нужно
            bytes[i]     // сам байт
        );
    }

    hex[len * 2] = '\0';
}


// функция для вычисления SHA-256
int calculate_sha256(
    const unsigned char *message, // наши входные данные
    size_t message_len,           // их длина в байтах
    unsigned char *hash,          // сюда запишем наш хеш
    unsigned int *hash_len        // сюда его длину
) {

    // создаем контекст хеширования OpenSSL
    // тут контекст - это типо объект, в котором наша библиотечка
    // хранит внутреннее состояние алгоритма(промежуточные значения,
    // информация о самом алгоритме, обработанные блоки и служебные данные)
    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    if (ctx == NULL) {
        return 0;
    }

    // пробуем инициализировать операцию хеширования
    if (EVP_DigestInit_ex(
        ctx,                // контекст
        EVP_sha256(),       // наш алгоритм SHA-256
        NULL                // это типо чтобы указать, что используем стандартную реализацию из OpenSSL
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    // передаем данные в алгоритм SHA-256
    if (EVP_DigestUpdate(
        ctx,            // контекст
        message,        // данные
        message_len     // длина данных
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    // это типо для завершения вычисления хеша
    // тут обрабатываются последние блоки, записывается итоговый хеш и его длина
    if (EVP_DigestFinal_ex(
        ctx,      // контекст
        hash,     // куда пишем хеш
        hash_len  // его длина
    ) != 1) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    EVP_MD_CTX_free(ctx);
    return 1;
}


// это функция для хеширования нашего введенного сообщения
void hash_user_message() {
    char message[MAX_INPUT];

    printf("Enter message: ");
    fgets(message, sizeof(message), stdin);

    message[strcspn(message, "\n")] = '\0';

    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;
    char hash_hex[HASH_HEX_LENGTH];

    double start = now_seconds();

    int success = calculate_sha256(
        (const unsigned char *)message,
        strlen(message),
        hash,
        &hash_len
    );

    double end = now_seconds();

    if (!success) {
        printf("Hash calculation error\n");
        return;
    }

    bytes_to_hex(hash, hash_len, hash_hex);

    printf("Input message: %s\n", message);
    printf("Input length: %zu bytes\n", strlen(message));
    printf("Hash length: %u bytes / %u bits\n", hash_len, hash_len * 8);
    printf("SHA-256: %s\n", hash_hex);
    printf("Calculation time: %.9f sec\n", end - start);
}


int main() {
    hash_user_message();
    return 0;
}