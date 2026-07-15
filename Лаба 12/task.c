#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <gmp.h>
#include <openssl/evp.h>

#define MAX_MESSAGE 4096

#define RSA_BITS 1024
#define ELGAMAL_BITS 1024
#define SCHNORR_P_BITS 1024
#define SCHNORR_Q_BITS 160

double now_seconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec / 1000000000.0;
}

void init_random(gmp_randstate_t state) {
    gmp_randinit_default(state);
    gmp_randseed_ui(state, (unsigned long)time(NULL));
}

void generate_prime(mpz_t result, gmp_randstate_t state, int bits) {
    mpz_urandomb(result, state, bits);
    mpz_setbit(result, bits - 1);
    mpz_nextprime(result, result);
}

void random_nonzero_mod(mpz_t result, gmp_randstate_t state, const mpz_t mod) {
    do {
        mpz_urandomm(result, state, mod);
    } while (mpz_cmp_ui(result, 0) == 0);
}

void print_short(const char *name, const mpz_t value) {
    char *str = mpz_get_str(NULL, 16, value);

    printf("%s = %s\n", name, str);

    free(str);
}

int sha256_to_mpz(mpz_t result, const unsigned char *data, size_t data_len) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();
    if (ctx == NULL) {
        return 0;
    }

    if (EVP_DigestInit_ex(ctx, EVP_sha256(), NULL) != 1) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    if (EVP_DigestUpdate(ctx, data, data_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    if (EVP_DigestFinal_ex(ctx, hash, &hash_len) != 1) {
        EVP_MD_CTX_free(ctx);
        return 0;
    }

    EVP_MD_CTX_free(ctx);

    mpz_import(result, hash_len, 1, 1, 0, 0, hash);

    return 1;
}

void hash_message_mod(mpz_t result, const char *message, const mpz_t mod) {
    sha256_to_mpz(result, (const unsigned char *)message, strlen(message));
    mpz_mod(result, result, mod);
}

void hash_message_and_number_mod(mpz_t result, const char *message, const mpz_t number, const mpz_t mod) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len = 0;

    char *number_hex = mpz_get_str(NULL, 16, number);

    EVP_MD_CTX *ctx = EVP_MD_CTX_new();

    EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);
    EVP_DigestUpdate(ctx, message, strlen(message));
    EVP_DigestUpdate(ctx, number_hex, strlen(number_hex));
    EVP_DigestFinal_ex(ctx, hash, &hash_len);

    EVP_MD_CTX_free(ctx);

    mpz_import(result, hash_len, 1, 1, 0, 0, hash);
    mpz_mod(result, result, mod);

    free(number_hex);
}

/* ============================================================
   RSA SIGNATURE
   ============================================================ */

typedef struct {
    mpz_t p, q, n, phi, e, d;
} RSAKey;

void rsa_init(RSAKey *key) {
    mpz_inits(key->p, key->q, key->n, key->phi, key->e, key->d, NULL);
}

void rsa_clear(RSAKey *key) {
    mpz_clears(key->p, key->q, key->n, key->phi, key->e, key->d, NULL);
}

void rsa_generate_keys(RSAKey *key, gmp_randstate_t state, int bits) {
    mpz_t p1, q1, gcd;
    mpz_inits(p1, q1, gcd, NULL);

    while (1) {
        generate_prime(key->p, state, bits / 2);
        generate_prime(key->q, state, bits / 2);

        mpz_mul(key->n, key->p, key->q);

        mpz_sub_ui(p1, key->p, 1);
        mpz_sub_ui(q1, key->q, 1);
        mpz_mul(key->phi, p1, q1);

        mpz_set_ui(key->e, 65537);
        mpz_gcd(gcd, key->e, key->phi);

        if (mpz_cmp_ui(gcd, 1) == 0) {
            mpz_invert(key->d, key->e, key->phi);
            break;
        }
    }

    mpz_clears(p1, q1, gcd, NULL);
}

void rsa_sign(mpz_t signature, const char *message, RSAKey *key) {
    mpz_t h;
    mpz_init(h);

    hash_message_mod(h, message, key->n);

    mpz_powm(signature, h, key->d, key->n);

    mpz_clear(h);
}

int rsa_verify(mpz_t signature, const char *message, RSAKey *key) {
    mpz_t h1, h2;
    mpz_inits(h1, h2, NULL);

    hash_message_mod(h1, message, key->n);

    mpz_powm(h2, signature, key->e, key->n);

    int result = (mpz_cmp(h1, h2) == 0);

    mpz_clears(h1, h2, NULL);

    return result;
}

/* ============================================================
   ELGAMAL SIGNATURE
   ============================================================ */

typedef struct {
    mpz_t p, q, g, x, y;
} ElGamalKey;

void elgamal_init(ElGamalKey *key) {
    mpz_inits(key->p, key->q, key->g, key->x, key->y, NULL);
}

void elgamal_clear(ElGamalKey *key) {
    mpz_clears(key->p, key->q, key->g, key->x, key->y, NULL);
}

void generate_safe_prime(mpz_t p, mpz_t q, gmp_randstate_t state, int bits) {
    while (1) {
        generate_prime(q, state, bits - 1);

        mpz_mul_ui(p, q, 2);
        mpz_add_ui(p, p, 1);

        if (mpz_probab_prime_p(p, 25)) {
            break;
        }
    }
}

void find_primitive_root_for_safe_prime(mpz_t g, const mpz_t p, const mpz_t q) {
    mpz_t test1, test2;
    mpz_inits(test1, test2, NULL);

    mpz_set_ui(g, 2);

    while (1) {
        mpz_powm_ui(test1, g, 2, p);
        mpz_powm(test2, g, q, p);

        if (mpz_cmp_ui(test1, 1) != 0 && mpz_cmp_ui(test2, 1) != 0) {
            break;
        }

        mpz_add_ui(g, g, 1);
    }

    mpz_clears(test1, test2, NULL);
}

void elgamal_generate_keys(ElGamalKey *key, gmp_randstate_t state, int bits) {
    generate_safe_prime(key->p, key->q, state, bits);
    find_primitive_root_for_safe_prime(key->g, key->p, key->q);

    random_nonzero_mod(key->x, state, key->p);
    mpz_powm(key->y, key->g, key->x, key->p);
}

void elgamal_sign(mpz_t a, mpz_t b, const char *message, ElGamalKey *key, gmp_randstate_t state) {
    mpz_t p_minus_1, h, k, gcd, k_inv, temp;
    mpz_inits(p_minus_1, h, k, gcd, k_inv, temp, NULL);

    mpz_sub_ui(p_minus_1, key->p, 1);
    hash_message_mod(h, message, p_minus_1);

    do {
        random_nonzero_mod(k, state, p_minus_1);
        mpz_gcd(gcd, k, p_minus_1);
    } while (mpz_cmp_ui(gcd, 1) != 0);

    mpz_powm(a, key->g, k, key->p);

    mpz_invert(k_inv, k, p_minus_1);

    mpz_mul(temp, key->x, a);
    mpz_sub(temp, h, temp);
    mpz_mod(temp, temp, p_minus_1);

    mpz_mul(b, temp, k_inv);
    mpz_mod(b, b, p_minus_1);

    mpz_clears(p_minus_1, h, k, gcd, k_inv, temp, NULL);
}

int elgamal_verify(mpz_t a, mpz_t b, const char *message, ElGamalKey *key) {
    mpz_t p_minus_1, h, left1, left2, left, right;
    mpz_inits(p_minus_1, h, left1, left2, left, right, NULL);

    mpz_sub_ui(p_minus_1, key->p, 1);
    hash_message_mod(h, message, p_minus_1);

    mpz_powm(left1, key->y, a, key->p);
    mpz_powm(left2, a, b, key->p);

    mpz_mul(left, left1, left2);
    mpz_mod(left, left, key->p);

    mpz_powm(right, key->g, h, key->p);

    int result = (mpz_cmp(left, right) == 0);

    mpz_clears(p_minus_1, h, left1, left2, left, right, NULL);

    return result;
}

/* ============================================================
   SCHNORR SIGNATURE
   ============================================================ */

typedef struct {
    mpz_t p, q, g, x, y;
} SchnorrKey;

void schnorr_init(SchnorrKey *key) {
    mpz_inits(key->p, key->q, key->g, key->x, key->y, NULL);
}

void schnorr_clear(SchnorrKey *key) {
    mpz_clears(key->p, key->q, key->g, key->x, key->y, NULL);
}

void generate_schnorr_pq(mpz_t p, mpz_t q, gmp_randstate_t state, int p_bits, int q_bits) {
    mpz_t r;
    mpz_init(r);

    generate_prime(q, state, q_bits);

    while (1) {
        mpz_urandomb(r, state, p_bits - q_bits);
        mpz_setbit(r, p_bits - q_bits - 1);
        mpz_clrbit(r, 0);

        mpz_mul(p, r, q);
        mpz_add_ui(p, p, 1);

        if (mpz_sizeinbase(p, 2) == (size_t)p_bits && mpz_probab_prime_p(p, 25)) {
            break;
        }
    }

    mpz_clear(r);
}

void schnorr_generate_g(mpz_t g, const mpz_t p, const mpz_t q, gmp_randstate_t state) {
    mpz_t h, exp;
    mpz_inits(h, exp, NULL);

    mpz_sub_ui(exp, p, 1);
    mpz_divexact(exp, exp, q);

    while (1) {
        random_nonzero_mod(h, state, p);
        mpz_powm(g, h, exp, p);

        if (mpz_cmp_ui(g, 1) > 0) {
            break;
        }
    }

    mpz_clears(h, exp, NULL);
}

void schnorr_generate_keys(SchnorrKey *key, gmp_randstate_t state, int p_bits, int q_bits) {
    mpz_t gx;
    mpz_init(gx);

    generate_schnorr_pq(key->p, key->q, state, p_bits, q_bits);
    schnorr_generate_g(key->g, key->p, key->q, state);

    random_nonzero_mod(key->x, state, key->q);

    mpz_powm(gx, key->g, key->x, key->p);
    mpz_invert(key->y, gx, key->p);

    mpz_clear(gx);
}

void schnorr_sign(mpz_t h, mpz_t b, const char *message, SchnorrKey *key, gmp_randstate_t state) {
    mpz_t k, a, temp;
    mpz_inits(k, a, temp, NULL);

    random_nonzero_mod(k, state, key->q);

    mpz_powm(a, key->g, k, key->p);

    hash_message_and_number_mod(h, message, a, key->q);

    mpz_mul(temp, key->x, h);
    mpz_add(temp, temp, k);
    mpz_mod(b, temp, key->q);

    mpz_clears(k, a, temp, NULL);
}

int schnorr_verify(mpz_t h, mpz_t b, const char *message, SchnorrKey *key) {
    mpz_t gb, yh, X, h_check;
    mpz_inits(gb, yh, X, h_check, NULL);

    mpz_powm(gb, key->g, b, key->p);
    mpz_powm(yh, key->y, h, key->p);

    mpz_mul(X, gb, yh);
    mpz_mod(X, X, key->p);

    hash_message_and_number_mod(h_check, message, X, key->q);

    int result = (mpz_cmp(h, h_check) == 0);

    mpz_clears(gb, yh, X, h_check, NULL);

    return result;
}

/* ============================================================
   DEMO
   ============================================================ */

void run_rsa_demo(const char *message, gmp_randstate_t state) {
    RSAKey key;
    rsa_init(&key);

    mpz_t signature;
    mpz_init(signature);

    printf("\n================ RSA SIGNATURE ================\n");

    double t1 = now_seconds();
    rsa_generate_keys(&key, state, RSA_BITS);
    double t2 = now_seconds();

    double t3 = now_seconds();
    rsa_sign(signature, message, &key);
    double t4 = now_seconds();

    double t5 = now_seconds();
    int ok = rsa_verify(signature, message, &key);
    double t6 = now_seconds();

    print_short("RSA signature S", signature);

    printf("Key generation time: %.9f sec\n", t2 - t1);
    printf("Signature generation time: %.9f sec\n", t4 - t3);
    printf("Signature verification time: %.9f sec\n", t6 - t5);
    printf("Verification result: %s\n", ok ? "VALID" : "INVALID");

    mpz_clear(signature);
    rsa_clear(&key);
}

void run_elgamal_demo(const char *message, gmp_randstate_t state) {
    ElGamalKey key;
    elgamal_init(&key);

    mpz_t a, b;
    mpz_inits(a, b, NULL);

    printf("\n============= ELGAMAL SIGNATURE =============\n");

    double t1 = now_seconds();
    elgamal_generate_keys(&key, state, ELGAMAL_BITS);
    double t2 = now_seconds();

    double t3 = now_seconds();
    elgamal_sign(a, b, message, &key, state);
    double t4 = now_seconds();

    double t5 = now_seconds();
    int ok = elgamal_verify(a, b, message, &key);
    double t6 = now_seconds();

    print_short("ElGamal signature a", a);
    print_short("ElGamal signature b", b);

    printf("Key generation time: %.9f sec\n", t2 - t1);
    printf("Signature generation time: %.9f sec\n", t4 - t3);
    printf("Signature verification time: %.9f sec\n", t6 - t5);
    printf("Verification result: %s\n", ok ? "VALID" : "INVALID");

    mpz_clears(a, b, NULL);
    elgamal_clear(&key);
}

void run_schnorr_demo(const char *message, gmp_randstate_t state) {
    SchnorrKey key;
    schnorr_init(&key);

    mpz_t h, b;
    mpz_inits(h, b, NULL);

    printf("\n============= SCHNORR SIGNATURE =============\n");

    double t1 = now_seconds();
    schnorr_generate_keys(&key, state, SCHNORR_P_BITS, SCHNORR_Q_BITS);
    double t2 = now_seconds();

    double t3 = now_seconds();
    schnorr_sign(h, b, message, &key, state);
    double t4 = now_seconds();

    double t5 = now_seconds();
    int ok = schnorr_verify(h, b, message, &key);
    double t6 = now_seconds();

    print_short("Schnorr signature h", h);
    print_short("Schnorr signature b", b);

    printf("Key generation time: %.9f sec\n", t2 - t1);
    printf("Signature generation time: %.9f sec\n", t4 - t3);
    printf("Signature verification time: %.9f sec\n", t6 - t5);
    printf("Verification result: %s\n", ok ? "VALID" : "INVALID");

    mpz_clears(h, b, NULL);
    schnorr_clear(&key);
}

int main() {
    char message[MAX_MESSAGE];

    gmp_randstate_t state;
    init_random(state);

    printf("Enter message: ");

    fgets(message, sizeof(message), stdin);
    message[strcspn(message, "\n")] = '\0';

    printf("\nInput message: %s\n", message);
    printf("Message length: %zu bytes\n", strlen(message));

    run_rsa_demo(message, state);
    run_elgamal_demo(message, state);
    run_schnorr_demo(message, state);

    gmp_randclear(state);

    return 0;
}