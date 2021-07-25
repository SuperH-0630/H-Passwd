#include <stdlib.h>
#include <string.h>
#include <time.h>

static char *passwd_char = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ123456789!@#$%^&*(){[}]:;\"'<,>.";
static unsigned long long seed = 2;
static unsigned long long a = 25214903917;
static unsigned long long b = 11;
static unsigned long long c = ((unsigned long long)2) << 48;

void randomInit(void) {
    seed = (long long)time(NULL);
}

unsigned long long getRandom(int min, int max) {
    seed = (a * seed + b) % c;
    return min + (seed % (max - min));
}

char *randomPasswd(void) {
    size_t size = getRandom(12, 17);
    char *passwd = calloc(size + 1, sizeof(char));

    for (int i = 0; i < size; i++)
        passwd[i] = passwd_char[getRandom(0, (int)strlen(passwd_char))];

    return passwd;
}
