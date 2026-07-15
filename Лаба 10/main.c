// код получился не простой, так что сейчас 
//попробую накинуть комментариев для лучшего понимания

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h> // доп библиотека для работы с большими числами

#define RSA_BITS 1024      // размер ключа для RSA
#define ELGAMAL_BITS 1024  // размер ключа для Эль-Гамаля(число p)


// ======================= TIMER =======================
double now_seconds() {
    struct timespec ts;
    // используем часики MONOTONIC, т.к. у нас относительное время ожидания
    clock_gettime(CLOCK_MONOTONIC, &ts);
    // ну и по итогу вернем время в секундах
    //(с помощью тех часиков мы получим время, 
    //которое хранится в 2 частях: секундах и наносекундах) 
    return ts.tv_sec + ts.tv_nsec / 1e9;
}

// ======================= BASE64 =======================

static const char base64_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

// функция для кодирования
char *base64_encode(
    const unsigned char *data, // исходный текст
    size_t input_length        // его длина
) {
    // тут вычисляем примерную выходную длину(на каждые 3 байта 
    //будет 4 символа примерно), реализуем также тут типо свое округление(ceil)
    size_t output_length = 4 * ((input_length + 2) / 3);

    char *encoded = malloc(output_length + 1);
    if (!encoded) return NULL;

    size_t i = 0, j = 0;

    while (i < input_length) {
        unsigned int octet_a = i < input_length ? data[i++] : 0;
        unsigned int octet_b = i < input_length ? data[i++] : 0;
        unsigned int octet_c = i < input_length ? data[i++] : 0;

        // склеиваем 3 байта ,которые мы взяли выше, в одно 24-битное число
        unsigned int triple = (octet_a << 16) | (octet_b << 8) | octet_c;

        // тут получается берем каждый раз по 6 битов и вот их кодируем 
        // и по итогу так и будет, 24 / 6 = 4, то есть на 3 байта 4 символа
        encoded[j++] = base64_table[(triple >> 18) & 0x3F];
        encoded[j++] = base64_table[(triple >> 12) & 0x3F];
        encoded[j++] = base64_table[(triple >> 6) & 0x3F];
        encoded[j++] = base64_table[triple & 0x3F];
    }

    // тут короче по сути Base64 требует блоки по 3 байта, поэтому если
    // еще остались меньше 3 байт, то вместо них будет =
    int mod = input_length % 3;
    if (mod) {
        encoded[output_length - 1] = '=';
        if (mod == 1) {
            encoded[output_length - 2] = '=';
        }
    }

    encoded[output_length] = '\0';
    return encoded;
}

// ======================= RANDOM =======================

// функция для настройки генератора больших чисел
void init_random(gmp_randstate_t state) {
    gmp_randinit_default(state); // инициализируем генератор стандартным алгоритмом
    gmp_randseed_ui(state, (unsigned long)time(NULL)); // задаем нач. значение
}

// функция для генерации большого простого числа
void generate_prime(
    mpz_t prime,            // переменная, куда мы положим результат
    gmp_randstate_t state,  // собственно наш генератор
    int bits                // скольки битное число нам нужно
) {
    mpz_urandomb(prime, state, bits);  // генерирует случайное число длиной bits
    mpz_setbit(prime, bits - 1);       // на всякий устанавливаем старший бит, чтобы число точно имело нужную длину
    mpz_nextprime(prime, prime);       // ну и берем следующее простое число, которое >= текущему
    // ну типо тут же может быть моментик, что мы получим и составное число выше,
    // потому что мы просто генерим рандомное число, вот поэтому последней функцией мы
    // точно ищем простое число
}

// ======================= BENCHMARK =======================

// ну это типо функция для первого задания, где нужно померить время 
// вычисления y = a^x mod n, в зависимости от параметров a, x, n
void benchmark_powmod() {
    gmp_randstate_t state;  // создаем генератор случайных чисел
    init_random(state);     // и инициализируем его

    // a - основание степени
    // x - показатель степени
    // n - модуль
    // y - результат
    mpz_t a, x, n, y;  // создаем большие числа(mpz_t специальный тип таких чисел)
    mpz_inits(a, x, n, y, NULL); // инициализируем все числа(так нужно перед использованием)

    int bases[] = {5, 17, 35}; // это будут наши ашки
    
    const char *exponents[] = {  // а это иксы
        "1009",
        "1000003",
        "1000000007",
        "10000000000037",
        "100000000000000000003",
        "100000000000000000000000000000000003"
    };

    int n_bits[] = {1024, 2048}; // это нки

    printf("\n=== Benchmark y = a^x mod n ===\n");
    printf("a;x_bits;n_bits;time_sec\n");

    for (int nb = 0; nb < 2; nb++) {
        generate_prime(n, state, n_bits[nb]); // генерируем простое число нужной битности

        for (int i = 0; i < 3; i++) {
            mpz_set_ui(a, bases[i]); // записываем типо просто число int в переменную для больших чисел

            for (int j = 0; j < 6; j++) {
                mpz_set_str(x, exponents[j], 10); // преобразуем текущее занчение икс из строки в большое число

                double start = now_seconds(); // начинаем замер времени
                mpz_powm(y, a, x, n); // это спешиал функция для модульного возведения в степень(удобно)
                double end = now_seconds(); // оканчиваем замер времени

                printf("%d;%zu;%d;%.9f\n",
                       bases[i],
                       mpz_sizeinbase(x, 2), // размер икса в битах
                       n_bits[nb],
                       end - start);
            }
        }
    }

    mpz_clears(a, x, n, y, NULL);  // чистим память, которая была занята нашими большими числами
    gmp_randclear(state);          // освобождаем генератор простых чисел
}

// ======================= RSA =======================

// создадим доп структурку для хранения всех параметров вместе(чисто для удобства)
typedef struct {
    mpz_t p,    // первое большое простое число
          q,    // второе большое простое число
          n,    // модуль
          phi,  // значение функции Эйлера
          e,    // открытый ключ
          d;    // закрытый ключ
} RSAKey;

// инициализируем все простые числа в структурке
void rsa_init(RSAKey *key) {
    mpz_inits(key->p, key->q, key->n, key->phi, key->e, key->d, NULL);
}

// чистим память, которая была занята большими числами в структурке, после работы с ними
void rsa_clear(RSAKey *key) {
    mpz_clears(key->p, key->q, key->n, key->phi, key->e, key->d, NULL);
}

// функция генерации RSA-ключа нужной длины
void rsa_generate_keys(
    RSAKey *key, 
    gmp_randstate_t state, 
    int bits
) {
    mpz_t p1, q1, gcd;
    mpz_inits(p1, q1, gcd, NULL);

    while (1) {
        // генерируем 2 простых числа по половине всей длины
        generate_prime(key->p, state, bits / 2);
        generate_prime(key->q, state, bits / 2);

        mpz_mul(key->n, key->p, key->q);

        mpz_sub_ui(p1, key->p, 1);
        mpz_sub_ui(q1, key->q, 1);
        mpz_mul(key->phi, p1, q1); // считаем функцию Эйлера

        mpz_set_ui(key->e, 65537);     // это наша ешка будет такой, задаем константой
        mpz_gcd(gcd, key->e, key->phi); // считаем НОД ешки и функции Эйлера

        if (mpz_cmp_ui(gcd, 1) == 0) { // нужен НОД = 1
            mpz_invert(key->d, key->e, key->phi); // и получаем d как обратное по модулю
            break;
        }
    }

    mpz_clears(p1, q1, gcd, NULL);
}


// шифрование 1 символа
void rsa_encrypt_char(
    mpz_t c,            // зашифрованное большое число
    unsigned char m,    // символ исходного текста
    RSAKey *key         // ключик
) {
    mpz_t message;
    mpz_init_set_ui(message, m); // пишем код символа в message

    mpz_powm(c, message, key->e, key->n); // шифруем по формуле

    mpz_clear(message);
}


// расшифрование 1 символа
unsigned char rsa_decrypt_char(mpz_t c, RSAKey *key) {
    mpz_t m;
    mpz_init(m);

    mpz_powm(m, c, key->d, key->n);  // расшифровываем по формуле

    unsigned char result = (unsigned char)mpz_get_ui(m); // преобразуем число обратно в символ

    mpz_clear(m);
    return result;
}

// сборка всего, чтобы показать работу RSA
void rsa_demo(const char *text) {
    gmp_randstate_t state;
    init_random(state);

    RSAKey key;
    rsa_init(&key);

    printf("\n=== RSA ===\n");

    rsa_generate_keys(&key, state, RSA_BITS);

    char *encoded = base64_encode((const unsigned char *)text, strlen(text));
    int len = strlen(encoded);

    mpz_t *cipher = malloc(sizeof(mpz_t) * len); // массив шифрблоков(каждый символ Base64 отдельно шифруется)

    double start_enc = now_seconds();

    for (int i = 0; i < len; i++) {
        mpz_init(cipher[i]);
        rsa_encrypt_char(cipher[i], (unsigned char)encoded[i], &key);
    }

    double end_enc = now_seconds();

    char *decrypted_base64 = malloc(len + 1);

    double start_dec = now_seconds();

    for (int i = 0; i < len; i++) {
        decrypted_base64[i] = rsa_decrypt_char(cipher[i], &key);
    }

    double end_dec = now_seconds();

    decrypted_base64[len] = '\0';

    printf("Original text: %s\n", text);
    printf("Base64 text:   %s\n", encoded);
    printf("Decrypted Base64: %s\n", decrypted_base64);

    printf("Encryption time: %.9f sec\n", end_enc - start_enc);
    printf("Decryption time: %.9f sec\n", end_dec - start_dec);

    size_t open_size = strlen(encoded); // размер открытого текста
    size_t cipher_size = len * (RSA_BITS / 8); // размер шифртекста RSA
    // Один RSA-блок при ключе 1024 бита занимает 128 байт


    printf("Open text size: %zu bytes\n", open_size);
    printf("Cipher text approximate size: %zu bytes\n", cipher_size);
    printf("Size growth coefficient: %.2f\n", (double)cipher_size / open_size);

    for (int i = 0; i < len; i++) {
        mpz_clear(cipher[i]);
    }

    free(cipher);
    free(encoded);
    free(decrypted_base64);

    rsa_clear(&key);
    gmp_randclear(state);
}

// ======================= ELGAMAL =======================

// опять же создадим доп структурку для хранения всех параметров вместе(чисто для удобства)
typedef struct {
    mpz_t p,   // большое простое число
          q,   // дополнительно нужно 
          g,   // первообразный корень
          x,   // секретный ключик
          y;   // открытая часть ключика
} ElGamalKey;

// инициализация всеъ больших чисел структурки
void elgamal_init(ElGamalKey *key) {
    mpz_inits(key->p, key->q, key->g, key->x, key->y, NULL);
}

// чистка памяти, занятой большими числами структурки
void elgamal_clear(ElGamalKey *key) {
    mpz_clears(key->p, key->q, key->g, key->x, key->y, NULL);
}

// генерация безопасного простого числа (p = 2q + 1)
void generate_safe_prime(
    mpz_t p, 
    mpz_t q, 
    gmp_randstate_t state, 
    int bits
) {
    while (1) {
        generate_prime(q, state, bits - 1);
        mpz_mul_ui(p, q, 2);
        mpz_add_ui(p, p, 1);

        if (mpz_probab_prime_p(p, 25)) { // проверка p на простоту
            break;
        }
    }
}

// функция поиска первообразого корня g по модулю p
void find_generator(
    mpz_t g, 
    mpz_t p, 
    mpz_t q
) {
    mpz_t test1, test2;
    mpz_inits(test1, test2, NULL);

    mpz_set_ui(g, 2);

    while (1) {
        // тут по сути хватит только проверок g^2 != 1 mod p и g^q != 1 mod p
        mpz_powm_ui(test1, g, 2, p);
        mpz_powm(test2, g, q, p);

        if (mpz_cmp_ui(test1, 1) != 0 && mpz_cmp_ui(test2, 1) != 0) {
            break;
        }

        mpz_add_ui(g, g, 1); // если не подошло, то ищем следующее число
    }

    mpz_clears(test1, test2, NULL);
}

// функция генерации ключиков
void elgamal_generate_keys(
    ElGamalKey *key, 
    gmp_randstate_t state, 
    int bits
) {
    generate_safe_prime(key->p, key->q, state, bits); // генерируем p
    find_generator(key->g, key->p, key->q);           // генерируем g

    mpz_urandomm(key->x, state, key->p);              // генерируем x
    if (mpz_cmp_ui(key->x, 2) < 0) {                
        mpz_set_ui(key->x, 2);
    }

    mpz_powm(key->y, key->g, key->x, key->p);      // считаем y = g^x mod p
}

// шифрование 1 символа
void elgamal_encrypt_char(
    mpz_t a, 
    mpz_t b, 
    unsigned char m, 
    ElGamalKey *key, 
    gmp_randstate_t state
) {
    mpz_t k, s, message;
    mpz_inits(k, s, message, NULL);

    mpz_urandomm(k, state, key->p);  // генерируем случайное k
    if (mpz_cmp_ui(k, 2) < 0) {
        mpz_set_ui(k, 2);
    }

    mpz_set_ui(message, m);

    mpz_powm(a, key->g, k, key->p);
    mpz_powm(s, key->y, k, key->p);

    mpz_mul(b, s, message);
    mpz_mod(b, b, key->p);

    mpz_clears(k, s, message, NULL);
}


// расшифрование 1 символа
unsigned char elgamal_decrypt_char(
    mpz_t a, 
    mpz_t b, 
    ElGamalKey *key
) {
    mpz_t s, s_inv, m;
    mpz_inits(s, s_inv, m, NULL);

    mpz_powm(s, a, key->x, key->p);
    mpz_invert(s_inv, s, key->p);

    mpz_mul(m, b, s_inv);
    mpz_mod(m, m, key->p);

    unsigned char result = (unsigned char)mpz_get_ui(m); // обратно в символ

    mpz_clears(s, s_inv, m, NULL);
    return result;
}

// сборка всего, чтобы показать работу Эль-Гамаля
void elgamal_demo(const char *text) {
    gmp_randstate_t state;
    init_random(state);

    ElGamalKey key;
    elgamal_init(&key);

    printf("\n=== ElGamal ===\n");

    elgamal_generate_keys(&key, state, ELGAMAL_BITS);

    char *encoded = base64_encode((const unsigned char *)text, strlen(text));
    int len = strlen(encoded);

    mpz_t *a = malloc(sizeof(mpz_t) * len);
    mpz_t *b = malloc(sizeof(mpz_t) * len);

    double start_enc = now_seconds();

    for (int i = 0; i < len; i++) {
        mpz_init(a[i]);
        mpz_init(b[i]);
        elgamal_encrypt_char(a[i], b[i], (unsigned char)encoded[i], &key, state);
    }

    double end_enc = now_seconds();

    char *decrypted_base64 = malloc(len + 1);

    double start_dec = now_seconds();

    for (int i = 0; i < len; i++) {
        decrypted_base64[i] = elgamal_decrypt_char(a[i], b[i], &key);
    }

    double end_dec = now_seconds();

    decrypted_base64[len] = '\0';

    printf("Original text: %s\n", text);
    printf("Base64 text:   %s\n", encoded);
    printf("Decrypted Base64: %s\n", decrypted_base64);

    printf("Encryption time: %.9f sec\n", end_enc - start_enc);
    printf("Decryption time: %.9f sec\n", end_dec - start_dec);

    size_t open_size = strlen(encoded);
    size_t cipher_size = len * 2 * (ELGAMAL_BITS / 8);

    printf("Open text size: %zu bytes\n", open_size);
    printf("Cipher text approximate size: %zu bytes\n", cipher_size);
    printf("Size growth coefficient: %.2f\n", (double)cipher_size / open_size);

    for (int i = 0; i < len; i++) {
        mpz_clear(a[i]);
        mpz_clear(b[i]);
    }

    free(a);
    free(b);
    free(encoded);
    free(decrypted_base64);

    elgamal_clear(&key);
    gmp_randclear(state);
}

// ======================= MAIN =======================

int main() {
    char text[512];

    printf("Enter your full name: ");
    fgets(text, sizeof(text), stdin);

    text[strcspn(text, "\n")] = '\0';

    benchmark_powmod();
    rsa_demo(text);
    elgamal_demo(text);

    return 0;
}