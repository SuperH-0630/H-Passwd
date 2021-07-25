#include "main.h"
#include "argument.h"
#include <stdio.h>

#ifndef INCLUDE_KEY
char *key = NULL;
#else
char *key = INCLUDE_KEY;
#endif

#define TIPS_SIZE (30)

char *name = NULL;
struct arg_define arg[] = {
        {.ch='v', .name="version", .flat='v', .argument=no_argument},
        {.ch='h', .name="help", .flat='h', .argument=no_argument},
        {.ch='s', .name="set-pw", .flat='s', .argument=no_argument},
        {.ch='g', .name="get-pw", .flat='g', .argument=no_argument},
#ifdef INCLUDE_KEY
        {.ch='c', .name="check-key", .flat='c', .argument=must_argument},
#else
        {.ch='i', .name="in-file", .flat='i', .argument=no_argument},
        {.ch='t', .name="tips", .flat='t', .argument=no_argument},
        {.ch='?', .name="set-tips", .flat='w', .argument=no_argument},
        {.ch='p', .name="print-label", .flat='p', .argument=no_argument},
#endif
        {.ch=0},
};
enum {
    no = 0,
    set_pw,
    get_pw,
} work = no;
bool in_file = false;  // 是否在文件中工作
bool print_passwd = false;  // 是否打印content

void printVersion(void);
void printHelp(void);
bool setPassWd(void);
bool getPassWd(void);
char *getTipsFromStdin(void);

int main(int argc, char **argv) {
    bool print_tips = false;
    bool set_tips = false;

    printf("Welecome to use H-Password.\n");
#ifdef INCLUDE_KEY
    printf("Exclusive custom user: %s\n", UserName);
#endif
    name = argv[0];
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
            case 'i':
                in_file = true;
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
    else if (in_file && print_passwd)
        printContent();

#ifndef INCLUDE_KEY
    free(key);
#endif
    if (!status)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;

    little_exit:
    if (argc != 2)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;

}

void printVersion(void) {
    printf("%s Version:\n%s\n%s\n", name, VERSION, VERSION_INFO);
}

void printHelp(void) {
    printf("Usage: %s <[option]..>\n", name);
    printf("Option: \n");
    printf(" -v --version       Show version.\n");
    printf(" -h --help          Show help.\n");
    printf(" -s --set-pw        Set Password.\n");
    printf(" -g --get-pw        Get Password.\n\n");

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
    printf("I'm super Huan. H in h-password is Huan in superhuan. \n"
           "Password means that the software is password software.\n\n");

}

#define READ_WORD(key, len, key_name, re) do { printf("Please Enter The " key_name ":"); \
    (key) = calloc((len) + 10, sizeof(char )); \
    fgets((key), (len) + 10, stdin); \
    if (strchr((key), '\n') == NULL) { \
        fprintf(stderr, key_name " too long for stdin.\n"); \
        goto re; \
    } else {*strchr((key), '\n') = 0;} \
    }while(0)

bool setPassWd(void) {
    char *account = NULL;
    char *passwd = NULL;
    char *note = NULL;
    char *in_file_name = NULL;
    char *passwd_str = NULL;

    READ_WORD(account, 100, "Your account", ERROR1);
    READ_WORD(passwd, 100, "Your password", ERROR2);

    if (*passwd == 0) {  // 未输入密码内容
        free(passwd);
        passwd = randomPasswd();  // 随机密码
        printf("random password: '%s'\n", passwd);
    }

    READ_WORD(note, 100, "Your note", ERROR3);

    if (in_file)
        READ_WORD(in_file_name, 100, "Account name", ERROR4);

    passwd_str = makePasswordString(account, passwd, note);
    if (passwd_str == NULL)
        goto ERROR5;

    printf("********************\n");
    fflush(stdout);
    printPasswdStr(account, passwd, note, passwd_str);

    if (in_file) {
        addConnect(in_file_name, passwd_str);
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
        READ_WORD(in_file_name, 50, "Your Label", ERROR1);
        passwd_str = findConnect(in_file_name);
        if (passwd_str == NULL) {
            fprintf(stderr, "name [%s] not found.", in_file_name);
            free(in_file_name);
            return false;
        }
        free(in_file_name);
    } else
        READ_WORD(passwd_str, 200, "Your Label", ERROR1);

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
