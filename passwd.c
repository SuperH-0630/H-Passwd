#include "main.h"

#include <ctype.h>
#include <stdio.h>

#define CHECK_PRINT(key, key_len, re, info) do{ for (int i = 0; i < (key_len); i++) { \
    if (!isprint((key)[i]) || isspace((key)[i])) { \
        fprintf(stderr, info " (at %d is %d).\n", i, (key)[i]); \
        return (re); \
    } \
}}while(0)

bool isLegalKey(char *key) {  // 判断是否为合法的密钥
    size_t key_len = strlen(key);
    if (key_len > KEY_MAX_LEN) {
        fprintf(stderr, "Key too long (> %d).\n", KEY_MAX_LEN);
        return false;
    }

    if (key_len < KEY_MIN_LEN) {
        fprintf(stderr, "Key too short (< %d).\n", KEY_MIN_LEN);
        return false;
    }

    CHECK_PRINT(key, key_len, false, "The key does not allow invisible characters");  // 检查是否可读可打印
    return true;
}

#define CHECK_STRLEN(key, re, info) do{ if ((key) == 0) { \
    fprintf(stderr, info "\n"); \
    return (re); \
}}while(0)

static char *makePasswordString_(char *account, char *passwd, char *note) {
    char *passwd_str;
    size_t account_len = strlen(account);
    size_t passwd_len = strlen(passwd);
    size_t note_len = strlen(note);
    size_t str_len = (account_len + passwd_len + note_len + 4);  // 4个标记长度

    CHECK_STRLEN(account_len, NULL, "Account must be entered.");
    CHECK_STRLEN(passwd_len, NULL, "Password must be entered.");
    CHECK_STRLEN(note_len, NULL, "Note must be entered.");

    if (str_len > 100) {  // 太大了char会放不下
        fprintf(stderr, "Account or passwd or note too long (The sum is greater than 100).\n");
        return NULL;
    }

    CHECK_PRINT(account, account_len, NULL, "Account contains illegal characters");
    CHECK_PRINT(passwd, passwd_len, NULL, "Passwd contains illegal characters");

    for (int i = 0; i < note_len; i++) {
        if (!isprint(note[i]) || (isspace(note[i]) && note[i] != ' ')) {  // 允许为空格
            fprintf(stderr, "Note contains illegal characters (at %d is %d).\n", i, note[i]);
            return NULL;
        }
    }

    passwd_str = calloc(str_len + 1, sizeof(char));
    passwd_str[0] = (char)str_len;
    passwd_str[1] = (char)account_len;
    passwd_str[2] = (char)passwd_len;
    passwd_str[3] = (char)note_len;
    strcpy(passwd_str + 4, account);
    strcpy(passwd_str + 4 + account_len, passwd);
    strcpy(passwd_str + 4 + account_len + passwd_len, note);

    return passwd_str;
}

char *makePasswordString(char *account, char *passwd, char *note){
    char *passwd_str = makePasswordString_(account, passwd, note);
    if (passwd_str == NULL)
        return NULL;

    char *base64 = base64Encode(passwd_str);
    free(passwd_str);
    return base64;
}

static bool getInfoFromPasswordString_(char *passwd_str, char **account, char **passwd, char **note) {
    size_t passwd_str_len = strlen(passwd_str);

    if (passwd_str_len < 4) {
        fprintf(stderr, "Key or label error.\n");
        return false;
    }

    size_t account_len = (size_t)passwd_str[1];
    size_t passwd_len = (size_t)passwd_str[2];
    size_t note_len = (size_t)passwd_str[3];
    size_t str_len = (size_t)passwd_str[0];

    if (passwd_str_len != str_len) {
        fprintf(stderr, "Key or label error.\n");
        return false;
    }

    *account = calloc(account_len + 1, sizeof(char ));
    *passwd = calloc(passwd_len + 1, sizeof(char ));
    *note = calloc(note_len + 1, sizeof(char ));
    memcpy(*account, passwd_str + 4, account_len);
    memcpy(*passwd, passwd_str + 4 + account_len, passwd_len);
    memcpy(*note, passwd_str + 4 + account_len + passwd_len, note_len);

    return true;
}

bool getInfoFromPasswordString(char *passwd_base64, char **account, char **passwd, char **note) {
    char *passwd_str = base64Decode(passwd_base64);
    bool re = getInfoFromPasswordString_(passwd_str, account, passwd, note);
    free(passwd_str);
    return re;
}

void printInfo(char *account, char *passwd, char *note) {
    printf("Account Information Print\n");
    printf("Account: '%s'\n", account);
    printf("Password: '%s'\n", passwd);
    printf("Note: '%s'\n", note);
    printf("H-Passwd: So that the password will not be forgotten.");
}

void printPasswdStr(char *account, char *passwd, char *note, char *passwd_str) {
    printf("Account Information Print\n");
    printf("Account: '%s'\n", account);
    printf("Password: '%s'\n", passwd);
    printf("Note: '%s'\n", note);
    printf("Label: '%s'\n", passwd_str);
    printf("You can retrieve account information via label and key. (Include Account, Password and Note.)");
}
