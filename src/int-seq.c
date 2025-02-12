#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

/* A function that maps one 32-bit integer to another one in a
   complicated way. */
unsigned int weird_func(unsigned int x);

/* Read a sequence of 32-bit numbers in hex, separated by spaces, from
   the supplied file handle. Keep reading only if the numbers are the
   right ones. */
unsigned int read_int_seq(FILE *fh) {
    /* The limited-size buffer where we write the sequence */
    unsigned int seq[4];
    int finished = 0;
    /* This variable holds the next value expected in the sequence */
    unsigned int next_int = 0x4271cafe;
    
    for (int i = 0; !finished; i++) {
	static char num_buf[9];
	unsigned int num;
	int c;

	/* Read 8 hex digits and convert them into an int */
	for (int j = 0; j < 8; j++) {
	    c = fgetc(fh);
	    if (isxdigit(c)) {
		num_buf[j] = c;
	    } else {
		fprintf(stderr,
			"Found character %c when expecting a hex digit\n", c);
		exit(1);
	    }
	}
	num_buf[8] = 0; /* null terminator*/
	num = strtol(num_buf, 0, 16);

	seq[i] = num;
	if (num != next_int) {
	    /* Stop reading if the number is not the expected one */
	    fprintf(stderr, "0x%08x is not the right number\n", num);
	    finished = 1;
	}
	printf("Read integer number #%d\n", i);
	/* Compute the next expected number */
	next_int = weird_func(num);
	seq[i] = num;
	c = fgetc(fh);
	if (c == '\n') {
	    finished = 1;
	} else if (c == ' ') {
	    /* continue to the next number */
	} else {
	    fprintf(stderr,
		    "Found character %c when expecting a space\n", c);
	    finished = 1;
	}
    }

    return seq[0];
}

int main(int argc, char **argv) {
    read_int_seq(stdin);
    return 0;
}
