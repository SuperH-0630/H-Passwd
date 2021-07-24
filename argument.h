#ifndef H_PASSWD_ARGUMENT_H
#define H_PASSWD_ARGUMENT_H
#include <stdbool.h>

extern int opt_i;
extern int opt_flat;
extern char *opt_val;

struct arg_define {
    char ch;
    char *name;
    int flat;
    enum {
        no_argument,
        can_argument,
        must_argument,
    } argument;
};

int getOpt(void);
bool initOpt(bool base, int argc, char **argv, struct arg_define *arg);

#endif //H_PASSWD_ARGUMENT_H
