#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LINE_ZERO "1.00"
#define LINE_ONE  "1.25"

#define KERN_ZERO "-0.03em"
#define KERN_ONE  "0.03em"

typedef struct {
    char *data;
    size_t len;
    size_t cap;
} Buffer;

void buf_init(Buffer *b) {
    b->cap = 4096;
    b->len = 0;
    b->data = malloc(b->cap);
    b->data[0] = '\0';
}

void buf_add(Buffer *b, const char *s) {
    size_t n = strlen(s);

    while (b->len + n + 1 > b->cap) {
        b->cap *= 2;
        b->data = realloc(b->data, b->cap);
    }

    memcpy(b->data + b->len, s, n);
    b->len += n;
    b->data[b->len] = '\0';
}

void buf_add_char(Buffer *b, char c) {
    char s[2] = {c, '\0'};
    buf_add(b, s);
}

char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");

    if (!f) {
        printf("Cannot open file: %s\n", path);
        exit(1);
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *data = malloc(size + 1);
    fread(data, 1, size, f);
    data[size] = '\0';

    fclose(f);
    return data;
}

void write_file(const char *path, const char *data) {
    FILE *f = fopen(path, "wb");

    if (!f) {
        printf("Cannot write file: %s\n", path);
        exit(1);
    }

    fwrite(data, 1, strlen(data), f);
    fclose(f);
}

void html_escape_slice(Buffer *out, const char *s, int len) {
    for (int i = 0; i < len; i++) {
        if (s[i] == '<') buf_add(out, "&lt;");
        else if (s[i] == '>') buf_add(out, "&gt;");
        else if (s[i] == '&') buf_add(out, "&amp;");
        else if (s[i] == '"') buf_add(out, "&quot;");
        else buf_add_char(out, s[i]);
    }
}

int utf8_len(unsigned char c) {
    if ((c & 0x80) == 0) return 1;
    if ((c & 0xE0) == 0xC0) return 2;
    if ((c & 0xF0) == 0xE0) return 3;
    if ((c & 0xF8) == 0xF0) return 4;
    return 1;
}

char *message_to_bits(const char *message, int *bit_count) {
    unsigned int len = strlen(message);
    int total = 32 + len * 8;

    char *bits = malloc(total + 1);
    int pos = 0;

    for (int i = 31; i >= 0; i--) {
        bits[pos++] = ((len >> i) & 1) ? '1' : '0';
    }

    for (unsigned int i = 0; i < len; i++) {
        unsigned char c = message[i];

        for (int j = 7; j >= 0; j--) {
            bits[pos++] = ((c >> j) & 1) ? '1' : '0';
        }
    }

    bits[pos] = '\0';
    *bit_count = total;

    return bits;
}

void bits_to_message(const char *bits, int bit_count) {
    if (bit_count < 32) {
        printf("Not enough bits.\n");
        return;
    }

    unsigned int len = 0;

    for (int i = 0; i < 32; i++) {
        len = (len << 1) | (bits[i] == '1');
    }

    if (bit_count < 32 + (int)len * 8) {
        printf("Message is damaged or incomplete.\n");
        return;
    }

    char *message = malloc(len + 1);

    for (unsigned int i = 0; i < len; i++) {
        unsigned char c = 0;

        for (int j = 0; j < 8; j++) {
            c = (c << 1) | (bits[32 + i * 8 + j] == '1');
        }

        message[i] = c;
    }

    message[len] = '\0';

    printf("Extracted message: %s\n", message);

    free(message);
}

void embed_line_spacing(const char *container_path, const char *out_path, const char *message) {
    char *text = read_file(container_path);

    int bit_count;
    char *bits = message_to_bits(message, &bit_count);
    int bit_pos = 0;

    Buffer out;
    buf_init(&out);

    buf_add(&out, "<!DOCTYPE html>\n<html><head><meta charset=\"UTF-8\"></head><body>\n");

    char *line = strtok(text, "\n");

    while (line) {
        const char *spacing = LINE_ZERO;

        if (bit_pos < bit_count) {
            spacing = bits[bit_pos] == '1' ? LINE_ONE : LINE_ZERO;
            bit_pos++;
        }

        buf_add(&out, "<p style=\"line-height:");
        buf_add(&out, spacing);
        buf_add(&out, "\">");

        html_escape_slice(&out, line, strlen(line));

        buf_add(&out, "</p>\n");

        line = strtok(NULL, "\n");
    }

    buf_add(&out, "</body></html>\n");

    if (bit_pos < bit_count) {
        printf("Container is too small. Need %d lines, found %d usable lines.\n", bit_count, bit_pos);
    } else {
        write_file(out_path, out.data);
        printf("Done. Stego file created: %s\n", out_path);
    }

    free(text);
    free(bits);
    free(out.data);
}

void extract_line_spacing(const char *stego_path) {
    char *html = read_file(stego_path);

    Buffer bits;
    buf_init(&bits);

    char *p = html;

    while ((p = strstr(p, "line-height:")) != NULL) {
        p += strlen("line-height:");

        if (strncmp(p, LINE_ONE, strlen(LINE_ONE)) == 0) {
            buf_add_char(&bits, '1');
        } else if (strncmp(p, LINE_ZERO, strlen(LINE_ZERO)) == 0) {
            buf_add_char(&bits, '0');
        }

        p++;
    }

    bits_to_message(bits.data, bits.len);

    free(html);
    free(bits.data);
}

void embed_kerning(const char *container_path, const char *out_path, const char *message) {
    char *text = read_file(container_path);

    int bit_count;
    char *bits = message_to_bits(message, &bit_count);
    int bit_pos = 0;

    Buffer out;
    buf_init(&out);

    buf_add(&out, "<!DOCTYPE html>\n<html><head><meta charset=\"UTF-8\"></head><body><p>\n");

    int i = 0;

    while (text[i] != '\0') {
        if (isspace((unsigned char)text[i])) {
            if (text[i] == '\n') buf_add(&out, "<br>\n");
            else buf_add_char(&out, text[i]);
            i++;
            continue;
        }

        int len1 = utf8_len((unsigned char)text[i]);

        if (text[i + len1] == '\0' || isspace((unsigned char)text[i + len1]) || bit_pos >= bit_count) {
            html_escape_slice(&out, text + i, len1);
            i += len1;
            continue;
        }

        int len2 = utf8_len((unsigned char)text[i + len1]);

        const char *spacing = bits[bit_pos] == '1' ? KERN_ONE : KERN_ZERO;
        bit_pos++;

        buf_add(&out, "<span style=\"letter-spacing:");
        buf_add(&out, spacing);
        buf_add(&out, "\">");

        html_escape_slice(&out, text + i, len1 + len2);

        buf_add(&out, "</span>");

        i += len1 + len2;
    }

    buf_add(&out, "\n</p></body></html>\n");

    if (bit_pos < bit_count) {
        printf("Container is too small. Need %d kerning pairs, found %d usable pairs.\n", bit_count, bit_pos);
    } else {
        write_file(out_path, out.data);
        printf("Done. Stego file created: %s\n", out_path);
    }

    free(text);
    free(bits);
    free(out.data);
}

void extract_kerning(const char *stego_path) {
    char *html = read_file(stego_path);

    Buffer bits;
    buf_init(&bits);

    char *p = html;

    while ((p = strstr(p, "letter-spacing:")) != NULL) {
        p += strlen("letter-spacing:");

        if (strncmp(p, KERN_ONE, strlen(KERN_ONE)) == 0) {
            buf_add_char(&bits, '1');
        } else if (strncmp(p, KERN_ZERO, strlen(KERN_ZERO)) == 0) {
            buf_add_char(&bits, '0');
        }

        p++;
    }

    bits_to_message(bits.data, bits.len);

    free(html);
    free(bits.data);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        return 1;
    }

    if (strcmp(argv[1], "line-embed") == 0) {
        if (argc < 5) {
            return 1;
        }

        embed_line_spacing(argv[2], argv[3], argv[4]);
    }
    else if (strcmp(argv[1], "line-extract") == 0) {
        extract_line_spacing(argv[2]);
    }
    else if (strcmp(argv[1], "kern-embed") == 0) {
        if (argc < 5) {
            return 1;
        }

        embed_kerning(argv[2], argv[3], argv[4]);
    }
    else if (strcmp(argv[1], "kern-extract") == 0) {
        extract_kerning(argv[2]);
    }
    else {
        return 1;
    }

    return 0;
}