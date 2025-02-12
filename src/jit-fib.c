#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>
#include <sys/mman.h>

/* The memory region used for generated code. code_buf points within
   code_area, and is page-aligned. */
unsigned char code_area[8192];
unsigned char *code_buf;

/* The type of the generated function. */
typedef unsigned long (*fib_func)(long, long);

/* The encoding of the three registers used in the generated code. */
#define RAX 0
#define RSI 6
#define RDI 7

/* Create a three-argument add instruction for 64-bit registers */
unsigned char *make_lea_add(unsigned char *p, int src1, int src2, int dest) {
    *p++ = 0x48; /* 64-bit */
    *p++ = 0x8d; /* lea */
    *p++ = 0x04 | (dest << 3); /* modrm, SIB to dest */
    *p++ = (src1 << 3) | src2;
    return p;
}

/* Generate a register-to-register move instruction for 64-bit registers */
unsigned char *make_mov(unsigned char *p, int src, int dest) {
    if (src == dest)
	return p;
    *p++ = 0x48; /* 64-bit */
    *p++ = 0x89; /* mov */
    *p++ = 0xc0 | (src << 3) | dest;
    return p;
}

/* Generate a return instruction */
unsigned char *make_ret(unsigned char *p) {
    *p++ = 0xc3; /* ret */
    return p;
}

/* Compute Fibonacci numbers using just-in-time compilation */
unsigned long fib(int n) {
    unsigned char *p = code_buf;
    fib_func the_func = (fib_func)code_buf;
    int regs[3] = {RDI, RSI, RAX};
    int i;
    if (n < 0) {
	fprintf(stderr, "Argument can't be negative\n");
	exit(1);
    } else if (n > 93) {
	fprintf(stderr, "Argument too large\n");
	exit(1);
    }
    /* Add consecutive terms, using three registers in cycle. */
    for (i = n; i >= 4; i -= 3) {
	p = make_lea_add(p, RDI, RSI, RAX);
	p = make_lea_add(p, RAX, RSI, RDI);
	p = make_lea_add(p, RAX, RDI, RSI);
    }
    /* Unroll the first one or two operations of the cycle if leftover
       operations are neeed. */
    if (i-- > 1)
	p = make_lea_add(p, RDI, RSI, RAX);
    if (i-- > 1)
	p = make_lea_add(p, RAX, RDI, RDI);
    /* Note that no additions at all are needed for fib(0) or fib(1) */

    /* Return the correct final result in %rax */
    p = make_mov(p, regs[n % 3], RAX);
    p = make_ret(p);

    /* Start the computation with fib(0) = 0 and fib(1) = 1. Note that
       this depends on the register calling convention putting the two
       arguments in %rdi and %rsi respectively. */
    return the_func(0, 1);
}

void test_fib(int n) {
    unsigned long fn;
    fn = fib(n);
    printf("fib(%d) = %lu\n", n, fn);
}

int main(int argc, char **argv) {
    int n, res;
    if (argc != 2 && argc != 1) {
	fprintf(stderr, "Usage: jit-fib [<n>]\n");
	exit(1);
    }

    /* Set up a page of memory that is both writable and executable to
       hold the generated code, as an exception to W xor X. */
    code_buf = (unsigned char *)((4095 + (unsigned long)code_area) & ~0xfff);
    res = mprotect(code_buf, 4096, PROT_READ|PROT_WRITE|PROT_EXEC);
    if (res != 0) {
	fprintf(stderr, "mprotect() failed: %s\n", strerror(errno));
	return 1;
    }

    if (argc == 2) {
	n = atoi(argv[1]);
	test_fib(n);
    } else {
	for (n = 0; n <= 20; n++)
	    test_fib(n);
    }
    return 0;
}
