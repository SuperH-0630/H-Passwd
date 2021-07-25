#ifndef H_PASSWD_MAIN_H
#define H_PASSWD_MAIN_H
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#ifndef KEY_MAX_LEN
#define KEY_MAX_LEN (60) /* 密钥最大长度 */
#endif

#ifndef KEY_MIN_LEN
#define KEY_MIN_LEN (10) /* 密钥最短长度 */
#endif

#ifndef VERSION
#define VERSION "Special version."
#define VERSION_INFO "Special characters, special uses."
#endif

char *base64Encode(char *str);
char *base64Decode(char *code);
void initBase64(char *key);

bool isLegalKey(char *key);
char *makePasswordString(char *account, char *passwd, char *note);
bool getInfoFromPasswordString(char *passwd_base64, char **account, char **passwd, char **note);
void printInfo(char *account, char *passwd, char *note);
void printPasswdStr(char *account, char *passwd, char *note, char *passwd_str);

#endif //H_PASSWD_MAIN_H
