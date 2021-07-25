#include "argument.h"

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int opt_i = 0;
int opt_flat = 0;
char *opt_val = NULL;
int argc_ = 0;
char **argv_ = NULL;

struct arg_define *arg_;

enum {
    normal = 0,
    at_end,
    continue_flat,  // 如处理-abc这样连续的短选项
} status;

int continue_index = 0;
bool exchange_one = false;  // 只发生了一次exchange, 意味着新arg的下一个arg与该arg并非连接的关系
// 如: zz -s -y -> -s zz -y，显然-s与zz并非连接关系

bool initOpt(bool base, int argc, char **argv, struct arg_define *arg) {
    if (base)
        opt_i = 1;
    else
        opt_i = 0;

    argc_ = argc;
    argv_ = argv;
    arg_ = arg;
    opt_flat = 0;
    opt_val = NULL;
    status = normal;
    return argc_ != 0 && arg != NULL;
}

static int getReturn(struct arg_define *arg) {
    switch (arg->argument) {
        case no_argument:
            return arg->flat;
        case can_argument:
            if (!exchange_one && (status != continue_flat || argv_[opt_i][continue_index + 1] == '\0') && opt_i + 1 < argc_) {
                opt_val = argv_[opt_i + 1];
            } else
                opt_val = NULL;
            return arg->flat;
        case must_argument:
            if (!exchange_one && (status != continue_flat && argv_[opt_i][continue_index + 1] == '\0') || opt_i + 1 >= argc_) {
                fprintf(stderr, "-%c --%s : Lack of argument.\n", arg->ch, arg->name);
                return '?';
            }
            opt_val = argv_[opt_i + 1];
            return arg->flat;
    }

    fprintf(stderr, "%s : Error argument.\n", argv_[opt_i]);
    return '?';
}

static int getShort(char ch) {
    struct arg_define *arg = arg_;
    while (arg->ch != 0) {
        if (arg->ch != '?' && arg->ch == ch)
            return getReturn(arg);
        arg++;
    }

    fprintf(stderr, "-%c : Don't support argument.\n", ch);
    return '?';
}

static int getLong(char *name) {
    struct arg_define *arg = arg_;
    while (arg->ch != 0) {
        if (arg->name != NULL && !strcmp(arg->name, name))
            return getReturn(arg);
        arg++;
    }

    fprintf(stderr, "--%s : Don't support argument.\n", name);
    return '?';
}

static int getOpt_(void) {
    if (status == at_end)
        return 0;

    if (status == continue_flat) {
        if (argv_[opt_i][continue_index] == '\0') {
            opt_i ++;
            status = normal;
            continue_index = 0;
        } else {
            int re = getShort(argv_[opt_i][continue_index]);
            if (opt_val != NULL) {
                opt_i += 2;
                status = normal;
                continue_index = 0;
            } else
                continue_index++;
            return re;
        }
    }

    if (opt_i >= argc_) {
        status = at_end;
        return 0;
    }

    if (!strcmp(argv_[opt_i], "--")) {  // 后面的参数全部当作非选项处理
        opt_i++;
        status = at_end;
        return 0;
    }

    if (strlen(argv_[opt_i]) < 2 || argv_[opt_i][0] != '-') {  // 如果是短选型起码是-a两个字符，如果是长选项必然--开头也大于两个字符
        for (int i = opt_i + 1; i < argc_; i++) {
            if (!strcmp(argv_[i], "--"))
                break;
            if (argv_[i][0] == '-') {  // 交换
                if (i + 1 < argc_ && argv_[i + 1][0] != '-') {  // argv_[i + 1]可能是参数, 若不是参数移动他也没有关系
                    char *tmp1 = argv_[i];
                    char *tmp2 = argv_[i + 1];
                    memmove(argv_ + opt_i + 2, argv_ + opt_i, (i - opt_i) * sizeof(char *));
                    argv_[opt_i] = tmp1;
                    argv_[opt_i + 1] = tmp2;
                } else {
                    char *tmp = argv_[i];
                    memmove(argv_ + opt_i + 1, argv_ + opt_i, (i - opt_i) * sizeof(char *));
                    argv_[opt_i] = tmp;
                    exchange_one = true;
                }

                goto continue_;
            }
        }

        // 寻找无果
        // opt_i不用增加1， 当前和后面的参数全部当作非选项处理
        status = at_end;
        return 0;
    }

    continue_:
    if (argv_[opt_i][0] == '-' && argv_[opt_i][1] != '-') {
        status = continue_flat;
        continue_index = 1;
        int re = getShort(argv_[opt_i][1]);
        if (opt_val != NULL) {
            opt_i += 2;
            status = normal;
        } else
            continue_index = 2;
        return re;
    }

    if (argv_[opt_i][0] == '-' && argv_[opt_i][1] == '-') {
        int re = getLong(argv_[opt_i] + 2);
        if (opt_val != NULL)
            opt_i += 2;
        else
            opt_i++;
        return re;
    }

    fprintf(stderr, "%s : Not support argument.\n", argv_[opt_i]);
    opt_i++;
    return '?';  // 理论上不应该会运算到这里
}

int getOpt(void) {
    exchange_one = false;
    opt_val = NULL;
    opt_flat = getOpt_();
    return opt_flat;
}