#include <stdio.h>
#include <string.h>

#define P_MOD 751
#define A_COEF -1
#define B_COEF 1

typedef struct {
    int x;
    int y;
    int infinity;
} Point;

Point INF() {
    Point p = {0, 0, 1};
    return p;
}

Point point(int x, int y) {
    Point p = {x, y, 0};
    return p;
}

int mod(int a, int m) {
    int r = a % m;
    if (r < 0) r += m;
    return r;
}

int egcd(int a, int b, int *x, int *y) {
    if (b == 0) {
        *x = 1;
        *y = 0;
        return a;
    }

    int x1, y1;
    int g = egcd(b, a % b, &x1, &y1);

    *x = y1;
    *y = x1 - (a / b) * y1;

    return g;
}

int inv_mod(int a, int m) {
    int x, y;
    int g = egcd(mod(a, m), m, &x, &y);

    if (g != 1) {
        printf("No inverse for %d mod %d\n", a, m);
        return 0;
    }

    return mod(x, m);
}

int is_on_curve(Point p) {
    if (p.infinity) return 1;

    int left = mod(p.y * p.y, P_MOD);
    int right = mod(p.x * p.x * p.x + A_COEF * p.x + B_COEF, P_MOD);

    return left == right;
}

Point neg_point(Point p) {
    if (p.infinity) return p;
    return point(p.x, mod(-p.y, P_MOD));
}

Point add_points(Point p, Point q) {
    if (p.infinity) return q;
    if (q.infinity) return p;

    if (p.x == q.x && mod(p.y + q.y, P_MOD) == 0) {
        return INF();
    }

    int lambda;

    if (p.x == q.x && p.y == q.y) {
        int numerator = mod(3 * p.x * p.x + A_COEF, P_MOD);
        int denominator = mod(2 * p.y, P_MOD);
        lambda = mod(numerator * inv_mod(denominator, P_MOD), P_MOD);
    } else {
        int numerator = mod(q.y - p.y, P_MOD);
        int denominator = mod(q.x - p.x, P_MOD);
        lambda = mod(numerator * inv_mod(denominator, P_MOD), P_MOD);
    }

    int x3 = mod(lambda * lambda - p.x - q.x, P_MOD);
    int y3 = mod(lambda * (p.x - x3) - p.y, P_MOD);

    return point(x3, y3);
}

Point sub_points(Point p, Point q) {
    return add_points(p, neg_point(q));
}

Point mul_point(int k, Point p) {
    Point result = INF();
    Point current = p;

    while (k > 0) {
        if (k % 2 == 1) {
            result = add_points(result, current);
        }

        current = add_points(current, current);
        k /= 2;
    }

    return result;
}

void print_point(Point p) {
    if (p.infinity) {
        printf("O");
    } else {
        printf("(%d, %d)", p.x, p.y);
    }
}

void task1_find_points() {
    printf("\n================ TASK 1.1 ================\n");
    printf("Points of E751(-1,1) for x from 551 to 585\n\n");

    for (int x = 551; x <= 585; x++) {
        int rhs = mod(x * x * x - x + 1, P_MOD);
        int found = 0;

        for (int y = 0; y < P_MOD; y++) {
            if (mod(y * y, P_MOD) == rhs) {
                printf("x = %d, y = %d -> ", x, y);
                print_point(point(x, y));
                printf("\n");
                found = 1;
            }
        }

        if (!found) {
            printf("x = %d -> no points\n", x);
        }
    }
}

void task1_operations() {
    printf("\n================ TASK 1.2 ================\n");

    int k = 11;
    int l = 3;

    Point P1 = point(72, 497);

    Point Q1_from_table = point(56, 474);
    Point Q1 = Q1_from_table;

    if (!is_on_curve(Q1_from_table)) {
        printf("WARNING: Q = (56, 474) from table is not on curve E751(-1,1).\n");
        printf("For x = 56 valid points are (56,332) and (56,419).\n");
        printf("Using Q = (56,419).\n\n");
        Q1 = point(56, 419);
    }

    Point R1 = point(90, 730);

    printf("Variant 10:\n");
    printf("k = %d, l = %d\n", k, l);

    printf("P = ");
    print_point(P1);
    printf("\n");

    printf("Q = ");
    print_point(Q1);
    printf("\n");

    printf("R = ");
    print_point(R1);
    printf("\n\n");

    Point result1 = mul_point(k, P1);
    Point result2 = add_points(P1, Q1);
    Point result3 = sub_points(add_points(mul_point(k, P1), mul_point(l, Q1)), R1);
    Point result4 = add_points(sub_points(P1, Q1), R1);

    printf("kP = ");
    print_point(result1);
    printf("\n");

    printf("P + Q = ");
    print_point(result2);
    printf("\n");

    printf("kP + lQ - R = ");
    print_point(result3);
    printf("\n");

    printf("P - Q + R = ");
    print_point(result4);
    printf("\n");
}

typedef struct {
    const char *letter;
    Point p;
} LetterPoint;

LetterPoint alphabet[] = {
    {"А", {189, 297, 0}}, {"Б", {189, 454, 0}},
    {"В", {192, 32, 0}},  {"Г", {192, 719, 0}},
    {"Д", {194, 205, 0}}, {"Е", {194, 546, 0}},
    {"Ж", {197, 145, 0}}, {"З", {197, 606, 0}},
    {"И", {198, 224, 0}}, {"Й", {198, 527, 0}},
    {"К", {200, 30, 0}},  {"Л", {200, 721, 0}},
    {"М", {203, 324, 0}}, {"Н", {203, 427, 0}},
    {"О", {205, 372, 0}}, {"П", {205, 379, 0}},
    {"Р", {206, 106, 0}}, {"С", {206, 645, 0}},
    {"Т", {209, 82, 0}},  {"У", {209, 669, 0}},
    {"Ф", {210, 31, 0}},  {"Х", {210, 720, 0}},
    {"Ц", {215, 247, 0}}, {"Ч", {215, 504, 0}},
    {"Ш", {218, 150, 0}}, {"Щ", {218, 601, 0}},
    {"Ъ", {221, 138, 0}}, {"Ы", {221, 613, 0}},
    {"Ь", {226, 9, 0}},   {"Э", {226, 742, 0}},
    {"Ю", {227, 299, 0}}, {"Я", {227, 452, 0}}
};

int alphabet_size = sizeof(alphabet) / sizeof(alphabet[0]);

Point get_letter_point(const char *letter) {
    for (int i = 0; i < alphabet_size; i++) {
        if (strcmp(alphabet[i].letter, letter) == 0) {
            return alphabet[i].p;
        }
    }

    return INF();
}

const char* get_letter_by_point(Point p) {
    for (int i = 0; i < alphabet_size; i++) {
        if (!p.infinity &&
            alphabet[i].p.x == p.x &&
            alphabet[i].p.y == p.y) {
            return alphabet[i].letter;
        }
    }

    return "?";
}

void task2_encrypt_decrypt() {
    printf("\n================ TASK 2 ================\n");
    printf("ECC ElGamal encryption/decryption\n\n");

    Point G = point(0, 1);

    int d = 18;
    Point Q = mul_point(d, G);

    printf("Generator G = ");
    print_point(G);
    printf("\n");

    printf("Private key d = %d\n", d);

    printf("Public key Q = dG = ");
    print_point(Q);
    printf("\n\n");

    const char *message[] = {
        "П", "О", "Д", "Ш", "И", "В", "А", "Л", "Е", "Н", "К", "О"
    };

    int len = sizeof(message) / sizeof(message[0]);

    printf("Message: ПОДШИВАЛЕНКО\n\n");

    printf("Letter | P(message) | k | C1 = kG | C2 = P + kQ | Decrypted\n");
    printf("--------------------------------------------------------------\n");

    for (int i = 0; i < len; i++) {
        Point M = get_letter_point(message[i]);

        int k = 2 + (i % 9);

        Point C1 = mul_point(k, G);
        Point C2 = add_points(M, mul_point(k, Q));

        Point dC1 = mul_point(d, C1);
        Point decrypted = sub_points(C2, dC1);

        printf("%s      | ", message[i]);

        print_point(M);
        printf(" | %d | ", k);

        print_point(C1);
        printf(" | ");

        print_point(C2);
        printf(" | ");

        print_point(decrypted);
        printf(" -> %s\n", get_letter_by_point(decrypted));
    }
}

void task3_ecdsa() {
    printf("\n================ TASK 3 ================\n");
    printf("ECDSA signature generation and verification\n\n");

    Point G = point(416, 55);
    int q = 13;

    int d = 10;
    Point Q = mul_point(d, G);

    int h = 10;

    int k = 2;

    Point kG = mul_point(k, G);
    int r = mod(kG.x, q);

    int t = inv_mod(k, q);

    int s = mod(t * (h + d * r), q);

    printf("Generator G = ");
    print_point(G);
    printf("\n");

    printf("q = %d\n", q);
    printf("Private key d = %d\n", d);

    printf("Public key Q = dG = ");
    print_point(Q);
    printf("\n");

    printf("H(M) = 10 because first letter is П: x = 205, 205 mod 13 = 10\n");
    printf("Chosen k = %d\n\n", k);

    printf("kG = ");
    print_point(kG);
    printf("\n");

    printf("r = x(kG) mod q = %d mod %d = %d\n", kG.x, q, r);
    printf("t = k^(-1) mod q = %d\n", t);
    printf("s = t * (H(M) + d*r) mod q = %d\n", s);

    printf("\nSignature: (r, s) = (%d, %d)\n\n", r, s);

    printf("Verification:\n");

    if (!(r > 0 && r < q && s > 0 && s < q)) {
        printf("Signature is INVALID: r or s is outside valid range.\n");
        return;
    }

    int w = inv_mod(s, q);
    int u1 = mod(w * h, q);
    int u2 = mod(w * r, q);

    Point X = add_points(mul_point(u1, G), mul_point(u2, Q));
    int v = mod(X.x, q);

    printf("w = s^(-1) mod q = %d\n", w);
    printf("u1 = w * H(M) mod q = %d\n", u1);
    printf("u2 = w * r mod q = %d\n", u2);

    printf("X = u1*G + u2*Q = ");
    print_point(X);
    printf("\n");

    printf("v = x(X) mod q = %d mod %d = %d\n", X.x, q, v);

    if (v == r) {
        printf("Verification result: VALID\n");
    } else {
        printf("Verification result: INVALID\n");
    }
}

int main() {
    printf("Elliptic curve: y^2 = x^3 - x + 1 mod 751\n");
    printf("Variant: 10\n");

    task1_find_points();
    task1_operations();
    task2_encrypt_decrypt();
    task3_ecdsa();

    return 0;
}