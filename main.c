#include "main.h"
#include "argument.h"
#include <stdio.h>
#include <ctype.h>

#ifndef INCLUDE_KEY
char *key = NULL;
#else
char *key = INCLUDE_KEY;
#endif

#define TIPS_SIZE (30)
#define IO_READ_SIZE (20)
#define PATH_SIZE (100)

char *program_name = NULL;
struct arg_define arg[] = {
        {.ch='v', .name="version", .flat='v', .argument=no_argument},
        {.ch='h', .name="help", .flat='h', .argument=no_argument},
        {.ch='s', .name="set-pw", .flat='s', .argument=no_argument},
        {.ch='g', .name="get-pw", .flat='g', .argument=no_argument},
        {.ch='t', .name="tips", .flat='t', .argument=no_argument},
        {.ch='f', .name="file", .flat='f', .argument=can_argument},
#ifdef INCLUDE_KEY
        {.ch='c', .name="check-key", .flat='c', .argument=must_argument},
#else
        {.ch='i', .name="in-file", .flat='i', .argument=no_argument},
        {.ch='?', .name="set-tips", .flat='w', .argument=no_argument},
        {.ch='p', .name="print-label", .flat='p', .argument=no_argument},
        {.ch='d', .name="delete", .flat='d', .argument=no_argument},
        {.ch='?', .name="delete-old", .flat='o', .argument=no_argument},
#endif
        {.ch=0},
};

enum {
    no = 0,
    set_pw,
    get_pw,
    del_pw,
    set_pw_file,
} work = no;

bool in_file = false;  // 是否在文件中工作
bool print_passwd = false;  // 是否打印content
bool del_old = false;

void printVersion(void);
void printHelp(void);
bool setPassWd(void);
bool setPassWdFromFile(char *path);
bool getPassWd(void);
bool delPassWd(void);
char *getTipsFromStdin(void);

int main(int argc, char **argv) {
    bool print_tips = false;
    bool set_tips = false;
    char *read_path = NULL;

    printf("Welcome to use H-Password.\n");
#ifdef INCLUDE_KEY
    printf("Exclusive custom user: %s\n", UserName);
#endif
    program_name = argv[0];
    randomInit();
    initOpt(true, argc, argv, arg);

    for (getOpt(); opt_flat != 0; getOpt()) {
        switch (opt_flat) {
            case 'h':
                printHelp();
                goto little_exit;
            case 'v':
                printVersion();
                goto little_exit;
#ifdef INCLUDE_KEY
            case 'c':
                if (strcmp(key, opt_val) != 0) {
                    printf("Key verification failed.\n");
                    printf("Tips: %s\n", KeyTips);
                } else
                    printf("Key verification passed.\n");
                goto little_exit;
#endif
            case 's':
                if (work != no)
                    goto what_do;
                work = set_pw;
                break;
            case 'g':
                if (work != no)
                    goto what_do;
                work = get_pw;
                break;
            case 'd':
                if (work != no)
                    goto what_do;
                work = del_pw;
                break;
            case 'o':
                del_old = true;
                break;
            case 'i':
                in_file = true;
                break;
            case 'f':
                if (work != no)
                    goto what_do;
                work = set_pw_file;
                if (opt_val == NULL || strcmp(opt_val, "stdin") == 0) {
                    char *enter_flat;
                    read_path = calloc(PATH_SIZE + 1, sizeof(char));
                    printf("Enter the file path[< %d]:", PATH_SIZE);
                    fgets(read_path, PATH_SIZE + 1, stdin);
                    if ((enter_flat = strchr(read_path, '\n')) == NULL) {
                        free(enter_flat);
                        fprintf(stdin, "Path too long[> %d].\n", PATH_SIZE);
                        exit(EXIT_FAILURE);
                    }
                } else {
                    read_path = calloc(strlen(opt_val) + 1, sizeof(char));
                    strcpy(read_path, opt_val);
                }
                break;
            case 't':
#ifdef INCLUDE_KEY
                printf("Tips: %s\n", KeyTips);
#else
                print_tips = true;
#endif
                break;
            case 'w':
                set_tips = true;
                break;
            case 'p':
                print_passwd = true;
                break;
            case 0:
                break;
            case '?':
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (work == no && print_tips == false && set_tips == false && print_passwd == false) {
        what_do:
        fprintf(stderr, "What should I do?\n");
        fflush(stderr);
        printHelp();
        exit(EXIT_FAILURE);
    }

    if (argc - opt_i > 1) {
        fprintf(stderr, "Too many argument.");
        exit(EXIT_FAILURE);
    } else if (argc - opt_i == 1) {
        if (key != NULL)
            exit(EXIT_FAILURE);
        key = calloc(strlen(argv[opt_i]) + 1, sizeof(char ));
        strcpy(key, argv[opt_i]);
        opt_i++;
    } else if (key == NULL) {
        char *enter_flat = NULL;
        printf("Please Enter The Key:");
        key = calloc(KEY_MAX_LEN + 10, sizeof(char ));
        fgets(key, KEY_MAX_LEN + 10, stdin);
        if ((enter_flat = strchr(key, '\n')) == NULL) {
            fprintf(stderr, "Key too long for stdin.\n");
            exit(EXIT_FAILURE);
        }
        *enter_flat = 0;
    }

    if (!isLegalKey(key))
        exit(EXIT_FAILURE);
    initBase64(key);

    if (in_file) {
        if (!initPasswdInit("passwd.hpd")) {
            fprintf(stderr, "Unable to open HPD file.\n");
            exit(EXIT_FAILURE);
        }
        if (print_tips)
            printFileTips();

        if (set_tips) {
            char *tips = getTipsFromStdin();
            if (tips == NULL)
                exit(EXIT_FAILURE);
            setFileTips(tips);
            free(tips);
        }

    } else if (print_tips || set_tips || print_passwd) {
        fprintf(stderr, "The -t/-p/--set-tips option relies on the -i option.\n");
        exit(EXIT_FAILURE);
    }

    bool status = true;
    if (work == set_pw)
        status = setPassWd();
    else if (work == get_pw)
        status = getPassWd();
    else if (work == del_pw) {
        if (!in_file) {
            fprintf(stderr, "The --delete option relies on the -i option.\n");
            exit(EXIT_FAILURE);
        }
        status = delPassWd();
    } else if (work == set_pw_file)
        status = setPassWdFromFile(read_path);
    else if (in_file && print_passwd)
        printContent();

#ifndef INCLUDE_KEY
    free(key);
#endif

    free(read_path);
    if (!status)
        return EXIT_FAILURE;

    if (in_file) {
        if (!writePasswdFile())  // 写入数据
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;

    little_exit:
    if (argc != 2)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;

}

void printVersion(void) {
    printf("%s Version:\n%s\n%s\n", program_name, VERSION, VERSION_INFO);
}

void printHelp(void) {
    printf("Usage: %s <[option]..> [key]\n", program_name);
    printf("Option: \n");
    printf(" -v --version       Show version.\n");
    printf(" -h --help          Show help.\n");
    printf(" -s --set-pw        Set Password.\n");
    printf(" -g --get-pw        Get Password.\n");
    printf(" -t --tips          Print the key tips.\n");
#ifndef INCLUDE_KEY
    printf(" -i --in-file       Save and get passwd from file.\n");
    printf(" -p --print-label   Prints all labels in the file.\n");
    printf(" -d --delete-label  Delete labels in the file.\n");
    printf(" --set-tips         Save or reset the key-tips in file.\n");
    printf(" --delete-old       Update password and delete same-name old password.\n");
#endif
    printf("\n");

#ifdef INCLUDE_KEY
    printf("!!! Exclusive custom user: %s !!!\n", UserName);
    printf("!!! Key has been written. !!!\n");
    printf("!!! You can check through the parameter --check-key (or -c). !!!\n\n");
#endif

    printf("How to use?\n");
    printf( "     You can choose a key and remember it. \n"
           "With set-pw, a tag is generated after you \n"
           "enter your key and the account information \n"
           "you want to store. Using get-pw, you can enter \n"
           "your own key and label to obtain the stored \n"
           "account information.\n"
           "     Account information includes the account, \n"
           "password, and remarks. The account and password \n"
           "can only be visible non-whitespace characters. \n"
           "Remarks Can be visible non-whitespace characters \n"
           "and Spaces. All three must be set and the total \n"
           "length must not exceed 100.\n\n");

    printf("Length limit:\n");
    printf("Key: [%d - %d]\n", KEY_MIN_LEN, KEY_MAX_LEN);
    printf("Tips: [0 - %d]\n\n", TIPS_SIZE);

    printf("Name origin:\n");
    printf("I'm super Huan. H in h-password is Huan in SuperHuan. \n"
           "Password means that the software is password software.\n\n");

}

#define GRAMMAR_INFO    "File format:\n" \
                        "[account] [passwd] \"note\" \"name\"\n"

#define INVALID_INFO    "Invalid file format\n" \
                         GRAMMAR_INFO \
                        "Note: Only three Spaces are allowed, \"name\" is not required.\n"

static char *readNoteFromFile(FILE *fp) {  // 读取note和name
    char *buf = calloc(IO_READ_SIZE + 1, sizeof(char ));
    int size = IO_READ_SIZE;
    int index = 0;
    int ch;

    if ((ch = getc(fp)) != '"') {
        ungetc(ch, fp);
        fprintf(stderr, "You need to start and end with a quote.\n");
        fprintf(stderr, GRAMMAR_INFO);
        return false;
    }

    while ((ch = getc(fp)) != '"') {
        if (ch == '\n' || ch == EOF) {
            fprintf(stderr, INVALID_INFO);
            return NULL;
        }

        buf[index] = (char)ch;
        index++;

        if (index == size) {
            char *new_buf = calloc(size + IO_READ_SIZE + 1, sizeof(char ));
            strcpy(new_buf, buf);
            free(buf);
            buf = new_buf;
        }
    }

    return buf;
}


static char *readAccountFromFile(FILE *fp) {  // 读取账号或者密码的函数
    char *buf = calloc(IO_READ_SIZE + 1, sizeof(char ));
    int size = IO_READ_SIZE;
    int index = 0;
    int ch;

    while ((ch = getc(fp)) != ' ') {
        if (ch == '\n' || ch == EOF) {
            fprintf(stderr, INVALID_INFO);
            return NULL;
        }

        buf[index] = (char)ch;
        index++;

        if (index == size) {
            char *new_buf = calloc(size + IO_READ_SIZE + 1, sizeof(char ));
            strcpy(new_buf, buf);
            free(buf);
            buf = new_buf;
        }
    }

    return buf;
}

bool setPassWdFromFile(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "File not fount: %s\n", path);
        return false;
    }

    int ch;
    while ((ch = getc(fp)) != EOF) {
        char *account = NULL;
        char *passwd = NULL;
        char *note = NULL;
        char *in_file_name = NULL;
        char *passwd_str = NULL;

        ungetc(ch, fp);
        account = readAccountFromFile(fp);
        if (account == NULL)
            goto ERROR1;

        passwd = readAccountFromFile(fp);
        if (passwd == NULL)
            goto ERROR2;

        note = readNoteFromFile(fp);
        if (note == NULL)
            goto ERROR3;

        if ((ch = getc(fp)) == ' ') {
            in_file_name = readNoteFromFile(fp);
            if (in_file_name == NULL)
                goto ERROR4;
            ch = getc(fp);
        }

        if (ch == '\r')
            ch = getc(fp);  // 跳过/r

        if (ch != '\n' && ch != EOF) {
            fprintf(stderr, INVALID_INFO);
            goto ERROR5;
        }

        if (in_file_name == NULL)
            in_file_name = note;  // 指针复制

        passwd_str = makePasswordString(account, passwd, note);
        if (passwd_str == NULL)
            goto ERROR5;

        printf("********************\n");
        fflush(stdout);
        printPasswdStr(account, passwd, note, passwd_str);

        if (in_file) {
            if (del_old)
                delContentByName(in_file_name);

            addContent(in_file_name, passwd_str);
            printf("The label has been written to the file. (name: %s)\n", in_file_name);
        }

        free(passwd_str);
        if (in_file_name != note)
            free(in_file_name);
        free(account);
        free(passwd);
        free(note);
        continue;
        
        ERROR5:
        if (in_file_name != note)
            free(in_file_name);
        ERROR4: free(note);
        ERROR3: free(passwd);
        ERROR2: free(account);
        fclose(fp);
        ERROR1: return false;
    }

    if (print_passwd)
        printContent();
    fclose(fp);
    return true;
}

#undef INVALID_INFO
#undef GRAMMAR_INFO

static bool readWord(char **word, int len, char *word_name) {
    char *ch;
    printf("Please Enter %s :", word_name);
    *word = calloc(len + 10, sizeof(char));
    fgets(*word, len + 10, stdin);

    if ((ch = strchr((*word), '\n')) == ((void *) 0)) {
        fprintf(stderr, "%s too long for stdin.\n", word_name);
        return false;
    }

    *ch = 0;
    return true;
}

bool setPassWd(void) {
    char *account = NULL;
    char *passwd = NULL;
    char *note = NULL;
    char *in_file_name = NULL;
    char *passwd_str = NULL;

    if (!readWord(&account, 100, "Your Account"))
        goto ERROR1;

    if (!readWord(&passwd, 100, "Your Password"))
        goto ERROR2;

    if (*passwd == 0) {  // 未输入密码内容
        free(passwd);
        passwd = randomPasswd();  // 随机密码
        printf("random password: '%s'\n", passwd);
    }

    if (!readWord(&note, 100, "Your note"))
        goto ERROR3;

    if (in_file) {
        if (!readWord(&in_file_name, 100, "Account name"))
            goto ERROR4;
    }

    passwd_str = makePasswordString(account, passwd, note);
    if (passwd_str == NULL)
        goto ERROR5;

    printf("********************\n");
    fflush(stdout);
    printPasswdStr(account, passwd, note, passwd_str);

    if (in_file) {
        if (del_old)
            delContentByName(in_file_name);

        addContent(in_file_name, passwd_str);
        printf("The label has been written to the file. (name: %s)\n", in_file_name);
        if (print_passwd)
            printContent();
    }

    if (in_file_name != NULL)
        free(in_file_name);
    free(account);
    free(passwd);
    free(note);
    free(passwd_str);
    return true;

    ERROR5:
    if (in_file_name != NULL)
        free(in_file_name);
    ERROR4: free(note);
    ERROR3: free(passwd);
    ERROR2: free(account);
    ERROR1: return false;
}

bool getPassWd(void) {
    char *account = NULL;
    char *passwd = NULL;
    char *note = NULL;
    char *in_file_name = NULL;
    char *passwd_str = NULL;

    if (in_file && print_passwd)
        printContent();

    if (in_file && !print_passwd) {
        if (readWord(&in_file_name, 50, "Password Name"))
            goto ERROR1;

        passwd_str = findContent(in_file_name);
        if (passwd_str == NULL) {
            fprintf(stderr, "name [%s] not found.", in_file_name);
            free(in_file_name);
            return false;
        }
        free(in_file_name);
    } else {
        if (readWord(&passwd_str, 200, "Your Label"))
            goto ERROR1;
    }

    if (!getInfoFromPasswordString(passwd_str, &account, &passwd, &note))
        goto ERROR2;
    printf("***********\n");
    printInfo(account, passwd, note);

    free(account);
    free(passwd);
    free(note);
    free(passwd_str);
    return true;

    ERROR2: free(passwd_str);
    ERROR1: return false;
}

static int charToInt(char ch) {
    switch (ch) {
        case '0': return 0;
        case '1': return 1;
        case '2': return 2;
        case '3': return 3;
        case '4': return 4;
        case '5': return 5;
        case '6': return 6;
        case '7': return 7;
        case '8': return 8;
        case '9': return 9;
        default: return -1;
    }
}

static bool getNumber(int *num) {
    int ch;
    int re = false;
    bool negative = false;  // 是否为负数
    int the_num = 0;

    while (isspace(ch = getc(stdin))) {  // 清除空格
        if (ch == '\n') {
            ungetc(ch, stdin);
            return false;
        }
    }

    if (ch == '-') {
        negative = true;
        ch = getc(stdin);
    }

    for (; isalnum(ch); ch = getc(stdin)) {
        re = true;
        int n = charToInt((char) ch);
        if (n == -1)
            return false;
        the_num = the_num * 10 + n;
    }

    ungetc(ch, stdin);

    if (!re)
        return false;

    *num = (negative? -the_num : the_num);
    return true;
}

bool delPassWd(void) {
    int i;
    int max = -1;
    size_t del_index[DEL_CONTENT_SZIE] = {0};

    if (print_passwd)
        printContent();

    printf("Enter the serial number of the label you want to delete (starting from 0)\n"
           "A maximum of %d parameters can be entered.\n", DEL_CONTENT_SZIE);

    for (i = 0; i < DEL_CONTENT_SZIE; i++) {
        int index = 0;
        if (!getNumber(&index))
            break;
        if (index <= max) {
            printf("Please enter in descending order, skipped %d.", index);
            continue;
        }

        printf("get index: %d\n", index);
        del_index[i] = index;
        max = index;
    }

    printf(" I am out\n");
    int ch;
    while ((ch = getc(stdin)) != '\n' && ch != EOF)
        continue;

    printf(" I am out 2");
    if (!delContent(del_index, i)) {
        fprintf(stderr, "Delete error\n");
        return false;
    }

    return true;
}

char *getTipsFromStdin(void) {
    char *tips = calloc(TIPS_SIZE + 1, sizeof(char));
    char *enter_flat;

    printf("Enter the tips [< %d]:", TIPS_SIZE);
    fgets(tips, TIPS_SIZE + 1, stdin);

    if ((enter_flat = strchr(tips, '\n')) == NULL) {
        fprintf(stderr, "Tips too long [> %d].\n", TIPS_SIZE);
        free(tips);
        return NULL;
    }

    *enter_flat = 0;
    return tips;
}
