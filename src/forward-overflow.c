#include <stdio.h>
#include <stdlib.h>

/* Like strcpy(), but with return-address checking */
void my_strcpy(char *dest, char *src) {
    register void *orig_ret = __builtin_return_address(0);

    char *p = src;
    char *q = dest;
    while (*p) {
	*q++ = *p++;
    }
    *q = 0;

    void *new_ret = __builtin_return_address(0);
    if (new_ret != orig_ret) {
	fprintf(stderr, "strcpy return address corrupted to %p\n", new_ret);
	exit(42);
    }
}

void func(char *attacker_controlled) {
    static void *orig_ret;
    orig_ret = __builtin_return_address(0);
    char buffer[50];

    my_strcpy(&buffer[0], attacker_controlled);

    void *new_ret = __builtin_return_address(0);
    if (new_ret != orig_ret) {
	fprintf(stderr, "func return address corrupted to %p\n", new_ret);
	exit(42);
    }
}

int main(int argc, char **argv) {
    if (argc == 2) {
	func(argv[1]);
    } else {
	func("short");
    }
    return 0;
}
