#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <inttypes.h>
#include <time.h>

typedef uint64_t h_size_t;
typedef char h_char;
const char *path = NULL;  // 文件地址
h_char *key_tips = NULL;

struct Content {
    h_char *name;
    h_char *passwd_str;
    h_char *date;
    struct Content *next;
};  // 链表

struct Content *content = NULL;
h_size_t content_size = 0;

static size_t fread_str(h_char *dest, size_t size, FILE *fp) {
    return fread(dest, sizeof(h_char), size, fp);
}

static size_t fwrite_str(h_char *dest, size_t size, FILE *fp) {
    return fwrite(dest, sizeof(h_char), size, fp);
}

static size_t fread_size_t(h_size_t *dest, FILE *fp) {
    return fread(dest, sizeof(h_size_t), 1, fp);
}

static size_t fwrite_size_t(h_size_t dest, FILE *fp) {
    return fwrite(&dest, sizeof(h_size_t), 1, fp);
}

static h_char get_fread_char(FILE *fp) {
    h_char ch;
    size_t ret = fread(&ch, sizeof(h_char), 1, fp);
    if (ret != 1)
        return 0;
    return ch;
}

static bool put_fwrite_enter(FILE *fp) {
    static char const ch = '\n';
    return fwrite(&ch, sizeof(h_char), 1, fp) == 1;
}

static bool readFileHead(FILE *fp, h_char *file_md5) {
    size_t ret;
    h_size_t tips_size;
    ret = fread_str(file_md5, MD5_STR_LEN + 1, fp);  // 先读取md5码, 包括NUL
    if (ret < MD5_STR_LEN + 1)
        return false;

    if (fread_size_t(&content_size, fp) != 1)
        return false;

    if (fread_size_t(&tips_size, fp) != 1)
        return false;

    key_tips = calloc(tips_size, sizeof(char));
    ret = fread_str(key_tips, tips_size, fp);
    if (ret < tips_size)
        return false;

    if (get_fread_char(fp) != '\n')
        return false;

    return true;
}

static bool writeFileHead(FILE *fp, h_char *md5str) {
    size_t ret;
    h_char *tips = "There isn't tips.";
    if (key_tips != NULL)
        tips = key_tips;

    ret = fwrite_str(md5str, MD5_STR_LEN + 1, fp);  // 写入NUL
    if (ret < MD5_STR_LEN + 1)
        return false;

    if (fwrite_size_t(content_size, fp) != 1)
        return false;

    if (fwrite_size_t(strlen(tips) + 1, fp) != 1)
        return false;

    ret = fwrite_str(tips, strlen(tips) + 1, fp);  // 写入NULL
    if (ret < strlen(tips) + 1)
        return false;

    if (!put_fwrite_enter(fp))
        return false;

    return true;
}

static bool readFileInfo(FILE *fp, struct Content *con) {
    size_t ret;
    h_size_t name_size;
    h_size_t passwd_str_size;
    h_size_t date_size;
    h_char *name = NULL;
    h_char *passwd_str = NULL;
    h_char *date_str = NULL;

    if (fread_size_t(&name_size, fp) != 1)
        return false;

    if (fread_size_t(&passwd_str_size, fp) != 1)
        return false;

    if (fread_size_t(&date_size, fp) != 1)
        return false;

    name = calloc(name_size, sizeof(h_char));  // 包含 NUL
    passwd_str = calloc(passwd_str_size, sizeof(h_char));  // 包含 NUL
    date_str = calloc(date_size, sizeof(h_char));  // 包含 NUL

    ret = fread_str(name, name_size, fp);
    if (ret < name_size)
        goto error;

    ret = fread_str(passwd_str, passwd_str_size, fp);
    if (ret < passwd_str_size)
        goto error;

    ret = fread_str(date_str, date_size, fp);
    if (ret < date_size)
        goto error;

    if (get_fread_char(fp) != '\n') {
    error:
        free(name);
        free(passwd_str);
        free(date_str);
        return false;
    }

    con->name = name;
    con->passwd_str = passwd_str;
    con->date = date_str;
    return true;
}

static bool writeFileInfo(FILE *fp, struct Content *con) {
    size_t ret;
    h_char *name = con->name;
    h_char *passwd_str = con->passwd_str;
    h_char *date_str = con->date;
    h_size_t name_size = strlen((char *)name);
    h_size_t passwd_str_size = strlen((char *)passwd_str);
    h_size_t date_size = strlen((char *)date_str);

    if (fwrite_size_t(name_size + 1, fp) != 1)  // 写入NUL
        return false;

    if (fwrite_size_t(passwd_str_size + 1, fp) != 1)
        return false;

    if (fwrite_size_t(date_size + 1, fp) != 1)
        return false;

    ret = fwrite_str(name, name_size + 1, fp);  // 写入NUL
    if (ret < name_size)
        goto error;

    ret = fwrite_str(passwd_str, passwd_str_size + 1, fp);
    if (ret < passwd_str_size)
        goto error;

    ret = fwrite_str(date_str, date_size + 1, fp);
    if (ret < date_size)
        goto error;

    if (!put_fwrite_enter(fp))
        goto error;

    return true;
    error: return false;
}

static struct Content *freeOneConten(struct Content *con) {
    struct Content *re = con->next;
    free(con->name);
    free(con->passwd_str);
    free(con->date);
    free(con);
    return re;
}

static void freeContent(void) {
    struct Content *con = content;
    for (int i = 0; i < content_size; i++)
        con = freeOneConten(con);
    content_size = 0;
    content = NULL;
}

static h_char *getContentMD5(void) {
    h_char *md5str = calloc(MD5_STR_LEN + 1, sizeof(h_char));
    h_char md5_value[MD5_SIZE];
    struct Content *con = content;
    MD5_CTX md5;

    MD5Init(&md5);
    for (int i = 0; i < content_size; i++, con = con->next) {
        h_char *tmp = calloc(strlen(con->name) + strlen(con->passwd_str) + strlen(con->date) + 1, sizeof(h_char));
        strcpy((char *)tmp, (char *)con->name);
        strcat((char *)tmp, (char *)con->passwd_str);
        strcat((char *)tmp, (char *)con->date);
        MD5Update(&md5, (unsigned char *)tmp, strlen((char *)tmp));
        free(tmp);
    }
    MD5Final(&md5, (unsigned char *)md5_value);

    for(int i = 0; i < MD5_SIZE; i++)
        snprintf((char *)md5str + i * 2, 2 + 1, "%02x", md5_value[i]);
    return md5str;
}

void writePasswdFile(void) {  // 写入数据
    FILE *fp = fopen(path, "wb");
    h_char *md5 = getContentMD5();

    if (!writeFileHead(fp, md5))
        goto error;

    struct Content *con = content;
    for (int i = 0; i < content_size; i++, con = con->next) {
        if (!writeFileInfo(fp, con))
            goto error;
    }

    if (!put_fwrite_enter(fp))
        goto error;

    free(md5);
    fclose(fp);
    return;

    error:
    free(md5);
    fclose(fp);
    fprintf(stderr, "File writing error occurred.\n");
}

void printContent(void) {
    struct Content *con = content;
    printf("********************\n");
    printf("Print all label in file.\n");
    printf("total: %d\n", content_size);
    printf("--------------------\n", content_size);
    for (int i = 0; i < content_size; i++, con = con->next)
        printf("%d. %s : name: %s, label: %s\n", i, con->date, con->name, con->passwd_str);
    printf("********************\n");
}

bool initPasswdInit(const char *path_) {
    h_char file_md5[MD5_STR_LEN + 1] = {0};
    h_char *get_md5;
    FILE *fp = fopen(path_, "rb");
    path = path_;
    if (fp == NULL) {
        content_size = 0;
        content = NULL;
        goto re;
    }

    if (!readFileHead(fp, file_md5))
        goto error;

    struct Content **con = &content;
    for (int i = 0; i < content_size; i++, con = &((*con)->next)) {
        *con = calloc(1, sizeof(struct Content));
        if (!readFileInfo(fp, *con))
            goto error;
    }

    if (get_fread_char(fp) != '\n')
        goto error;

    get_md5 = getContentMD5();
    if (strcmp((char *)get_md5, (char *)file_md5) != 0)
        goto error;

    fclose(fp);

    re:
    atexit(freeContent);
    return true;

    error:
    freeContent();
    fclose(fp);
    return false;
}

void addContent(char *name, char *passwd_str) {
    time_t rawtime;
    char buffer [128];
    time(&rawtime);
    strftime(buffer,sizeof(buffer), "%Y/%m/%d-%H:%M:%S", localtime(&rawtime));

    h_char *name_cp = calloc(strlen(name) + 1, sizeof(h_char));
    h_char *passwd_str_cp = calloc(strlen(passwd_str) + 1, sizeof(h_char));
    h_char *date_str = calloc(strlen(buffer) + 1, sizeof(h_char));
    strcpy(name_cp, name);
    strcpy(passwd_str_cp, passwd_str);
    strcpy(date_str, buffer);


    struct Content *new = calloc(1, sizeof(struct Content));
    new->name = name_cp;
    new->passwd_str = passwd_str_cp;
    new->date = date_str;
    new->next = content;
    content = new;
    content_size++;
}

char *findContent(char *name) {
    struct Content *con = content;
    for (int i = 0; i < content_size; i++, con = content->next) {
        if (!strcmp(con->name, name)) {
            char *re = calloc(strlen(con->passwd_str) + 1, sizeof(char));
            strcpy(re, con->passwd_str);
            return re;
        }
    }
    return NULL;
}

void printFileTips(void) {
    if (key_tips != NULL)
        printf("Tips in file: %s\n", key_tips);
    else
        printf("Tips in file: There isn't tips.");
}

void setFileTips(char *tips) {
    if (key_tips != NULL)
        free(key_tips);

    key_tips = calloc(strlen(tips) + 1, sizeof(char));
    strcpy(key_tips, tips);
}

static struct Content **findContentByIndex(size_t index) {
    struct Content **re = &content;
    if (index >= content_size)
        return NULL;

    for (int i = 0; i < index; i++)
        re = &((*re)->next);

    return re;
}

bool delContent(size_t del_index[], size_t size) {  // del_index按从小到大的顺序
    struct Content **(del_content[DEL_CONTENT_SZIE]);

    for (int i = 0; i < size; i++) {
        struct Content **con = findContentByIndex(del_index[i]);
        if (con == NULL)
            return false;
        del_content[i] = con;
    }

    for (int i = 0; i < size; i++) {
        *del_content[i] = freeOneConten(*(del_content[i]));  // 删除指向的content，并且指向该content的下一个content
        content_size--;
    }

    return true;
}
