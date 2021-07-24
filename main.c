#include "main.h"
#include "argument.h"
#include <stdio.h>

char *key = NULL;
char *name = NULL;
struct arg_define arg[] = {
        {.ch='v', .name="version", .flat='v', .argument=no_argument},
        {.ch='h', .name="help", .flat='h', .argument=no_argument},
        {.ch='s', .name="set-pw", .flat='s', .argument=no_argument},
        {.ch='g', .name="get-pw", .flat='g', .argument=no_argument},
        {.ch=0},
};
enum {
    no = 0,
    set_pw,
    get_pw,
} work = no;

void printVersion(void);
void printHelp(void);
bool setPassWd(void);
bool getPassWd(void);

int main(int argc, char **argv) {
    name = argv[0];
    initOpt(true, argc, argv, arg);

    for (getOpt(); opt_flat != 0; getOpt()) {
        switch (opt_flat) {
            case 'h':
                printHelp();
                exit(EXIT_SUCCESS);
            case 'v':
                printVersion();
                exit(EXIT_SUCCESS);
            case 's':
                if (work != no)
                    exit(EXIT_FAILURE);
                work = set_pw;
                break;
            case 'g':
                if (work != no)
                    exit(EXIT_FAILURE);
                work = get_pw;
                break;
            case 0:
                break;
            case '?':
            default:
                exit(EXIT_FAILURE);
        }
    }

    if (work == no) {
        fprintf(stderr, "What should I do?\n");
        printHelp();
        exit(EXIT_FAILURE);
    }

    if (argc - opt_i > 1)
        exit(EXIT_FAILURE);
    else if (argc - opt_i == 1) {
        if (key != NULL)
            exit(EXIT_FAILURE);
        key = calloc(strlen(argv[opt_i]) + 1, sizeof(char ));
        strcpy(key, argv[opt_i]);
        opt_i++;
    } else if (key == NULL) {
        printf("Please Enter The Key:\n");
        key = calloc(KEY_MAX_LEN + 10, sizeof(char ));
        fgets(key, KEY_MAX_LEN + 10, stdin);
        if (strchr(key, '\n') == NULL) {
            fprintf(stderr, "Key too long for stdin.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (!isLegalKey(key))
        exit(EXIT_FAILURE);
    initBase64(key);

    bool status = false;
    if (work == set_pw)
        status = setPassWd();
    else
        status = getPassWd();

    free(key);
    if (!status)
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
    printf("Key: [%d - %d]\n\n", KEY_MIN_LEN, KEY_MAX_LEN);

    printf("Name origin:\n");
    printf("I'm super Huan. H in h-password is Huan in superhuan. \n"
           "Password means that the software is password software.\n\n");

}

#define READ_WORD(key, len, key_name, re) do { printf("Please Enter The " key_name ":\n"); \
    (key) = calloc((len) + 10, sizeof(char )); \
    fgets((key), (len) + 10, stdin); \
    if (strchr((key), '\n') == NULL) { \
        fprintf(stderr, key_name " too long for stdin.\n"); \
        goto re; \
    } else {*strchr((key), '\n') = 0;} \
    }while(0)

bool setPassWd(void) {
    char *account;
    char *passwd;
    char *note;
    char *passwd_str;

    READ_WORD(account, 100, "Your account", ERROR1);
    READ_WORD(passwd, 100, "Your password", ERROR2);
    READ_WORD(note, 100, "Your note", ERROR3);

    passwd_str = makePasswordString(account, passwd, note);
    if (passwd_str == NULL)
        goto ERROR4;
    printPasswdStr(account, passwd, note, passwd_str);

    free(account);
    free(passwd);
    free(note);
    free(passwd_str);
    return true;

    ERROR4: free(note);
    ERROR3: free(passwd);
    ERROR2: free(account);
    ERROR1: return false;
}

bool getPassWd(void) {
    char *account;
    char *passwd;
    char *note;
    char *passwd_str;

    READ_WORD(passwd_str, 200, "Your Label", ERROR1);

    if (!getInfoFromPasswordString(passwd_str, &account, &passwd, &note))
        goto ERROR2;
    printInfo(account, passwd, note);

    free(account);
    free(passwd);
    free(note);
    free(passwd_str);
    return true;

    ERROR2: free(passwd_str);
    ERROR1: return false;
}
