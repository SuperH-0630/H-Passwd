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

#define MD5_SIZE (16)
#define MD5_STR_LEN (MD5_SIZE * 2)

#define DEL_CONTENT_SZIE (20)

static void *s_calloc(size_t n, size_t s) {
   void *new = calloc(n, s);
   if (new == NULL)
       exit(EXIT_FAILURE);
   return new;
}

#define free(p) (((p) != NULL ? free(p) : NULL), ((p) = NULL))
#define calloc(n, s) (s_calloc(n, s))

struct MD5_CTX {
    unsigned int count[2];
    unsigned int state[4];
    unsigned char buffer[64];
};

typedef struct MD5_CTX MD5_CTX;

void MD5Init(MD5_CTX *context);
void MD5Update(MD5_CTX *context,unsigned char *input,unsigned int input_len);
void MD5Final(MD5_CTX *context,unsigned char digest[16]);

char *base64Encode(char *str);
char *base64Decode(char *code);
void initBase64(char *key);

bool isLegalKey(char *key);
char *makePasswordString(char *account, char *passwd, char *note);
bool getInfoFromPasswordString(char *passwd_base64, char **account, char **passwd, char **note);
void printInfo(char *account, char *passwd, char *note);
void printPasswdStr(char *account, char *passwd, char *note, char *passwd_str);

void randomInit(void);
unsigned long long getRandom(int min, int max);
char *randomPasswd(void);
bool initPasswdInit(const char *path_);
void addContent(char *name, char *passwd_str);
char *findContent(char *name);
void printFileTips(void);
void setFileTips(char *tips);
void printContent(void);
bool writePasswdFile(void);
bool delContent(size_t del_index[], size_t size);
void delContentByName(char *name);

#endif //H_PASSWD_MAIN_H
