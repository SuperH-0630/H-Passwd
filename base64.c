#include "main.h"

//定义base64编码表
static char *base = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";  // base-64的基础编码 + 等号
static char base64_table[64] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static unsigned char table_base64[128] = {0};
static char fill = '=';  // 默认填充字符

void initBase64(char *_key) {  // 生成编码序列
    int count = 0;

    if (strlen(_key) > 0) {
        char *key = base64Encode(_key);  // 先按基础base64编码
        size_t key_len = strlen(key);
        fill = key[0];

        memset(base64_table, 0, 64);
        for (int i = 1; i < key_len; i++) {  // 跳过第一个字符
            if (key[i] != fill && strchr(base64_table, key[i]) == NULL) {
                base64_table[count] = key[i];
                count++;
            }
        }

        free(key);
    }

    for (int i = 0; count < 64; i++) {  // 填充base的内容
        if (base[i] != fill && strchr(base64_table, base[i]) == NULL) {
            base64_table[count] = base[i];
            count++;
        }
    }

    for (int i = 0; i < 64; i++)
        table_base64[base64_table[i]] = i;

}

char *base64Encode(char *str) {
    size_t len;
    size_t str_len;
    size_t str_len_3;
    char *res;

    str_len = strlen(str);
    str_len_3 = (str_len / 3) * 3;

    if(str_len % 3 == 0)
        len = str_len / 3 * 4;  // 每三个char共3 * 8 = 24(bit)，用4个char来表示（这4个char其实只用了 6 bit）
    else
        len = (str_len / 3 + 1) * 4;

    res = calloc(len + 1, sizeof(char));

    for(int i = 0, j = 0; j < str_len_3; j += 3, i += 4)  // i是res索引, j是str索引
    {
        res[i + 0] = base64_table[(unsigned)str[j] >> 2];
        res[i + 1] = base64_table[(unsigned)(str[j] & 0x03) << 4 | (unsigned)(str[j + 1] >> 4)];
        res[i + 2] = base64_table[(unsigned)(str[j + 1] & 0x0f) << 2 | (unsigned)(str[j + 2] >> 6)];
        res[i + 3] = base64_table[(unsigned)str[j + 2] & 0x3f];
    }

    switch(str_len % 3)
    {
        case 1:
            res[len - 4] = base64_table[(unsigned)str[str_len_3] >> 2];
            res[len - 3] = base64_table[(unsigned)(str[str_len_3]&0x3) << 4];
            res[len - 2] = fill;
            res[len - 1] = fill;
            break;
        case 2:
            res[len - 4] = base64_table[(unsigned)str[str_len_3] >> 2];
            res[len - 3] = base64_table[(unsigned)(str[str_len_3] & 0x3) << 4 | (unsigned)(str[str_len_3 + 1] >> 4)];
            res[len - 2] = base64_table[(unsigned)(str[str_len_3 + 1] & 0xf) << 2];
            res[len - 1] = fill;
            break;
        default:
            break;
    }

    return res;
}


unsigned int base64ToASCII(char ascii) {
    char *p = strchr(base64_table, (int)ascii);
    if (p == NULL)
        exit(EXIT_FAILURE);

    return p - base64_table;
}


char *base64Decode(char *code) {
    size_t len;
    size_t str_len;
    char *res;
    int i, j;

    len = strlen(code);
    str_len = len / 4 * 3;
    if (code[len - 1] == fill) {
        str_len--;
        if (code[len - 2] == fill)
            str_len--;
    }

    res = calloc(str_len + 1, sizeof(char));

    for (i = 0, j = 0; i < len - 2; j += 3, i += 4) {
        res[j] = table_base64[code[i]] << 2 | (table_base64[code[i + 1]] >> 4);
        res[j + 1] = (table_base64[code[i + 1]] << 4) | (table_base64[code[i + 2]] >> 2);
        res[j + 2] = (table_base64[code[i + 2]] << 6) | (table_base64[code[i + 3]]);
    }

    return res;
}
